#ifndef OUTPUT_CONVERSION_HEADER
#define OUTPUT_CONVERSION_HEADER

#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

/** Initialize module's internal state. */
ModuleDestructor initializeOutputConversionModule();

/**
 * Executes the output conversion stage, transforming the generated MIDI file
 * into one or more audio formats, depending on the compiler state and
 * environment configuration.
 */
void executeOutputConversion(const CompilerState * compilerState);

#endif


