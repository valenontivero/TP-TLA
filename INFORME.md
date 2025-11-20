# Informe Técnico: Compilador DSL para Máquina de Ritmos

**Autores:**  Pablo Germano, Federico Kloberdanz, Valentin Ontivero, Mauro Vella
**Institución:** ITBA
**Fecha:** Noviembre 2024

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
- 34 casos de prueba (16 aceptación + 18 rechazo) con 100% de éxito

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
tempo 140              /* Velocidad: 140 pulsaciones por minuto */
compasses 16           /* Estructura: 16 compases */
steps 8                /* Resolución: 8 pasos por compás (corcheas) */

/* === SECCIÓN 3: DEFINICIÓN DE PATRONES === */
pattern kickPattern {
    /* Patrón de bombo: golpe en tiempos 1 y 5 (síncopa) */
    rhythm [x,.,.,.,x,.,.,.]
}

pattern snarePattern {
    /* Patrón de caja: golpe en tiempos 3 y 7 (backbeat) */
    rhythm [.,.,x,.,.,.,x,.]
}

pattern hihatPattern {
    /* Patrón de hi-hat: corcheas constantes */
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

##### Orden de patrones y modo PUSH

El orden de las reglas en `FlexPatterns.l` es **crítico** para la correcta tokenización: primero se ubican comentarios, luego palabras clave, luego los tokens más específicos (`x`, `.`, notas) y al final el patrón de identificador general.  
El lexer opera en modo **PUSH**: cada regla crea un `Token` con su valor semántico (entero o string) y lo envía explícitamente al parser, lo que permite desacoplar el ciclo Flex/Bison y manejar mejor errores y logs.

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

El AST implementa una arquitectura de **nodos de datos + wrappers de lista**:

- `Program` agrupa `imports`, `declarations`, `patterns` e `instruments`.
- Los nodos de datos (`Pattern`, `Instrument`, `RhythmElement`, `ActiveRange`) no contienen `next`.
- Las listas se representan con wrappers (`PatternList`, `InstrumentList`, `RhythmElementList`) que sí tienen `next`, lo que simplifica el recorrido y la destrucción recursiva.

##### Acciones Semánticas

Cada producción gramatical tiene una acción semántica asociada que construye el nodo AST correspondiente:

```c
/* Ejemplo: Construcción de patrón
 * Producción: pattern_def: PATTERN ID OPEN_BRACE RHYTHM rhythm_expr CLOSE_BRACE
 */
Pattern * PatternSemanticAction(char * name, RhythmExpression * rhythm) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Pattern *pattern = calloc(1, sizeof(Pattern));
    if (pattern == NULL) {
        logError(_logger, "Memory allocation failed for Pattern");
        return NULL;
    }

    pattern->name = name;      /* El string ya viene alocado desde el lexer */
    pattern->rhythm = rhythm;
    return pattern;
}

/* Ejemplo: Construcción de lista de elementos rítmicos
 * Producción:
 *   rhythm_element_list : rhythm_element
 *                        | rhythm_element_list COMMA rhythm_element
 */
RhythmElementList * RhythmElementListSemanticAction(RhythmElement * element,
                                                    RhythmElementList * next) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    RhythmElementList *newNode = calloc(1, sizeof(RhythmElementList));
    if (newNode == NULL) {
        logError(_logger, "Memory allocation failed for RhythmElementList");
        return next;
    }
    newNode->element = element;
    newNode->next = NULL;

    /* Caso base: primer elemento de la lista */
    if (next == NULL) {
        return newNode;
    }

    /* CRÍTICO: appendar al FINAL para preservar el orden de la melodía */
    RhythmElementList *current = next;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
    return next;
}

/* Ejemplo: Operador de repetición
 * Producción: rhythm_expr: rhythm_array MUL INTEGER
 */
RhythmExpression * RhythmRepetitionSemanticAction(RhythmArray * array, int repetitions) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    RhythmExpression *expr = calloc(1, sizeof(RhythmExpression));
    if (expr == NULL) {
        logError(_logger, "Memory allocation failed for RhythmExpression");
        return NULL;
    }

    expr->type = RHYTHM_REPETITION;
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

**Implementación**: hash table dinámica usando el algoritmo de hash **djb2**.  
Las estructuras reales son:

```c
typedef enum {
    SYMBOL_PATTERN,
    SYMBOL_INSTRUMENT
} SymbolType;

typedef struct Symbol {
    char * name;
    SymbolType type;
    union {
        Pattern   * pattern;     /* Para SYMBOL_PATTERN    */
        Instrument * instrument; /* Para SYMBOL_INSTRUMENT */
    } data;
    struct Symbol * next;        /* Chaining por colisiones */
} Symbol;

typedef struct SymbolTable {
    Symbol ** buckets;  /* Array dinámico de buckets */
    int capacity;       /* Cantidad de buckets       */
    int size;           /* Símbolos almacenados      */
} SymbolTable;

/* Hash function: djb2 (mismo algoritmo que en la cátedra) */
static unsigned int _hash(const char * str, int capacity) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  /* hash * 33 + c */
    }
    return hash % capacity;
}
```

**Operaciones implementadas** (siguiendo `Tabla de Simbolos`):

1. **Create** (Inserción):
```c
SymbolTable * createSymbolTable(int capacity) {
    SymbolTable * table = calloc(1, sizeof(SymbolTable));
    if (table == NULL) return NULL;

    table->capacity = capacity;
    table->size     = 0;
    table->buckets  = calloc(capacity, sizeof(Symbol *));
    if (table->buckets == NULL) {
        free(table);
        return NULL;
    }
    return table;
}

