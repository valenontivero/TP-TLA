# Parser (Syntactic Analysis) - Complete

## Quick Start

### Run Tests
```bash
./test-parser.sh
```

### Manual Test
```bash
docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh src/test/c/accept/11-simple-program"
```

---

## Implementation Summary

### Grammar Structure

The implemented grammar follows the specification from NextSteps.md:

**G = ⟨Σ, Ν, Π, S⟩**

- **Σ (Alphabet)**: 24 tokens defined in lexical analysis
- **Ν (Non-terminals)**: program, import_list, import_stmt, declarations, pattern_list, pattern_def, rhythm_expr, rhythm_array, instrument_list, instrument_def, active_range, active_range_list
- **Π (Productions)**: Context-free rules (Type 2, Chomsky)
- **S (Start symbol)**: program

### Non-Terminals

```c
program              // imports + declarations + patterns + instruments
import_list          // list of remember statements
import_stmt          // individual remember statement
declarations         // tempo, compasses, steps
pattern_list         // list of pattern definitions
pattern_def          // pattern definition
rhythm_expr          // rhythm expression (array, concatenation, repetition)
rhythm_array         // [x,.,x,.]
rhythm_element_list  // list of rhythm elements
rhythm_element       // x, ., note
instrument_list      // list of instruments
instrument_def       // instrument definition
active_range         // active range (1-16)
active_range_list    // concatenated active ranges (1-4 + 6-8)
```

### Productions

```
program → import_list? declarations? pattern_list? instruments?

import_list → import_stmt
            | import_list import_stmt

import_stmt → REMEMBER STRING_LITERAL

declarations → TEMPO INTEGER
             | TEMPO INTEGER COMPASSES INTEGER
             | TEMPO INTEGER COMPASSES INTEGER STEPS INTEGER

pattern_list → pattern_def
             | pattern_list pattern_def

pattern_def → PATTERN ID { RHYTHM rhythm_expr }

rhythm_expr → rhythm_array
            | rhythm_expr + rhythm_expr
            | rhythm_array * INTEGER

rhythm_array → [ rhythm_element_list ]

rhythm_element_list → rhythm_element
                    | rhythm_element_list , rhythm_element

rhythm_element → x | . | NOTE

instrument_list → instrument_def
                | instrument_list instrument_def

instrument_def → ID { PATTERN ID ACTIVE active_range_list }

active_range_list → active_range
                  | active_range_list + active_range

active_range → INTEGER - INTEGER
```

### AST Structures

The AST structures are defined in `AbstractSyntaxTree.h`:

- **Program**: imports + declarations + patterns + instruments
- **ImportStatement**: file path (string)
- **ImportList**: linked list of import statements
- **Declarations**: tempo, compasses, steps (integers)
- **Pattern**: name + rhythm expression
- **RhythmExpression**: array, concatenation, or repetition
- **RhythmArray**: list of elements
- **RhythmElement**: HIT (x), SILENCE (.), NOTE
- **Instrument**: name + pattern reference + active range list
- **ActiveRange**: start, end (integers) + next pointer for concatenation

### Semantic Actions

Implemented in `BisonActions.c`:

- `ProgramSemanticAction`: Builds the AST root node
- `ImportListSemanticAction`: Builds import list
- `ImportStatementSemanticAction`: Creates individual remember statement
- `DeclarationsSemanticAction`: Creates declarations node
- `PatternSemanticAction`: Creates pattern definition
- `RhythmArrayExpressionSemanticAction`: Creates array expression
- `RhythmConcatenationSemanticAction`: Handles + operator
- `RhythmRepetitionSemanticAction`: Handles * operator
- `InstrumentSemanticAction`: Creates instrument definition
- `ActiveRangeSemanticAction`: Creates active range
- `ActiveRangeConcatenationSemanticAction`: Handles range concatenation

---

## Example

**Input:**
```
remember "lib/patterns.dsl"

tempo 120
compasses 16
steps 4

pattern kickPattern {
    rhythm [x,.,x,.] * 4
}

pattern bassPattern {
    rhythm [E2,G2,A2,.] + [E2,G2,A#2,A2]
}

instruments {
    kick {
        pattern kickPattern
        active 1-4 + 6-8 + 10-16
    }
    bass {
        pattern bassPattern
        active 8-16
    }
}
```

**Output:**
```
=== Drum Machine Program ===

Tempo: 120 BPM
Compasses: 16
Steps: 4

Patterns:
    Pattern: bassPattern
    Pattern: kickPattern

Instruments:
    Instrument: bass
        Pattern: bassPattern
        Active: 8-16
    Instrument: kick
        Pattern: kickPattern
        Active: 1-4 + 6-8 + 10-16

=== End of Program ===
```

---

## Important Notes

### Precedence and Associativity

```c
%left ADD   // Concatenation (+): left associativity
%left MUL   // Repetition (*): higher precedence
```

This means:
- `[x] + [.] + [x]` evaluates as `([x] + [.]) + [x]`
- `[x] * 2 + [.]` evaluates as `([x] * 2) + [.]`

### Memory Management

- All AST nodes are created with `calloc()`
- Destructors recursively free all memory
- Strings (ID, NOTE) are copied with `strdup()` and freed in destructors

### Operator Semantics

**Repetition (`* N`):**
```
[x,.,x,.] * 4
```
Repeats the array 4 times.

**Concatenation (`+`):**
```
[E2,G2] + [A2,C3]
```
Concatenates two rhythm expressions.

**Range Concatenation:**
```
active 1-4 + 6-8 + 10-12
```
Creates multiple active ranges for an instrument.

---

## Implementation Details

### Files Modified

1. **AbstractSyntaxTree.h** - New AST structures
2. **AbstractSyntaxTree.c** - Destructors for all structures
3. **BisonGrammar.y** - Complete grammar with productions
4. **BisonActions.h/c** - Semantic actions to build AST
5. **Generator.c** - Simple output generator
6. **Calculator.c** - Disabled (not relevant for DSL)
7. **EntryPoint.c** - Modified to use generator directly

### Key Design Decisions

**1. RhythmExpression as union:**
```c
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
```
Allows representing three types of rhythm expressions without wasting memory.

**2. Linked lists for patterns, instruments, and active ranges:**
Simplifies AST construction in bottom-up mode (LALR parser).

**3. Unified silence:**
- `.` (SILENCE) - universal silence for both percussion and melody
- No melodic-specific silence token

**4. Active range concatenation:**
- Active ranges use linked list structure
- Supports multiple discontinuous ranges per instrument
- Example: `active 1-4 + 6-8 + 10-12`

---

## Status

Syntactic Analysis: COMPLETE
- Context-free grammar implemented
- Complete AST constructed
- Semantic actions working
- Destructors implemented
- Test cases passing
- No shift/reduce or reduce/reduce conflicts

Next Phase: Backend - Audio/MIDI generation

See `NextSteps.md` for next implementation steps.
