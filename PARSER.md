# Parser (Análisis Sintáctico) - Complete ✓

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

La gramática implementada sigue la especificación de NextSteps.md:

**G = ⟨Σ, Ν, Π, S⟩**

- **Σ (Alfabeto)**: 24 tokens definidos en el análisis léxico
- **Ν (No-terminales)**: program, declarations, pattern_list, pattern_def, rhythm_expr, rhythm_array, instrument_list, instrument_def, active_range
- **Π (Producciones)**: Reglas libres de contexto (tipo 2, Chomsky)
- **S (Símbolo inicial)**: program

### Non-Terminals

```c
program              // declarations + patterns + instruments
declarations         // tempo, compasses, steps
pattern_list         // lista de definiciones de patrones
pattern_def          // definición de un patrón
rhythm_expr          // expresión rítmica (array, concatenación, repetición)
rhythm_array         // [x,.,x,.]
rhythm_element_list  // lista de elementos rítmicos
rhythm_element       // x, ., nota, -
instrument_list      // lista de instrumentos
instrument_def       // definición de un instrumento
active_range         // rango activo (1-16)
```

### Productions

```
program → declarations pattern_list? INSTRUMENTS { instrument_list }

declarations → TEMPO INTEGER COMPASSES INTEGER STEPS INTEGER

pattern_list → pattern_def
             | pattern_list pattern_def

pattern_def → PATTERN ID { RHYTHM rhythm_expr }

rhythm_expr → rhythm_array
            | rhythm_expr + rhythm_expr
            | rhythm_array * INTEGER

rhythm_array → [ rhythm_element_list ]

rhythm_element_list → rhythm_element
                    | rhythm_element_list , rhythm_element

rhythm_element → x | . | NOTE | -

instrument_list → instrument_def
                | instrument_list instrument_def

instrument_def → ID { PATTERN ID active_range }

active_range → ACTIVE INTEGER - INTEGER
```

### AST Structures

Las estructuras del AST se definen en `AbstractSyntaxTree.h`:

- **Program**: declarations + patterns + instruments
- **Declarations**: tempo, compasses, steps (enteros)
- **Pattern**: nombre + rhythm expression
- **RhythmExpression**: array, concatenación, o repetición
- **RhythmArray**: lista de elementos
- **RhythmElement**: HIT (x), SILENCE (.), NOTE, MELODIC_SILENCE (-)
- **Instrument**: nombre + pattern reference + active range
- **ActiveRange**: start, end (enteros)

### Semantic Actions

Implementadas en `BisonActions.c`:

- `ProgramSemanticAction`: Construye el nodo raíz del AST
- `DeclarationsSemanticAction`: Crea nodo de declaraciones
- `PatternSemanticAction`: Crea definición de patrón
- `RhythmArrayExpressionSemanticAction`: Crea expresión de array
- `RhythmConcatenationSemanticAction`: Maneja operador +
- `RhythmRepetitionSemanticAction`: Maneja operador *
- `InstrumentSemanticAction`: Crea definición de instrumento
- `ActiveRangeSemanticAction`: Crea rango activo

---

## Example

**Input:**
```
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
        active 1-16
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
        Active: 1-16

=== End of Program ===
```

---

## Test Files

Located in `src/test/c/accept/`:
- `11-simple-program` - Programa simple con un patrón y un instrumento
- `12-complex-program` - Programa completo con repetición (*) y concatenación (+)

---

## Important Notes

### Precedence and Associativity

```c
%left ADD   // Concatenación (+): asociatividad izquierda
%left MUL   // Repetición (*): mayor precedencia
```

Esto significa que:
- `[x] + [.] + [-]` se evalúa como `([x] + [.]) + [-]`
- `[x] * 2 + [.]` se evalúa como `([x] * 2) + [.]`

### Memory Management

- Todos los nodos del AST se crean con `calloc()`
- Los destructores liberan recursivamente toda la memoria
- Las cadenas (ID, NOTE) se copian con `strdup()` y se liberan en los destructores

### Operator Semantics

**Repetición (`* N`):**
```
[x,.,x,.] * 4
```
Repite el array 4 veces.

**Concatenación (`+`):**
```
[E2,G2] + [A2,C3]
```
Concatena dos expresiones rítmicas.

---

## Implementation Details

### Files Modified

1. **AbstractSyntaxTree.h** - Nuevas estructuras AST
2. **AbstractSyntaxTree.c** - Destructores para todas las estructuras
3. **BisonGrammar.y** - Gramática completa con producciones
4. **BisonActions.h/c** - Acciones semánticas para construir AST
5. **Generator.c** - Generador de salida simple
6. **Calculator.c** - Desactivado (no relevante para DSL)
7. **EntryPoint.c** - Modificado para usar generador directamente

### Key Design Decisions

**1. RhythmExpression como unión:**
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
Permite representar tres tipos de expresiones rítmicas sin desperdiciar memoria.

**2. Listas enlazadas para patterns e instruments:**
Simplifica la construcción del AST en modo bottom-up (LALR parser).

**3. Separación de MELODIC_SILENCE y SILENCE:**
- `.` (SILENCE) - silencio de percusión
- `-` (MELODIC_SILENCE) - silencio melódico Y separador en active ranges

---

## Status

✓ **Syntactic Analysis: COMPLETE**
- Gramática libre de contexto implementada
- AST completo construido
- Acciones semánticas funcionando
- Destructores implementados
- Casos de prueba pasando
- Sin conflictos shift/reduce ni reduce/reduce

**Next Phase:** Backend - Generación de audio/MIDI

See `NextSteps.md` for next implementation steps.
