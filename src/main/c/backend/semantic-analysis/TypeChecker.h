#ifndef TYPE_CHECKER_HEADER
#define TYPE_CHECKER_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/**
 * Type Checker for the Drum Machine DSL.
 *
 * This module validates type compatibility and consistency throughout the DSL:
 * - Rhythm element types: HIT (x), SILENCE (.), NOTE (E2, A#2)
 * - Rhythm expression types: ARRAY, CONCATENATION, REPETITION
 * - Active range types: INTEGER pairs with - separator
 * - Operator type checking (*, +)
 */

/** Initialize module's internal state. */
ModuleDestructor initializeTypeCheckerModule();

/**
 * Validate a rhythm element.
 * @param element The rhythm element to validate
 * @return true if valid, false otherwise
 */
bool validateRhythmElement(RhythmElement * element);

/**
 * Validate a rhythm array.
 * Ensures all elements are valid rhythm elements (x, ., NOTE).
 * @param array The rhythm array to validate
 * @return true if valid, false otherwise
 */
bool validateRhythmArray(RhythmArray * array);

/**
 * Validate a rhythm expression.
 * Checks:
 * - Arrays contain valid elements
 * - Repetition (*): left is array, right is positive integer
 * - Concatenation (+): both sides are valid rhythm expressions
 * @param expression The rhythm expression to validate
 * @return true if valid, false otherwise
 */
bool validateRhythmExpression(RhythmExpression * expression);

/**
 * Validate an active range.
 * Checks:
 * - Start and end are positive
 * - Start <= end
 * @param range The active range to validate
 * @return true if valid, false otherwise
 */
bool validateActiveRange(ActiveRange * range);

/**
 * Validate active ranges are within compass bounds.
 * @param range The active range to validate
 * @param maxCompass The maximum compass number (from declarations)
 * @return true if valid, false otherwise
 */
bool validateActiveRangeWithinBounds(ActiveRange * range, int maxCompass);

/**
 * Validate declarations.
 * Checks:
 * - tempo > 0
 * - compasses > 0
 * - steps > 0
 * @param declarations The declarations to validate
 * @return true if valid, false otherwise
 */
bool validateDeclarations(Declarations * declarations);

/**
 * Validate a note string format.
 * Expected format: [A-G][#b]?[0-9]
 * @param note The note string (e.g., "E2", "A#2", "Gb3")
 * @return true if valid, false otherwise
 */
bool validateNoteFormat(const char * note);

#endif
