#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/**
 * Type definitions for Drum Machine DSL AST.
 */

typedef enum RhythmExpressionType RhythmExpressionType;
typedef enum RhythmElementType RhythmElementType;

typedef struct Program Program;
typedef struct Declarations Declarations;
typedef struct PatternList PatternList;
typedef struct Pattern Pattern;
typedef struct RhythmExpression RhythmExpression;
typedef struct RhythmArray RhythmArray;
typedef struct RhythmElementList RhythmElementList;
typedef struct RhythmElement RhythmElement;
typedef struct InstrumentList InstrumentList;
typedef struct Instrument Instrument;
typedef struct ActiveRange ActiveRange;

/**
 * Node types for the AST.
 */

enum RhythmExpressionType {
	RHYTHM_ARRAY,
	RHYTHM_CONCATENATION,
	RHYTHM_REPETITION
};

enum RhythmElementType {
	ELEMENT_HIT,
	ELEMENT_SILENCE,
	ELEMENT_NOTE,
	ELEMENT_MELODIC_SILENCE
};

/**
 * Declarations: tempo, compasses, steps
 */
struct Declarations {
	int tempo;
	int compasses;
	int steps;
};

/**
 * Rhythm elements: x, ., notes, -
 */
struct RhythmElement {
	RhythmElementType type;
	char * noteValue;  // Only for ELEMENT_NOTE
};

struct RhythmElementList {
	RhythmElement * element;
	RhythmElementList * next;
};

/**
 * Rhythm array: [x,.,x,.]
 */
struct RhythmArray {
	RhythmElementList * elements;
};

/**
 * Rhythm expressions: arrays, concatenation (+), repetition (*)
 */
struct RhythmExpression {
	RhythmExpressionType type;
	union {
		RhythmArray * array;
		struct {
			RhythmExpression * left;
			RhythmExpression * right;
		} concatenation;
		struct {
			RhythmArray * array;
			int repetitions;
		} repetition;
	};
};

/**
 * Pattern: name + rhythm expression
 */
struct Pattern {
	char * name;
	RhythmExpression * rhythm;
};

struct PatternList {
	Pattern * pattern;
	PatternList * next;
};

/**
 * Active range: 1-16
 */
struct ActiveRange {
	int start;
	int end;
};

/**
 * Instrument: name + pattern reference + active range
 */
struct Instrument {
	char * name;
	char * patternName;
	ActiveRange * activeRange;
};

struct InstrumentList {
	Instrument * instrument;
	InstrumentList * next;
};

/**
 * Program: declarations + patterns + instruments
 */
struct Program {
	Declarations * declarations;
	PatternList * patterns;
	InstrumentList * instruments;
};

/**
 * Node destructors.
 */

void destroyProgram(Program * program);
void destroyDeclarations(Declarations * declarations);
void destroyPatternList(PatternList * patternList);
void destroyPattern(Pattern * pattern);
void destroyRhythmExpression(RhythmExpression * rhythmExpression);
void destroyRhythmArray(RhythmArray * rhythmArray);
void destroyRhythmElementList(RhythmElementList * elementList);
void destroyRhythmElement(RhythmElement * element);
void destroyInstrumentList(InstrumentList * instrumentList);
void destroyInstrument(Instrument * instrument);
void destroyActiveRange(ActiveRange * activeRange);

#endif
