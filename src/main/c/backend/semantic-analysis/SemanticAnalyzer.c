#include "SemanticAnalyzer.h"
#include "SymbolTable.h"
#include "TypeChecker.h"
#include <string.h>

static Logger * _logger = NULL;
static SymbolTable * _symbolTable = NULL;

static void _destroySemanticAnalyzerModule(void) {
	if (_symbolTable != NULL) {
		destroySymbolTable(_symbolTable);
		_symbolTable = NULL;
	}
	if (_logger != NULL) {
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeSemanticAnalyzerModule() {
	_logger = createLogger("SemanticAnalyzer");
	_symbolTable = NULL;
	return _destroySemanticAnalyzerModule;
}

SymbolTable * getSymbolTable(void) {
	return _symbolTable;
}

/**
 * Build symbol table from the program AST.
 * Collects all patterns and instruments into the symbol table.
 */
static CompilationStatus _buildSymbolTable(Program * program) {
	if (program == NULL) {
		logError(_logger, "Cannot build symbol table from NULL program");
		return FAILED;
	}

	// Create symbol table (capacity = 64 buckets, enough for most programs)
	_symbolTable = createSymbolTable(64);
	if (_symbolTable == NULL) {
		logError(_logger, "Failed to create symbol table");
		return FAILED;
	}

	logInformation(_logger, "Building symbol table...");

	// Collect all patterns
	int patternCount = 0;
	PatternList * patternList = program->patterns;
	while (patternList != NULL) {
		Pattern * pattern = patternList->pattern;
		if (pattern == NULL || pattern->name == NULL) {
			logError(_logger, "Pattern list contains NULL pattern or name");
			return FAILED;
		}

		if (!insertPattern(_symbolTable, pattern)) {
			logError(_logger, "Duplicate pattern name: '%s'", pattern->name);
			return FAILED;
		}

		patternCount++;
		patternList = patternList->next;
	}

	logInformation(_logger, "Collected %d patterns", patternCount);

	// Collect all instruments
	int instrumentCount = 0;
	InstrumentList * instrumentList = program->instruments;
	while (instrumentList != NULL) {
		Instrument * instrument = instrumentList->instrument;
		if (instrument == NULL || instrument->name == NULL) {
			logError(_logger, "Instrument list contains NULL instrument or name");
			return FAILED;
		}

		if (!insertInstrument(_symbolTable, instrument)) {
			logError(_logger, "Duplicate instrument name: '%s'", instrument->name);
			return FAILED;
		}

		instrumentCount++;
		instrumentList = instrumentList->next;
	}

	logInformation(_logger, "Collected %d instruments", instrumentCount);
	logInformation(_logger, "Symbol table built successfully (total symbols: %d)", patternCount + instrumentCount);

	return SUCCEEDED;
}

/**
 * Validate all patterns in the program.
 */
static CompilationStatus _validatePatterns(Program * program) {
	if (program == NULL) {
		return SUCCEEDED; // No patterns to validate
	}

	logInformation(_logger, "Validating patterns...");

	int validatedCount = 0;
	PatternList * patternList = program->patterns;

	while (patternList != NULL) {
		Pattern * pattern = patternList->pattern;

		if (pattern == NULL) {
			logError(_logger, "NULL pattern in pattern list");
			return FAILED;
		}

		logDebugging(_logger, "Validating pattern '%s'", pattern->name);

		if (pattern->rhythm == NULL) {
			logError(_logger, "Pattern '%s' has NULL rhythm expression", pattern->name);
			return FAILED;
		}

		if (!validateRhythmExpression(pattern->rhythm)) {
			logError(_logger, "Pattern '%s' has invalid rhythm expression", pattern->name);
			return FAILED;
		}

		validatedCount++;
		patternList = patternList->next;
	}

	logInformation(_logger, "Validated %d patterns", validatedCount);
	return SUCCEEDED;
}

/**
 * Validate all instruments in the program.
 */
static CompilationStatus _validateInstruments(Program * program) {
	if (program == NULL || program->declarations == NULL) {
		return SUCCEEDED; // No instruments to validate
	}

	logInformation(_logger, "Validating instruments...");

	int validatedCount = 0;
	int maxCompass = program->declarations->compasses;
	InstrumentList * instrumentList = program->instruments;

	while (instrumentList != NULL) {
		Instrument * instrument = instrumentList->instrument;

		if (instrument == NULL) {
			logError(_logger, "NULL instrument in instrument list");
			return FAILED;
		}

		logDebugging(_logger, "Validating instrument '%s'", instrument->name);

		// Validate pattern reference exists
		if (instrument->patternName == NULL) {
			logError(_logger, "Instrument '%s' has NULL pattern reference", instrument->name);
			return FAILED;
		}

		Pattern * referencedPattern = lookupPattern(_symbolTable, instrument->patternName);
		if (referencedPattern == NULL) {
			logError(_logger, "Instrument '%s' references undefined pattern '%s'",
				instrument->name, instrument->patternName);
			return FAILED;
		}

		logDebugging(_logger, "Instrument '%s' references valid pattern '%s'",
			instrument->name, instrument->patternName);

		// Validate active ranges
		if (instrument->activeRange == NULL) {
			logError(_logger, "Instrument '%s' has NULL active range", instrument->name);
			return FAILED;
		}

		ActiveRange * range = instrument->activeRange;
		int rangeCount = 0;

		while (range != NULL) {
			rangeCount++;

			if (!validateActiveRange(range)) {
				logError(_logger, "Instrument '%s' has invalid active range at position %d",
					instrument->name, rangeCount);
				return FAILED;
			}

			if (!validateActiveRangeWithinBounds(range, maxCompass)) {
				logError(_logger, "Instrument '%s' has active range %d-%d that exceeds compass count %d",
					instrument->name, range->start, range->end, maxCompass);
				return FAILED;
			}

			range = range->next;
		}

		logDebugging(_logger, "Instrument '%s' has %d valid active ranges", instrument->name, rangeCount);

		validatedCount++;
		instrumentList = instrumentList->next;
	}

	logInformation(_logger, "Validated %d instruments", validatedCount);
	return SUCCEEDED;
}

CompilationStatus analyzeProgram(Program * program) {
	if (program == NULL) {
		logError(_logger, "Cannot analyze NULL program");
		return FAILED;
	}

	logInformation(_logger, "Starting semantic analysis...");

	// Step 1: Validate declarations
	logInformation(_logger, "Step 1: Validating declarations...");
	if (program->declarations == NULL) {
		logError(_logger, "Program has NULL declarations");
		return FAILED;
	}

	if (!validateDeclarations(program->declarations)) {
		logError(_logger, "Invalid declarations");
		return FAILED;
	}

	// Step 2: Build symbol table
	logInformation(_logger, "Step 2: Building symbol table...");
	CompilationStatus status = _buildSymbolTable(program);
	if (status != SUCCEEDED) {
		logError(_logger, "Failed to build symbol table");
		return FAILED;
	}

	// Step 3: Validate patterns
	logInformation(_logger, "Step 3: Validating patterns...");
	status = _validatePatterns(program);
	if (status != SUCCEEDED) {
		logError(_logger, "Pattern validation failed");
		return FAILED;
	}

	// Step 4: Validate instruments
	logInformation(_logger, "Step 4: Validating instruments...");
	status = _validateInstruments(program);
	if (status != SUCCEEDED) {
		logError(_logger, "Instrument validation failed");
		return FAILED;
	}

	logInformation(_logger, "Semantic analysis completed successfully");
	return SUCCEEDED;
}
