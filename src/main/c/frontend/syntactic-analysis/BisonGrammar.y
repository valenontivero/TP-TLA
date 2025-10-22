%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	/** Terminals. */

	signed int integer;
	char * string;
	TokenLabel token;

	/** Non-terminals. */

	Program * program;
	ImportList * importList;
	ImportStatement * importStatement;
	Declarations * declarations;
	PatternList * patternList;
	Pattern * pattern;
	RhythmExpression * rhythmExpression;
	RhythmArray * rhythmArray;
	RhythmElementList * rhythmElementList;
	RhythmElement * rhythmElement;
	InstrumentList * instrumentList;
	Instrument * instrument;
	ActiveRange * activeRange;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { destroyImportList($$); } <importList>
%destructor { destroyImportStatement($$); } <importStatement>
%destructor { destroyDeclarations($$); } <declarations>
%destructor { destroyPatternList($$); } <patternList>
%destructor { destroyPattern($$); } <pattern>
%destructor { destroyRhythmExpression($$); } <rhythmExpression>
%destructor { destroyRhythmArray($$); } <rhythmArray>
%destructor { destroyRhythmElementList($$); } <rhythmElementList>
%destructor { destroyRhythmElement($$); } <rhythmElement>
%destructor { destroyInstrumentList($$); } <instrumentList>
%destructor { destroyInstrument($$); } <instrument>
%destructor { destroyActiveRange($$); } <activeRange>

/** String destructors for tokens that allocate memory */
%destructor { if ($$) free($$); } <string>

/** Terminals. */

/* Keywords */
%token <token> IMPORT
%token <token> TEMPO
%token <token> COMPASSES
%token <token> STEPS
%token <token> PATTERN
%token <token> RHYTHM
%token <token> INSTRUMENTS
%token <token> ACTIVE

/* Identifiers and Literals */
%token <string> ID
%token <integer> INTEGER
%token <string> NOTE
%token <string> STRING_LITERAL
%token <token> HIT
%token <token> SILENCE

/* Operators */
%token <token> ADD
%token <token> MUL

/* Delimiters */
%token <token> OPEN_BRACE
%token <token> CLOSE_BRACE
%token <token> OPEN_BRACKET
%token <token> CLOSE_BRACKET
%token <token> COMMA
%token <token> RANGE_SEPARATOR

/* Comments (kept for backward compatibility) */
%token <token> OPEN_COMMENT
%token <token> CLOSE_COMMENT

/* Special */
%token <token> IGNORED
%token <token> UNKNOWN

/** Non-terminals. */
%type <program> program
%type <program> full_program
%type <importList> import_list
%type <importList> import_list_opt
%type <importStatement> import_stmt
%type <declarations> declarations
%type <patternList> pattern_list
%type <patternList> pattern_list_opt
%type <pattern> pattern_def
%type <rhythmExpression> rhythm_expr
%type <rhythmArray> rhythm_array
%type <rhythmElementList> rhythm_element_list
%type <rhythmElement> rhythm_element
%type <instrumentList> instrument_list
%type <instrumentList> instrument_list_opt
%type <instrument> instrument_def
%type <activeRange> active_range
%type <activeRange> active_range_list
%type <instrumentList> instruments_opt
%type <declarations> declarations_opt

/**
 * Precedence and associativity.
 *
 * @see https://en.cppreference.com/w/cpp/language/operator_precedence.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */
%left ADD
%left MUL

%%

/**
 * Drum Machine DSL Grammar
 *
 * Modified to accept both full programs and individual components for testing
 */

program: full_program									{ $$ = $1; }
	;

full_program: import_list_opt declarations_opt pattern_list_opt instruments_opt	{ $$ = ProgramSemanticAction($1, $2, $3, $4); }
	;

/* Note: test_input rules commented out to eliminate shift/reduce conflicts with import_list_opt.
 * Lexer tests still work via ./test-lexer.sh. Parser tests use complete programs.
 */
/*
test_input: single_element								{ / * Single token tests * / }
	| element_sequence								{ / * Multiple token tests * / }
	| simple_declarations							{ / * Simple declaration tests * / }
	| pattern_component								{ / * Pattern-related tests * / }
	| import_stmt									{ / * Import statement tests * / }
	;
*/

