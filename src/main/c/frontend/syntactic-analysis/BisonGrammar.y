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

	Constant * constant;
	Expression * expression;
	Factor * factor;
	Program * program;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { destroyConstant($$); } <constant>
%destructor { destroyExpression($$); } <expression>
%destructor { destroyFactor($$); } <factor>

/** Terminals. */

/* Keywords */
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
%token <token> HIT
%token <token> SILENCE
%token <token> MELODIC_SILENCE

/* Operators */
%token <token> ADD
%token <token> MUL

/* Delimiters */
%token <token> OPEN_BRACE
%token <token> CLOSE_BRACE
%token <token> OPEN_BRACKET
%token <token> CLOSE_BRACKET
%token <token> COMMA

/* Comments (kept for backward compatibility) */
%token <token> OPEN_COMMENT
%token <token> CLOSE_COMMENT

/* Special */
%token <token> IGNORED
%token <token> UNKNOWN

/** Non-terminals. */
%type <constant> constant
%type <expression> expression
%type <factor> factor
%type <program> program

/**
 * Precedence and associativity.
 *
 * @see https://en.cppreference.com/w/cpp/language/operator_precedence.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */
%left ADD
%left MUL

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.
// NOTE: This is a temporary placeholder grammar for lexical analysis testing.
//       The full syntactic grammar will be implemented in the next phase.

program: INTEGER											{ $$ = ExpressionProgramSemanticAction(NULL); }
	| TEMPO												{ $$ = ExpressionProgramSemanticAction(NULL); }
	| ID												{ $$ = ExpressionProgramSemanticAction(NULL); }
	| NOTE												{ $$ = ExpressionProgramSemanticAction(NULL); }
	| HIT												{ $$ = ExpressionProgramSemanticAction(NULL); }
	| SILENCE											{ $$ = ExpressionProgramSemanticAction(NULL); }
	| MELODIC_SILENCE									{ $$ = ExpressionProgramSemanticAction(NULL); }
	;

expression: expression[left] ADD expression[right]			{ $$ = ArithmeticExpressionSemanticAction($left, $right, ADDITION); }
	| expression[left] MUL expression[right]				{ $$ = ArithmeticExpressionSemanticAction($left, $right, MULTIPLICATION); }
	| factor												{ $$ = FactorExpressionSemanticAction($1); }
	;

factor: constant											{ $$ = ConstantFactorSemanticAction($1); }
	;

constant: INTEGER											{ $$ = IntegerConstantSemanticAction($1); }
	;

%%
