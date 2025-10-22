# Drum Machine DSL - Quick Start

## Verify Everything Works (1 command)

```bash
./test-complete.sh
```

**Expected result:**
```
ALL TESTS PASSED!
Total tests: 25
Passed: 25
Failed: 0
```

If you see this, everything is working correctly.

---

## What's Implemented?

### Lexical Analysis (Lexer)
- 24 token types recognized
- Keywords: `remember`, `tempo`, `compasses`, `steps`, `pattern`, `rhythm`, `instruments`, `active`
- Literals: numbers, musical notes (E2, A#2, Bb3), file paths ("lib/patterns.dsl")
- Symbols: `x` (hit), `.` (universal silence)
- Operators: `+` (concatenation), `*` (repetition)
- Delimiters: `{}`, `[]`, `,`, `-` (range separator for active ranges)

### Syntactic Analysis (Parser)
- Complete context-free grammar
- Complete AST (Abstract Syntax Tree)
- Working semantic actions
- Memory management without leaks
- Support for `remember`: Allows referencing external pattern files
- Support for active range concatenation: `active 1-4 + 6-8 + 10-12`

---

## Basic Usage

### 1. Create a program

```bash
cat > my-program.dsl << 'EOF'
remember "lib/basic-patterns.dsl"

tempo 120
compasses 8
steps 4

pattern kickPattern {
    rhythm [x,.,x,.] * 2
}

pattern bassPattern {
    rhythm [E2,G2,A2,.] + [E2,G2,A#2,A2]
}

instruments {
    kick {
        pattern kickPattern
        active 1-4 + 6-8
    }
    bass {
        pattern bassPattern
        active 1-8
    }
}
EOF
```

Note: There's a future consideration to allow declaring instruments within `pattern{}` blocks, but this is not currently implemented.

### 2. Compile

```bash
docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh my-program.dsl"
```

### 3. View result

```
=== Drum Machine Program ===

Tempo: 120 BPM
Compasses: 8
Steps: 4

Patterns:
    Pattern: bassPattern
    Pattern: kickPattern

Instruments:
    Instrument: bass
        Pattern: bassPattern
        Active: 1-8
    Instrument: kick
        Pattern: kickPattern
        Active: 1-4 + 6-8

=== End of Program ===
```

---

## Complete Documentation

- **TESTING.md** - Complete testing guide
- **PARSER.md** - Parser documentation
- **LEXER.md** - Lexer documentation
- **NextSteps.md** - Original project requirements

---

## Language Syntax

```
program ::= imports? declarations? patterns? instruments?

imports ::=
    [remember STRING_LITERAL]*

declarations ::=
    tempo INTEGER [compasses INTEGER [steps INTEGER]?]?

patterns ::=
    [pattern ID { rhythm rhythm_expr }]*

rhythm_expr ::=
    | rhythm_array
    | rhythm_expr + rhythm_expr      // concatenation
    | rhythm_array * INTEGER         // repetition

rhythm_array ::=
    [ rhythm_element [, rhythm_element]* ]

rhythm_element ::=
    | x           // hit (percussion)
    | .           // silence (universal)
    | NOTE        // musical note (E2, A#2, Bb3, etc.)

instruments ::=
    instruments {
        [ID {
            pattern ID
            active active_range_list
        }]*
    }

active_range_list ::=
    | active_range
    | active_range_list + active_range   // range concatenation

active_range ::= INTEGER - INTEGER
```

### `remember` Syntax (optional)

```
remember "path/to/file.dsl"
```

**Usage:**
- Allows referencing external files with pattern definitions
- Must appear at the start of the program, before declarations
- Currently recognized and parsed (file loading not yet implemented)

**Example:**
```
remember "lib/kick-patterns.dsl"
remember "lib/snare-patterns.dsl"

tempo 120
compasses 4
steps 4
...
```

---

## Quick Tests

### Complete Test (recommended)
```bash
./test-complete.sh
```

### Lexer Only
```bash
./test-lexer.sh
```

### Parser Only
```bash
./test-parser.sh
```

---

## Troubleshooting

### Error: "command not found"
```bash
chmod +x test-complete.sh test-lexer.sh test-parser.sh
```

### Rebuild the project
```bash
docker compose run --rm compiler bash -c "src/main/bash/build.sh"
```

### View detailed logs
```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <file>" 2>&1 | less
```

---

## Examples

### Example 1: Simple program
```
tempo 120
compasses 4
steps 4

pattern simple {
    rhythm [x,.,x,.]
}

instruments {
    kick {
        pattern simple
        active 1-4
    }
}
```

### Example 2: With repetition
```
tempo 140
compasses 8
steps 4

pattern repeating {
    rhythm [x,.,x,.] * 4
}

instruments {
    kick {
        pattern repeating
        active 1-8
    }
}
```

### Example 3: With concatenation
```
tempo 120
compasses 8
steps 4

pattern melody {
    rhythm [E2,G2] + [A2,C3] + [E2,G2] + [A2,.]
}

instruments {
    bass {
        pattern melody
        active 1-8
    }
}
```

### Example 4: Multiple instruments with range concatenation
```
tempo 120
compasses 16
steps 4

pattern kick {
    rhythm [x,.,x,.]
}

pattern snare {
    rhythm [.,x,.,x]
}

pattern hihat {
    rhythm [x,x,x,x]
}

instruments {
    kick {
        pattern kick
        active 1-4 + 6-8 + 10-16
    }
    snare {
        pattern snare
        active 1-16
    }
    hihat {
        pattern hihat
        active 5-16
    }
}
```

---

Next phase: Backend - Audio/MIDI generation
