#ifndef SEMANTIC_ANALYZER_HEADER
#define SEMANTIC_ANALYZER_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/ModuleDestructor.h"
#include "SymbolTable.h"

/**
 * Semantic Analyzer for the Drum Machine DSL.
 *
 * This module performs semantic analysis on the AST, including:
 * - Building the symbol table (patterns and instruments)
 * - Type checking all expressions
 * - Validating semantic constraints:
 *   - No duplicate pattern/instrument names
 *   - All pattern references exist
 *   - Active ranges are within compass bounds
 *   - All declarations are valid (positive values)
 *   - All rhythm expressions are type-correct
 */

/** Initialize module's internal state. */
ModuleDestructor initializeSemanticAnalyzerModule();

/**
 * Perform complete semantic analysis on a program.
 * This is the main entry point for semantic analysis.
 *
 * Steps:
 * 1. Build symbol table (collect all patterns and instruments)
 * 2. Validate declarations
 * 3. Validate all patterns (type checking, no duplicates)
 * 4. Validate all instruments (pattern references, active ranges)
 *
 * @param program The program AST to analyze
 * @return SUCCEEDED if semantic analysis passes, FAILED otherwise
 */
CompilationStatus analyzeProgram(Program * program);

/**
 * Get the symbol table built during semantic analysis.
 * This can be used by subsequent compilation phases.
 * Note: The symbol table is owned by the SemanticAnalyzer module
 * and will be destroyed when the module is destroyed.
 * @return The symbol table, or NULL if analysis hasn't been run yet
 */
SymbolTable * getSymbolTable(void);

#endif
