#include "Calculator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownCalculatorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Calculator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeCalculatorModule() {
	_logger = createLogger("Calculator");
	return _shutdownCalculatorModule;
}

/** PRIVATE FUNCTIONS */

static ComputationResult _invalidComputation();


/**
 * A computation that always returns an invalid result.
 */
static ComputationResult _invalidComputation() {
	ComputationResult computationResult = {
		.succeeded = false,
		.value = 0
	};
	return computationResult;
}

/** PUBLIC FUNCTIONS */

ComputationResult add(const int leftAddend, const int rightAddend) {
	return _invalidComputation();
}

ComputationResult divide(const int dividend, const int divisor) {
	return _invalidComputation();
}

ComputationResult multiply(const int multiplicand, const int multiplier) {
	return _invalidComputation();
}

ComputationResult subtract(const int minuend, const int subtract) {
	return _invalidComputation();
}

ComputationResult executeCalculator(CompilerState * compilerState) {
	logDebugging(_logger, "Calculator is not implemented for Drum Machine DSL");
	return _invalidComputation();
}
