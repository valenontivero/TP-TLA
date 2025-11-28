#ifndef OUTPUT_CONVERSION_HEADER
#define OUTPUT_CONVERSION_HEADER

#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

/** Initialize module's internal state. */
ModuleDestructor initializeOutputConversionModule();

void executeOutputConversion(const CompilerState * compilerState);

#endif


