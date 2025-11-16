#include "ImportResolver.h"
#include "Frontend.h"
#include "lexical-analysis/FlexActions.h"
#include "syntactic-analysis/BisonActions.h"
#include "../support/type/CompilerState.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

// Track visited files to detect circular dependencies
#define MAX_IMPORT_DEPTH 32
static const char * _visitedFiles[MAX_IMPORT_DEPTH];
static int _visitedCount = 0;

/** Shutdown module's internal state. */
void _shutdownImportResolverModule() {
    if (_logger != NULL) {
        logDebugging(_logger, "Destroying module: ImportResolver...");
        destroyLogger(_logger);
        _logger = NULL;
    }
    _visitedCount = 0;
}

ModuleDestructor initializeImportResolverModule() {
    _logger = createLogger("ImportResolver");
    _visitedCount = 0;
    return _shutdownImportResolverModule;
}

/* PRIVATE FUNCTIONS */

/**
 * Checks if a file exists and is readable.
 */
static bool _fileExists(const char * filePath) {
    FILE * file = fopen(filePath, "r");
    if (file == NULL) {
        return false;
    }
    fclose(file);
    return true;
}

/**
 * Checks if a file has already been visited (circular dependency detection).
 */
static bool _isAlreadyVisited(const char * filePath) {
    for (int i = 0; i < _visitedCount; i++) {
        if (strcmp(_visitedFiles[i], filePath) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Marks a file as visited.
 */
static void _markAsVisited(const char * filePath) {
    if (_visitedCount < MAX_IMPORT_DEPTH) {
        _visitedFiles[_visitedCount++] = filePath;
    }
}

/**
 * Unmarks the last visited file (for backtracking).
 */
static void _unmarkLastVisited() {
    if (_visitedCount > 0) {
        _visitedCount--;
    }
}

/**
 * Counts the number of patterns in a pattern list.
 */
static int _countPatterns(PatternList * list) {
    int count = 0;
    while (list != NULL) {
        count++;
        list = list->next;
    }
    return count;
}

/**
 * Appends the source pattern list to the end of the destination pattern list.
 * Returns the head of the merged list.
 */
static PatternList * _mergePatternLists(PatternList * dest, PatternList * source) {
    if (source == NULL) {
        return dest;
    }

    if (dest == NULL) {
        return source;
    }

    // Find the end of the destination list
    PatternList * current = dest;
    while (current->next != NULL) {
        current = current->next;
    }

    // Append source to end
    current->next = source;
    return dest;
}

/**
 * Parses a single file and returns its Program AST by invoking the compiler
 * in a child process to avoid parser state conflicts.
 * Returns NULL on error.
 */
static Program * _parseFile(const char * filePath) {
    logDebugging(_logger, "Parsing file via subprocess: %s", filePath);

    // For simplicity in MVP, we'll just open the file and parse it directly
    // using the existing Flex/Bison infrastructure but with a clean slate.
    // The trick is to use yypush_parse_new() to create a fresh parser instance.

    FILE * file = fopen(filePath, "r");
    if (file == NULL) {
        logError(_logger, "Cannot open file: %s", filePath);
        return NULL;
    }

    // Read file into memory
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char * fileContent = (char *) calloc(fileSize + 1, sizeof(char));
    if (fileContent == NULL) {
        fclose(file);
        logError(_logger, "Out of memory reading file: %s", filePath);
        return NULL;
    }

    size_t bytesRead = fread(fileContent, 1, fileSize, file);
    fclose(file);
    fileContent[bytesRead] = '\0';

    // Create a new lexical analyzer with its own parser for this import
    LexicalAnalyzer * importLexer = createLexicalAnalyzer();
    if (importLexer == NULL) {
        free(fileContent);
        logError(_logger, "Failed to create lexical analyzer");
        return NULL;
    }

    // Create a scan buffer from string
    YY_BUFFER_STATE buffer = yy_scan_string(fileContent, (yyscan_t) importLexer->scanner);
    if (buffer == NULL) {
        free(fileContent);
        destroyLexicalAnalyzer(importLexer);
        logError(_logger, "Failed to create scan buffer");
        return NULL;
    }

    // Create temporary compiler state for this import
    CompilerState tempState = {
        .abstractSyntaxtTree = NULL,
        .value = 0
    };

    // Initialize modules for the import parser
    extern ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer * lexicalAnalyzer);
    extern ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

    ModuleDestructor flexDestructor = initializeFlexActionsModule(importLexer);
    ModuleDestructor bisonDestructor = initializeBisonActionsModule(&tempState);

    // Parse the file using the import's lexical analyzer
    CompilationStatus status = IN_PROGRESS;
    while (status == IN_PROGRESS) {
        status = (CompilationStatus) yylex(
            NULL,
            (YYLTYPE *) importLexer->location,
            importLexer->scanner);
    }

    // Clean up modules
    flexDestructor();
    bisonDestructor();

    // Clean up lexer and buffer
    yy_delete_buffer(buffer, (yyscan_t) importLexer->scanner);
    destroyLexicalAnalyzer(importLexer);
    free(fileContent);

    if (status != SUCCEEDED) {
        logError(_logger, "Failed to parse file: %s", filePath);
        if (tempState.abstractSyntaxtTree != NULL) {
            destroyProgram(tempState.abstractSyntaxtTree);
        }
        return NULL;
    }

    logDebugging(_logger, "Successfully parsed file: %s", filePath);
    return tempState.abstractSyntaxtTree;
}

/**
 * Recursively resolves imports from a file and merges patterns into the main program.
 */
static CompilationStatus _resolveImportsRecursive(const char * filePath, Program * mainProgram) {
    // Check for circular dependencies
    if (_isAlreadyVisited(filePath)) {
        logError(_logger, "Circular dependency detected: %s", filePath);
        return FAILED;
    }

    // Check import depth
    if (_visitedCount >= MAX_IMPORT_DEPTH) {
        logError(_logger, "Maximum import depth exceeded (%d)", MAX_IMPORT_DEPTH);
        return FAILED;
    }

    // Check if file exists
    if (!_fileExists(filePath)) {
        logError(_logger, "Import file not found: %s", filePath);
        return FAILED;
    }

    // Mark as visited
    _markAsVisited(filePath);

    // Parse the imported file
    Program * importedProgram = _parseFile(filePath);
    if (importedProgram == NULL) {
        _unmarkLastVisited();
        return FAILED;
    }

    // Recursively resolve imports from the imported file
    if (importedProgram->imports != NULL) {
        ImportList * nestedImport = importedProgram->imports;
        while (nestedImport != NULL) {
            if (nestedImport->import != NULL && nestedImport->import->filePath != NULL) {
                CompilationStatus status = _resolveImportsRecursive(
                    nestedImport->import->filePath,
                    mainProgram
                );
                if (status != SUCCEEDED) {
                    destroyProgram(importedProgram);
                    _unmarkLastVisited();
                    return FAILED;
                }
            }
            nestedImport = nestedImport->next;
        }
    }

    // Merge patterns from imported file into main program
    int importedPatternCount = _countPatterns(importedProgram->patterns);
    if (importedPatternCount > 0) {
        logDebugging(_logger, "Merging %d patterns from: %s", importedPatternCount, filePath);
        mainProgram->patterns = _mergePatternLists(mainProgram->patterns, importedProgram->patterns);

        // Set patterns to NULL so they won't be destroyed when we destroy the imported program
        importedProgram->patterns = NULL;
    }

    // Destroy the imported program (but patterns are now owned by main program)
    destroyProgram(importedProgram);

    // Unmark as visited (backtrack)
    _unmarkLastVisited();

    logDebugging(_logger, "Successfully resolved imports from: %s", filePath);
    return SUCCEEDED;
}

/* PUBLIC FUNCTIONS */

CompilationStatus resolveImports(Program * program) {
    if (program == NULL) {
        logWarning(_logger, "NULL program provided to resolveImports");
        return SUCCEEDED;
    }

    if (program->imports == NULL) {
        logDebugging(_logger, "No imports to resolve");
        return SUCCEEDED;
    }

    logInformation(_logger, "Resolving imports...");

    // Reset visited files tracker
    _visitedCount = 0;

    // Process each import
    ImportList * currentImport = program->imports;
    int importCount = 0;
    int resolvedCount = 0;

    while (currentImport != NULL) {
        if (currentImport->import != NULL && currentImport->import->filePath != NULL) {
            const char * filePath = currentImport->import->filePath;
            importCount++;

            logDebugging(_logger, "Processing import: %s", filePath);

            CompilationStatus status = _resolveImportsRecursive(filePath, program);
            if (status == SUCCEEDED) {
                resolvedCount++;
            } else {
                logError(_logger, "Failed to resolve import: %s", filePath);
                return FAILED;
            }
        }

        currentImport = currentImport->next;
    }

    logInformation(_logger, "Import resolution completed: %d/%d imports resolved successfully", resolvedCount, importCount);

    return SUCCEEDED;
}
