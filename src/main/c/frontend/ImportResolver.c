#include "ImportResolver.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownImportResolverModule() {
    if (_logger != NULL) {
        logDebugging(_logger, "Destroying module: ImportResolver...");
        destroyLogger(_logger);
        _logger = NULL;
    }
}

ModuleDestructor initializeImportResolverModule() {
    _logger = createLogger("ImportResolver");
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

    logDebugging(_logger, "Starting import validation...");

    ImportList * currentImport = program->imports;
    int importCount = 0;
    int validCount = 0;

    while (currentImport != NULL) {
        if (currentImport->import != NULL && currentImport->import->filePath != NULL) {
            const char * filePath = currentImport->import->filePath;
            importCount++;

            logDebugging(_logger, "Validating import: %s", filePath);

            if (_fileExists(filePath)) {
                logDebugging(_logger, "  ✓ File exists: %s", filePath);
                validCount++;
            } else {
                logWarning(_logger, "  ⚠ File not found: %s (will be ignored)", filePath);
            }
        }

        currentImport = currentImport->next;
    }

    logDebugging(_logger, "Import validation completed: %d/%d files found", validCount, importCount);

    // Note: We return SUCCEEDED even if some files don't exist,
    // allowing the program to continue with just the patterns defined in the main file.
    // In a production system, you might want to return FAILED if critical imports are missing.

    return SUCCEEDED;
}