static bool _insertSymbol(SymbolTable * table,
                          const char * name,
                          SymbolType type,
                          void * data) {
    if (symbolExists(table, name)) {
        logError(_logger, "Symbol '%s' already declared", name);
        return false;
    }

    Symbol * symbol = calloc(1, sizeof(Symbol));
    if (symbol == NULL) {
        logError(_logger, "Failed to allocate symbol '%s'", name);
        return false;
    }

    symbol->name = strdup(name);
    symbol->type = type;
    if (type == SYMBOL_PATTERN) {
        symbol->data.pattern = (Pattern *) data;
    } else {
        symbol->data.instrument = (Instrument *) data;
    }

    unsigned int index = _hash(name, table->capacity);
    symbol->next = table->buckets[index];
    table->buckets[index] = symbol;
    table->size++;
    return true;
}

bool insertPattern(SymbolTable * table, Pattern * pattern) {
    if (pattern == NULL || pattern->name == NULL) return false;
    return _insertSymbol(table, pattern->name, SYMBOL_PATTERN, pattern);
}

bool insertInstrument(SymbolTable * table, Instrument * instrument) {
    if (instrument == NULL || instrument->name == NULL) return false;
    return _insertSymbol(table, instrument->name, SYMBOL_INSTRUMENT, instrument);
}
```

2. **Read** (Búsqueda):
```c
Symbol * lookupSymbol(SymbolTable * table, const char * name) {
    if (table == NULL || name == NULL) return NULL;

    unsigned int index = _hash(name, table->capacity);
    Symbol * symbol = table->buckets[index];

    while (symbol != NULL) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
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

Implementa las funciones de verificación descritas en `Sistema-de-Tipos.md`, pero en el código real se modelan como validadores booleanos (`true` = válido, `false` = error) en lugar de un enum de tipos explícito.

**Validaciones implementadas (en resumen):**

- **Formato de notas**: `validateNoteFormat` comprueba que la nota siga `[A-G][#b]?[0-9…]`, con letra A–G, alteración opcional y al menos un dígito de octava.  
- **Elementos y expresiones rítmicas**: `validateRhythmElement/Array/Expression` aseguran que solo se usen `HIT`, `SILENCE` o `NOTE` válidas, que los arrays no estén vacíos y que las repeticiones tengan factor positivo.  
- **Rangos activos**: `validateActiveRange` verifica que `start` y `end` sean positivos y `start ≤ end`, mientras que `validateActiveRangeWithinBounds` exige además `end ≤ compasses`.  
- **Declaraciones**: `validateDeclarations` exige que `tempo`, `compasses` y `steps` sean estrictamente positivos.

##### Analizador Semántico

**Archivo**: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.h/c`

Orquesta la validación completa en 4 fases claramente separadas (ver `SemanticAnalyzer.c`):

1. **Validar declaraciones** (si existen):  
   Se llama a `validateDeclarations(program->declarations)` y, si falla, se aborta el análisis.
2. **Construir la tabla de símbolos**:  
   `_buildSymbolTable(program)` crea una `SymbolTable` con capacidad 64 e inserta todos los patrones e instrumentos mediante `insertPattern` / `insertInstrument`. Detecta nombres duplicados.
3. **Validar patrones**:  
   `_validatePatterns(program)` recorre la `PatternList` y utiliza `validateRhythmExpression` para asegurar que todas las expresiones rítmicas sean válidas.
4. **Validar instrumentos**:  
   `_validateInstruments(program)` comprueba que:
   - cada instrumento referencia un patrón existente (`lookupPattern` en la tabla de símbolos global `_symbolTable`);
   - todos los rangos activos (`ActiveRange *activeRange` y su lista encadenada) sean válidos y estén dentro de `compasses` (`validateActiveRange` + `validateActiveRangeWithinBounds`).

El punto de entrada público queda:

```c
CompilationStatus analyzeProgram(Program * program) {
    if (program == NULL) return FAILED;

    /* Paso 1: declaraciones */
    if (program->declarations != NULL) {
        if (!validateDeclarations(program->declarations)) {
            return FAILED;
        }
    }

    /* Paso 2: tabla de símbolos (patrones + instrumentos) */
    if (_buildSymbolTable(program) != SUCCEEDED) {
            return FAILED;
    }

    /* Paso 3: validar patrones */
    if (_validatePatterns(program) != SUCCEEDED) {
            return FAILED;
    }

    /* Paso 4: validar instrumentos */
    if (_validateInstruments(program) != SUCCEEDED) {
            return FAILED;
    }

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
/* Punto de entrada del backend (Generator.c) */
void executeGenerator(CompilerState * compilerState) {
    logDebugging(_logger, "Generating final output...");

    /* 1. Imprimir resumen textual del programa */
    _generateProgram(compilerState->abstractSyntaxtTree);

    /* 2. Generar archivo MIDI solo si el programa está completo */
    Program * program = compilerState->abstractSyntaxtTree;
    if (program != NULL && program->declarations != NULL && program->instruments != NULL) {
        CompilationStatus status = generateMidiFile(program, "output.mid");
        if (status == SUCCEEDED) {
            printf("\n✓ MIDI file generated: output.mid\n");
        } else {
            printf("\n✗ MIDI generation failed\n");
        }
    } else {
        logDebugging(_logger, "Skipping MIDI generation (pattern library or incomplete program)");
    }
}

/* Generación del archivo MIDI (MidiGenerator.c, simplificado) */
CompilationStatus generateMidiFile(Program * program, const char * outputFilename) {
    /* Abrir archivo, contar instrumentos y escribir header MThd */
    /* ... */

    /* Track 0: metadata (tempo + time signature) */
    _generateMetadataTrack(program);

    /* Tracks de instrumentos: uno por Instrument en InstrumentList */
    InstrumentList * instList = program->instruments;
    SymbolTable * symbolTable = getSymbolTable(); /* construida en el análisis semántico */

    while (instList != NULL && instList->instrument != NULL) {
        Instrument * instrument = instList->instrument;
        Pattern * pattern = lookupPattern(symbolTable, instrument->patternName);
        if (pattern == NULL) {
            /* error: patrón no encontrado */
            return FAILED;
        }

        _generateInstrumentTrack(program, instrument, pattern);
        instList = instList->next;
    }

    /* Cerrar archivo y devolver estado */
    return SUCCEEDED;
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

**Implementación del header y eventos** (resumen):

- El header `MThd` se escribe en **big-endian**, con tamaño fijo 6 bytes, formato 1 y `numTracks = 1 + número de instrumentos`.  
- Los **delta times** se codifican como *Variable-Length Quantity* (VLQ), empaquetando grupos de 7 bits con un bit de continuación; esta lógica está centralizada en una función auxiliar (ver `MidiGenerator.c`).  
- Los eventos Note On/Off se generan escribiendo: delta time (VLQ), status byte (`0x9?` / `0x8?` + canal) y dos data bytes (nota, velocity).

##### Mapeo de Elementos Rítmicos a MIDI

- `HIT` se mapea a canal 10 (índice 9) y nota 36 (Bass Drum 1, General MIDI).  
- `SILENCE` no genera eventos.  
- `NOTE` se mapea a canal melódico (canal 1) y número de nota MIDI calculado desde notación anglosajona (`[A-G][#b]?[0-9...]`).

##### Cálculo de Timing

El timing preciso es **crítico** para la reproducción correcta.  
El compilador fija `TPQN = 480` (ticks por negra) y toma cada `step` como **una negra completa**:

- Ticks por step: `ticksPerStep = TPQN`.  
- Ticks por compás: `ticksPerCompass = TPQN * steps`.  
- Tick absoluto de un evento: `((compás - 1) * ticksPerCompass) + (índiceStep * ticksPerStep)`.

Los detalles finos de implementación (incluyendo los bugs de timing y distribución de elementos) se documentan en la sección **3.4. Dificultades Encontradas**.

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

En términos generales, el algoritmo:

1. Mantiene una **pila de archivos visitados** (`_visitedFiles`, `_visitedCount`) para detectar ciclos y respetar una profundidad máxima de imports.  
2. Para cada `remember "file.dsl"`:
   - Verifica existencia del archivo.
   - Crea un **lexer/parser independientes** y parsea el archivo completo a un `Program * importedProgram`.
3. Resuelve recursivamente los imports anidados de `importedProgram`.  
4. Hace *merge* de la `PatternList` importada dentro del programa principal (`Program * mainProgram`), transfiriendo el ownership de los patrones.  
5. Destruye el AST importado y hace *backtracking* en la pila de visitados para permitir patrones en diamante sin falsos positivos de ciclo.

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

**Síntoma**: Las melodías se reproducían al revés (`[E2,G2,A2]` sonaba como `A2,G2,E2`).

**Causa raíz**: La acción semántica que construía `RhythmElementList` hacía *prepending* en lugar de *appending*, invirtiendo el orden natural de los elementos al reducir `rhythm_element_list: rhythm_element_list COMMA rhythm_element`.

**Solución**: Cambiar la acción para recorrer la lista hasta el final y appendar el nuevo nodo, preservando el orden de aparición en el código fuente.

#### Bug #2: Distribución de Elementos en Compases

**Síntoma**: Todos los elementos del patrón se reproducían en cada compás, generando un efecto de “muro de sonido” y aceleración artificial.

**Causa raíz**: El bucle de generación MIDI recorría la lista expandida completa para cada compás, en lugar de tomar solo el “slice” correspondiente a ese compás.

**Solución**: Calcular cuántos elementos caben por compás (`steps`), dividir el patrón expandido en chunks y, para cada compás, reproducir únicamente el chunk adecuado, ciclando con módulo cuando el instrumento está activo más compases que los que ocupa el patrón.

#### Bug #3: Timing MIDI (Reproducción 4x Más Rápida)

**Síntoma**: Los archivos MIDI se reproducían 4 veces más rápido de lo esperado. Un programa con `tempo 120` sonaba como tempo 480.

**Causa raíz**: Se interpretó erróneamente que cada `step` era una fracción de negra, y se calculó `ticksPerStep = TPQN / steps`. En el diseño final del lenguaje, cada `step` **es** una negra completa, por lo que debía usarse `ticksPerStep = TPQN`.

**Solución**: Fijar `ticksPerStep = TPQN` y derivar a partir de allí los ticks por compás y los tiempos absolutos. El bug mostró lo delicado que es interpretar correctamente la especificación MIDI.

#### Bug #4: Declaraciones Obligatorias en Archivos de Biblioteca

**Síntoma**: Los tests CI/CD fallaban al intentar parsear archivos de biblioteca (pattern-only files) que no contenían declaraciones (`tempo`, `compasses`, `steps`).

**Causa raíz**: El analizador semántico asumía que `program->declarations` nunca era `NULL`, ignorando que la gramática permite archivos con solo patrones (bibliotecas).

**Solución**: Hacer que `analyzeProgram` trate las declaraciones como opcionales (validarlas solo si existen) y que tanto la validación de rangos activos como la generación de MIDI se salten cuando el archivo es solo una librería de patrones. Esto respeta el modelo de *scope global* para patrones.

#### Bug #5: Fugas de Memoria Durante Importación

**Síntoma**: AddressSanitizer reportaba pequeñas fugas de memoria al ejecutar tests con imports encadenados.

**Causa raíz**: Cada import inicializaba nuevamente los módulos de Flex/Bison, creando loggers adicionales que nunca se destruían.

**Solución**: Aplicar un patrón de inicialización única: `initializeFlexActionsModule` e `initializeBisonActionsModule` ahora solo crean el logger si `_logger == NULL`, compartiéndolo entre todas las instancias de lexer/parser de imports. Esto eliminó las fugas sin complicar el diseño.

---

## 4. Futuras Extensiones

El compilador actual implementa un MVP funcional completo. A futuro, se identificaron posibles líneas de trabajo, agrupadas en cuatro ejes:

- **Extensiones del lenguaje**:
  - Velocity variable por evento (`x@127`, `x@piano`).
  - Selección explícita de instrumentos y canales MIDI por instrumento.
  - Duración de notas configurable, más allá de 1 step fijo.
  - Azúcar sintáctico (constantes, operadores adicionales como interleaving).

- **Extensiones de análisis**:
  - Advertencias de optimización (repeticiones innecesarias, patrones silenciosos, etc.).
  - Detección de polirritmias y patrones que no calzan exactamente en la grilla de compases.

- **Extensiones del backend**:
  - Exportar a otros formatos (JSON, MusicXML) reutilizando el AST expandido.
  - Opcionalmente, generación de audio (WAV) a través de un runtime externo.

- **Herramientas y performance**:
  - Visualizador de patrones (ASCII/HTML) para debugging rápido.
  - REPL interactivo orientado a experimentación.
  - Compilación incremental y posibles optimizaciones de rendimiento (paralelización, MIDI más compacto).

### Resumen de Extensiones

| Motivación | Extensión | Justificación |
|-----------|-----------|---------------|
| 🔴 Alta | Velocity variable | Esencial para expresividad musical |
| 🔴 Alta | Selección de instrumentos MIDI | Amplía dramáticamente posibilidades sonoras |
| 🟡 Media | Duración de notas | Mejora expresividad, pero workaround posible |
| 🟡 Media | Visualizador de patrones | Muy útil para debugging |
| 🟡 Media | Exportación JSON | Abre integración con otras herramientas |
| 🟢 Baja | REPL | Nice-to-have, no crítico |
| 🟢 Baja | Optimización MIDI | Mejora marginal |
| ⚪ Futuro | Generación WAV | Viola arquitectura actual (runtime.md) |

Estas ideas quedan como hoja de ruta y no forman parte del alcance de la versión descrita en este informe.

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

- **Orden en el lexer**: El orden de las reglas en Flex es determinante; los patrones específicos (comentarios, keywords, `x`, `.`, notas) deben ir antes que el identificador genérico para evitar clasificaciones erróneas.
- **Memoria en C**: Aun con destructores cuidadosos, es fácil introducir fugas o *use-after-free*; usar sanitizers desde el inicio simplifica enormemente el debugging.
- **Testing continuo**: Los bugs más sutiles (melodías invertidas, timing incorrecto, leaks en imports) aparecieron solo gracias a una combinación de tests de aceptación/rechazo, escuchar el MIDI y ejecutar AddressSanitizer.
- **Especificaciones binarias**: Implementar MIDI obligó a respetar al detalle TPQN, VLQ y estructura de chunks; no basta con aproximaciones o tutoriales incompletos.
- **Separación compiler/runtime**: Generar solo MIDI y delegar la síntesis al entorno del usuario dio como resultado un compilador más portable, fácil de probar y sencillo de integrar con distintas herramientas.

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

Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2006). *Compilers: Principles, Techniques, and Tools* (2nd ed.). Addison-Wesley.

Levine, J. (2009). *flex & bison: Text Processing Tools*. O'Reilly Media.

Paxson, V., & Estes, W. (2020). *Flex - The Fast Lexical Analyzer* (Version 2.6.4). Free Software Foundation.

Donnelly, C., & Stallman, R. (2020). *Bison - The GNU Parser Generator* (Version 3.8.2). Free Software Foundation.

MIDI Manufacturers Association. (1996). *The Complete MIDI 1.0 Detailed Specification* (Document Version 96.1). MIDI Manufacturers Association.

Music Technology Group, McGill University. *Standard MIDI File Format*. McGill University.

---

## 7. Bibliografía

Hopcroft, J. E., Motwani, R., & Ullman, J. D. (2006). *Introduction to Automata Theory, Languages, and Computation* (3rd ed.). Pearson.

Sipser, M. (2012). *Introduction to the Theory of Computation* (3rd ed.). Cengage Learning.

Cooper, K. D., & Torczon, L. (2011). *Engineering a Compiler* (2nd ed.). Morgan Kaufmann.

---