# 🎵 Drum Machine DSL - Quick Start

## ✅ Verificar que TODO funciona (1 comando)

```bash
./test-complete.sh
```

**Resultado esperado:**
```
✓ ALL TESTS PASSED!
Total tests: 13
Passed: 13
Failed: 0
```

Si ves esto, **TODO está funcionando al 100%** ✨

---

## 📖 ¿Qué está implementado?

### ✅ Análisis Léxico (Lexer)
- 24 tipos de tokens reconocidos
- Keywords: `tempo`, `compasses`, `steps`, `pattern`, `rhythm`, `instruments`, `active`
- Literales: números, notas musicales (E2, A#2, Bb3)
- Símbolos: `x` (hit), `.` (silence), `-` (melodic silence)
- Operadores: `+` (concatenación), `*` (repetición)
- Delimitadores: `{}`, `[]`, `,`

### ✅ Análisis Sintáctico (Parser)
- Gramática libre de contexto completa
- AST (Abstract Syntax Tree) completo
- Acciones semánticas funcionando
- Gestión de memoria sin leaks

---

## 🚀 Uso Básico

### 1. Crear un programa

```bash
cat > mi-programa.dsl << 'EOF'
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
        active 1-8
    }
    bass {
        pattern bassPattern
        active 1-8
    }
}
EOF
```

### 2. Compilar

```bash
docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh mi-programa.dsl"
```

### 3. Ver resultado

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
        Active: 1-8

=== End of Program ===
```

---

## 📚 Documentación Completa

- **TESTING.md** - Guía completa de testing
- **PARSER.md** - Documentación del parser
- **LEXER.md** - Documentación del lexer
- **NextSteps.md** - Requisitos originales del proyecto

---

## 🎯 Sintaxis del Lenguaje

```
programa ::= declarations patterns instruments

declarations ::=
    tempo INTEGER
    compasses INTEGER
    steps INTEGER

patterns ::=
    [pattern ID { rhythm rhythm_expr }]*

rhythm_expr ::=
    | rhythm_array
    | rhythm_expr + rhythm_expr      // concatenación
    | rhythm_array * INTEGER         // repetición

rhythm_array ::=
    [ rhythm_element [, rhythm_element]* ]

rhythm_element ::=
    | x           // hit (percusión)
    | .           // silence (percusión)
    | NOTE        // nota musical (E2, A#2, Bb3, etc.)
    | -           // melodic silence

instruments ::=
    instruments {
        [ID {
            pattern ID
            active INTEGER - INTEGER
        }]*
    }
```

---

## ⚡ Tests Rápidos

### Test Completo (recomendado)
```bash
./test-complete.sh
```

### Solo Lexer
```bash
./test-lexer.sh
```

### Solo Parser
```bash
./test-parser.sh
```

---

## 🐛 Troubleshooting

### Error: "command not found"
```bash
chmod +x test-complete.sh test-lexer.sh test-parser.sh
```

### Rebuild del proyecto
```bash
docker compose run --rm compiler bash -c "src/main/bash/build.sh"
```

### Ver logs detallados
```bash
docker compose run --rm -e LOGGING_LEVEL=ALL compiler bash -c \
  "src/main/bash/run.sh <archivo>" 2>&1 | less
```

---

## ✨ Ejemplos

### Ejemplo 1: Programa simple
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

### Ejemplo 2: Con repetición
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

### Ejemplo 3: Con concatenación
```
tempo 120
compasses 8
steps 4

pattern melody {
    rhythm [E2,G2] + [A2,C3] + [E2,G2] + [A2,-]
}

instruments {
    bass {
        pattern melody
        active 1-8
    }
}
```

### Ejemplo 4: Múltiples instrumentos
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
        active 1-16
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

## 📊 Estado del Proyecto

| Fase | Estado | Tests |
|------|--------|-------|
| Análisis Léxico | ✅ Completo | 100% |
| Análisis Sintáctico | ✅ Completo | 100% |
| AST | ✅ Completo | 100% |
| Backend/Generación | 🔜 Por hacer | - |

---

## 🎓 Resumen para el TP

**Frontend del compilador: COMPLETO ✅**

- ✅ Lexer: 24 tipos de tokens
- ✅ Parser: Gramática libre de contexto (tipo 2, Chomsky)
- ✅ AST: Árbol completo con todas las construcciones
- ✅ Acciones semánticas: Construcción del AST
- ✅ Memory management: Sin leaks
- ✅ Tests: 13 tests, todos pasan
- ✅ Documentación: Completa

**Para demostrar:**
1. Ejecuta `./test-complete.sh`
2. Muestra los 13 tests pasando
3. Ejecuta un programa de ejemplo
4. Muestra el AST construido

**Siguiente fase:** Backend - Generación de audio/MIDI
