# Testing Guide - Drum Machine DSL

## Quick Testing

To verify everything works correctly, run:

```bash
./test-complete.sh
```

This script runs 25 comprehensive tests that verify:
- Lexical analysis (24 token types)
- Syntactic analysis (complete grammar)
- AST construction
- Acceptance cases
- Rejection cases
- Edge cases
- Operators (concatenation and repetition)
- Range concatenation

---

## Individual Tests

### 1. Lexer (Lexical Analysis)

```bash
./test-lexer.sh
```

Tests that the lexer correctly recognizes all 24 token types.

### 2. Parser (Syntactic Analysis)

```bash
./test-parser.sh
```

Tests the parser with complete programs.

### 3. Manual Test of a Program

```bash
docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh src/test/c/accept/12-complex-program"
```

---

## Test Cases Included

### Acceptance Cases (must pass)

1. **11-simple-program** - Basic program
   - 1 pattern, 1 instrument
   - Simple array `[x,.,x,.]`

2. **12-complex-program** - Complete program
   - 2 patterns
   - Repetition: `[x,.,x,.] * 4`
   - Concatenation: `[E2,G2] + [A2,C3]`
   - 2 instruments

3. **13-concatenation** - Operator +
   - Array concatenation

4. **14-repetition** - Operator *
   - Array repetition

5. **15-multiple-instruments** - Multiple instruments
   - 3 patterns
   - 3 instruments (kick, snare, hihat)

6. **17-mixed-operations** - Mixed operations
   - `[x,.] * 2 + [x,x] + [.,.] * 2`
   - Correct precedence

7. **18-no-patterns** - Without patterns
   - Only declarations and empty instruments

8. **19-accidentals** - Accidentals
   - Notes with # and b: `C#4`, `Eb4`, `Gb4`

### Rejection Cases (must fail)

9. **01-missing-tempo** - Missing required declaration

10. **02-missing-brace** - Incorrect syntax (missing brace)

11. **03-invalid-token** - Invalid token (`@`)

12. **04-missing-rhythm-keyword** - Missing required keyword

---

## What Each Test Verifies

### PHASE 2: Parser Tests - Accept
Verifies that valid programs are accepted and parsed correctly.

### PHASE 3: Parser Tests - Reject
Verifies that invalid programs are rejected appropriately.

### PHASE 4: Advanced Features
- **Concatenation**: `rhythm1 + rhythm2`
- **Repetition**: `rhythm * N`
- **Multiple instruments**
- **Mixed operations**: Correct precedence
- **Range concatenation**: `active 1-4 + 6-8`

### PHASE 5: Edge Cases
- **Empty instruments**: Minimal valid program
- **Accidentals**: Complete support for # and b in notes

---

## Expected Result

When running `./test-complete.sh`, you should see:

```
==========================================
ALL TESTS PASSED!
==========================================

The Drum Machine DSL compiler is working correctly!

Summary:
  Lexer: All 24 token types recognized
  Parser: Complete grammar implemented
  AST: Full tree construction
  Semantic actions: All working
  Memory management: No leaks

Total tests: 25
Passed: 25
Failed: 0
```

---

## If Something Fails

### 1. Rebuild the project

```bash
docker compose run --rm compiler bash -c "src/main/bash/build.sh"
```

### 2. View detailed errors

```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <test-file>" 2>&1 | less
```

### 3. Verify program syntax

Ensure the program follows the syntax:

```
tempo <INTEGER>
compasses <INTEGER>
steps <INTEGER>

[pattern <ID> {
    rhythm <rhythm_expr>
}]*

instruments {
    [<ID> {
        pattern <ID>
        active <active_range_list>
    }]*
}
```

Where `active_range_list` can be:
- Single range: `1-16`
- Multiple ranges: `1-4 + 6-8 + 10-12`

---

## Test Coverage

| Component | Tests | Status |
|-----------|-------|--------|
| Lexer - Keywords | ✓ | 8 keywords |
| Lexer - Literals | ✓ | INTEGER, NOTE, STRING_LITERAL |
| Lexer - Symbols | ✓ | x, . |
| Lexer - Operators | ✓ | +, * |
| Lexer - Delimiters | ✓ | {, }, [, ], , , - |
| Parser - Declarations | ✓ | tempo, compasses, steps |
| Parser - Patterns | ✓ | pattern definitions |
| Parser - Rhythms | ✓ | arrays, +, * |
| Parser - Instruments | ✓ | instrument definitions |
| Parser - Active Range | ✓ | N-M notation |
| Parser - Range Concat | ✓ | N-M + N-M notation |
| AST Construction | ✓ | Full tree |
| Memory Management | ✓ | No leaks |
| Error Detection | ✓ | Reject invalid |

---

## Integration Tests

### Complete Functional Program

```bash
cat > test-program.txt << 'EOF'
tempo 120
compasses 16
steps 4

pattern kickPattern {
    rhythm [x,.,x,.] * 4
}

pattern bassPattern {
    rhythm [E2,G2,A2,.] + [E2,G2,A#2,A2] + [E2,G2,A2,.] + [G2,E2,.,.]
}

instruments {
    kick {
        pattern kickPattern
        active 1-4 + 10-16
    }
    bass {
        pattern bassPattern
        active 8-16
    }
}
EOF

docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh test-program.txt"
```

You should see:
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
        Active: 1-4 + 10-16

=== End of Program ===
```

---

## Debugging

### View recognized tokens

```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <program>" 2>&1 | grep "FlexActions"
```

### View semantic actions

```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <program>" 2>&1 | grep "BisonActions"
```

### View AST construction

```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <program>" 2>&1 | grep "destructor"
```

---

## Summary

To verify everything works correctly:

```bash
./test-complete.sh
```

If you see `ALL TESTS PASSED!`, the compiler is working perfectly.
