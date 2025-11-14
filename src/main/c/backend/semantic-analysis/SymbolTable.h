#ifndef SYMBOL_TABLE_HEADER
#define SYMBOL_TABLE_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/**
 * Symbol Table for tracking pattern and instrument declarations.
 *
 * This module maintains a hash table of symbols (patterns and instruments)
 * declared in the DSL program. It supports:
 * - Creating and storing pattern definitions
 * - Creating and storing instrument definitions
 * - Looking up symbols by name
 * - Detecting redeclarations
 * - Validating pattern references in instruments
 */

/** Initialize module's internal state. */
ModuleDestructor initializeSymbolTableModule();

/** Symbol types */
typedef enum {
	SYMBOL_PATTERN,
	SYMBOL_INSTRUMENT
} SymbolType;

/** Symbol table entry */
typedef struct Symbol {
	char * name;
	SymbolType type;
	union {
		Pattern * pattern;      // For SYMBOL_PATTERN
		Instrument * instrument; // For SYMBOL_INSTRUMENT
	} data;
	struct Symbol * next;  // For hash collision chaining
} Symbol;

/** Symbol table structure */
typedef struct SymbolTable {
	Symbol ** buckets;
	int capacity;
	int size;
} SymbolTable;

/**
 * Create a new symbol table with the specified capacity.
 * @param capacity The number of hash buckets (recommended: power of 2)
 * @return A newly allocated symbol table, or NULL on failure
 */
SymbolTable * createSymbolTable(int capacity);

/**
 * Destroy a symbol table and free all resources.
 * Note: This does NOT destroy the AST nodes (Pattern/Instrument) stored in the table,
 * as those are owned by the AST itself.
 * @param table The symbol table to destroy
 */
void destroySymbolTable(SymbolTable * table);

/**
 * Insert a pattern into the symbol table.
 * @param table The symbol table
 * @param pattern The pattern to insert (AST node)
 * @return true if insertion succeeded, false if pattern name already exists
 */
bool insertPattern(SymbolTable * table, Pattern * pattern);

/**
 * Insert an instrument into the symbol table.
 * @param table The symbol table
 * @param instrument The instrument to insert (AST node)
 * @return true if insertion succeeded, false if instrument name already exists
 */
bool insertInstrument(SymbolTable * table, Instrument * instrument);

/**
 * Look up a symbol by name.
 * @param table The symbol table
 * @param name The name to search for
 * @return The symbol if found, NULL otherwise
 */
Symbol * lookupSymbol(SymbolTable * table, const char * name);

/**
 * Look up a pattern by name.
 * @param table The symbol table
 * @param name The pattern name
 * @return The pattern if found, NULL otherwise
 */
Pattern * lookupPattern(SymbolTable * table, const char * name);

/**
 * Look up an instrument by name.
 * @param table The symbol table
 * @param name The instrument name
 * @return The instrument if found, NULL otherwise
 */
Instrument * lookupInstrument(SymbolTable * table, const char * name);

/**
 * Check if a symbol with the given name exists.
 * @param table The symbol table
 * @param name The name to check
 * @return true if symbol exists, false otherwise
 */
bool symbolExists(SymbolTable * table, const char * name);

#endif
