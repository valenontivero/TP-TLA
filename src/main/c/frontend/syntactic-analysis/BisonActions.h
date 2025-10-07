#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule();

/**
 * Bison semantic actions for Drum Machine DSL.
 */

Program * ProgramSemanticAction(Declarations * declarations, PatternList * patterns, InstrumentList * instruments);
Declarations * DeclarationsSemanticAction(int tempo, int compasses, int steps);
PatternList * PatternListSemanticAction(Pattern * pattern, PatternList * next);
Pattern * PatternSemanticAction(char * name, RhythmExpression * rhythm);
RhythmExpression * RhythmArrayExpressionSemanticAction(RhythmArray * array);
RhythmExpression * RhythmConcatenationSemanticAction(RhythmExpression * left, RhythmExpression * right);
RhythmExpression * RhythmRepetitionSemanticAction(RhythmArray * array, int repetitions);
RhythmArray * RhythmArraySemanticAction(RhythmElementList * elements);
RhythmElementList * RhythmElementListSemanticAction(RhythmElement * element, RhythmElementList * next);
RhythmElement * HitElementSemanticAction();
RhythmElement * SilenceElementSemanticAction();
RhythmElement * NoteElementSemanticAction(char * noteValue);
RhythmElement * MelodicSilenceElementSemanticAction();
InstrumentList * InstrumentListSemanticAction(Instrument * instrument, InstrumentList * next);
Instrument * InstrumentSemanticAction(char * name, char * patternName, ActiveRange * activeRange);
ActiveRange * ActiveRangeSemanticAction(int start, int end);

#endif
