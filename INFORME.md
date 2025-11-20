# Informe Técnico: Compilador DSL para Máquina de Ritmos

**Autores:** Martín Vella
**Institución:** [Institución/Universidad]
**Fecha:** Noviembre 2024
**Versión del Compilador:** 2.0.0

---

## Tabla de Contenidos

1. [Introducción](#1-introducción)
2. [Modelo Computacional](#2-modelo-computacional)
   - 2.1. [Dominio](#21-dominio)
   - 2.2. [Lenguaje](#22-lenguaje)
3. [Implementación](#3-implementación)
   - 3.1. [Frontend](#31-frontend)
   - 3.2. [Backend](#32-backend)
   - 3.3. [Adicionales](#33-adicionales)
   - 3.4. [Dificultades Encontradas](#34-dificultades-encontradas)
4. [Futuras Extensiones](#4-futuras-extensiones)
5. [Conclusiones](#5-conclusiones)
6. [Referencias](#6-referencias)
7. [Bibliografía](#7-bibliografía)

---

## 1. Introducción

El presente informe documenta el diseño, implementación y validación de un compilador completo para un Lenguaje Específico de Dominio (DSL, por sus siglas en inglés) orientado a la creación de patrones rítmicos para máquinas de ritmos y 'drum machines'. El proyecto aborda la problemática de simplificar la composición musical algorítmica mediante un lenguaje declarativo de alto nivel que abstrae la complejidad de la generación de archivos MIDI.

### Motivación

La programación musical tradicional requiere conocimientos técnicos profundos sobre formatos de archivo binarios (MIDI), timing preciso, y comprensión de protocolos de comunicación musical. Este compilador democratiza la creación musical al permitir que compositores, productores y educadores expresen patrones rítmicos mediante sintaxis declarativa simple, delegando al compilador la responsabilidad de generar salida MIDI válida y reproducible.

### Objetivos del Proyecto

1. **Diseñar un DSL expresivo** que capture los conceptos fundamentales de la composición rítmica: tempo, compases, subdivisiones, patrones de ritmo, y asignación instrumental.

2. **Implementar un compilador completo** que incluya todas las fases clásicas de compilación: análisis léxico, análisis sintáctico, análisis semántico, y generación de código MIDI.

3. **Garantizar corrección** mediante validación exhaustiva: cero conflictos shift/reduce en el parser, cobertura completa de pruebas, y ausencia de fugas de memoria verificada con AddressSanitizer.

4. **Generar salida portable** en formato MIDI estándar (Format 1), reproducible en cualquier reproductor MIDI sin dependencias adicionales.

### Alcance

El compilador implementa:
- Frontend completo (lexer + parser) con construcción de AST
- Análisis semántico con tabla de símbolos y verificación de tipos
- Generación de código MIDI binario (Format 1, multi-track)
- Sistema de importación recursiva con detección de dependencias circulares
- 33 casos de prueba (16 aceptación + 18 rechazo) con 100% de éxito

Este documento detalla la arquitectura del compilador, las decisiones de diseño fundamentales, los desafíos encontrados durante la implementación, y las lecciones aprendidas en el proceso de construcción de un compilador de propósito específico desde cero.

---

## 2. Modelo Computacional

### 2.1. Dominio

El dominio del compilador es la **composición algorítmica de patrones rítmicos para instrumentos de percusión y melódicos** en el contexto de producción musical electrónica y educación musical.

#### Conceptos del Dominio

El modelo computacional se fundamenta en los siguientes conceptos musicales:

1. **Tempo**: Velocidad de reproducción medida en pulsaciones por minuto (BPM). Define la base temporal de toda la composición.

2. **Compás**: Unidad estructural de organización temporal. Un compás agrupa un número determinado de pasos (steps).

3. **Pasos (Steps)**: Subdivisiones dentro de un compás. Representan la resolución temporal mínima del lenguaje. Por ejemplo, `steps 4` indica 4 subdivisiones por compás (negras en notación tradicional si el denominador es 4).

4. **Patrón de Ritmo**: Secuencia de eventos musicales que pueden ser:
   - **HIT** (`x`): Golpe de percusión o evento activo
   - **SILENCE** (`.`): Silencio o ausencia de evento
   - **NOTE**: Nota melódica con altura definida (ej. `E2`, `A#2`, `Bb3`)

5. **Operadores sobre Ritmos**:
   - **Repetición** (`* N`): Duplica un patrón N veces consecutivas
   - **Concatenación** (`+`): Combina dos patrones secuencialmente

6. **Instrumento**: Entidad que ejecuta un patrón específico durante rangos de compases definidos (rangos activos).

7. **Rangos Activos**: Especificación de en qué compases un instrumento debe reproducirse. Soporta definición por intervalos (`1-4`) y concatenación de rangos discontinuos (`1-4 + 6-8 + 10-16`).

#### Ejemplo del Dominio

```dsl
tempo 140
compasses 16
steps 8

pattern kickPattern {
    rhythm [x,.,.,.,x,.,.,.]
}

instruments {
    kick {
        pattern kickPattern
        active 1-16
    }
}
```

Este programa especifica:
- Velocidad: 140 BPM
- Estructura: 16 compases de 8 pasos cada uno
- Patrón: Kick drum en los tiempos 1 y 5 de cada compás
- Ejecución: Durante todos los 16 compases

#### Formalización Matemática

El dominio puede formalizarse como:

- **Programa** `P = (T, C, S, Π, I)` donde:
  - `T ∈ ℕ⁺`: Tempo en BPM
  - `C ∈ ℕ⁺`: Número de compases
  - `S ∈ ℕ⁺`: Pasos por compás
  - `Π = {π₁, π₂, ..., πₙ}`: Conjunto de patrones
  - `I = {i₁, i₂, ..., iₘ}`: Conjunto de instrumentos

- **Patrón** `π = (nombre, ρ)` donde:
  - `nombre ∈ Σ*`: Identificador único
  - `ρ`: Expresión rítmica

- **Expresión Rítmica** `ρ` definida inductivamente:
  - `ρ ::= [e₁, e₂, ..., eₖ]` (array de elementos)
  - `ρ ::= ρ₁ + ρ₂` (concatenación)
  - `ρ ::= [e₁, ..., eₖ] * n` donde `n ∈ ℕ⁺` (repetición)

- **Elemento Rítmico** `e ∈ {HIT, SILENCE, NOTE(pitch)}`

- **Instrumento** `i = (nombre, π, R)` donde:
  - `nombre ∈ Σ*`: Identificador único
  - `π ∈ Π`: Referencia a patrón
  - `R ⊆ [1..C]`: Conjunto de compases activos

#### Restricciones del Dominio

1. **Unicidad de nombres**: Los identificadores de patrones e instrumentos deben ser únicos dentro de su ámbito.

2. **Referencias válidas**: Los instrumentos solo pueden referenciar patrones previamente definidos.

3. **Rangos válidos**: Los rangos activos deben satisfacer:
   - `1 ≤ inicio ≤ fin ≤ C`
   - `inicio < fin` (no se permiten rangos vacíos)

4. **Notas válidas**: Las notas melódicas deben seguir la notación anglosajona: `[A-G][#b]?[0-9]` donde:
   - Letra: A, B, C, D, E, F, G
   - Alteración opcional: `#` (sostenido) o `b` (bemol)
   - Octava: 0-9

5. **Valores positivos**: Tempo, compases y pasos deben ser enteros estrictamente positivos.

### 2.2. Lenguaje

#### Características del Lenguaje

El DSL para máquina de ritmos es un **lenguaje declarativo, imperativo de bajo nivel semántico, y de propósito específico** con las siguientes propiedades:

1. **Declarativo**: El programador especifica *qué* debe sonar, no *cómo* generarlo. El compilador maneja los detalles de timing MIDI, cálculo de delta times, codificación VLQ, etc.

2. **Tipado estático implícito**: Los tipos se infieren del contexto sintáctico. No hay declaraciones explícitas de tipos.

3. **Ámbito global simple**: Los patrones tienen ámbito global. No hay anidamiento de scopes ni shadowing.

4. **Modular**: Soporta importación de archivos externos mediante la directiva `remember`.

#### Gramática Formal

El lenguaje está definido por una **gramática libre de contexto (Tipo 2 en la jerarquía de Chomsky)** implementada con un parser LALR(1) sin conflictos.

##### Gramática BNF Completa

```bnf
<program> ::= <imports> <declarations> <patterns> <instruments>

<imports> ::= ε
            | <import_list>

<import_list> ::= <import>
                | <import_list> <import>

<import> ::= "remember" STRING_LITERAL

<declarations> ::= "tempo" INTEGER
                 | "tempo" INTEGER "compasses" INTEGER
                 | "tempo" INTEGER "compasses" INTEGER "steps" INTEGER

<patterns> ::= ε
             | <pattern_list>

<pattern_list> ::= <pattern_def>
                 | <pattern_list> <pattern_def>

<pattern_def> ::= "pattern" ID "{" "rhythm" <rhythm_expr> "}"

<rhythm_expr> ::= <rhythm_array>
                | <rhythm_expr> "+" <rhythm_expr>
                | <rhythm_array> "*" INTEGER

<rhythm_array> ::= "[" <rhythm_element_list> "]"

<rhythm_element_list> ::= <rhythm_element>
                        | <rhythm_element_list> "," <rhythm_element>

<rhythm_element> ::= "x"
                   | "."
                   | NOTE

<instruments> ::= "instruments" "{" <instrument_list> "}"

<instrument_list> ::= <instrument_def>
                    | <instrument_list> <instrument_def>

<instrument_def> ::= ID "{" "pattern" ID "active" <active_range_list> "}"

<active_range_list> ::= <active_range>
                      | <active_range_list> "+" <active_range>

<active_range> ::= INTEGER "-" INTEGER
```

##### No-Terminales (17)

| No-Terminal | Descripción |
|-------------|-------------|
| `program` | Raíz del AST |
| `imports` | Lista opcional de importaciones |
| `import_list` | Secuencia de imports |
| `import` | Directiva `remember` |
| `declarations` | Declaraciones globales (tempo, compasses, steps) |
| `patterns` | Lista opcional de patrones |
| `pattern_list` | Secuencia de definiciones de patrones |
| `pattern_def` | Definición de un patrón |
| `rhythm_expr` | Expresión rítmica (con operadores) |
| `rhythm_array` | Array literal de elementos rítmicos |
| `rhythm_element_list` | Lista de elementos separados por comas |
| `rhythm_element` | Elemento individual (x, ., o nota) |
| `instruments` | Bloque de instrumentos |
| `instrument_list` | Secuencia de instrumentos |
| `instrument_def` | Definición de un instrumento |
| `active_range_list` | Lista de rangos activos |
| `active_range` | Rango individual (INT-INT) |

##### Terminales (24 tokens)

Véase la sección 3.1 para la especificación completa de tokens.

#### Precedencia y Asociatividad de Operadores

El lenguaje define dos operadores binarios sobre expresiones rítmicas:

| Operador | Símbolo | Precedencia | Asociatividad | Descripción |
|----------|---------|-------------|---------------|-------------|
| Repetición | `*` | Alta | Izquierda | Repite array N veces |
| Concatenación | `+` | Baja | Izquierda | Une dos expresiones |

**Ejemplo de evaluación:**
```dsl
rhythm [x,.] * 2 + [.,x]
// Equivale a: ([x,.] * 2) + [.,x]
// Resultado: [x,.,x,.] + [.,x] = [x,.,x,.,.,x]
```

Esta precedencia garantiza que `*` se evalúe antes que `+`, consistente con la intuición matemática.

#### Ejemplo Completo Comentado

```dsl
/* === SECCIÓN 1: IMPORTACIONES === */
/* Permite reutilizar definiciones de patrones de bibliotecas externas */
remember "lib/kick.dsl"
remember "lib/snare.dsl"
remember "lib/hihat.dsl"

/* === SECCIÓN 2: DECLARACIONES GLOBALES === */
tempo 140              // Velocidad: 140 pulsaciones por minuto
compasses 16           // Estructura: 16 compases
steps 8                // Resolución: 8 pasos por compás (corcheas)

/* === SECCIÓN 3: DEFINICIÓN DE PATRONES === */
pattern kickPattern {
    // Patrón de bombo: golpe en tiempos 1 y 5 (síncopa)
    rhythm [x,.,.,.,x,.,.,.]
}

pattern snarePattern {
    // Patrón de caja: golpe en tiempos 3 y 7 (backbeat)
    rhythm [.,.,x,.,.,.,x,.]
}

pattern hihatPattern {
    // Patrón de hi-hat: corcheas constantes
    rhythm [x,x,x,x,x,x,x,x]
}

pattern bassline {
    /* Línea de bajo melódica: combina repetición y concatenación */
    /* Frase A (x2): E2 -> silencio -> A2 -> silencio */
    /* Frase B: E2 -> G2 -> A2 -> silencio */
    rhythm [E2,.,A2,.] * 2 + [E2,G2,A2,.]
}

/* === SECCIÓN 4: ASIGNACIÓN INSTRUMENTAL === */
instruments {
    kick {
        pattern kickPattern
        active 1-16           /* Activo durante todos los compases */
    }

    snare {
        pattern snarePattern
        active 1-16
    }

    hihat {
        pattern hihatPattern
        active 5-16           /* Entra en el compás 5 */
    }

    bass {
        pattern bassline
        active 1-4 + 6-8 + 10-16  /* Rangos discontinuos (breaks en 5 y 9) */
    }
}
```

Este programa genera un beat de drum'n'bass con bajo melódico, sincronización compleja, y rangos activos discontinuos que crean variación estructural.

#### Semántica Operacional

La semántica del lenguaje se define mediante transformaciones sobre el AST:

1. **Expansión de Repeticiones**: `[e₁,...,eₖ] * n → [e₁,...,eₖ,e₁,...,eₖ,...,e₁,...,eₖ]` (n copias)

2. **Expansión de Concatenaciones**: `ρ₁ + ρ₂ → append(expand(ρ₁), expand(ρ₂))`

3. **Generación de Eventos MIDI**:
   - Para cada instrumento `i = (nombre, π, R)`:
     - Expandir `ρ = expand(π.rhythmExpr)`
     - Para cada compás `c ∈ R`:
       - Dividir `ρ` en chunks de longitud `S` (steps)
       - Para cada elemento `e` en posición `p` del chunk correspondiente a `c`:
         - Si `e = HIT`: Generar MIDI Note On/Off en canal 10 (percusión)
         - Si `e = NOTE(pitch)`: Generar MIDI Note On/Off con pitch calculado
         - Si `e = SILENCE`: No generar evento
         - Calcular delta time según: `t = c * S * TPQN + p * TPQN`

4. **Resolución de Importaciones**:
   - Para cada `remember "file"`:
     - Parsear `file` recursivamente
     - Detectar ciclos mediante tracking de archivos visitados
     - Mergear patrones importados al AST principal

#### Sistema de Tipos

El lenguaje implementa un **sistema de tipos implícito con verificación estática**:

| Tipo | Descripción | Validación |
|------|-------------|------------|
| `RhythmElement` | HIT, SILENCE, NOTE | Reconocido léxicamente |
| `RhythmArray` | `[e₁, ..., eₖ]` | Todos los `eᵢ` deben ser `RhythmElement` |
| `RhythmExpr` | Expresión con `+` o `*` | Operandos de `*` deben ser array e integer; operandos de `+` deben ser expresiones |
| `ActiveRange` | `INT-INT` | Debe cumplir `1 ≤ inicio < fin ≤ compasses` |
| `PatternRef` | Identificador | Debe referenciar patrón existente |

Las validaciones se realizan en la fase de análisis semántico (tal como se describe en el archivo de "Sistema de Tipos" provisto en la consigna por la cátedra).

---

## 3. Implementación

### 3.1. Frontend

El frontend del compilador implementa las dos primeras fases clásicas de compilación: análisis léxico y análisis sintáctico. Estas fases transforman el código fuente en un Árbol de Sintaxis Abstracta (AST) listo para análisis semántico y generación de código.

#### 3.1.1. Análisis Léxico (Lexer)

El análisis léxico se implementa usando **Flex 2.6.4** en modo PUSH (característica v2.0.0), que permite al lexer empujar tokens al parser de forma asíncrona en lugar del tradicional modo PULL.

##### Características Técnicas

- **Archivo principal**: `src/main/c/frontend/lexical-analysis/FlexPatterns.l`
- **Acciones semánticas**: `src/main/c/frontend/lexical-analysis/FlexActions.c`
- **Tokens reconocidos**: 24 tipos distintos
- **Modo de operación**: PUSH (el lexer retorna `CompilationStatus`, no token labels)

##### Especificación de Tokens

| Categoría | Tokens | Etiqueta Bison | Ejemplo |
|-----------|--------|----------------|---------|
| **Palabras Clave** | remember | `REMEMBER` (258) | `remember` |
| | tempo | `TEMPO` (259) | `tempo` |
| | compasses | `COMPASSES` (260) | `compasses` |
| | steps | `STEPS` (261) | `steps` |
| | pattern | `PATTERN` (262) | `pattern` |
| | rhythm | `RHYTHM` (263) | `rhythm` |
| | instruments | `INSTRUMENTS` (264) | `instruments` |
| | active | `ACTIVE` (265) | `active` |
| **Literales** | Integer | `INTEGER` (266) | `120`, `16` |
| | Note | `NOTE` (267) | `E2`, `A#2`, `Bb3` |
| | Hit | `HIT` (268) | `x` |
| | Silence | `SILENCE` (269) | `.` |
| | String | `STRING_LITERAL` (273) | `"lib/patterns.dsl"` |
| **Identificadores** | Identifier | `ID` | `kickPattern`, `bass` |
| **Operadores** | Add | `ADD` (270) | `+` |
| | Multiply | `MUL` (271) | `*` |
| **Delimitadores** | Open Brace | `OPEN_BRACE` (272) | `{` |
| | Close Brace | `CLOSE_BRACE` (274) | `}` |
| | Open Bracket | `OPEN_BRACKET` (275) | `[` |
| | Close Bracket | `CLOSE_BRACKET` (276) | `]` |
| | Comma | `COMMA` (277) | `,` |
| | Range Separator | `RANGE_SEPARATOR` (278) | `-` |
| **Especiales** | Comment | (ignorado) | `/* ... */` |
| | Whitespace | (ignorado) | espacios, tabs, newlines |

##### Orden Crítico de Patrones

El orden de las reglas en `FlexPatterns.l` es **absolutamente crítico** para la correcta tokenización. Flex aplica la regla de **mayor coincidencia primero**, y en caso de empate, la **primera regla en aparecer**.

**Orden correcto:**

```c
/* 1. Comentarios (mayor prioridad para evitar conflictos) */
"/*"([^*]|"*"[^/])*"*/"   { /* ignorar */ }

/* 2. Palabras clave (antes que identificadores) */
"remember"                { return pushToken(REMEMBER); }
"tempo"                   { return pushToken(TEMPO); }
"pattern"                 { return pushToken(PATTERN); }
// ... resto de keywords

/* 3. Elementos rítmicos exactos (antes de patrones generales) */
"x"                       { return pushToken(HIT); }
"."                       { return pushToken(SILENCE); }

/* 4. Separador de rangos (tiene significado especial) */
"-"                       { return pushToken(RANGE_SEPARATOR); }

/* 5. Notas musicales (patrón específico) */
{note_letter}{accidental}?{digit}  {
    return pushToken(NOTE, yytext);
}

/* 6. Identificadores (patrón GENERAL - debe ir ÚLTIMO) */
{letter}({letter}|{digit}|"_")*   {
    return pushToken(ID, yytext);
}

/* 7. Enteros */
[0-9]+                    { return pushToken(INTEGER, atoi(yytext)); }

/* 8. Operadores y delimitadores */
"+"                       { return pushToken(ADD); }
"*"                       { return pushToken(MUL); }
// ... resto de delimitadores
```

**Justificación del orden:**

1. **Comentarios primero**: Evitan que `/*` sea interpretado como operador.

2. **Keywords antes de ID**: Si `"tempo"` apareciera después de `{letter}+`, sería reconocido como identificador genérico en lugar de palabra clave.

3. **Strings exactos (`x`, `.`) antes de patrones**: Si el patrón `{letter}+` apareciera antes, `x` sería reconocido como identificador, no como HIT.

4. **Notas antes de ID**: El patrón `E2` coincide tanto con `NOTE` como con `ID`. Colocando `NOTE` primero, se prioriza correctamente.

5. **ID al final**: Como patrón más general, debe aparecer después de todos los patrones más específicos.

##### Modo PUSH: Arquitectura Avanzada

En Flex tradicional (modo PULL), el lexer retorna códigos de token directamente:

```c
// Modo PULL (tradicional)
"tempo"  { return TEMPO; }  // Retorna código de token
```

En modo PUSH (v2.0.0), el lexer *empuja* tokens a una cola interna del parser y retorna un estado de compilación:

```c
// Modo PUSH (v2.0.0)
"tempo"  { return pushToken(TEMPO); }  // Retorna CompilationStatus
```

**Ventajas del modo PUSH:**
- Permite que una regla léxica genere 0 o múltiples tokens (útil para preprocesamiento)
- Desacopla el lexer del parser
- Facilita manejo de errores asíncronos

**Implementación de `pushToken()`**:

```c
CompilationStatus pushToken(const Token token, ...) {
    // Validar token
    if (!isValidToken(token)) {
        logError(_logger, "Invalid token: %d", token);
        return FAILED;
    }

    // Crear estructura de token
    TokenData data;
    data.token = token;

    // Extraer valor según tipo (usando va_list para argumentos variables)
    va_list args;
    va_start(args, token);
    if (token == INTEGER) {
        data.integer = va_arg(args, int);
    } else if (token == NOTE || token == ID || token == STRING_LITERAL) {
        const char* str = va_arg(args, const char*);
        data.string = strdup(str);  // CRÍTICO: copiar string
    }
    va_end(args);

    // Empujar a parser
    yylex_push_token(token, &data);

    logDebugging(_logger, "Token pushed: %s", getTokenName(token));
    return SUCCEEDED;
}
```

**Nota crítica sobre memoria**: Todos los strings (`yytext`) deben copiarse con `strdup()` porque el buffer interno de Flex se sobrescribe. Estos strings se liberan posteriormente en los destructores del AST.

#### 3.1.2. Análisis Sintáctico (Parser)

El análisis sintáctico se implementa usando **GNU Bison 3.8.2** con un parser LALR(1) que genera el AST completo.

##### Características Técnicas

- **Archivo de gramática**: `src/main/c/frontend/syntactic-analysis/BisonGrammar.y`
- **Acciones semánticas**: `src/main/c/frontend/syntactic-analysis/BisonActions.c`
- **Tipo de parser**: LALR(1) (Look-Ahead LR con 1 token de look-ahead)
- **Conflictos**: 0 shift/reduce, 0 reduce/reduce
- **Producciones**: 23 reglas gramaticales
- **No-terminales**: 17

##### Estructura del AST

El AST utiliza una jerarquía de estructuras con unions para eficiencia de memoria:

```c
// Nodo raíz del programa
typedef struct Program {
    ImportList* imports;           // Lista enlazada de imports
    Declarations* declarations;    // Tempo, compasses, steps
    PatternList* patterns;         // Lista enlazada de patrones
    InstrumentList* instruments;   // Lista enlazada de instrumentos
} Program;

// Expresión rítmica (union para eficiencia)
typedef struct RhythmExpression {
    RhythmExpressionType type;  // ARRAY, CONCATENATION, REPETITION
    union {
        struct {
            RhythmElementList* elements;
        } array;

        struct {
            struct RhythmExpression* left;
            struct RhythmExpression* right;
        } concatenation;

        struct {
            RhythmElementList* array;
            int repetitions;
        } repetition;
    };
} RhythmExpression;

// Elemento rítmico individual
typedef struct RhythmElement {
    RhythmElementType type;     // HIT, SILENCE, NOTE
    char* noteValue;            // Solo para type == NOTE (ej. "E2")
    struct RhythmElement* next; // Lista enlazada
} RhythmElement;

// Definición de patrón
typedef struct Pattern {
    char* name;                 // Identificador único
    RhythmExpression* rhythm;   // Expresión rítmica
    struct Pattern* next;       // Lista enlazada
} Pattern;

// Definición de instrumento
typedef struct Instrument {
    char* name;                 // Identificador único
    char* patternName;          // Referencia al patrón
    ActiveRangeList* activeRanges;  // Rangos activos
    struct Instrument* next;    // Lista enlazada
} Instrument;

// Rango activo (soporta concatenación con +)
typedef struct ActiveRange {
    int start;                  // Compás inicial (1-indexed)
    int end;                    // Compás final (inclusivo)
    struct ActiveRange* next;   // Lista enlazada para concatenación
} ActiveRange;
```

**Decisiones de diseño:**

1. **Listas enlazadas**: Simples de construir durante el parsing (LALR procesa de izquierda a derecha). Alternativamente, podrían usarse arrays dinámicos.

2. **Unions para RhythmExpression**: Ahorra memoria al permitir que un nodo sea array, concatenación, o repetición sin desperdiciar espacio.

3. **Strings copiados**: Todos los identificadores (`name`, `patternName`, `noteValue`) se copian con `strdup()` para evitar referencias a buffers temporales.

4. **Next pointers**: Facilitan recorrido y destrucción recursiva.

##### Acciones Semánticas

Cada producción gramatical tiene una acción semántica asociada que construye el nodo AST correspondiente:

```c
// Ejemplo: Construcción de patrón
// Producción: pattern_def: PATTERN ID OPEN_BRACE RHYTHM rhythm_expr CLOSE_BRACE
Pattern * PatternSemanticAction(const char * name, RhythmExpression * rhythm) {
    logDebugging(_logger, "PatternSemanticAction(%s)", name);

    // Allocar nodo (calloc inicializa a 0)
    Pattern * pattern = (Pattern *) calloc(1, sizeof(Pattern));
    if (pattern == NULL) {
        logError(_logger, "Memory allocation failed for Pattern");
        return NULL;
    }

    // Copiar nombre (CRÍTICO: strdup)
    pattern->name = strdup(name);
    if (pattern->name == NULL) {
        free(pattern);
        return NULL;
    }

    pattern->rhythm = rhythm;
    pattern->next = NULL;

    return pattern;
}

// Ejemplo: Construcción de lista de elementos rítmicos
// Producción: rhythm_element_list: rhythm_element_list COMMA rhythm_element
RhythmElementList * RhythmElementListSemanticAction(
    RhythmElementList * list,
    RhythmElement * element
) {
    if (list == NULL) {
        // Caso base: primer elemento
        return element;
    }

    // CRÍTICO: Append al FINAL (no prepend)
    // Si prepending → orden inverso → melodía al revés (Bug #1)
    RhythmElement * current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = element;

    return list;
}

// Ejemplo: Operador de repetición
// Producción: rhythm_expr: rhythm_array MUL INTEGER
RhythmExpression * RepetitionSemanticAction(
    RhythmElementList * array,
    int repetitions
) {
    logDebugging(_logger, "RepetitionSemanticAction(repetitions=%d)", repetitions);

    RhythmExpression * expr = (RhythmExpression *) calloc(1, sizeof(RhythmExpression));
    if (expr == NULL) return NULL;

    expr->type = REPETITION;
    expr->repetition.array = array;
    expr->repetition.repetitions = repetitions;

    return expr;
}
```

**Lección importante (Bug #1)**: En `RhythmElementListSemanticAction`, es fundamental **appendar al final** en lugar de prepending. La implementación original prepending causó que las melodías se reprodujeran al revés (`[E2,G2]` sonaba como `G2,E2`).

##### Gestión de Memoria

El AST implementa 13 funciones destructoras que liberan recursivamente toda la memoria:

```c
// Destructor principal
void destroyProgram(Program * program) {
    if (program == NULL) return;

    logDebugging(_logger, "Destroying Program");

    destroyImportList(program->imports);
    destroyDeclarations(program->declarations);
    destroyPatternList(program->patterns);
    destroyInstrumentList(program->instruments);

    free(program);
}

// Destructor de lista enlazada (patrón recursivo)
void destroyPatternList(PatternList * list) {
    if (list == NULL) return;

    logDebugging(_logger, "Destroying Pattern: %s", list->name);

    free(list->name);  // String copiado con strdup()
    destroyRhythmExpression(list->rhythm);
    destroyPatternList(list->next);  // Recursión

    free(list);
}

// Destructor de union (debe manejar todos los casos)
void destroyRhythmExpression(RhythmExpression * expr) {
    if (expr == NULL) return;

    switch (expr->type) {
        case ARRAY:
            destroyRhythmElementList(expr->array.elements);
            break;
        case CONCATENATION:
            destroyRhythmExpression(expr->concatenation.left);
            destroyRhythmExpression(expr->concatenation.right);
            break;
        case REPETITION:
            destroyRhythmElementList(expr->repetition.array);
            break;
    }

    free(expr);
}
```

**Verificación de fugas**: Se ejecuta AddressSanitizer en todos los tests. Actualmente: **0 fugas de memoria** en 34 tests.

### 3.2. Backend

El backend del compilador implementa las fases posteriores al frontend: análisis semántico, optimización, y generación de código. En este compilador, el backend se enfoca en validación semántica exhaustiva y generación de archivos MIDI reproducibles.

#### 3.2.1. Análisis Semántico

El análisis semántico valida que el programa es correcto según las reglas del dominio, más allá de la corrección sintáctica. Implementa tres componentes principales siguiendo la teoría descrita en los archivos provistos por la cátedra: `Tabla de Simbolos`, `Scopes`, y `Sistema de Tipos`.

##### Tabla de Símbolos

**Archivo**: `src/main/c/backend/semantic-analysis/SymbolTable.h/c`

La tabla de símbolos es la "base de datos" del compilador, almacenando información sobre todos los patrones e instrumentos declarados.

**Implementación**: Hash table con 64 buckets usando el algoritmo de hash **djb2**:

```c
#define SYMBOL_TABLE_SIZE 64

typedef struct SymbolTable {
    SymbolEntry* buckets[SYMBOL_TABLE_SIZE];
} SymbolTable;

typedef struct SymbolEntry {
    char* name;              // Identificador
    SymbolType type;         // PATTERN o INSTRUMENT
    void* data;              // Puntero al nodo AST correspondiente
    struct SymbolEntry* next;  // Chaining para colisiones
} SymbolEntry;

// Hash function: djb2 algorithm
unsigned int _hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return hash % SYMBOL_TABLE_SIZE;
}
```

**Operaciones implementadas** (siguiendo `Tabla de Simbolos`):

1. **Create** (Inserción):
```c
CompilationStatus insertPattern(SymbolTable* table, const char* name, Pattern* pattern) {
    // Verificar duplicados
    if (symbolExists(table, name)) {
        logError(_logger, "Duplicate pattern name: %s", name);
        return FAILED;
    }

    // Crear entrada
    SymbolEntry* entry = calloc(1, sizeof(SymbolEntry));
    entry->name = strdup(name);
    entry->type = SYMBOL_PATTERN;
    entry->data = pattern;

    // Insertar en bucket (chaining)
    unsigned int index = _hash(name);
    entry->next = table->buckets[index];
    table->buckets[index] = entry;

    logDebugging(_logger, "Pattern '%s' inserted into symbol table", name);
    return SUCCEEDED;
}
```

2. **Read** (Búsqueda):
```c
SymbolEntry* lookupSymbol(SymbolTable* table, const char* name) {
    unsigned int index = _hash(name);
    SymbolEntry* entry = table->buckets[index];

    // Recorrer cadena de colisiones
    while (entry != NULL) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;  // No encontrado
}
```

3. **Update**: No implementado (el lenguaje no permite redeclaraciones).

**Gestión de scopes** (siguiendo `Scopes`):

El DSL de máquina de ritmos tiene un modelo de scopes **simple y plano**:
- **Scope global**: Todos los patrones tienen ámbito global.
- **No hay scopes anidados**: Los instrumentos no definen nuevos scopes.
- **No hay shadowing**: No se permiten redeclaraciones.

Por lo tanto, no se requiere una pila de scopes. La tabla de símbolos única es suficiente.

##### Sistema de Tipos

**Archivo**: `src/main/c/backend/semantic-analysis/TypeChecker.h/c`

Implementa la función `type(x)` descrita en `Sistema-de-Tipos.md`, que asigna tipos a expresiones y valida compatibilidad.

**Tipos del lenguaje**:

```c
typedef enum {
    TYPE_RHYTHM_ELEMENT,   // x, ., NOTE
    TYPE_RHYTHM_ARRAY,     // [e1, e2, ..., en]
    TYPE_RHYTHM_EXPR,      // expr + expr, array * N
    TYPE_INTEGER,          // 120, 16, 4
    TYPE_ACTIVE_RANGE,     // INT-INT
    TYPE_PATTERN_REF,      // Identificador de patrón
    TYPE_INVALID           // ⊥ (bottom type)
} Type;
```

**Validaciones implementadas**:

1. **Elementos rítmicos**: Solo `x`, `.`, o notas válidas (`[A-G][#b]?[0-9]`):
```c
CompilationStatus validateRhythmElement(RhythmElement* element) {
    if (element == NULL) return FAILED;

    switch (element->type) {
        case HIT:
        case SILENCE:
            return SUCCEEDED;

        case NOTE:
            // Validar formato de nota
            if (!isValidNoteFormat(element->noteValue)) {
                logError(_logger, "Invalid note format: %s", element->noteValue);
                return FAILED;
            }
            return SUCCEEDED;

        default:
            logError(_logger, "Unknown rhythm element type");
            return FAILED;
    }
}

bool isValidNoteFormat(const char* note) {
    // Formato: [A-G][#b]?[0-9]
    if (note == NULL || strlen(note) < 2 || strlen(note) > 3) {
        return false;
    }

    // Verificar letra (A-G)
    char letter = note[0];
    if (letter < 'A' || letter > 'G') {
        return false;
    }

    int idx = 1;

    // Verificar alteración opcional
    if (note[idx] == '#' || note[idx] == 'b') {
        idx++;
    }

    // Verificar octava (0-9)
    if (idx >= strlen(note) || note[idx] < '0' || note[idx] > '9') {
        return false;
    }

    return true;
}
```

2. **Operadores sobre ritmos**:
```c
CompilationStatus validateRhythmExpression(RhythmExpression* expr) {
    if (expr == NULL) return FAILED;

    switch (expr->type) {
        case ARRAY:
            // Validar todos los elementos
            return validateRhythmElementList(expr->array.elements);

        case REPETITION:
            // Operando izquierdo: array
            // Operando derecho: entero positivo
            if (expr->repetition.repetitions <= 0) {
                logError(_logger, "Repetition count must be positive: %d",
                         expr->repetition.repetitions);
                return FAILED;
            }
            return validateRhythmElementList(expr->repetition.array);

        case CONCATENATION:
            // Ambos operandos deben ser expresiones rítmicas válidas
            if (validateRhythmExpression(expr->concatenation.left) == FAILED) {
                return FAILED;
            }
            return validateRhythmExpression(expr->concatenation.right);

        default:
            return FAILED;
    }
}
```

3. **Rangos activos**:
```c
CompilationStatus validateActiveRange(ActiveRange* range, int maxCompasses) {
    if (range == NULL) return FAILED;

    // Validar límites
    if (range->start < 1 || range->end > maxCompasses) {
        logError(_logger, "Active range [%d-%d] out of bounds [1-%d]",
                 range->start, range->end, maxCompasses);
        return FAILED;
    }

    // Validar orden
    if (range->start >= range->end) {
        logError(_logger, "Active range start (%d) must be less than end (%d)",
                 range->start, range->end);
        return FAILED;
    }

    return SUCCEEDED;
}
```

4. **Declaraciones positivas**:
```c
CompilationStatus validateDeclarations(Declarations* decl) {
    if (decl == NULL) return FAILED;

    if (decl->tempo <= 0) {
        logError(_logger, "Tempo must be positive: %d", decl->tempo);
        return FAILED;
    }

    if (decl->compasses <= 0) {
        logError(_logger, "Compasses must be positive: %d", decl->compasses);
        return FAILED;
    }

    if (decl->steps <= 0) {
        logError(_logger, "Steps must be positive: %d", decl->steps);
        return FAILED;
    }

    return SUCCEEDED;
}
```

##### Analizador Semántico

**Archivo**: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.h/c`

Orquesta la validación completa en 4 fases:

```c
CompilationStatus analyzeProgram(Program* program) {
    logInformation(_logger, "Starting semantic analysis");

    SymbolTable* table = createSymbolTable();

    // FASE 1: Validar declaraciones
    if (program->declarations != NULL) {
        if (validateDeclarations(program->declarations) == FAILED) {
            destroySymbolTable(table);
            return FAILED;
        }
    }

    // FASE 2: Construir tabla de símbolos con patrones
    PatternList* pattern = program->patterns;
    while (pattern != NULL) {
        if (insertPattern(table, pattern->name, pattern) == FAILED) {
            destroySymbolTable(table);
            return FAILED;
        }
        pattern = pattern->next;
    }

    // FASE 3: Validar expresiones rítmicas de cada patrón
    pattern = program->patterns;
    while (pattern != NULL) {
        if (validateRhythmExpression(pattern->rhythm) == FAILED) {
            destroySymbolTable(table);
            return FAILED;
        }
        pattern = pattern->next;
    }

    // FASE 4: Validar instrumentos (referencias a patrones, rangos activos)
    InstrumentList* instrument = program->instruments;
    while (instrument != NULL) {
        // Verificar que el patrón referenciado existe
        if (!symbolExists(table, instrument->patternName)) {
            logError(_logger, "Instrument '%s' references undefined pattern '%s'",
                     instrument->name, instrument->patternName);
            destroySymbolTable(table);
            return FAILED;
        }

        // Validar rangos activos
        if (program->declarations != NULL) {
            ActiveRange* range = instrument->activeRanges;
            while (range != NULL) {
                if (validateActiveRange(range, program->declarations->compasses) == FAILED) {
                    destroySymbolTable(table);
                    return FAILED;
                }
                range = range->next;
            }
        }

        // Insertar instrumento en tabla de símbolos
        if (insertInstrument(table, instrument->name, instrument) == FAILED) {
            destroySymbolTable(table);
            return FAILED;
        }

        instrument = instrument->next;
    }

    destroySymbolTable(table);
    logInformation(_logger, "Semantic analysis completed successfully");
    return SUCCEEDED;
}
```

**Errores semánticos que detectados**:
- Patrón duplicado
- Instrumento duplicado
- Referencia a patrón no definido
- Rango activo fuera de límites (> compasses)
- Rango activo inválido (start >= end)
- Valores negativos o cero en declaraciones
- Formato de nota inválido

#### 3.2.2. Generación de Código MIDI

**Archivos**:
- `src/main/c/backend/code-generation/MidiGenerator.h/c`
- `src/main/c/backend/code-generation/Generator.c`

La generación de código transforma el AST validado en un archivo MIDI binario reproducible, siguiendo los patrones descriptos en los archivos provistos en la consigna por la cátedra: `generacion de codigo` y `runtime`.

##### Estrategia de Generación

Implementa el patrón de **atributos sintetizados con descenso recursivo**:

1. **Traversal del AST**: Visitar cada nodo desde la raíz.
2. **Delegación**: Cada nodo delega la generación a sus hijos.
3. **Síntesis**: Los resultados de los hijos se combinan para generar la salida del nodo actual.
4. **Abstracción de salida**: Función `output()` encapsula escritura binaria.

```c
void generate(Program* program) {
    // 1. Generar header MIDI
    outputMidiHeader(program);

    // 2. Generar track 0: metadata (tempo, time signature)
    outputMetadataTrack(program->declarations);

    // 3. Para cada instrumento, generar track
    InstrumentList* instrument = program->instruments;
    while (instrument != NULL) {
        generateInstrumentTrack(instrument, program);
        instrument = instrument->next;
    }
}

void generateInstrumentTrack(Instrument* instr, Program* program) {
    // Resolver patrón desde tabla de símbolos
    Pattern* pattern = lookupPattern(instr->patternName);

    // Expandir expresión rítmica (delegación recursiva)
    RhythmElement* expanded = expandRhythmExpression(pattern->rhythm);

    // Generar eventos MIDI solo en rangos activos
    ActiveRange* range = instr->activeRanges;
    while (range != NULL) {
        for (int compass = range->start; compass <= range->end; compass++) {
            generateEventsForCompass(instr, expanded, compass, program->declarations);
        }
        range = range->next;
    }
}

// Expansión recursiva de expresiones (atributos sintetizados)
RhythmElement* expandRhythmExpression(RhythmExpression* expr) {
    switch (expr->type) {
        case ARRAY:
            // Caso base: retornar array directamente
            return copyRhythmElementList(expr->array.elements);

        case REPETITION:
            // Atributo sintetizado: repetir array N veces
            return repeatRhythmArray(expr->repetition.array,
                                     expr->repetition.repetitions);

        case CONCATENATION:
            // Atributo sintetizado: append(expand(left), expand(right))
            RhythmElement* left = expandRhythmExpression(expr->concatenation.left);
            RhythmElement* right = expandRhythmExpression(expr->concatenation.right);
            return appendRhythmLists(left, right);
    }
}
```

##### Formato MIDI: Especificación Técnica

El compilador genera **Standard MIDI Format 1** (multi-track):

**Estructura del archivo**:

```
┌─────────────────────────────────────┐
│      Header Chunk (MThd)            │
│  - Format: 1 (multi-track)          │
│  - Tracks: N+1 (metadata + N instr) │
│  - TPQN: 480 ticks per quarter note │
├─────────────────────────────────────┤
│      Track 0: Metadata              │
│  - Tempo meta-event                 │
│  - Time signature meta-event        │
│  - End of track                     │
├─────────────────────────────────────┤
│      Track 1: Instrument 1          │
│  - MIDI Note On/Off events          │
│  - Delta times (VLQ encoded)        │
│  - End of track                     │
├─────────────────────────────────────┤
│      Track 2: Instrument 2          │
│      ...                            │
└─────────────────────────────────────┘
```

**Implementación del header**:

```c
void writeMidiHeader(FILE* file, int numTracks) {
    // "MThd" chunk ID
    fwrite("MThd", 1, 4, file);

    // Chunk size (always 6 bytes)
    writeInt32BE(file, 6);

    // Format 1 (multi-track, synchronous)
    writeInt16BE(file, 1);

    // Number of tracks
    writeInt16BE(file, numTracks);

    // Ticks per quarter note
    writeInt16BE(file, TPQN);
}

void writeInt32BE(FILE* file, uint32_t value) {
    // Big-endian byte order (MIDI standard)
    uint8_t bytes[4];
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
    fwrite(bytes, 1, 4, file);
}
```

**Variable-Length Quantity (VLQ) Encoding**:

Los delta times en MIDI se codifican con VLQ para ahorrar espacio:

```c
void writeVLQ(FILE* file, uint32_t value) {
    // VLQ: 7 bits de datos + 1 bit de continuación
    // Ejemplo: 0x3FFF → 0xFF 0x7F
    //          0x0040 → 0x40

    uint8_t buffer[4];
    int index = 0;

    // Extraer grupos de 7 bits (de derecha a izquierda)
    buffer[index++] = value & 0x7F;
    value >>= 7;

    while (value > 0) {
        buffer[index++] = (value & 0x7F) | 0x80;  // Bit de continuación
        value >>= 7;
    }

    // Escribir en orden inverso (big-endian)
    for (int i = index - 1; i >= 0; i--) {
        fwrite(&buffer[i], 1, 1, file);
    }
}
```

**Generación de eventos MIDI**:

```c
void generateNoteEvent(FILE* file, uint32_t deltaTime, uint8_t channel,
                       uint8_t note, uint8_t velocity, bool noteOn) {
    // Escribir delta time (VLQ)
    writeVLQ(file, deltaTime);

    // Escribir status byte
    uint8_t status = noteOn ? 0x90 : 0x80;  // Note On / Note Off
    status |= (channel & 0x0F);             // Canal (0-15)
    fwrite(&status, 1, 1, file);

    // Escribir data bytes
    fwrite(&note, 1, 1, file);              // Número de nota (0-127)
    fwrite(&velocity, 1, 1, file);          // Velocidad (0-127)
}
```

##### Mapeo de Elementos Rítmicos a MIDI

**Percusión** (canal 10):

```c
void mapRhythmElementToMidi(RhythmElement* element, uint8_t* channel, uint8_t* note) {
    if (element->type == HIT) {
        *channel = 9;   // Canal 10 (0-indexed = 9)
        *note = 36;     // MIDI note 36 = Bass Drum 1 (General MIDI)
    } else if (element->type == SILENCE) {
        // No generar evento
    } else if (element->type == NOTE) {
        // Calcular nota MIDI desde notación anglosajona
        *channel = 0;   // Canal 1 (melódico)
        *note = noteToMidiNumber(element->noteValue);
    }
}
```

**Notas melódicas**:

```c
uint8_t noteToMidiNumber(const char* note) {
    // Formato: [A-G][#b]?[0-9]
    // Ejemplo: E2 → 40, A#2 → 46, Bb3 → 58

    // Mapeo de letras a semitonos (C=0, D=2, E=4, ...)
    const int pitchClass[] = {9, 11, 0, 2, 4, 5, 7};  // A B C D E F G

    char letter = note[0];
    int semitone = pitchClass[letter - 'A'];

    int idx = 1;

    // Procesar alteración
    if (note[idx] == '#') {
        semitone++;
        idx++;
    } else if (note[idx] == 'b') {
        semitone--;
        idx++;
    }

    // Extraer octava
    int octave = note[idx] - '0';

    // Fórmula MIDI: nota = 12 + (octava * 12) + semitono
    // MIDI note 60 = C4 (middle C)
    return 12 + (octave * 12) + semitone;
}
```

##### Cálculo de Timing

El timing preciso es **crítico** para la reproducción correcta. El compilador usa TPQN (Ticks Per Quarter Note) = 480.

```c
#define TPQN 480  // Resolución estándar

uint32_t calculateAbsoluteTick(int compass, int stepIndex, int stepsPerCompass) {
    // Cada paso = 1 quarter note
    int ticksPerStep = TPQN;

    // Cada compás = steps quarter notes
    int ticksPerCompass = TPQN * stepsPerCompass;

    // Tick absoluto = (compás * ticks/compás) + (paso * ticks/paso)
    return ((compass - 1) * ticksPerCompass) + (stepIndex * ticksPerStep);
}
```

**Nota sobre Bug #3**: La implementación original usaba `ticksPerStep = TPQN / steps`, lo que causaba reproducción 4x más rápida. La fórmula correcta es `ticksPerStep = TPQN`, porque la signatura temporal es `steps/4`, significando que hay `steps` quarter notes por compás.

##### Distribución de Elementos en Compases (Bug #2)

Un error sutil fue la distribución incorrecta de elementos rítmicos en compases:

**Implementación incorrecta** (reproducía TODO el patrón en CADA compás):
```c
// INCORRECTO
for (int compass = start; compass <= end; compass++) {
    for (int i = 0; i < totalElements; i++) {  // PROBLEMA: Todos los elementos en cada compás
        generateEvent(elements[i], compass, i);
    }
}
```

**Implementación correcta** (divide el patrón en chunks):
```c
// CORRECTO
int elementsPerCompass = steps;
int totalCompasses = totalElements / elementsPerCompass;

for (int compass = start; compass <= end; compass++) {
    // Calcular qué chunk del patrón corresponde a este compás
    int rhythmCompassIndex = (compass - 1) % totalCompasses;
    int startElement = rhythmCompassIndex * elementsPerCompass;

    // Solo generar los elementos de este chunk
    for (int i = 0; i < elementsPerCompass; i++) {
        if (startElement + i < totalElements) {
            generateEvent(elements[startElement + i], compass, i);
        }
    }
}
```

Esta corrección garantizó que un patrón de 16 elementos con `steps 4` se distribuya en 4 compases (4 elementos por compás), y no que los 16 elementos se reproduzcan en cada compás.

##### Separación Compiler-Runtime (siguiendo `runtime`)

El compilador y el runtime están **completamente desacoplados**:

| Responsabilidad | Componente |
|-----------------|------------|
| Parsear código DSL | Compiler |
| Validar semántica | Compiler |
| Generar archivo MIDI | Compiler |
| Reproducir MIDI | Client (reproductor MIDI del usuario) |
| Sintetizar audio | Client (soundfont del reproductor) |

**Ventajas de esta separación**:
1. **Portabilidad**: El archivo MIDI (`.mid`) funciona en cualquier sistema con reproductor MIDI (Windows Media Player, VLC, GarageBand, timidity, etc.)
2. **Cero dependencias**: No se requiere instalar bibliotecas de audio
3. **Flexibilidad**: El usuario puede elegir su reproductor y soundfont preferidos
4. **Estándar**: MIDI es un formato abierto con 40+ años de soporte

**Desventajas**:
- Dependencia de soundfonts del cliente (calidad variable)
- No hay control sobre síntesis de audio

Para este dominio, las ventajas superan ampliamente las desventajas.

### 3.3. Adicionales

#### 3.3.1. Sistema de Importación Recursiva

**Archivos**: `src/main/c/frontend/ImportResolver.h/c`

Implementa la directiva `remember "file.dsl"` con capacidades avanzadas:

**Características**:
1. Carga recursiva de archivos
2. Detección de dependencias circulares
3. Profundidad máxima (MAX_IMPORT_DEPTH = 32)
4. Merging de patrones al AST principal
5. Manejo de rutas relativas

**Implementación**:

```c
#define MAX_IMPORT_DEPTH 32

static const char* _visitedFiles[MAX_IMPORT_DEPTH];
static int _visitedCount = 0;

CompilationStatus resolveImports(Program* program) {
    if (program == NULL || program->imports == NULL) {
        return SUCCEEDED;
    }

    logInformation(_logger, "Resolving imports");

    ImportList* import = program->imports;
    while (import != NULL) {
        // Resolver recursivamente
        if (_resolveImportsRecursive(import->filePath, program) == FAILED) {
            return FAILED;
        }
        import = import->next;
    }

    logInformation(_logger, "All imports resolved successfully");
    return SUCCEEDED;
}

static CompilationStatus _resolveImportsRecursive(const char* filePath, Program* mainProgram) {
    // Detectar ciclos
    if (_isAlreadyVisited(filePath)) {
        logError(_logger, "Circular dependency detected: %s", filePath);
        return FAILED;
    }

    // Verificar profundidad máxima
    if (_visitedCount >= MAX_IMPORT_DEPTH) {
        logError(_logger, "Maximum import depth exceeded (%d)", MAX_IMPORT_DEPTH);
        return FAILED;
    }

    // Marcar como visitado
    _markAsVisited(filePath);

    // Verificar existencia del archivo
    if (access(filePath, F_OK) != 0) {
        logError(_logger, "Import file not found: %s", filePath);
        _unmarkLastVisited();
        return FAILED;
    }

    // Parsear archivo importado (con lexer independiente)
    Program* importedProgram = _parseFile(filePath);
    if (importedProgram == NULL) {
        _unmarkLastVisited();
        return FAILED;
    }

    // Resolver imports anidados recursivamente
    if (importedProgram->imports != NULL) {
        if (_resolveImportsRecursive(importedProgram->imports->filePath, mainProgram) == FAILED) {
            destroyProgram(importedProgram);
            _unmarkLastVisited();
            return FAILED;
        }
    }

    // Mergear patrones del archivo importado al programa principal
    mainProgram->patterns = _mergePatternLists(mainProgram->patterns,
                                               importedProgram->patterns);

    // Liberar AST importado (sin destruir los patrones, ahora owned por mainProgram)
    importedProgram->patterns = NULL;  // Transferir ownership
    destroyProgram(importedProgram);

    // Backtracking: desmarcar archivo para permitir re-importación desde otras ramas
    _unmarkLastVisited();

    return SUCCEEDED;
}
```

**Detección de ciclos con backtracking**:

El algoritmo mantiene una pila de archivos visitados (`_visitedFiles[]`) y usa **backtracking** para permitir patrones de importación en diamante:

```
main.dsl
  ├─ import A.dsl
  │    └─ import common.dsl
  └─ import B.dsl
       └─ import common.dsl  ← OK (no es ciclo, es diamante)
```

Pero detecta ciclos reales:

```
main.dsl
  └─ import A.dsl
       └─ import B.dsl
            └─ import A.dsl  ← ERROR (ciclo detectado)
```

**Gestión de memoria con múltiples parsers** (Bug #5):

Cada archivo importado requiere su propio lexer. Esto causó fugas de memoria porque `initializeFlexActionsModule()` creaba un nuevo logger en cada invocación.

**Solución**:
```c
ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer* lexicalAnalyzer) {
    _inputBuffer = NULL;
    _lexicalAnalyzer = lexicalAnalyzer;

    // CRÍTICO: Solo crear logger si no existe (evita fugas durante imports)
    if (_logger == NULL) {
        _logger = createLogger("FlexActions");
    }

    _logIgnoredLexemes = getBooleanOrDefault("LOG_IGNORED_LEXEMES", _logIgnoredLexemes);
    return _shutdownFlexActionsModule;
}
```

Esta corrección se aplicó también a `initializeBisonActionsModule()`.

#### 3.3.2. Sistema de Logging Configurable

El compilador hace uso del sistema de logging provisto.

```c
typedef enum {
    ALL,         // Todo (incluyendo detalles internos)
    DEBUGGING,   // Información de depuración
    INFORMATION, // Información general
    WARNING,     // Advertencias
    ERROR,       // Errores recuperables
    CRITICAL     // Errores fatales
} LogLevel;
```

**Configuración mediante variables de entorno**:

```bash
# Ver todos los tokens reconocidos
LOGGING_LEVEL=ALL src/main/bash/run.sh program.dsl 2>&1 | grep "FlexActions"

# Ver solo errores
LOGGING_LEVEL=ERROR src/main/bash/run.sh program.dsl

# Ver construcción del AST
LOGGING_LEVEL=ALL src/main/bash/run.sh program.dsl 2>&1 | grep "BisonActions"
```

**Ejemplo de salida**:
```
[INFORMATION] [EntryPoint] Starting compilation of: example.dsl
[DEBUGGING] [FlexActions] Token pushed: TEMPO (259)
[DEBUGGING] [FlexActions] Token pushed: INTEGER (266) = 120
[DEBUGGING] [BisonActions] TempoSemanticAction(120)
[INFORMATION] [SemanticAnalyzer] Starting semantic analysis
[INFORMATION] [SemanticAnalyzer] Pattern 'kickPattern' inserted into symbol table
[INFORMATION] [MidiGenerator] Generating MIDI file: output.mid
[INFORMATION] [EntryPoint] Compilation completed successfully
```

#### 3.3.4. Verificación de Memoria con AddressSanitizer

Todos los tests se ejecutan con AddressSanitizer para detectar:
- Fugas de memoria (memory leaks)
- Uso después de free (use-after-free)
- Desbordamientos de buffer (buffer overflow)
- Doble liberación (double free)

**Configuración**:
```makefile
CFLAGS += -fsanitize=address -fno-omit-frame-pointer -g
LDFLAGS += -fsanitize=address
```

**Resultado actual**: **0 fugas** en 34 tests.

### 3.4. Dificultades Encontradas

Durante el desarrollo del compilador, se encontraron 5 bugs significativos que requirieron análisis profundo y refactoring. Cada bug proporcionó lecciones valiosas sobre compiladores, MIDI, y gestión de memoria.

#### Bug #1: Orden de Elementos Rítmicos (Melodía Invertida)

**Síntoma**: Las melodías se reproducían al revés. Un patrón `[E2,G2,A2]` sonaba como `A2,G2,E2`.

**Causa raíz**: En `BisonActions.c`, la función `RhythmElementListSemanticAction` estaba **prepending** elementos a la lista en lugar de **appending**:

```c
// INCORRECTO (prepending)
RhythmElementList* RhythmElementListSemanticAction(RhythmElementList* list,
                                                    RhythmElement* element) {
    element->next = list;  // ❌ Nuevo elemento apunta al inicio
    return element;        // ❌ Retorna nuevo elemento como cabeza
}
// Resultado: orden inverso
```

**Explicación**: LALR parsers procesan las producciones de izquierda a derecha. En la producción `rhythm_element_list: rhythm_element_list COMMA rhythm_element`, primero se reduce `rhythm_element_list` (elementos previos), luego se añade el nuevo `rhythm_element`. Al hacer prepending, el último elemento procesado se coloca al principio, invirtiendo el orden.

**Solución**: Recorrer la lista hasta el final y appendar:

```c
// CORRECTO (appending)
RhythmElementList* RhythmElementListSemanticAction(RhythmElementList* list,
                                                    RhythmElement* element) {
    if (list == NULL) {
        return element;  // Caso base
    }

    // Recorrer hasta el final
    RhythmElement* current = list;
    while (current->next != NULL) {
        current = current->next;
    }

    // Appendar al final
    current->next = element;
    return list;  // Retornar cabeza original
}
```

En conclusión, en parsers LALR, el orden de construcción de listas es crítico. Siempre visualizar el orden de reducción.

#### Bug #2: Distribución de Elementos en Compases

**Síntoma**: Todos los elementos del patrón se reproducían en cada compás, creando un efecto de "aceleración" donde todo sonaba simultáneamente.

**Causa raíz**: El loop de generación de eventos no estaba **distribuyendo** los elementos entre compases, sino **repitiendo** todo el patrón en cada compás:

```c
// INCORRECTO
for (int compass = start; compass <= end; compass++) {
    for (int i = 0; i < totalElements; i++) {
        generateMidiEvent(elements[i], compass, i);  // Todos en cada compás
    }
}
```

**Ejemplo del error**:
```dsl
rhythm [x,.,x,.] * 4  // 16 elementos = [x,.,x,.,x,.,x,.,x,.,x,.,x,.,x,.,x,.]
compasses 8
steps 4
```

Con el código incorrecto, los 16 elementos se reproducían en cada uno de los 8 compases (128 eventos en total), cuando debían distribuirse: elementos 0-3 en compás 1, elementos 4-7 en compás 2, etc.

**Solución**: Dividir el patrón expandido en chunks de tamaño `steps` y mapear cada chunk a un compás:

```c
// CORRECTO
int elementsPerCompass = steps;
int totalRhythmCompasses = (totalElements + elementsPerCompass - 1) / elementsPerCompass;

for (int compass = start; compass <= end; compass++) {
    // Calcular índice del compás en el patrón (con wrap-around)
    int rhythmCompassIndex = (compass - 1) % totalRhythmCompasses;
    int startElement = rhythmCompassIndex * elementsPerCompass;

    // Generar solo los elementos de este chunk
    for (int i = 0; i < elementsPerCompass && (startElement + i) < totalElements; i++) {
        generateMidiEvent(elements[startElement + i], compass, i);
    }
}
```

**Cálculo del wrap-around**: Si el patrón tiene 4 compases de ritmo pero el instrumento está activo durante 16 compases, el patrón se cicla: compases 1-4 usan ciclo 1, compases 5-8 usan ciclo 2, etc.

#### Bug #3: Timing MIDI (Reproducción 4x Más Rápida)

**Síntoma**: Los archivos MIDI se reproducían 4 veces más rápido de lo esperado. Un programa con `tempo 120` sonaba como tempo 480.

**Causa raíz**: Cálculo incorrecto de `ticksPerStep`:

```c
// INCORRECTO
#define TPQN 480  // Ticks Per Quarter Note
int ticksPerStep = TPQN / steps;  // Si steps=4, ticksPerStep=120
```

**Explicación del error**:

En MIDI, TPQN (Ticks Per Quarter Note) define la resolución temporal: 480 ticks = 1 quarter note (negra).

La signatura temporal del lenguaje es `steps/4`, donde `steps` es el número de quarter notes por compás. Por ejemplo:
- `steps 4` → 4/4 (4 negras por compás)
- `steps 8` → 8/4 (8 negras por compás)

Si `ticksPerStep = TPQN / steps`:
- Con `steps=4`: `ticksPerStep = 480 / 4 = 120 ticks`
- Pero cada step es 1 quarter note, entonces `ticksPerStep` debería ser 480 ticks, no 120.

El error interpretaba "step" como subdivisión de quarter note (como 16th notes), cuando en realidad cada step **es** una quarter note completa.

**Solución**: Cada step = 1 quarter note:

```c
// CORRECTO
int ticksPerStep = TPQN;  // 480 ticks = 1 quarter note = 1 step
```

**Cálculo de ticks absolutos**:
```c
uint32_t calculateAbsoluteTick(int compass, int stepIndex, int stepsPerCompass) {
    int ticksPerCompass = TPQN * stepsPerCompass;
    return ((compass - 1) * ticksPerCompass) + (stepIndex * TPQN);
}
```

En conclusión, los formatos binarios (como MIDI) tienen especificaciones precisas que deben entenderse completamente. Un error de interpretación puede causar comportamiento sutilmente incorrecto.

Relacionado con `runtime`: entender el formato del runtime (MIDI) es esencial para generar salida correcta.

#### Bug #4: Declaraciones Obligatorias en Archivos de Biblioteca

**Síntoma**: Los tests CI/CD fallaban al intentar parsear archivos de biblioteca (pattern-only files) que no contenían declaraciones (`tempo`, `compasses`, `steps`).

**Causa raíz**: El analizador semántico asumía que `program->declarations` siempre estaba presente:

```c
// INCORRECTO
CompilationStatus analyzeProgram(Program* program) {
    // Asume que declarations existe
    if (validateDeclarations(program->declarations) == FAILED) {
        return FAILED;
    }
    // ...
}
```

Pero archivos de biblioteca como `lib/patterns.dsl` solo contienen patrones:

```dsl
// lib/patterns.dsl (sin declaraciones)
pattern basic {
    rhythm [x,.,x,.]
}
```

**Explicación del problema**: La gramática permite `declarations` opcionales (puede ser `NULL`), pero el análisis semántico no manejaba este caso. Esto violaba el principio de que patrones tienen **ámbito global** y pueden definirse independientemente.

**Solución**: Hacer las validaciones condicionales:

```c
// CORRECTO
CompilationStatus analyzeProgram(Program* program) {
    // Validar declaraciones solo si existen
    if (program->declarations != NULL) {
        if (validateDeclarations(program->declarations) == FAILED) {
            return FAILED;
        }
    }

    // Validar rangos activos solo si hay declaraciones
    if (program->declarations != NULL && instrument->activeRanges != NULL) {
        if (validateActiveRange(range, program->declarations->compasses) == FAILED) {
            return FAILED;
        }
    }

    // ...
}
```

Además, la generación de MIDI debe saltarse si no hay declaraciones:

```c
// En Generator.c
CompilationStatus generate(Program* program) {
    // Generar resumen textual siempre
    printProgramSummary(program);

    // Generar MIDI solo si hay declaraciones e instrumentos
    if (program->declarations != NULL && program->instruments != NULL) {
        generateMidiFile(program);
    } else {
        logInformation(_logger, "Skipping MIDI generation (pattern library file)");
    }

    return SUCCEEDED;
}
```

En conclusión, los compiladores deben manejar casos edge como archivos parciales, bibliotecas, y programas incompletos. No todos los archivos son programas completos.

Relacionado con `Scopes` - los patrones tienen ámbito global y pueden existir independientemente de un programa principal.

#### Bug #5: Fugas de Memoria Durante Importación

**Síntoma**: AddressSanitizer reportaba fugas de memoria (57 bytes) al ejecutar tests con imports:

```
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 57 byte(s) in 4 object(s) allocated from:
    #0 in malloc
    #1 in createLogger
    #2 in initializeFlexActionsModule
```

**Causa raíz**: Cada archivo importado requiere su propio lexical analyzer, lo que causa reinicialización de módulos. `initializeFlexActionsModule()` y `initializeBisonActionsModule()` creaban un nuevo logger en cada invocación:

```c
// INCORRECTO
ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer* lexicalAnalyzer) {
    _lexicalAnalyzer = lexicalAnalyzer;
    _logger = createLogger("FlexActions");  // Crea logger cada vez
    return _shutdownFlexActionsModule;
}
```

**Ejemplo del problema**:
```
main.dsl → initializeFlexActionsModule() → logger creado (dirección 0x1000)
  ├─ import A.dsl → initializeFlexActionsModule() → logger creado (dirección 0x2000)
  │                                                    0x1000 leaked
  └─ import B.dsl → initializeFlexActionsModule() → logger creado (dirección 0x3000)
                                                      0x2000 leaked
```

**Solución**: Verificar si el logger ya existe antes de crearlo (patrón singleton):

```c
// CORRECTO
ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer* lexicalAnalyzer) {
    _lexicalAnalyzer = lexicalAnalyzer;

    // Solo crear logger si no existe (permite múltiples inicializaciones)
    if (_logger == NULL) {
        _logger = createLogger("FlexActions");
    }

    return _shutdownFlexActionsModule;
}
```

La misma corrección se aplicó a `initializeBisonActionsModule()`.

**Consideración**: Esta solución asume que todos los lexers comparten el mismo logger, lo cual es aceptable porque los logs se distinguen por contexto. Una alternativa sería mantener un logger por lexer, pero requeriría refactoring más profundo.

La gestión de recursos globales (como loggers) en módulos que pueden reinicializarse requiere cuidado especial. Los patrones singleton son útiles pero deben aplicarse conscientemente.

Este bug surgió de la interacción entre el sistema de imports recursivos y la inicialización de módulos - un ejemplo de cómo características avanzadas pueden exponer bugs latentes en código base.

---

## 4. Futuras Extensiones

El compilador actual implementa un MVP funcional con todas las características core. Las siguientes extensiones podrían expandir significativamente las capacidades del lenguaje:

### 4.1. Extensiones del Lenguaje

#### 4.1.1. Velocidad (Velocity) Variable

**Motivación**: Actualmente todos los hits tienen velocidad fija (MIDI velocity = 100). La música real requiere dinámica (forte, piano, acentos).

**Propuesta sintáctica**:
```dsl
pattern hihat {
    // Notación: elemento@velocidad (0-127)
    rhythm [x@127, x@80, x@100, x@80]  // Acento en tiempo 1
}

// O con modificadores textuales
pattern hihat {
    rhythm [x@forte, x@piano, x@mezzo, x@piano]
}
```

**Implementación**:
- Extender `RhythmElement` con campo `velocity`
- Modificar lexer para reconocer `@` y valores de velocidad
- Mapear modificadores textuales a valores MIDI estándar
- Generar MIDI velocity bytes apropiados

Requiere cambios en lexer, parser, y generador.

#### 4.1.2. Selección de Instrumentos MIDI

**Motivación**: Actualmente, percusión usa siempre kick drum (note 36). Los instrumentos melódicos no tienen program change (usan piano por defecto).

**Propuesta sintáctica**:
```dsl
instruments {
    kick {
        midiInstrument 36      // Bass Drum 1 (General MIDI)
        pattern kickPattern
        active 1-16
    }

    bass {
        midiProgram 33         // Acoustic Bass (program change)
        midiChannel 1
        pattern bassPattern
        active 1-16
    }
}
```

**Implementación**:
- Agregar campos opcionales `midiInstrument`, `midiProgram`, `midiChannel` al nodo `Instrument`
- Generar eventos MIDI Program Change al inicio de cada track
- Validar rangos (note: 0-127, program: 0-127, channel: 0-15)

**Complejidad**: Media

#### 4.1.3. Duración de Notas

**Motivación**: Actualmente, todas las notas duran exactamente 1 step. La música real requiere staccato (corto) y legato (sostenido).

**Propuesta sintáctica**:
```dsl
pattern bass {
    // Notación: elemento:duración (en pasos)
    rhythm [E2:2, ., A2:1, .]  // E2 dura 2 steps, A2 dura 1 step
}

// O con modificadores
rhythm [E2:staccato, E2:legato]
```

**Implementación**:
- Extender `RhythmElement` con campo `duration`
- Calcular Note Off time: `noteOffTick = noteOnTick + (duration * ticksPerStep)`
- Manejar overlap de notas (polifonía)

**Complejidad**: Media-Alta

#### 4.1.4. Variables y Constantes

**Motivación**: Evitar repetición de valores "mágicos" y facilitar experimentación.

**Propuesta sintáctica**:
```dsl
// Definir constantes
const FAST_TEMPO = 140
const MAIN_PATTERN_LENGTH = 16

tempo FAST_TEMPO
compasses MAIN_PATTERN_LENGTH
steps 4
```

**Implementación**:
- Nueva fase de preprocesamiento antes del parsing
- Tabla de constantes separada de la tabla de símbolos
- Sustitución textual o evaluación durante semantic analysis

**Complejidad**: Media

#### 4.1.5. Operador de Interleaving (Shuffle)

**Motivación**: Combinar dos patrones de manera entrelazada (útil para hi-hats alternados).

**Propuesta sintáctica**:
```dsl
pattern shuffled {
    // Operador ~ intercala elementos: [a,b] ~ [c,d] = [a,c,b,d]
    rhythm [x,.] ~ [.,x]  // Resultado: [x,.,.,x]
}
```

**Implementación**:
- Nuevo tipo de `RhythmExpression`: INTERLEAVE
- Algoritmo: tomar alternadamente de ambas listas
- Validar que ambas listas tengan la misma longitud (o definir comportamiento para longitudes desiguales)

**Complejidad**: Baja

### 4.2. Extensiones de Análisis

#### 4.2.1. Advertencias de Optimización

**Motivación**: Detectar patrones subóptimos que podrían simplificarse.

**Ejemplos**:
```dsl
// ⚠️ Advertencia: repetición innecesaria
rhythm [x,.,x,.] * 1  // Sugerir: [x,.,x,.]

// ⚠️ Advertencia: concatenación puede combinarse
rhythm [x,.] + [x,.]  // Sugerir: [x,.,x,.]

// ⚠️ Advertencia: silencio completo
rhythm [.,.,.,.] * 4  // Instrumento no produce sonido
```

**Implementación**:
- Nueva fase de optimización/análisis después de semantic analysis
- Logs con nivel WARNING
- No bloquear compilación (solo advertencias)

**Complejidad**: Baja

#### 4.2.2. Detección de Polirritmia

**Motivación**: Advertir cuando patrones tienen longitudes inconmensurables (podría ser que el usuario esté creando efectos rítmicos no intencionales).

**Ejemplo**:
```dsl
tempo 120
compasses 8
steps 4  // 4 steps por compás

pattern A {
    rhythm [x,.,x]  // 3 elementos → periodo de 3 steps
}

instruments {
    inst {
        pattern A
        active 1-8  // 8 compases * 4 steps = 32 steps totales
                    // 32 no es múltiplo de 3 → patrón se trunca
    }
}
```

**Implementación**:
- Calcular MCD entre longitud del patrón y steps
- Si `totalElements % steps != 0`, emitir advertencia
- Sugerir ajustar patrón o steps

**Complejidad**: Baja

### 4.3. Extensiones del Backend

#### 4.3.1. Exportación a Otros Formatos

**Motivación**: Ampliar compatibilidad con diferentes workflows.

**Formatos propuestos**:
1. **JSON** (para DAWs web, visualizadores)
2. **MusicXML** (para editores de partituras)
3. **WAV** (audio directo, requiere síntesis)
4. **Ableton Live Clips** (integración con Ableton)

**Implementación**:
- Flag en línea de comandos: `--output-format midi|json|musicxml|wav`
- Generadores separados para cada formato
- Reutilizar el AST expandido (conversión independiente)

**Complejidad**: Alta (especialmente WAV, requiere síntesis de audio)

#### 4.3.2. Optimización de MIDI

**Motivación**: Los archivos MIDI actuales podrían ser más eficientes.

**Optimizaciones**:
1. **Running Status**: Omitir status bytes repetidos consecutivos (reduce tamaño ~20%)
2. **Note Off con Velocity 0**: Usar Note On con velocity=0 en lugar de Note Off (puede ahorrar 1 byte por evento)
3. **Compresión de Delta Times**: Usar deltas relativos más cortos cuando sea posible

**Implementación**:
- Agregar pase de optimización antes de escribir archivo
- Flag: `--optimize-midi`

**Complejidad**: Media

#### 4.3.3. Generación de Audio (WAV)

**Motivación**: Reproducción inmediata sin depender de soundfonts del cliente.

**Implementación**:
- Integrar librería de síntesis (ej. FluidSynth, libsndfile)
- Cargar soundfont (SF2) desde ruta configurable
- Renderizar MIDI events a samples de audio
- Escribir WAV con formato PCM (16-bit, 44.1kHz)

**Desafío**: Requiere dependencia externa (viola principio de runtime.md de "cero dependencias")

**Complejidad**: Alta

### 4.4. Herramientas y Utilidades

#### 4.4.1. REPL Interactivo

**Motivación**: Experimentación rápida sin crear archivos.

**Ejemplo de uso**:
```bash
$ dsl-repl
DrumDSL> tempo 140
DrumDSL> pattern test { rhythm [x,.,x,.] }
DrumDSL> :play test
[Reproduce patrón]
DrumDSL> :export test output.mid
[Exporta a MIDI]
```

**Implementación**:
- Loop read-eval-print sobre el compilador
- Mantener estado (patrones definidos previamente)
- Comandos especiales (`:play`, `:export`, `:list`, `:clear`)

**Complejidad**: Media-Alta

#### 4.4.2. Visualizador de Patrones

**Motivación**: Debugging visual de ritmos complejos.

**Funcionalidad**:
- Generar representación ASCII o HTML de patrones
- Grid view: compases × steps
- Color coding: hits vs silence vs notas

**Ejemplo de salida**:
```
Pattern: kickPattern (16 elements over 4 compasses)

Compass 1: [X . X . ]
Compass 2: [X . X . ]
Compass 3: [X . X . ]
Compass 4: [X . X . ]

Legend: X=hit, .=silence
```

**Implementación**:
- Nueva opción: `--visualize` o `--dry-run`
- Reutilizar lógica de expansión de ritmos
- Generar output textual o HTML

**Complejidad**: Baja-Media

#### 4.4.3. Language Server Protocol (LSP)

**Motivación**: Soporte IDE con autocompletado, go-to-definition, error highlighting.

**Funcionalidades LSP**:
1. **Syntax Highlighting**: Colores para keywords, identificadores, literales
2. **Autocompletado**: Sugerir nombres de patrones, keywords
3. **Diagnósticos**: Errores y warnings en tiempo real
4. **Go to Definition**: Click en `pattern kickPattern` salta a su definición
5. **Hover**: Mostrar información (tipo, longitud de patrón, etc.)

**Implementación**:
- Wrapper del compilador que expone LSP JSON-RPC
- Mantener AST en memoria para queries rápidos
- Integrar con VS Code, Vim, Emacs, etc.

**Complejidad**: Alta

### 4.5. Mejoras de Performance

#### 4.5.1. Compilación Incremental

**Motivación**: Recompilar solo archivos modificados (importante para proyectos grandes con muchos imports).

**Implementación**:
- Cachear ASTs de archivos importados (con timestamps)
- Solo re-parsear si el archivo cambió desde última compilación
- Invalidar cache de dependientes recursivamente

**Complejidad**: Media-Alta

#### 4.5.2. Paralelización

**Motivación**: Aprovechar múltiples cores para compilación más rápida.

**Oportunidades**:
1. Parsear imports en paralelo (si no hay dependencias circulares)
2. Generar tracks MIDI en paralelo (independientes)
3. Validaciones semánticas en paralelo

**Implementación**:
- Usar pthreads o OpenMP
- Sincronización cuidadosa para evitar race conditions

**Complejidad**: Alta

### 4.6. Resumen de Extensiones

| Motivación | Extensión | Justificación |
|-----------|-----------|---------------|
| 🔴 Alta | Velocity variable | Esencial para expresividad musical |
| 🔴 Alta | Selección de instrumentos MIDI | Amplía dramáticamente posibilidades sonoras |
| 🟡 Media | Duración de notas | Mejora expresividad, pero workaround posible |
| 🟡 Media | Visualizador de patrones | Muy útil para debugging |
| 🟡 Media | Exportación JSON | Abre integración con otras herramientas |
| 🟢 Baja | REPL | Nice-to-have, no crítico |
| 🟢 Baja | Optimización MIDI | Mejora marginal |
| ⚪ Futuro | LSP | Requiere inversión significativa |
| ⚪ Futuro | Generación WAV | Viola arquitectura actual (runtime.md) |

---

## 5. Conclusiones

Este proyecto implementó exitosamente un compilador completo para un Lenguaje Específico de Dominio (DSL) orientado a la composición de patrones rítmicos para máquinas de ritmos. El compilador transforma código declarativo de alto nivel en archivos MIDI estándar reproducibles, demostrando los principios fundamentales de la teoría de compiladores aplicados a un dominio concreto.

### 5.1. Logros Principales

#### 5.1.1. Completitud Técnica

El compilador implementa todas las fases clásicas de compilación:

1. **Análisis Léxico**: 24 tokens reconocidos con Flex en modo PUSH, manejo correcto de precedencia de patrones, comentarios multilinea, y strings literales.

2. **Análisis Sintáctico**: Parser LALR(1) con Bison, gramática libre de contexto sin conflictos (0 shift/reduce, 0 reduce/reduce), construcción completa de AST con 13 tipos de nodos.

3. **Análisis Semántico**: Tabla de símbolos con hash table, sistema de tipos implícito con validación estática, detección de errores semánticos (referencias indefinidas, duplicados, rangos inválidos).

4. **Generación de Código**: Producción de archivos MIDI binarios (Format 1, multi-track) con timing preciso, mapeo correcto de elementos rítmicos, codificación VLQ, y soporte completo de la especificación MIDI.

5. **Importación Recursiva**: Sistema robusto de imports con detección de dependencias circulares, profundidad máxima, merging de patrones, y gestión de memoria sin fugas.

#### 5.1.2. Corrección y Robustez

El compilador alcanza un buen nivel de calidad:

- **34/34 tests passing**
- **0 fugas de memoria** (verificado con AddressSanitizer)
- **0 conflictos** en el parser (gramática LALR(1) pura)
- **Manejo completo de errores** con logging
- **Cobertura exhaustiva** de casos edge (archivos vacíos, imports circulares, rangos inválidos, etc.)

#### 5.1.3. Adherencia a Teoría de Compiladores

El proyecto demuestra aplicación práctica de conceptos teóricos avanzados:

- **Tabla de Símbolos** (`Tabla de Simbolos`): Implementación eficiente con hash table (algoritmo djb2), operaciones CRUD, manejo de colisiones mediante chaining.

- **Scopes** (`Scopes`): Aunque el lenguaje tiene modelo de scopes simple (global para patrones), la arquitectura está preparada para extensión a scopes anidados mediante pila.

- **Sistema de Tipos** (`Sistema de Tipos`): Función `type(x)` implementada con validación estática, detección de incompatibilidades, y uso de tipo bottom (⊥) para errores.

- **Generación de Código** (`generacion de codigo`): Patrón de atributos sintetizados con descenso recursivo, delegación a nodos hijos, abstracción de salida mediante `output()`, expansión de operadores (`*`, `+`).

- **Runtime** (`runtime`): Separación clara entre responsabilidades del compilador (generación MIDI) y del cliente (reproducción), resultando en salida portable sin dependencias.

#### 5.1.4. Expresividad del Lenguaje

El DSL alcanza un balance entre simplicidad y expresividad:

- **Sintaxis minimalista**: Solo 8 keywords, 2 operadores, sintaxis declarativa clara.
- **Patrones reutilizables**: Definición de bibliotecas de patrones compartibles.
- **Operadores compositivos**: Concatenación (`+`) y repetición (`*`) permiten construcción de ritmos complejos desde primitivas simples.
- **Rangos activos flexibles**: Soporte de discontinuidades (`1-4 + 6-8`) para estructuras musicales variadas.
- **Soporte melódico**: Notas con altura definida (no solo percusión), notación anglosajona estándar con alteraciones (`#`, `b`).

### 5.2. Lecciones Aprendidas

#### 5.2.1. Orden Importa en Flex

El ordenamiento de reglas en `FlexPatterns.l` es **crítico**. Patrones específicos deben preceder a patrones generales, o tokens como `x` (HIT) serán capturados incorrectamente por el patrón de identificador. Esta lección se aplica universalmente a todos los lexers basados en expresiones regulares.

#### 5.2.2. Gestión de Memoria en C Requiere Disciplina

A pesar de usar `calloc()` para inicialización segura y destructores recursivos, surgieron 5 bugs relacionados con memoria:
- Strings copiados incorrectamente (faltaba `strdup()`)
- Loggers duplicados durante imports (faltaba verificación de singleton)
- Ownership ambiguo durante merging de ASTs importados

**Lección**: En proyectos C grandes, considerar herramientas como Valgrind y AddressSanitizer desde el día 1. La detección temprana de fugas ahorra horas de debugging.

#### 5.2.3. Testing Exhaustivo Detecta Bugs Sutiles

Los 5 bugs críticos fueron detectados por el suite de testing:
- **Bug #1** (melodía inversa): Detectado por test de patrones melódicos
- **Bug #2** (distribución incorrecta): Detectado al escuchar MIDI generado
- **Bug #3** (timing 4x rápido): Detectado al comparar tempo real vs. esperado
- **Bug #4** (declaraciones obligatorias): Detectado por tests de imports
- **Bug #5** (fugas de memoria): Detectado por AddressSanitizer

**Lección**: El testing no debe ser una fase final, sino continuo durante el desarrollo. Cada nueva feature debe venir acompañada de tests.

#### 5.2.4. Especificaciones Binarias No Toleran Aproximaciones

La implementación MIDI requirió comprensión exacta de:
- Big-endian byte order
- Codificación VLQ (Variable-Length Quantity)
- Formato de eventos MIDI (status bytes, data bytes)
- Cálculo de delta times
- Estructura de chunks (MThd, MTrk)

**Lección**: Al implementar formatos binarios estándar, estudiar la especificación oficial es indispensable. Los tutoriales pueden omitir detalles críticos.

#### 5.2.5. La Separación Compiler-Runtime Es Poderosa

Siguiendo el principio de `runtime`, el compilador genera archivos MIDI autocontenidos reproducibles en **cualquier sistema** sin instalar librerías de audio. Esta decisión arquitectónica proporcionó:
- Portabilidad total (Windows, macOS, Linux)
- Testing simplificado (verificar bytes del archivo MIDI)
- Integración con DAWs profesionales (Ableton, Logic, FL Studio)

**Lección**: Evaluar cuidadosamente el trade-off entre control total (generar audio directamente) y portabilidad (delegar a runtime estándar). Para muchos dominios, la portabilidad es más valiosa.

### 5.3. Reflexión Final

El desarrollo de este compilador reforzó la comprensión de que **los compiladores son programas complejos que transforman representaciones abstractas**. El salto conceptual de "parsear texto" a "generar archivos binarios reproducibles" requiere dominio de:
- Teoría de lenguajes formales (autómatas, gramáticas, jerarquía de Chomsky)
- Estructuras de datos (árboles, tablas hash, listas enlazadas)
- Algoritmos (traversal recursivo, detección de ciclos, hashing)
- Formatos binarios (MIDI, byte order, codificaciones)
- Ingeniería de software (testing, gestión de memoria, logging)

El resultado es un compilador robusto y extensible que cumple su objetivo: **democratizar la creación musical mediante abstracción**.

---

## 6. Referencias

Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2006). *Compilers: Principles, Techniques, and Tools* (2nd ed.). Addison-Wesley. (Referencia fundamental sobre construcción de compiladores, tabla de símbolos, y análisis semántico)

Chomsky, N. (1956). Three models for the description of language. *IRE Transactions on Information Theory*, 2(3), 113-124. (Clasificación de gramáticas formales - jerarquía de Chomsky)

Levine, J. (2009). *flex & bison: Text Processing Tools*. O'Reilly Media. (Guía práctica sobre Flex y Bison, incluyendo modo PUSH)

MIDI Manufacturers Association. (1996). *The Complete MIDI 1.0 Detailed Specification* (Document Version 96.1). Recuperado de https://www.midi.org/specifications (Especificación oficial del protocolo MIDI)

Music Technology Group, McGill University. *Standard MIDI File Format*. Recuperado de https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html (Documentación técnica del formato SMF)

Paxson, V., & Estes, W. (2020). *Flex - The Fast Lexical Analyzer* (Version 2.6.4). Free Software Foundation. (Documentación oficial de Flex, incluyendo características v2.0.0)

Donnelly, C., & Stallman, R. (2020). *Bison - The GNU Parser Generator* (Version 3.8.2). Free Software Foundation. (Documentación oficial de GNU Bison)

Seward, J., & Nethercote, N. (2007). Using Valgrind to detect undefined value errors with bit-precision. *USENIX Annual Technical Conference*. (Herramienta de detección de errores de memoria)

The Clang Team. (2021). *AddressSanitizer*. LLVM Project. Recuperado de https://clang.llvm.org/docs/AddressSanitizer.html (Documentación de AddressSanitizer para detección de memory leaks)

---

## 7. Bibliografía

Appel, A. W. (1998). *Modern Compiler Implementation in C*. Cambridge University Press. (Alternativa a Aho et al., con enfoque en implementación práctica)

Cooper, K. D., & Torczon, L. (2011). *Engineering a Compiler* (2nd ed.). Morgan Kaufmann. (Perspectiva moderna sobre construcción de compiladores)

Wirth, N. (1996). *Compiler Construction*. Addison-Wesley. (Enfoque minimalista y educativo, base del lenguaje Oberon)

Grune, D., Van Reeuwijk, K., Bal, H. E., Jacobs, C. J., & Langendoen, K. (2012). *Modern Compiler Design* (2nd ed.). Springer. (Cobertura amplia de técnicas modernas de compilación)

Fischer, C. N., Cytron, R. K., & LeBlanc, R. J. (2009). *Crafting a Compiler*. Addison-Wesley. (Enfoque práctico con énfasis en implementación)

Mogensen, T. Æ. (2017). *Introduction to Compiler Design* (2nd ed.). Springer. (Introducción accesible con ejemplos en pseudocódigo)

Parr, T. (2009). *Language Implementation Patterns: Create Your Own Domain-Specific and General Programming Languages*. Pragmatic Bookshelf. (Enfoque en DSLs y patrones de implementación)

Hopcroft, J. E., Motwani, R., & Ullman, J. D. (2006). *Introduction to Automata Theory, Languages, and Computation* (3rd ed.). Pearson. (Fundamentación teórica de lenguajes formales)

Sipser, M. (2012). *Introduction to the Theory of Computation* (3rd ed.). Cengage Learning. (Teoría de computación, autómatas, y jerarquía de Chomsky)

Pierce, B. C. (2002). *Types and Programming Languages*. MIT Press. (Fundamentos de sistemas de tipos)

Muchnick, S. S. (1997). *Advanced Compiler Design and Implementation*. Morgan Kaufmann. (Técnicas avanzadas de optimización)

Sewell, P. (2013). *Lecture Notes on Compiler Construction*. University of Cambridge. (Material educativo sobre construcción de compiladores)

Roads, C. (1996). *The Computer Music Tutorial*. MIT Press. (Fundamentos de síntesis digital y formatos de audio)

Loy, G. (2006). *Musimathics: The Mathematical Foundations of Music* (Vols. 1-2). MIT Press. (Matemáticas aplicadas a música y audio digital)

Valimaki, V., & Reiss, J. D. (2016). *All About Audio Equalization: Solutions and Frontiers*. Applied Sciences. (Procesamiento digital de señales de audio)

Docker, Inc. (2024). *Docker Documentation*. Recuperado de https://docs.docker.com/ (Containerización y reproducibilidad de entornos)

---