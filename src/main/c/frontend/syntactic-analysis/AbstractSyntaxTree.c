#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* PUBLIC FUNCTIONS */

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroyImportList(program->imports);
		destroyDeclarations(program->declarations);
		destroyPatternList(program->patterns);
		destroyInstrumentList(program->instruments);
		free(program);
	}
}

void destroyImportList(ImportList * importList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (importList != NULL) {
		destroyImportStatement(importList->import);
		destroyImportList(importList->next);
		free(importList);
	}
}

void destroyImportStatement(ImportStatement * importStatement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (importStatement != NULL) {
		if (importStatement->filePath != NULL) {
			free(importStatement->filePath);
		}
		free(importStatement);
	}
}

void destroyDeclarations(Declarations * declarations) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (declarations != NULL) {
		free(declarations);
	}
}

void destroyPatternList(PatternList * patternList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (patternList != NULL) {
		destroyPattern(patternList->pattern);
		destroyPatternList(patternList->next);
		free(patternList);
	}
}

void destroyPattern(Pattern * pattern) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (pattern != NULL) {
		if (pattern->name != NULL) {
			free(pattern->name);
		}
		destroyRhythmExpression(pattern->rhythm);
		free(pattern);
	}
}

void destroyRhythmExpression(RhythmExpression * rhythmExpression) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (rhythmExpression != NULL) {
		switch (rhythmExpression->type) {
			case RHYTHM_ARRAY:
				destroyRhythmArray(rhythmExpression->array);
				break;
			case RHYTHM_CONCATENATION:
				destroyRhythmExpression(rhythmExpression->concatenation.left);
				destroyRhythmExpression(rhythmExpression->concatenation.right);
				break;
			case RHYTHM_REPETITION:
				destroyRhythmArray(rhythmExpression->repetition.array);
				break;
		}
		free(rhythmExpression);
	}
}

void destroyRhythmArray(RhythmArray * rhythmArray) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (rhythmArray != NULL) {
		destroyRhythmElementList(rhythmArray->elements);
		free(rhythmArray);
	}
}

void destroyRhythmElementList(RhythmElementList * elementList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (elementList != NULL) {
		destroyRhythmElement(elementList->element);
		destroyRhythmElementList(elementList->next);
		free(elementList);
	}
}

void destroyRhythmElement(RhythmElement * element) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (element != NULL) {
		if (element->noteValue != NULL) {
			free(element->noteValue);
		}
		free(element);
	}
}

void destroyInstrumentList(InstrumentList * instrumentList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (instrumentList != NULL) {
		destroyInstrument(instrumentList->instrument);
		destroyInstrumentList(instrumentList->next);
		free(instrumentList);
	}
}

void destroyInstrument(Instrument * instrument) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (instrument != NULL) {
		if (instrument->name != NULL) {
			free(instrument->name);
		}
		if (instrument->patternName != NULL) {
			free(instrument->patternName);
		}
		destroyActiveRange(instrument->activeRange);
		free(instrument);
	}
}

void destroyActiveRange(ActiveRange * activeRange) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (activeRange != NULL) {
        destroyActiveRange(activeRange->next);  // Destruir recursivamente
        free(activeRange);
    }
}