/*
single_element: IMPORT									{ / * Keywords * / }
	| TEMPO
	| COMPASSES
	| STEPS
	| PATTERN
	| RHYTHM
	| INSTRUMENTS
	| ACTIVE
	| INTEGER										{ / * Values * / }
	| NOTE
	| HIT
	| SILENCE
	| MELODIC_SILENCE
	| ID
	| STRING_LITERAL
	;

element_sequence: single_element single_element			{ / * Two elements * / }
	| element_sequence single_element				{ / * More elements * / }
	;

simple_declarations: TEMPO INTEGER						{ / * tempo 120 * / }
	;

pattern_component: rhythm_array							{ / * [x,.,x,.] * / }
	| rhythm_expr									{ / * Complex rhythm expressions * / }
	| pattern_def									{ / * Pattern definitions * / }
	;
*/

import_list_opt: import_list								{ $$ = $1; }
	| %empty											{ $$ = NULL; }
	;

import_list: import_stmt								{ $$ = ImportListSemanticAction($1, NULL); }
	| import_list import_stmt							{ $$ = ImportListSemanticAction($2, $1); }
	;

import_stmt: IMPORT STRING_LITERAL							{ $$ = ImportStatementSemanticAction($2); }
	;

declarations_opt: declarations							{ $$ = $1; }
	| %empty											{ $$ = NULL; }
	;

instruments_opt: INSTRUMENTS OPEN_BRACE instrument_list_opt CLOSE_BRACE
														{ $$ = $3; }
	| %empty											{ $$ = NULL; }
	;

declarations: TEMPO INTEGER								{ $$ = DeclarationsSemanticAction($2, 4, 4); }
	| TEMPO INTEGER COMPASSES INTEGER				{ $$ = DeclarationsSemanticAction($2, $4, 4); }
	| TEMPO INTEGER COMPASSES INTEGER STEPS INTEGER	{ $$ = DeclarationsSemanticAction($2, $4, $6); }
	;

pattern_list_opt: pattern_list							{ $$ = $1; }
	| %empty											{ $$ = NULL; }
	;

pattern_list: pattern_def								{ $$ = PatternListSemanticAction($1, NULL); }
	| pattern_list pattern_def							{ $$ = PatternListSemanticAction($2, $1); }
	;

pattern_def: PATTERN ID OPEN_BRACE RHYTHM rhythm_expr CLOSE_BRACE
														{ $$ = PatternSemanticAction($2, $5); }
	;

rhythm_expr: rhythm_array								{ $$ = RhythmArrayExpressionSemanticAction($1); }
	| rhythm_expr[left] ADD rhythm_expr[right]			{ $$ = RhythmConcatenationSemanticAction($left, $right); }
	| rhythm_array MUL INTEGER							{ $$ = RhythmRepetitionSemanticAction($1, $3); }
	;

rhythm_array: OPEN_BRACKET rhythm_element_list CLOSE_BRACKET
														{ $$ = RhythmArraySemanticAction($2); }
	;

rhythm_element_list: rhythm_element						{ $$ = RhythmElementListSemanticAction($1, NULL); }
	| rhythm_element_list COMMA rhythm_element			{ $$ = RhythmElementListSemanticAction($3, $1); }
	;

rhythm_element: HIT										{ $$ = HitElementSemanticAction(); }
	| SILENCE											{ $$ = SilenceElementSemanticAction(); }
	| NOTE												{ $$ = NoteElementSemanticAction($1); }
	;

instrument_list_opt: instrument_list					{ $$ = $1; }
	| %empty											{ $$ = NULL; }
	;

instrument_list: instrument_def							{ $$ = InstrumentListSemanticAction($1, NULL); }
	| instrument_list instrument_def					{ $$ = InstrumentListSemanticAction($2, $1); }
	;

instrument_def: ID OPEN_BRACE PATTERN ID ACTIVE active_range_list CLOSE_BRACE
														{ $$ = InstrumentSemanticAction($1, $4, $6); }
	;

active_range_list: active_range							{ $$ = $1; }
    | active_range_list ADD active_range				{ $$ = ActiveRangeConcatenationSemanticAction($1, $3); }
    ;

active_range: INTEGER RANGE_SEPARATOR INTEGER	{ $$ = ActiveRangeSemanticAction($1, $3); }
	;

%%
