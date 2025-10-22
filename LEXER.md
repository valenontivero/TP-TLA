# Lexical Analyzer - Complete

## Quick Start

### Run Tests
```bash
./test-lexer.sh
```

### Manual Test
```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh src/test/c/accept/01-tempo" 2>&1 | grep FlexActions
```

---

## Implementation Summary

### 24 Token Types Implemented

**Keywords (8):**
`remember`, `tempo`, `compasses`, `steps`, `pattern`, `rhythm`, `instruments`, `active`

**Literals (4):**
- `INTEGER` - Numbers (120, 16, 4)
- `NOTE` - Musical notes (E2, A#2, Bb3)
- `STRING_LITERAL` - File paths ("lib/patterns.dsl")
- `HIT` - Percussion hit: `x`
- `SILENCE` - Universal silence: `.`

**Identifiers (1):**
- `ID` - Pattern/instrument names (kickPattern, bass)

**Operators (2):**
- `+` (concatenation), `*` (repetition)

**Delimiters (6):**
- `{`, `}`, `[`, `]`, `,`
- `-` (RANGE_SEPARATOR - used only in active ranges like `1-4`)

**Special (3):**
- Comments (multiline with `/* */`)
- Whitespace (ignored)
- Unknown tokens (error reporting)

---

## Example

**Input:**
```
remember "lib/patterns.dsl"

tempo 120
pattern kickPattern {
    rhythm [x,.,x,.] * 4
}
```

**Tokens recognized:**
```
REMEMBER       "remember"
STRING_LITERAL "lib/patterns.dsl"
TEMPO          "tempo"
INTEGER        120
PATTERN        "pattern"
ID             "kickPattern"
OPEN_BRACE     "{"
RHYTHM         "rhythm"
OPEN_BRACKET   "["
HIT            "x"
COMMA          ","
SILENCE        "."
...
```

---

## Token Label Reference

```
258 = IMPORT         266 = INTEGER          274 = CLOSE_BRACE (})
259 = TEMPO          267 = NOTE             275 = OPEN_BRACKET ([)
260 = COMPASSES      268 = HIT (x)          276 = CLOSE_BRACKET (])
261 = STEPS          269 = SILENCE (.)      277 = COMMA (,)
262 = PATTERN        270 = ADD (+)          278 = RANGE_SEPARATOR (-)
263 = RHYTHM         271 = MUL (*)
264 = INSTRUMENTS    272 = OPEN_BRACE ({)
265 = ACTIVE         273 = STRING_LITERAL
```

---

## Important Notes

### Pattern Order in FlexPatterns.l

The order of patterns is critical for correct tokenization:

```c
1. Comments (/* ... */)
2. Keywords (exact strings: "tempo", "pattern", etc.)
3. Special symbols (x, .)
4. Range separator (-)
5. Notes (specific pattern: [A-G][#b]?[0-9])
6. Identifiers (general pattern: [a-zA-Z][a-zA-Z0-9_]*)
7. Integers ([0-9]+)
8. Operators and delimiters
```

This order ensures:
- Keywords match before identifiers
- `x` matches as HIT before being caught by identifier pattern
- Notes match before general identifiers

### Range Separator

The `-` character is ONLY used as RANGE_SEPARATOR in active range definitions (e.g., `active 1-4`). It is NOT a rhythm element. For silence in rhythms, use `.` instead.

### Silence

There is only one type of silence: `.` (SILENCE). It is used for both percussion and melodic silence.

---

## Implementation Details

### Files Modified

1. **BisonGrammar.y** - Token definitions
2. **FlexPatterns.l** - Regex patterns for all tokens
3. **FlexActions.c** - Token recognition functions
4. **SemanticValue union** - Added string type

### Key Pattern Order Fix

Pattern precedence issue resolved:
- Moved exact patterns (`"x"`, `"."`) BEFORE identifier pattern
- Now `x` is recognized as HIT, not as identifier
- All percussion symbols work correctly

---

## Status

Lexical Analysis: COMPLETE
- All 24 token types working
- Pattern matching correct
- Semantic values stored
- Test suite created
- Build system functional

Next Phase: Syntactic Analysis (Parser)

See `NextSteps.md` for requirements and implementation plan.
