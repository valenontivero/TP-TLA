#ifndef IMPORT_RESOLVER_HEADER
#define IMPORT_RESOLVER_HEADER

#include "../support/logging/Logger.h"
#include "../support/type/CompilationStatus.h"
#include "../support/type/ModuleDestructor.h"
#include "syntactic-analysis/AbstractSyntaxTree.h"
#include <stdbool.h>
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeImportResolverModule();

/**
 * Resolves all imports in a program by recursively loading and parsing
 * imported files, then merging their pattern definitions into the main program.
 *
 * @param program The main program with potential imports
 * @return SUCCEEDED if all imports were resolved successfully, FAILED otherwise
 */
CompilationStatus resolveImports(Program * program);

#endif
