#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

/* IMPORTED FUNCTIONS */

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS */

Program * ProgramSemanticAction(ImportList * imports, Declarations * declarations, PatternList * patterns, InstrumentList * instruments) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->imports = imports;
	program->declarations = declarations;
	program->patterns = patterns;
	program->instruments = instruments;
	_compilerState->abstractSyntaxtTree = program;
	return program;
}

ImportList * ImportListSemanticAction(ImportStatement * import, ImportList * next) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ImportList * list = calloc(1, sizeof(ImportList));
	list->import = import;
	list->next = next;
	return list;
}

ImportStatement * ImportStatementSemanticAction(char * filePath) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ImportStatement * stmt = calloc(1, sizeof(ImportStatement));
	stmt->filePath = filePath;
	return stmt;
}

Declarations * DeclarationsSemanticAction(int tempo, int compasses, int steps) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Declarations * declarations = calloc(1, sizeof(Declarations));
	declarations->tempo = tempo;
	declarations->compasses = compasses;
	declarations->steps = steps;
	return declarations;
}

PatternList * PatternListSemanticAction(Pattern * pattern, PatternList * next) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	PatternList * list = calloc(1, sizeof(PatternList));
	list->pattern = pattern;
	list->next = next;
	return list;
}

Pattern * PatternSemanticAction(char * name, RhythmExpression * rhythm) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Pattern * pattern = calloc(1, sizeof(Pattern));
	pattern->name = name;
	pattern->rhythm = rhythm;
	return pattern;
}

RhythmExpression * RhythmArrayExpressionSemanticAction(RhythmArray * array) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RhythmExpression * expr = calloc(1, sizeof(RhythmExpression));
	expr->type = RHYTHM_ARRAY;
	expr->array = array;
	return expr;
}

RhythmExpression * RhythmConcatenationSemanticAction(RhythmExpression * left, RhythmExpression * right) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RhythmExpression * expr = calloc(1, sizeof(RhythmExpression));
	expr->type = RHYTHM_CONCATENATION;
	expr->concatenation.left = left;
	expr->concatenation.right = right;
	return expr;
}

RhythmExpression * RhythmRepetitionSemanticAction(RhythmArray * array, int repetitions) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RhythmExpression * expr = calloc(1, sizeof(RhythmExpression));
	expr->type = RHYTHM_REPETITION;
	expr->repetition.array = array;
	expr->repetition.repetitions = repetitions;
	return expr;
}

RhythmArray * RhythmArraySemanticAction(RhythmElementList * elements) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RhythmArray * array = calloc(1, sizeof(RhythmArray));
	array->elements = elements;
	return array;
}

RhythmElementList * RhythmElementListSemanticAction(RhythmElement * element, RhythmElementList * next) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RhythmElementList * newNode = calloc(1, sizeof(RhythmElementList));
	newNode->element = element;
	newNode->next = NULL;

	// If next is NULL, this is the first element
	if (next == NULL) {
		return newNode;
	}

	// Otherwise, append to the END of the list to maintain order
	RhythmElementList * current = next;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = newNode;

	return next;
}

RhythmElement * HitElementSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RhythmElement * element = calloc(1, sizeof(RhythmElement));
	element->type = ELEMENT_HIT;
	element->noteValue = NULL;
	return element;
}

RhythmElement * SilenceElementSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RhythmElement * element = calloc(1, sizeof(RhythmElement));
	element->type = ELEMENT_SILENCE;
	element->noteValue = NULL;
	return element;
}

RhythmElement * NoteElementSemanticAction(char * noteValue) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	RhythmElement * element = calloc(1, sizeof(RhythmElement));
	element->type = ELEMENT_NOTE;
	element->noteValue = noteValue;
	return element;
}


InstrumentList * InstrumentListSemanticAction(Instrument * instrument, InstrumentList * next) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	InstrumentList * list = calloc(1, sizeof(InstrumentList));
	list->instrument = instrument;
	list->next = next;
	return list;
}

Instrument * InstrumentSemanticAction(char * name, char * patternName, ActiveRange * activeRange) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Instrument * instrument = calloc(1, sizeof(Instrument));
	instrument->name = name;
	instrument->patternName = patternName;
	instrument->activeRange = activeRange;
	return instrument;
}

ActiveRange * ActiveRangeSemanticAction(int start, int end) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ActiveRange * range = calloc(1, sizeof(ActiveRange));
	range->start = start;
	range->end = end;
	range->next = NULL;
	return range;
}

ActiveRange * ActiveRangeConcatenationSemanticAction(ActiveRange * left, ActiveRange * right) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    ActiveRange * current = left;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = right;
    return left;
}
