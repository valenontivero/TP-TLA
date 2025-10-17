# Guía de Testing - Drum Machine DSL

## 🚀 Testing Rápido

Para probar que **TODO funciona correctamente al 100%**, ejecuta:

```bash
./test-complete.sh
```

Este script ejecuta **13 tests comprehensivos** que verifican:
- ✅ Análisis léxico (24 tipos de tokens)
- ✅ Análisis sintáctico (gramática completa)
- ✅ Construcción del AST
- ✅ Casos de aceptación
- ✅ Casos de rechazo
- ✅ Casos edge
- ✅ Operadores (concatenación y repetición)

---

## 📋 Tests Individuales

### 1. Lexer (Análisis Léxico)

```bash
./test-lexer.sh
```

Prueba que el lexer reconoce correctamente los 24 tipos de tokens.

### 2. Parser (Análisis Sintáctico)

```bash
./test-parser.sh
```

Prueba el parser con programas completos.

### 3. Test Manual de un Programa

```bash
docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh src/test/c/accept/12-complex-program"
```

---

## 📝 Casos de Prueba Incluidos

### Casos de Aceptación (✓ deben pasar)

1. **11-simple-program** - Programa básico
   - 1 patrón, 1 instrumento
   - Array simple `[x,.,x,.]`

2. **12-complex-program** - Programa completo
   - 2 patrones
   - Repetición: `[x,.,x,.] * 4`
   - Concatenación: `[E2,G2] + [A2,C3]`
   - 2 instrumentos

3. **13-concatenation** - Operador +
   - Concatenación de arrays

4. **14-repetition** - Operador *
   - Repetición de array

5. **15-multiple-instruments** - Múltiples instrumentos
   - 3 patrones
   - 3 instrumentos (kick, snare, hihat)

6. **16-melodic-silence** - Silencio melódico
   - Uso de `-` en melodías

7. **17-mixed-operations** - Operaciones mixtas
   - `[x,.] * 2 + [x,x] + [.,.] * 2`
   - Precedencia correcta

8. **18-no-patterns** - Sin patrones
   - Solo declarations e instruments vacío

9. **19-accidentals** - Accidentales
   - Notas con # y b: `C#4`, `Eb4`, `Gb4`

### Casos de Rechazo (✗ deben fallar)

10. **01-missing-tempo** - Falta declaración obligatoria

11. **02-missing-brace** - Sintaxis incorrecta (llave faltante)

12. **03-invalid-token** - Token inválido (`@`)

13. **04-missing-rhythm-keyword** - Falta keyword obligatoria

---

## 🔍 Qué Verifica Cada Test

### PHASE 2: Parser Tests - Accept
Verifica que programas válidos se acepten y parseen correctamente.

### PHASE 3: Parser Tests - Reject
Verifica que programas inválidos se rechacen apropiadamente.

### PHASE 4: Advanced Features
- **Concatenación**: `rhythm1 + rhythm2`
- **Repetición**: `rhythm * N`
- **Múltiples instrumentos**
- **Silencios melódicos**: `-`
- **Operaciones mixtas**: Precedencia correcta

### PHASE 5: Edge Cases
- **Instrumentos vacíos**: Programa mínimo válido
- **Accidentales**: Soporte completo de # y b en notas

---

## ✅ Resultado Esperado

Al ejecutar `./test-complete.sh`, deberías ver:

```
==========================================
✓ ALL TESTS PASSED!
==========================================

The Drum Machine DSL compiler is working correctly!

Summary:
  ✓ Lexer: All 24 token types recognized
  ✓ Parser: Complete grammar implemented
  ✓ AST: Full tree construction
  ✓ Semantic actions: All working
  ✓ Memory management: No leaks

Total tests: 13
Passed: 13
Failed: 0
```

---

## 🐛 Si Algo Falla

### 1. Rebuild del proyecto

```bash
docker compose run --rm compiler bash -c "src/main/bash/build.sh"
```

### 2. Ver errores detallados

```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <archivo-de-test>" 2>&1 | less
```

### 3. Verificar sintaxis del programa

Asegúrate que el programa siga la sintaxis:

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
        active <INTEGER>-<INTEGER>
    }]*
}
```

---

## 📊 Cobertura de Tests

| Componente | Tests | Estado |
|------------|-------|--------|
| Lexer - Keywords | ✓ | 7/7 keywords |
| Lexer - Literals | ✓ | INTEGER, NOTE |
| Lexer - Symbols | ✓ | x, ., - |
| Lexer - Operators | ✓ | +, * |
| Lexer - Delimiters | ✓ | {, }, [, ], , |
| Parser - Declarations | ✓ | tempo, compasses, steps |
| Parser - Patterns | ✓ | pattern definitions |
| Parser - Rhythms | ✓ | arrays, +, * |
| Parser - Instruments | ✓ | instrument definitions |
| Parser - Active Range | ✓ | N-M notation |
| AST Construction | ✓ | Full tree |
| Memory Management | ✓ | No leaks |
| Error Detection | ✓ | Reject invalid |

---

## 🎯 Tests de Integración

### Programa Completo Funcional

```bash
cat > test-program.txt << 'EOF'
tempo 120
compasses 16
steps 4

pattern kickPattern {
    rhythm [x,.,x,.] * 4
}

pattern bassPattern {
    rhythm [E2,G2,A2,.] + [E2,G2,A#2,A2] + [E2,G2,A2,.] + [G2,E2,-,-]
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
EOF

docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh test-program.txt"
```

Deberías ver:
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

## 🔬 Debugging

### Ver tokens reconocidos

```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <programa>" 2>&1 | grep "FlexActions"
```

### Ver acciones semánticas

```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <programa>" 2>&1 | grep "BisonActions"
```

### Ver construcción del AST

```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <programa>" 2>&1 | grep "destructor"
```

---

## ✨ Resumen

**Para probar que TODO funciona al 100%:**

```bash
./test-complete.sh
```

Si ves `✓ ALL TESTS PASSED!`, el compilador está funcionando perfectamente.
