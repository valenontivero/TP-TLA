%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

/**
 * @see https://westes.github.io/flex/manual/Definitions-Section.html#Definitions-Section
 */

extern CompilationStatus DelimiterLexemeAction(TokenLabel label);
extern CompilationStatus EnterMultilineCommentLexemeAction(FlexContext context);
extern CompilationStatus EOFLexemeAction();
extern CompilationStatus HitLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus IntegerLexemeAction();
extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus LeaveMultilineCommentLexemeAction();
extern CompilationStatus NoteLexemeAction();
extern CompilationStatus OperatorLexemeAction(TokenLabel label);
extern CompilationStatus SilenceLexemeAction();
extern CompilationStatus StringLiteralLexemeAction();
extern CompilationStatus UnknownLexemeAction();

%}

/**
 * Flex contexts (a.k.a. start conditions).
 *
 * @see https://westes.github.io/flex/manual/Start-Conditions.html
 */
%x MULTILINE_COMMENT

/**
 * Reusable patterns.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */
digit				[0-9]
letter				[a-zA-Z]
note_letter			[A-G]
accidental			[#b]
whitespace			[[:space:]]+

%%

%{

/**
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

"/*"								{ return EnterMultilineCommentLexemeAction(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"				{ return LeaveMultilineCommentLexemeAction(); }
<MULTILINE_COMMENT>[^*]+			{ return IgnoredLexemeAction(); }
<MULTILINE_COMMENT>.				{ return IgnoredLexemeAction(); }

"remember"							{ return KeywordLexemeAction(IMPORT); }
"tempo"								{ return KeywordLexemeAction(TEMPO); }
"compasses"							{ return KeywordLexemeAction(COMPASSES); }
"steps"								{ return KeywordLexemeAction(STEPS); }
"pattern"							{ return KeywordLexemeAction(PATTERN); }
"rhythm"							{ return KeywordLexemeAction(RHYTHM); }
"instruments"						{ return KeywordLexemeAction(INSTRUMENTS); }
"active"							{ return KeywordLexemeAction(ACTIVE); }

"x"									{ return HitLexemeAction(); }
"."									{ return SilenceLexemeAction(); }

\"([^\\\"]|\\.)*\"					{ return StringLiteralLexemeAction(); }

{note_letter}{accidental}?{digit}	{ return NoteLexemeAction(); }

{letter}({letter}|{digit}|"_")*		{ return IdentifierLexemeAction(); }

{digit}+							{ return IntegerLexemeAction(); }

"+"									{ return OperatorLexemeAction(ADD); }
"*"									{ return OperatorLexemeAction(MUL); }

"{"									{ return DelimiterLexemeAction(OPEN_BRACE); }
"}"									{ return DelimiterLexemeAction(CLOSE_BRACE); }
"["									{ return DelimiterLexemeAction(OPEN_BRACKET); }
"]"									{ return DelimiterLexemeAction(CLOSE_BRACKET); }
","									{ return DelimiterLexemeAction(COMMA); }
"-"									{ return DelimiterLexemeAction(RANGE_SEPARATOR); }

{whitespace}						{ return IgnoredLexemeAction(); }
.									{ return UnknownLexemeAction(); }

<<EOF>>								{ return EOFLexemeAction(); }

%%

/**
 * @see https://westes.github.io/flex/manual/User-Code-Section.html#User-Code-Section
 */

#include "FlexActions.h"
#include "FlexExport.h"
