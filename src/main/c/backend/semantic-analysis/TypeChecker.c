#include "TypeChecker.h"
#include <ctype.h>
#include <string.h>

static Logger * _logger = NULL;

static void _destroyTypeCheckerModule(void) {
	if (_logger != NULL) {
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeTypeCheckerModule() {
	_logger = createLogger("TypeChecker");
	return _destroyTypeCheckerModule;
}

bool validateNoteFormat(const char * note) {
	if (note == NULL || strlen(note) < 2) {
		logError(_logger, "Invalid note: NULL or too short");
		return false;
	}

	// First character must be A-G
	if (note[0] < 'A' || note[0] > 'G') {
		logError(_logger, "Invalid note '%s': first character must be A-G", note);
		return false;
	}

	int i = 1;

	// Optional accidental (# or b)
	if (note[i] == '#' || note[i] == 'b') {
		i++;
	}

	// Must have at least one digit
	if (!isdigit(note[i])) {
		logError(_logger, "Invalid note '%s': missing octave number", note);
		return false;
	}

	// All remaining characters must be digits
	while (note[i] != '\0') {
		if (!isdigit(note[i])) {
			logError(_logger, "Invalid note '%s': invalid character '%c' in octave", note, note[i]);
			return false;
		}
		i++;
	}

	logDebugging(_logger, "Valid note format: '%s'", note);
	return true;
}

bool validateRhythmElement(RhythmElement * element) {
	if (element == NULL) {
		logError(_logger, "NULL rhythm element");
		return false;
	}

	switch (element->type) {
		case ELEMENT_HIT:
		case ELEMENT_SILENCE:
			logDebugging(_logger, "Valid rhythm element: %s",
				(element->type == ELEMENT_HIT) ? "HIT" : "SILENCE");
			return true;

		case ELEMENT_NOTE:
			if (element->noteValue == NULL) {
				logError(_logger, "NOTE element with NULL noteValue");
				return false;
			}
			return validateNoteFormat(element->noteValue);

		default:
			logError(_logger, "Unknown rhythm element type: %d", element->type);
			return false;
	}
}

bool validateRhythmArray(RhythmArray * array) {
	if (array == NULL) {
		logError(_logger, "NULL rhythm array");
		return false;
	}

	if (array->elements == NULL) {
		logError(_logger, "Rhythm array with NULL elements");
		return false;
	}

	int elementCount = 0;
	RhythmElementList * current = array->elements;

	while (current != NULL) {
		elementCount++;
		if (!validateRhythmElement(current->element)) {
			logError(_logger, "Invalid element at position %d in rhythm array", elementCount);
			return false;
		}
		current = current->next;
	}

	logDebugging(_logger, "Valid rhythm array with %d elements", elementCount);
	return true;
}

bool validateRhythmExpression(RhythmExpression * expression) {
	if (expression == NULL) {
		logError(_logger, "NULL rhythm expression");
		return false;
	}

	switch (expression->type) {
		case RHYTHM_ARRAY:
			return validateRhythmArray(expression->array);

		case RHYTHM_CONCATENATION:
			logDebugging(_logger, "Validating concatenation expression");
			if (!validateRhythmExpression(expression->concatenation.left)) {
				logError(_logger, "Invalid left side of concatenation");
				return false;
			}
			if (!validateRhythmExpression(expression->concatenation.right)) {
				logError(_logger, "Invalid right side of concatenation");
				return false;
			}
			logDebugging(_logger, "Valid concatenation expression");
			return true;

		case RHYTHM_REPETITION:
			logDebugging(_logger, "Validating repetition expression (factor=%d)", expression->repetition.repetitions);
			if (expression->repetition.repetitions <= 0) {
				logError(_logger, "Repetition factor must be positive, got %d", expression->repetition.repetitions);
				return false;
			}
			if (!validateRhythmArray(expression->repetition.array)) {
				logError(_logger, "Invalid array in repetition");
				return false;
			}
			logDebugging(_logger, "Valid repetition expression");
			return true;

		default:
			logError(_logger, "Unknown rhythm expression type: %d", expression->type);
			return false;
	}
}

bool validateActiveRange(ActiveRange * range) {
	if (range == NULL) {
		logError(_logger, "NULL active range");
		return false;
	}

	if (range->start <= 0) {
		logError(_logger, "Active range start must be positive, got %d", range->start);
		return false;
	}

	if (range->end <= 0) {
		logError(_logger, "Active range end must be positive, got %d", range->end);
		return false;
	}

	if (range->start > range->end) {
		logError(_logger, "Active range start (%d) must be <= end (%d)", range->start, range->end);
		return false;
	}

	logDebugging(_logger, "Valid active range: %d-%d", range->start, range->end);
	return true;
}

bool validateActiveRangeWithinBounds(ActiveRange * range, int maxCompass) {
	if (!validateActiveRange(range)) {
		return false;
	}

	if (range->end > maxCompass) {
		logError(_logger, "Active range end (%d) exceeds maximum compass count (%d)", range->end, maxCompass);
		return false;
	}

	logDebugging(_logger, "Active range %d-%d is within bounds (max=%d)", range->start, range->end, maxCompass);
	return true;
}

bool validateDeclarations(Declarations * declarations) {
	if (declarations == NULL) {
		logError(_logger, "NULL declarations");
		return false;
	}

	if (declarations->tempo <= 0) {
		logError(_logger, "Tempo must be positive, got %d", declarations->tempo);
		return false;
	}

	if (declarations->compasses <= 0) {
		logError(_logger, "Compasses must be positive, got %d", declarations->compasses);
		return false;
	}

	if (declarations->steps <= 0) {
		logError(_logger, "Steps must be positive, got %d", declarations->steps);
		return false;
	}

	logDebugging(_logger, "Valid declarations: tempo=%d, compasses=%d, steps=%d",
		declarations->tempo, declarations->compasses, declarations->steps);
	return true;
}
