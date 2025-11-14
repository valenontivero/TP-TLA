#include "SymbolTable.h"
#include <string.h>

static Logger * _logger = NULL;

static void _destroySymbolTableModule(void) {
	if (_logger != NULL) {
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeSymbolTableModule() {
	_logger = createLogger("SymbolTable");
	return _destroySymbolTableModule;
}

/**
 * Simple hash function (djb2 algorithm)
 */
static unsigned int _hash(const char * str, int capacity) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}
	return hash % capacity;
}

SymbolTable * createSymbolTable(int capacity) {
	SymbolTable * table = (SymbolTable *) calloc(1, sizeof(SymbolTable));
	if (table == NULL) {
		logError(_logger, "Failed to allocate memory for symbol table");
		return NULL;
	}

	table->buckets = (Symbol **) calloc(capacity, sizeof(Symbol *));
	if (table->buckets == NULL) {
		logError(_logger, "Failed to allocate memory for symbol table buckets");
		free(table);
		return NULL;
	}

	table->capacity = capacity;
	table->size = 0;

	logDebugging(_logger, "Created symbol table with capacity %d", capacity);
	return table;
}

void destroySymbolTable(SymbolTable * table) {
	if (table == NULL) {
		return;
	}

	logDebugging(_logger, "Destroying symbol table (size=%d, capacity=%d)", table->size, table->capacity);

	// Free all symbol entries (but NOT the AST nodes they reference)
	for (int i = 0; i < table->capacity; i++) {
		Symbol * symbol = table->buckets[i];
		while (symbol != NULL) {
			Symbol * next = symbol->next;
			free(symbol->name);  // Free the duplicated name
			free(symbol);        // Free the symbol entry itself
			symbol = next;
		}
	}

	free(table->buckets);
	free(table);
}

static bool _insertSymbol(SymbolTable * table, const char * name, SymbolType type, void * data) {
	if (table == NULL || name == NULL) {
		return false;
	}

	// Check if symbol already exists
	if (symbolExists(table, name)) {
		logError(_logger, "Symbol '%s' already declared", name);
		return false;
	}

	// Create new symbol
	Symbol * symbol = (Symbol *) calloc(1, sizeof(Symbol));
	if (symbol == NULL) {
		logError(_logger, "Failed to allocate memory for symbol '%s'", name);
		return false;
	}

	symbol->name = strdup(name);
	symbol->type = type;

	if (type == SYMBOL_PATTERN) {
		symbol->data.pattern = (Pattern *) data;
	} else {
		symbol->data.instrument = (Instrument *) data;
	}

	// Insert into hash table
	unsigned int index = _hash(name, table->capacity);
	symbol->next = table->buckets[index];
	table->buckets[index] = symbol;
	table->size++;

	const char * typeStr = (type == SYMBOL_PATTERN) ? "pattern" : "instrument";
	logDebugging(_logger, "Inserted %s '%s' (hash=%u, size=%d)", typeStr, name, index, table->size);

	return true;
}

bool insertPattern(SymbolTable * table, Pattern * pattern) {
	if (pattern == NULL || pattern->name == NULL) {
		logError(_logger, "Cannot insert NULL pattern");
		return false;
	}
	return _insertSymbol(table, pattern->name, SYMBOL_PATTERN, pattern);
}

bool insertInstrument(SymbolTable * table, Instrument * instrument) {
	if (instrument == NULL || instrument->name == NULL) {
		logError(_logger, "Cannot insert NULL instrument");
		return false;
	}
	return _insertSymbol(table, instrument->name, SYMBOL_INSTRUMENT, instrument);
}

Symbol * lookupSymbol(SymbolTable * table, const char * name) {
	if (table == NULL || name == NULL) {
		return NULL;
	}

	unsigned int index = _hash(name, table->capacity);
	Symbol * symbol = table->buckets[index];

	while (symbol != NULL) {
		if (strcmp(symbol->name, name) == 0) {
			logDebugging(_logger, "Found symbol '%s' (type=%s)",
				name, (symbol->type == SYMBOL_PATTERN) ? "pattern" : "instrument");
			return symbol;
		}
		symbol = symbol->next;
	}

	logDebugging(_logger, "Symbol '%s' not found", name);
	return NULL;
}

Pattern * lookupPattern(SymbolTable * table, const char * name) {
	Symbol * symbol = lookupSymbol(table, name);
	if (symbol != NULL && symbol->type == SYMBOL_PATTERN) {
		return symbol->data.pattern;
	}
	return NULL;
}

Instrument * lookupInstrument(SymbolTable * table, const char * name) {
	Symbol * symbol = lookupSymbol(table, name);
	if (symbol != NULL && symbol->type == SYMBOL_INSTRUMENT) {
		return symbol->data.instrument;
	}
	return NULL;
}

bool symbolExists(SymbolTable * table, const char * name) {
	return lookupSymbol(table, name) != NULL;
}
