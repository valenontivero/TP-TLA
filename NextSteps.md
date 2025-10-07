# Pasos ya Hechos
El diseño del lenguaje ya está avanzado.
Se diseñó el esquema de un lenguaje para la creación de ritmos tipo "drum machine".
El lenguaje permite definir instrumentos de percusión (bombo, snare, hi-hat, platillo, etc.),
instrumentos melódicos (bajo, teclado, etc.), especificar el patrón rítmico de cada uno, el
volúmen de cada uno(decibeles) y especificar el tempo general del programa(BPM). También
soportará bucles (loops), variaciones, silencios, y la posibilidad de concatenar o superponer
patrones para construir ritmos más complejos.

## Contstrucciones

El lenguaje desarrollado debería ofrecer las siguientes construcciones, prestaciones y
funcionalidades:
1. Definir tempo, indicando los Beats Por Minuto (BPM) (e.g. tempo 120 )
2. Soportar la exportación de los patrones finalizados en audio.
3. Declarar instrumentos de percusión (e.g. kick, snare, hihat)
4. Declarar instrumentos melódicos (e.g. teclado, bajo)
5. Crear patrones rítmicos con pasos de compás( pattern{...}).
6. Soportar silencios (-), simples o prolongados
7. Soportar intensidad de golpe (con operadores relacionales)
8. Concatenar patrones (+)
9. Iterar patrones ( *n repeticiones)
10. Aplicar transformaciones rítmicas (e.g. doble tempo, medio tempo)
11. Definir variaciones dentro de patrón (e.g. alternar un golpe en cada repetición con
    función
12. Reutilizar patrones con variables ( pattern groove = {...} )
13. Guardar y cargar patrones desde archivos para reutilizarlos en distintos programas.
14. Crear patrones rítmicos con pasos de compás (pattern con subdivisiones en corcheas,
    semicorcheas, etc.)

## Casos de Prueba
Se proponen los siguientes casos iniciales de prueba de aceptación:
1. Un programa que crea un ritmo con tempo 120 bpm y en 4/4.
2. Un programa que agrega ritmos de percusión: bombo y de snare.
3. Un programa que agrega ritmos melódicos: bajo y teclado.
4. Un programa que agrega ritmos mixtos: bajo y kick.
5. Un programa que carga un sample de kick para utilizar en su ritmo.
6. Un programa que pueda definir patrones particulares para reutilizar.
7. Un programa que soporte una progresión con instrumentos y silencios.
8. Un programa que combina dos patrones guardados en un solo ritmo.
9. Un programa que permita el agregado (+) de diferentes ritmos para el mismo
instrumento.
10. Un programa que permita la iteración (*) de diferentes ritmos para el mismo
instrumento.
11. Un programa que tenga patrones con diferentes figuras musicales (e.g. corcheas y
negras).
12. Un programa que exporte como .WAV el ritmo creado.
Además, los siguientes casos de prueba de rechazo:
1. Un programa que genere acordes o una composición compleja de notas e.g. ligaduras de
guitarra.
2. Un programa que genere un ritmo sin instrumentos asignados.
3. Un programa que concatene patrones de distinta métrica.
4. Un programa que defina un compás ó tempo inválido.
5. Un programa que defina el mismo instrumento en múltiples patrones.
6. Un programa que contenga un patrón fuera de la métrica 4/4.


## Ejemplo de Sintaxis
La sintaxis del lenguaje será de la siguiente forma:
```
tempo 120
compasses 16
steps 4

pattern kickPattern {
    rhythm [x,.,x,.] * 4
}

pattern bassPattern {
    rhythm  [E2,G2,A2, .] + [E2,G2,A#2,A2] + [E2,G2,A2, .] + [G2, E2, -, -]
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
En este caso tenemos un programa que corre un ritmo a 120 bpm, con 2 instrumentos cada uno con su patrón.
En los patrones de percusion (como kick) tendremos golpes (x) y silencios(-), mientras que en los melódicos (como bass) tendremos notas (cifrado americano).
Steps es la subdividision dentro de cada compás.
compasses será la cantidad de compases que se loopearán (la duración de la cancion)
En este caso kickPattern contiene un patron '[x,-,x,-]' que dura un compás (todo lo que esta entre corchetes dura un compás y debe tener la misma cantidad de golpes o notas que 'steps'). 
Y bassPattern tiene una melodía que consta de notas divididas en 4 compases, se muestra que están concatenadas con el simbolo '+'. Tambien podría repetirse una parte con el símbolo *.
Active muestra en qué compases sonara cada instrumento.

# Siguientes Pasos
Debo comenzar a desarrollar el frontend de la solución en este repositorio (src/main/c/frontend).
Para esto se debe comenzar por establecer la definición de una gramática G = ⟨Σ, Ν, Π, S⟩, donde Σ es el alfabeto, Ν es el conjunto de símbolos no-terminales, Π es el conjunto de producciones libres de contexto (tipo 2, según la Jerarquía de Chomsky), y S es el símbolo no-terminal inicial de la gramática.
1. Análisis Léxico: Transformar un programa de entrada en el lenguaje desarrollado en un stream de tokens como muestro a continuación:
2. Análisis Sintáctico: [Todavía no especificado. POR DESARROLLAR en el futuro]
3. Permitir la correcta ejecución del script de testing disponible en el repositorio de base, aceptando o rechazando cada uno de los casos de prueba según corresponda.

Nótese que el analizador léxico será el que defina el alfabeto Σ, mientras que el analizador sintáctico es quien define Ν, Π y S, es decir, ambas fases conforman la gramática G que genera el lenguaje a implementar.

## Análisis Léxico: Transformación

### Introducción
Sea G = ⟨Σ, Ν, Π, S⟩ una gramática, donde Σ es el alfabeto, Ν es el conjunto de símbolos no-terminales, Π es el conjunto de producciones y S es el símbolo no-terminal inicial, se define un analizador léxico como aquel cuyo objetivo principal es transformar una secuencia de símbolos en una nueva secuencia compuesta exclusivamente por símbolos del alfabeto Σ.
Para las siguientes secciones, el analizador léxico utilizado será Flex, pero las ideas y conceptos son generales a otros analizadores.

### Alfabeto
Luego de diseñar la sintaxis de un nuevo lenguaje formal (o de disponer de la especificación de uno existente), debe ser posible tomar un programa de entrada como el siguiente:

[HttpGet]
[ProducesResponseType(200)]
public async Task<ActionResult<Manifest>> GetManifestAsync() {
return new Manifest();
}


y construir una secuencia equivalente de símbolos pero que se restrinja a aquellos símbolos de un alfabeto Σ definido para la gramática del lenguaje en cuestión (en el ejemplo, C#).
Los motivos por los que se debe realizar un mapeo entre esta secuencia de símbolos (idealmente codificados en UTF-8 según el estándar Unicode), y los símbolos de Σ, son al menos dos:

Finitud: Naturalmente un alfabeto es finito por definición, sin embargo, tomando casi cualquier lenguaje como ejemplo, es claro que uno podría definir un símbolo atómico por cada identificador de variable o función, o por cada número entero, con lo cual el alfabeto debería ser infinito. Para eliminar esta situación, un conjunto infinito de símbolos en el programa de entrada se mapean hacia un único símbolo atómico en Σ.

Rendimiento: En general, un analizador léxico es menos potente que un analizador sintáctico (la siguiente fase del compilador), en términos de su capacidad de computabilidad (i.e., lo que puede o no puede hacer, independientemente de su capacidad de procesamiento y almacenamiento). Sin embargo, para mejorar la eficiencia se emplea un autómata que permite realizar el mapeo de símbolos (y el agrupamiento de los mismos), utilizando recursos más limitados, incrementando el ancho de banda y reduciendo la carga en el analizador sintáctico debido a que la cantidad de símbolos es inferior (debido al agrupamiento de los mismos).

Por ejemplo, es factible realizar los siguientes mapeos desde secuencias de símbolos codificadas en UTF-8 hacia un único símbolo de Σ:

STRING:
"Al principio Dios creó el cielo y la tierra."
"La tierra era algo informe y vacío,"
"las tinieblas cubrían el abismo,"
"y el soplo de Dios se cernía sobre las aguas."

INTEGER:
0
123
10000

ID:
ActionResult
x
y
User
Credential

Evidentemente, las secuencias de símbolos a la izquierda representan conjuntos infinitos (fragmentos de texto, números enteros e identificadores de variables o funciones, respectivamente), pero son fácilmente identificables y representados por símbolos atómicos finitos, suponiendo que Σ = { ID, INTEGER, STRING }, en este caso.
Adicionalmente, está claro que la transformación de las secuencias de entrada incrementa el ancho de banda hacia la segunda fase, por ejemplo, los 4 extractos del libro del Génesis utilizan 156 símbolos (160 bytes codificados en UTF-8), pero luego del mapeo solo 4 (uno por cada texto). Si este símbolo STRING de Σ se codifica mediante un entero de 2 bytes, entonces se utilizarían 8 bytes en lugar de 160 (20 veces mejor).
Entonces, ¿qué secuencias forman parte del alfabeto? Todos aquellos símbolos o agrupamientos de símbolos que tengan valor o semántica (significado) por sí mismos y que carezcan del mismo si se aplica una subdivisión, indicando que efectivamente son atómicos.
Ejemplos claros de símbolos del alfabeto son: palabras reservadas, conjuntos literales de ciertos tipos de datos (enteros, reales, cadenas de texto, caracteres, identificadores de variables o funciones, etc.), símbolos de operadores aritméticos, relacionales, lógicos o matemáticos en general, y signos de puntuación o agrupamiento (paréntesis, llaves, comillas, puntos y comas, dos puntos, etc.).

### Expresiones Regulares

⚠️IMPORTANTE: Las acciones de Flex utilizadas en esta sección se corresponden al modo de operación PULL. La diferencia está en que las mismas devuelven la etiqueta del token (token label), mientras que en el nuevo modo PUSH, deben devolver el estado de la compilación (la etiqueta viaja por otro medio). Este nuevo mecanismo se introdujo en la versión v2.0.0, es decir, las versiones anteriores utilizan el modo PULL.


El mapeo de un símbolo o grupo de símbolos en la entrada del analizador léxico hacia un símbolo del alfabeto Σ, se realiza en general mediante el uso de autómatas finitos determinísticos (DFA), debido a su simplicidad pero a la vez eficiencia y composicionalidad (i.e., la capacidad de unir múltiples autómatas para construir uno más complejo).
Debido a que un DFA es equivalente a una expresión regular, los analizadores léxicos usualmente se construyen automáticamente desde una lista de expresiones regulares que el programador asocia con cada símbolo del alfabeto final. De esta manera, las expresiones regulares permiten definir tanto conjuntos finitos o infinitos de posibles secuencias de caracteres que serán mapeadas eficientemente a sus correspondientes símbolos de Σ.

En particular, Flex es un generador de analizadores léxicos que opera de esta manera: toma una especificación de expresiones regulares (desde un documento *.l), junto con sus mapeos hacia Σ, y emite un analizador léxico eficiente (también llamado escáner), basado en un DFA.
Utilizando como base el proyecto base Flex-Bison-Compiler, se pueden identificar las siguientes reglas de mapeo:

"-"                                 { return SUB; }
"*"                                 { return MUL; }
"/"                                 { return DIV; }
"+"                                 { return ADD; }

"("                                 { return OPEN_PARENTHESIS; }
")"                                 { return CLOSE_PARENTHESIS; }

[[:digit:]]+                        { return INTEGER; }

[[:space:]]+                        { ; }
.                                   { return UNKNOWN; }


Nótese que al igual que en los ejemplos anteriores, del lado izquierdo se encuentra la expresión regular que acepta las secuencias válidas de entrada, mientras que el lado derecho de esta especificación efectivamente emite un símbolo del alfabeto Σ = { SUB, MUL, DIV, ADD, (…) }.
Cabe destacar algunas situaciones particulares. Por ejemplo, en el caso de la secuencia definida por la expresión regular [[:space:]]+, no se emite ningún mapeo, ya que esta secuencia representa grupos de espacios en blanco (whitespace), lo que produce un DFA que ignora dichas secuencias, y por lo tanto la siguiente fase de análisis sintáctico evitará sus efectos en la gramática de forma transparente.
Por otro lado, la expresión regular final emite un símbolo UNKNOWN, utilizado para representar un concepto que no es propio de Σ, ya que representa una secuencia de símbolos desconocida, que no es posible mapear ni ignorar y que por lo tanto representa un error de sintaxis.

#### Algoritma de Flex
¿En qué orden se evalúan los mapeos?
Flex utiliza el algoritmo Longest Prefix Match, lo que implica que siempre se intenta consumir la mayor cantidad de símbolos antes de realizar el mapeo y emitir un símbolo del alfabeto Σ. Por ejemplo, para los siguientes mapeos:

"class"                             { return CLASS; }

[[:alpha:]][[:alnum:]]+             { return ID; }


y si la secuencia de entrada es "class123", se aplicará el segundo mapeo emitiendo un símbolo ID, ya que dicha regla permite consumir más símbolos que la primera, con lo cual, "class123" se interpretará correctamente como un identificador de variable en lugar de la keyword class.
Por otro lado, si la secuencia fuese "class 123", Flex retorna CLASS, ya que el espacio entre "class" y "123" impide que se ejecute la segunda regla.
En el caso de que múltiples expresiones regulares coincidan con la secuencia de entrada, por ejemplo sobre la cadena "unaVariable":

[[:alpha:]]*                        { return TEXT; }

[[:alpha:]][[:alnum:]]+             { return ID; }


tiene mayor precedencia quién aparece primero, en este caso, la regla de mapeo que emite el símbolo TEXT del alfabeto (nótese que en este caso, ambos mapeos coinciden con la cadena de entrada).

#### Token
El objetivo de un analizador léxico es transformar la secuencia de entrada en un stream o flujo de unidades atómicas denominadas tokens. Un token es la unión de un atributo y un lexema, donde lexema es literalmente la porción de símbolos de la cadena de entrada que se está mapeando hacia un símbolo del alfabeto, mientras que los atributos simplemente son metadatos o valores semánticos que se asocian al lexema para transportar información adicional hacia la siguiente etapa (el analizador sintáctico).
Por ejemplo, retomando el siguiente fragmento de un programa en C#:

[HttpGet]
[ProducesResponseType(200)]
public async Task<ActionResult<Manifest>> GetManifestAsync() {
return new Manifest();
}


se puede descomponer en un flujo de lexemas (ignorando espacios):

"[", "HttpGet", "]", "[", "ProducesResponseType", "(", "200", ")", "]", "public", "async", "Task", "<", "ActionResult", "<", "Manifest", ">", ">", "GetManifestAsync", "(", ")", "{", "return", "new", "Manifest", "(", ")", ";", "}"


para luego mapearse hacia símbolos del alfabeto:

OPEN_BRACKET, ID, CLOSE_BRACKET, OPEN_BRACKET, ID, OPEN_PARENTHESIS, INTEGER, CLOSE_PARENTHESIS, CLOSE_BRACKET, PUBLIC, ASYNC, ID, LESS_THAN, ID, LESS_THAN, ID, GREATER_THAN, GREATER_THAN, ID, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, OPEN_BRACE, RETURN, NEW, ID, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, SEMICOLON, CLOSE_BRACE

hora bien, cada uno de estos símbolos puede o no acarrear metadatos (un atributo adicional), por ejemplo, podría ser relevante que el símbolo INTEGER contenga efectivamente el valor numérico 200 en su interior, y que los símbolos ID transporten el lexema original que los produjo, a modo de identificar esas variables, constantes o nombres de clases y funciones específicamente.
En este sentido, y representando un token como una tupla de tipo (Σ, V), donde Σ es el alfabeto y V es el conjunto de valores semánticos que un símbolo de Σ puede tomar, se obtiene el flujo final de tokens:

(OPEN_BRACKET, ∅), (ID, "HttpGet"), (CLOSE_BRACKET, ∅), (OPEN_BRACKET, ∅), (ID, "ProducesResponseType"), (OPEN_PARENTHESIS, ∅), (INTEGER, 200), (CLOSE_PARENTHESIS, ∅), (CLOSE_BRACKET, ∅), (PUBLIC, ∅), (ASYNC, ∅), (ID, "Task"), (LESS_THAN, ∅), (ID, "ActionResult"), (LESS_THAN, ∅), (ID, "Manifest"), (GREATER_THAN, ∅), (GREATER_THAN, ∅), (ID, "GetManifestAsync"), (OPEN_PARENTHESIS, ∅), (CLOSE_PARENTHESIS, ∅), (OPEN_BRACE, ∅), (RETURN, ∅), (NEW, ∅), (ID, "Manifest"), (OPEN_PARENTHESIS, ∅), (CLOSE_PARENTHESIS, ∅), (SEMICOLON, ∅), (CLOSE_BRACE, ∅)


Nótese que para los valores semánticos vacíos (donde no tiene sentido transportar un atributo o metadato más allá del símbolo del alfabeto), se utilizó ∅.
Transformada la secuencia de entrada en este flujo de tokens, el analizador sintáctico se puede encargar libremente de construir el árbol de sintaxis abstracta (AST).

### Contexto
Si bien es una prestación que no pertenece a las máquinas de tipo DFA, Flex permite manipular el analizador léxico generado de una manera que le provee mayor potencia para separar la lista de mapeos en diferentes grupos o contextos.
Si una expresión regular permite mapear un lexema hacia un símbolo del alfabeto, entonces es factible pensar que bajo ciertas circunstancias, un mismo lexema se puede reinterpretar como diferentes símbolos.
Por ejemplo, una palabra compuesta de símbolos del alfabeto español podría considerarse el nombre de un identificador, y por lo tanto mapearse hacia un símbolo ID. Sin embargo, si la palabra se encuentra dentro de un comentario en el programa de entrada, debe ignorarse, o en casos más complejos (como aquellos dónde se infieren datos desde documentación estructurada como Javadoc, JSDoc o Sphinx), debe interpretarse de otro modo.
Para manipular un lexema de múltiples maneras, se debe indicar el momento en el cual ocurre un cambio de contexto:

"/*"                                { BEGIN(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"             { BEGIN(INITIAL); }
<MULTILINE_COMMENT>[[:space:]]+     { ; }
<MULTILINE_COMMENT>[^*]+            { ; }
<MULTILINE_COMMENT>.                { ; }

De esta manera, al identificar el lexema “/*” en la secuencia de entrada se ingresará al contexto denominado MULTILINE_COMMENT. Dentro de este contexto, el grupo de expresiones regulares activas queda comprendido por todas aquellas que comienzan con la etiqueta <MULTILINE_COMMENT>.
Esto tiene el efecto de apagar o encender cada grupo de expresiones regulares según sea conveniente, luego de identificar cierto patrón en la entrada. Es análogo a utilizar un DFA, pero cambiar a uno completamente diferente si este se topa con un lexema determinado en la entrada.
¿Cómo se sale del contexto? Ingresando al contexto por defecto de Flex: INITIAL. Este contexto ya viene predefinido y se utiliza para volver al DFA original. En este caso, se debe salir del contexto al identificar la secuencia “*/”, pero nótese que esta secuencia solo se identifica dentro del contexto MULTILINE_COMMENT, con lo cual si se halla fuera de él, Flex emitirá un error de sintaxis como es debido.
¿Cómo se crea un contexto? Para ello existe una directiva especial de Flex:

%x MULTILINE_COMMENT


En este caso el contexto MULTILINE_COMMENT es de tipo exclusivo, lo que implica que cuando se ingrese a dicho contexto, solo aquellas expresiones regulares etiquetadas con <MULTILINE_COMMENT> se encontrarán activas. Por otro lado, es posible utilizar %s en lugar de %x para definir un contexto inclusivo que, al contrario del exclusivo, permite que dentro del contexto también permanezcan activas aquella reglas sin etiquetar (asociadas al contexto por defecto INITIAL).

### Detalles de Implementación
Debido a que la implementación de Flex es una reescritura del viejo generador Lex, y que por decisión de diseño los autores optaron por mantener su interfaz de programación, la DX de la herramienta es significativamente pobre, con lo cual en el repositorio de base Flex-Bison-Compiler, se desarrollaron ciertas abstracciones alrededor del analizador para simplificar o minimizar tanto la complejidad de la implementación como la posible introducción de errores de programación y memory-leaks.
#### Interfaz Analizador Léxico-Sintáctico
El analizador léxico se encarga de generar una secuencia o flujo de tokens (token stream). Esta secuencia será consumida por el analizador sintáctico o parser (implementado mediante la herramienta Bison), quien a su vez intentará construir un AST (Árbol de Sintaxis Abstracta), sobre dicha secuencia, a modo de representar la estructura sintáctica del programa de entrada.
Si bien el mecanismo de comunicación entre estas dos fases de análisis es la secuencia o flujo de tokens, existen al menos dos modos de control disponibles: PULL y PUSH.
El modo PULL implica que el analizador sintáctico solicita el siguiente token al analizador léxico directamente, cada vez que lo requiera. De este modo, un patrón en la entrada identificadomediante el analizador léxico produce un token que el parser consume. El parser ejecuta todas las acciones semánticas derivadas de dicho consumo hasta detenerse. Luego repite el proceso; solicita un nuevo token del escáner o lexer hasta consumir el flujo completo. La limitación es clara: el analizador léxico solo puede emitir un token a la vez.
En el modo PUSH, en cambio, es el analizador léxico el que emite los tokens hacia el parser, con lo cual la limitación anterior desaparece. Un escáner puede emitir 0 (cero), 1 (uno), o múltiples tokens cada vez, aunque el patrón identificado en la entrada (el lexema), sea único.
La versión v2.0.0 emplea el nuevo modo PUSH, que es particularmente útil para resolver la sintaxis de lenguajes que utilizan indentación basada en espacios, similar a Python o Haskell (ver Fromme, 2007).
Dado que el modo afecta la comunicación entre Flex y Bison, pero no la semántica general (es decir, se sigue transfiriendo una secuencia de tokens al igual que bajo el modo PULL), el parser, particularmente sus acciones semánticas, no sufren modificación alguna.

#### Estado del Analizador Léxico
[MODO PULL] La abstracción principal consiste en agrupar el estado del analizador léxico dentro de una instancia de la clase LexicalAnalyzerContext, la cual se puede inyectar en cada acción de Flex mediante la función createLexicalAnalyzerContext(), quien se encarga de crear una instancia completamente nueva en cada llamado utilizando memoria dinámica.
Flex se basa en el uso intensivo de variables globales a lo largo de su implementación (yytext, yyleng, yylval, etc.), y debido a que esto es una mala práctica, se optó por copiar el contenido de dichas variables durante la instanciación de un nuevo contexto (no confundir este contexto, con el sistema de contextos utilizado para los comentarios multilínea presentado en la Sección § 4: Contextos).
Actualmente el contexto inyectado provee acceso al siguiente estado:

currentContext: Un número que indica el número de contexto actual (INITIAL, MULTILINE_COMMENT, etc.), el cual es especialmente útil si el lenguaje requiere del uso de contextos anidados. Por defecto, el contexto INITIAL siempre se representa mediante el número 0 (cero).

length: El largo en caracteres del lexema que se activó (según la semántica de la función estándar strlen).

lexeme: El lexema propiamente dicho, copiado y reservado mediante memoria dinámica, lo que permite modificar el mismo libremente sin destruir el estado interno de Flex.

line: El número de línea dónde Flex encontró el lexema activo.

semanticValue: El atributo asociado al lexema que conformará el token final. La definición de este campo se detallará en la sección de "Análisis Sintáctico", ya que sus detalles son provistos por el generador Bison.

#### Acciones
[MODO PULL] Las acciones de Flex se colocan a la derecha de cada expresión regular tal cual se mostró en la Sección § 3: Expresiones Regulares. Estas acciones admiten código en C de cualquier tipo, pero lo usual es que se respeta el siguiente esquema idiomático para mantener tanto la consistencia como la legibilidad del código:

Una función dedicada a cada expresión regular, como por ejemplo es el caso de la acción IntegerLexemeAction(), aunque también es válido agrupar múltiples expresiones bajo la misma función, si es posible determinar el token a emitir desde el lexema, como ocurre con ArithmeticOperatorLexemeAction() que manipula todos los operadores. La función deberá retornar el token asociado al lexema, UNKNOWN en caso de que el lexema sea inválido, o nada (void), si el lexema debe ser ignorado por completo.

La inyección sobre cada acción de un nuevo contexto mediante el llamado a createLexicalAnalyzerContext().

El uso de la directiva BEGIN al inicio de la acción al ingresar a un contexto, y al final de la misma sobre la acción de salida del mismo (esto garantiza que las acciones se ejecutan dentro del contexto por completo).

## Análisis Semántico

### Introducción
Nuevamente, sea G = ⟨Σ, Ν, Π, S⟩ una gramática libre de contexto, donde Σ es el alfabeto, Ν es el conjunto de símbolos no-terminales, Π es el conjunto de producciones y S es el símbolo no-terminal inicial, se define un analizador sintáctico (también conocido como parser), como aquel cuyo objetivo principal es transformar una secuencia de símbolos del alfabeto Σ en uno o varios árboles de sintaxis abstracta (AST)*.
La cantidad de árboles generados dependerá del tipo de analizador y de la secuencia analizada en su entrada, de modo tal que si la entrada pertenece al lenguaje descrito por G, se emite un árbol por cada posible secuencia de derivaciones que genere dicha entrada (si G no es ambigua, siempre se emite un único árbol a lo sumo), pero un conjunto vacío en caso de que la entrada no pertenezca a L(G).
Para las siguientes secciones, el analizador sintáctico utilizado será Bison (un generador de analizadores LALR(1)), pero las ideas y conceptos son generales a otros analizadores. Este analizador solo emite un único AST en caso de tener éxito al analizar la secuencia de entrada, debido a que el generador es inferior a un DPDA (Deterministic Pushdown Automaton), y por ende, no admite gramáticas ni lenguajes ambiguos.
*Es importante recalcar el hecho de que un analizador sintáctico genera árboles de sintaxis debido a que en este contexto el documento se restringe a gramáticas libres de contexto, pero efectivamente para gramáticas de mayor jerarquía el concepto de árbol como estructura de datos es inaplicable

### Gramática
Todo lenguaje conlleva la definición de su sintaxis y su correspondiente semántica. Para estructurar la sintaxis del lenguaje se emplea una gramática que define las construcciones válidas que el lenguaje ofrece. Estas construcciones se deben especificar mediante las 4 componentes de la gramática.
Para definir la lista de tokens que definen el alfabeto Σ de la gramática mediante el generador Bison, se debe utilizar la siguiente directiva:

%token <semanticValueName> TOKEN_NAME


Donde TOKEN_NAME es el nombre del token que se emplea en las acciones del respectivo escáner generado por Flex, tales como ADD, MUL u OPEN_PARENTHESIS. Debido a que estos tokens conforman el alfabeto, representan el conjunto de símbolos terminales de la gramática.
No obstante, para definir el conjunto de símbolos no-terminales, se emplea una directiva similar:

%type <semanticValueName> nonTerminalName


De forma análoga, nonTerminalName es el nombre de un símbolo no-terminal disponible para utilizar en la definición de las reglas de producción de la gramática G (conjunto Π). Nótese que por convención, se emplea SCREAMING_SNAKE_CASE para los terminales, pero camelCase para los no-terminales.
Tanto terminales como no-terminales acarrean un atributo o metadato que representa su valor semántico (cuyo nombre se indica mediante el parámetro semanticValueName en ambos casos). La definición y uso de ellos será detallada en la Sección § 3: Semántica.
Con ambos conjuntos, ya es posible especificar la gramática, en este caso, un ejemplo de programa que admite expresiones aritméticas sobre constantes enteras:

program: expression
;

expression: expression ADD expression
| expression DIV expression
| expression MUL expression
| expression SUB expression
| factor
;

factor: OPEN_PARENTHESIS expression CLOSE_PARENTHESIS
| constant
;

constant: INTEGER
;


Nótese que el símbolo “;” (semicolon), indica el final de una secuencia de producciones que poseen el mismo no-terminal del lado izquierdo, mientras que “:” (colon) reemplaza el clásico símbolo “⟶” de una regla de derivación. En el caso de que exista la necesidad de utilizar una regla de producción cuyo lado derecho sea λ, se deberá dejar el espacio vacío luego del “|” (pipe), o bien, utilizar la directiva %empty en su lugar.
En este ejemplo, es fácil ver que un programa en este lenguaje es en realidad una expresión, y que la misma se segrega en una combinación de factores y constantes que solo pueden ser de tipo enteras, interactuando mediante los operadores básicos de la aritmética (suma, resta, multiplicación y división), posiblemente agrupando ciertas subexpresiones mediante paréntesis.
En la gramática, el símbolo INTEGER debería representar un único elemento del alfabeto, y en este caso evidentemente es así, pero nótese que debido a la integración de Bison con Flex, es sabido que dicho token puede generarse desde infinitos lexemas posibles, específicamente aquellos que representan secuencias finitas de dígitos, pero arbitrariamente largas*.
*Es posible, y en general deseable, que el largo de dichas secuencias se limite de alguna manera mediante las expresiones regulares de Flex, en particular si dicha secuencia, por ejemplo, será traducida y almacenada en una variable de tipo int de C, que no admite más que lo que quepa en 4 bytes, al menos en la mayoría de plataformas hasta la fecha.

#### Precedencia y Asociatividad
La gramática de expresiones aritméticas expuesta en la sección anterior no impone ningún orden natural sobre las operaciones admitidas. Es evidente que, jerárquicamente, una expresión es un concepto más general que un factor, y que un factor es más general que una constante, pero nada se dice acerca de la relación que existe entre la suma y la multiplicación, o entre la suma y la resta.
En cualquier lenguaje de programación (e incluso en aquellos que no tienen por objetivo el diseño de aplicaciones pero que constan de un grupo de operadores), es deseable definir un orden en particular para dos situaciones particulares: (I) la combinación de operadores del mismo tipo, y (II) las de diferente tipo.
Cuando se dispone de una secuencia de operaciones, donde el operador es el mismo en sucesivas aplicaciones (situación I):

A + B + C + D + …

es relevante hablar de asociatividad, es decir, del agrupamiento natural que posee dicho operador sobre sus operandos. La asociatividad se puede dar por izquierda si:

A + B + C + D + … = ((((A + B) + C) + D) + … ),

por derecha:

A + B + C + D + … = (A + (B + (C + (D + … )))),

o como es el caso del operador de suma, por ambas.
Por otro lado, en los casos en que el operador se encuentre en una secuencia junto a otros de diferente tipo (situación II), se hablará de precedencia, ya que en estos casos no es relevante el agrupamiento natural de cada operador (podría haber una única ocurrencia de cada uno dentro de una misma expresión, con lo cual las reglas de asociatividad no aplican), sino más bien el orden en el cual dichas operaciones se deben ejecutar para evaluar o computar el valor de la expresión final.
La precedencia usualmente se especifica como una simple jerarquización de operadores (ver por ejemplo, C Operator Precedence), donde aquellos con mayor precedencia se ejecutan o computan antes que aquellos con menor precedencia. Dentro de clases o niveles de la misma jerarquía, los operadores se ejecutan en el mismo orden en el que se encuentran o escanean en la secuencia de entrada (de izquierda a derecha).
Para especificar la asociatividad en Bison, se disponen 2 (dos) directivas adicionales: %left y %right, que indican la asociatividad a izquierda o a derecha respectivamente. Para indicar la precedencia, no obstante, se utiliza el mismo orden en el cual dichas directivas fueron redactadas:

%left ADD SUB
%left MUL DIV


donde el hecho de que la asociatividad de MUL y DIV se haya definido luego de la de los operadores ADD y SUB, implica que estos últimos poseen menor precedencia, con lo cual, una expresión que involucre símbolos de ambos niveles de jerarquía, priorizará el cómputo de la multiplicación o división antes que la de la suma y la resta, lo que es consistente con el comportamiento de dichos operadores en la aritmética.
Este mecanismo simplifica enormemente la definición de la gramática, ya que de otro modo, las reglas de producción deberán ser las que definan explícitamente la precedencia y asociatividad naturalmente como un efecto secundario del proceso de derivación de la secuencia de entrada.

### Semántica
Una gramática definida en Bison no produce ningún efecto adicional más allá de reconocer que la secuencia de entrada es generada por la gramática o lo que es igual, pertenece al lenguaje que dicha gramática genera.
No obstante, un compilador en general aplica transformaciones y validaciones adicionales sobre la secuencia de entrada. Para ello, de alguna manera debe poder representar la secuencia de entrada mediante alguna estructura de datos conveniente (y no simplemente como un stream de tokens), y permitir el agregado de información adicional.
El objetivo entonces es lograr llevar el poder de una gramática libre de contexto más allá de sus capacidades (a un nivel más alto en la Jerarquía de Chomsky), y para ello se hará uso del concepto de gramática atribuida.

#### Gramática Atribuída
La teoría de este modelo matemático desarrollado por D. E. Knuth (1968) será desarrollada en otros documentos, pero a modo simplificado, Bison considera que cada token puede acarrear un atributo o metadato que representa de alguna manera la semántica del mismo. Este valor semántico puede ser de cualquier tipo de dato, y se puede asignar tanto a símbolos terminales como no-terminales.
Para definir la lista de posibles valores semánticos disponibles dentro del analizador Bison, se debe utilizar una directiva especial denominada %union:

%union {
/** Terminals. */

    signed int integer;
    TokenLabel token;

    /** Non-terminals. */

    Constant * constant;
    Expression * expression;
    Factor * factor;
    Program * program;
}


En este caso, un extracto del repositorio de base, específicamente obtenido del documento BisonGrammar.y, que define una estructura de datos compuesta por varios campos de diferente tipo.
Recordando que en el lenguaje C es posible definir estructuras de datos mediante struct y union, donde union especifica un grupo de campos que se encuentran superpuestos en memoria, efectivamente esta definición provee un valor semántico polimórfico, es decir, que puede comportarse o interpretarse de diferentes maneras, según que campo del mismo se utilice.
De este modo, asignar un 0 (cero) al valor semántico de una instancia X del tipo de esta unión, se podrá interpretar como un número entero en caso de ser accedida mediante X.integer, o como un puntero a NULL en caso de utilizar X.constant o X.factor, entre otros.
¿Cómo se indica para cada terminal y no-terminal el tipo de dato del valor semántico (atributo o metadato), que acarrea? Utilizando las directivas %token y %type presentadas en la Sección § 2: Gramática, específicamente utilizando el parámetro semanticValueName. Por ejemplo, las siguientes definiciones:

/** Terminals. */
%token <integer> INTEGER
%token <token> ADD
...

/** Non-terminals. */
%type <constant> constant
...


indican que los terminales INTEGER y ADD poseen un valor semántico de tipo int y Token respectivamente, ya que los campos denominados integer y token efectivamente poseen este tipo de dato en la unión. Por otro lado, el no-terminal constant, posee un valor semántico de tipo Constant*, y es accedido mediante un campo que se denomina igual que el símbolo.
¿Cómo se asigna un valor semántico a un terminal provisto por Flex? Utilizando el contexto provisto por la función createLexicalAnalyzerContext(), el cual provee una copia única del campo semanticValue que justamente es del tipo de dato de esta unión definida en Bison. Este campo se genera para cada terminal, y por ende cada uno de ellos puede almacenar sus propios metadatos sin afectar a las demás ocurrencias, incluso del mismo token.
Finalmente, para asignar el valor semántico de un no-terminal en Bison, es necesario utilizar las reglas de producción:

...
expression: expression ADD expression                        { $$ = f($1, $3); }
| ...
;

factor: OPEN_PARENTHESIS expression CLOSE_PARENTHESIS        { $$ = g($2); }
| constant                                               { $$ = h($1); }
;

constant: INTEGER                                            { $$ = $1; }
;


Para cada regla de la forma A ⟶ B(1) … B(n), $$ representa el valor semántico del símbolo A, mientras que $k, representa el valor semántico del símbolo B(k) (con 1 ≤ k ≤ n). De este modo, la acción asociada a la expresión de suma, le asigna a la expresión resultante (no-terminal expression del lado izquierdo de la regla), el valor semántico $$, obtenido luego de retornar de la función f($1, $3), donde $1 y $3 efectivamente representan a su vez el valor semántico de las expresiones involucradas en la suma (operando izquierdo y derecho, respectivamente).
Nótese que para computar f($1, $3), no se hizo uso del valor $2, ya que no es relevante en este caso utilizar el atributo dentro del token ADD. En situaciones como las de la acción asociada a la regla constant ⟶ INTEGER, el valor semántico $1 simplemente se propaga hacia $$.
Debido a que por las directivas %token y %type, se especificó que para el no-terminal constant su tipo de la unión debía ser Constant*, mientras que para el token INTEGER debía ser int, está claro que la asignación $$ = $1 es incompatible en sus tipos de datos, y por ende producirá un error al ser compilado. Esto implica que dicha asignación es imposible verbatim, y que deberá transformarse el valor de $1 en uno que quepa dentro de $$, por ejemplo, aplicando un wrapping con alguna estructura adicional, como Constant:
typedef struct Constant Constant;

struct Constant {
int value;
};


En este caso la asignación es simple, porque solo involucra un tipo de dato único, sin embargo nótese que el valor semántico de factor puede ser conformado desde una expression, o desde una constant. Para estos casos, es preferible realizar alguna especie de “polimorfismo manual”, aprovechando el uso de los enumerados, estructuras y uniones de C:

typedef struct FactorType FactorType;

typedef struct Factor Factor;

enum FactorType {
CONSTANT,
EXPRESSION
};

struct Factor {
union {
Constant * constant;
Expression * expression;
};
FactorType type;
};


Ahora un Factor puede acarrear una constante o expresión (sin desperdiciar memoria gracias al uso de la unión). Por otro lado, puede ser manipulado sin ambigüedad gracias al enumerado FactorType, que permite identificar explícitamente qué campo de la unión es el que efectivamente se está transportando dentro de una instancia de esta estructura.
Finalmente, en ciertas ocasiones el uso de las etiquetas $1, $2, $3, …, puede dificultar la lectura de la gramática. Para ello, Bison permite el uso de named references (entre corchetes):

expression: expression[left] ADD expression[right]    { $$ = f($left, $right); }
| ...
;

#### Árbol de Sintaxis Abstracta

El primer paso está completo: la gramática transporta y computa nuevos atributos o metadatos a lo largo del análisis sintáctico pero, ¿qué sucede si fases posteriores a dicho análisis deben efectuar alguna transformación sobre la secuencia de entrada y por ende deben conocer y navegar la estructura sintáctica de la misma? Para resolver este problema se hace evidente la necesidad de mapear la secuencia de tokens inicial en la entrada a una estructura de datos que provea ambas características: (I) debe ser navegable, y (II) debe ser representativa de la sintaxis.
La estructura que provee ambas características se denomina Árbol de Sintaxis Abstracta (AST), y debe ser un árbol debido a que una gramática libre de contexto impone naturalmente dicha topología: para cada producción de la forma A ⟶ B(1) … B(n), A es un nodo padre, y B(k) es alguno de sus n hijos.
Es factible imaginar que debido a las siguientes cuestiones:

Cada símbolo terminal y no-terminal de una gramática puede transportar un valor semántico.

Cada no-terminal efectivamente se encontrará del lado izquierdo de una producción y por ende será el padre de varios nodos.

Cada terminal conformará un nodo hoja en este árbol (porque nunca estará del lado izquierdo de una regla).

se podría utilizar el valor semántico de los símbolos no-terminales para transportar los nodos que conforman este AST. Ahora bien, ya que Bison es un analizador sintáctico de tipo LALR(1), el analizador ejecuta las reducciones de cada regla de forma ascendente, es decir, el AST subyacente que la gramática define naturalmente para cierta entrada se genera en post-orden.
Esto significa que al ejecutar una reducción sobre una regla de producción, se ejecuta su acción (el código de C encerrado entre {...}), pero solo cuando ya se han ejecutado las acciones asociadas a todos los símbolos del lado derecho de dicha producción. Convenientemente, este orden de ejecución del algoritmo subyacente de Bison permite que la creación de un nodo en el AST para el no-terminal a la izquierda de A ⟶ B(1) … B(n), se pueda construir y enlazar directamente a sus n nodos hijo B(k), donde cada nodo B(k) está completamente construído, incluso si estos nodos son raíces de otros subárboles dentro del AST.
Para ejemplificar el orden de ejecución de estas acciones, se aplicará el análisis sobre la expresión 1 + 2 * 3 utilizando el repositorio base Flex-Bison-Compiler:

[DEBUG][SyntacticAnalyzer] Parsing...
[DEBUG][FlexActions] IntegerLexemeAction: 1 (context = 0, length = 1, line = 1)
[DEBUG][BisonActions] IntegerConstantSemanticAction
[DEBUG][BisonActions] ConstantFactorSemanticAction
[DEBUG][BisonActions] FactorExpressionSemanticAction
[DEBUG][FlexActions] ArithmeticOperatorLexemeAction: + (context = 0, length = 1, line = 1)
[DEBUG][FlexActions] IntegerLexemeAction: 2 (context = 0, length = 1, line = 1)
[DEBUG][BisonActions] IntegerConstantSemanticAction
[DEBUG][BisonActions] ConstantFactorSemanticAction
[DEBUG][BisonActions] FactorExpressionSemanticAction
[DEBUG][FlexActions] ArithmeticOperatorLexemeAction: * (context = 0, length = 1, line = 1)
[DEBUG][FlexActions] IntegerLexemeAction: 3 (context = 0, length = 1, line = 1)
[DEBUG][BisonActions] IntegerConstantSemanticAction
[DEBUG][BisonActions] ConstantFactorSemanticAction
[DEBUG][BisonActions] FactorExpressionSemanticAction
[DEBUG][BisonActions] ArithmeticExpressionSemanticAction
[DEBUG][BisonActions] ArithmeticExpressionSemanticAction
[DEBUG][BisonActions] ExpressionProgramSemanticAction
[DEBUG][SyntacticAnalyzer] Parsing is done.

Del log anterior reportado durante la ejecución de las acciones es posible ver que:

Flex identifica y genera el atributo para un token INTEGER de valor semántico 1.
Bison aplica la reducción constant ⟶ INTEGER.
Bison aplica la reducción factor ⟶ constant.
Bison aplica la reducción expression ⟶ factor.
Flex identifica y genera el atributo para un token ADD.
Flex identifica y genera el atributo para un token INTEGER de valor semántico 2.
Bison aplica la reducción constant ⟶ INTEGER (esta vez para el valor semántico 2).
Bison aplica la reducción factor ⟶ constant.
Bison aplica la reducción expression ⟶ factor.
Flex identifica y genera el atributo para un token MUL.
Flex identifica y genera el atributo para un token INTEGER de valor semántico 3.
Bison aplica la reducción constant ⟶ INTEGER (esta vez para el valor semántico 3).
Bison aplica la reducción factor ⟶ constant.
Bison aplica la reducción expression ⟶ factor.
Bison aplica la reducción expression ⟶ expression MUL expression.
Bison aplica la reducción expression ⟶ expression ADD expression.
Bison aplica la reducción program ⟶ expression.

Como AST, la secuencia de operaciones se ejecuta en el orden esperado (post-orden, ascendente):

[FALTA DIAGRAMA]

En otras palabras, al ejecutar por ejemplo, la reducción:

expression ⟶ expression ADD expression,

los valores semánticos $1 y $3 contendrán los nodos del AST para cada expresión del lado derecho de esta producción, es decir, los nodos raíz de los subárboles de cada expresión asociada a cada operando de la suma. Por otro lado, con este par de subárboles se espera que la acción asociada construya el nodo del no-terminal a la izquierda de la regla, y que asigne dicho valor semántico a $$, dando por finalizada la construcción de esta parte del AST.
La aplicación sistemática de este proceso culmina con la creación del nodo raíz program, que representa toda la estructura sintáctica de la secuencia de entrada original, y la única raíz del AST completo.

### Conflictos
Debido a que Bison tiene una capacidad de análisis sintáctico LALR(1), ciertas gramáticas libres de contexto no podrán producir un AST como corresponde, ya sea porque dichas gramáticas son ambiguas, o porque el lenguaje que generan require capacidades no-determinísticas (como es el caso del lenguaje de los palíndromos, que es no-determinístico, libre de contexto y no-ambiguo). Cuando la gramática excede las capacidades del analizador, este produce uno o más conflictos.

#### Desplazamiento/Reducción
El primer tipo de conflicto se produce en general cuando no existe un orden claro de precedencia entre ciertas construcciones de la gramática, tales como es el caso de las expresiones aritméticas definidas en la Sección § 2: Gramática. Si la precedencia no fuera especificada mediante las directivas %left y %right, las 4 (cuatro) operaciones matemáticas podrían ejecutarse en cualquier orden, y por ende el AST producido podría incurrir en múltiples variantes posibles.
En el contexto de un analizador, un desplazamiento (shift) implica consumir un símbolo adicional de la secuencia de entrada, mientras que una reducción (reduce) indica la ejecución de una regla de producción determinada. Por lo tanto, un conflicto desplazamiento/reducción (shift/reduce), demuestra que el analizador sintáctico se encuentra frente a la posibilidad de efectuar ambas operaciones, situación bajo la cual decide por defecto aplicar el desplazamiento pero, no obstante, sin conocer los efectos que esto representa sobre la semántica final del AST generado. Por este motivo, cualquier conflicto presente y detectado por Bison, debe ser apropiadamente eliminado.

#### Reducción/Reducción
En este caso, el conflicto implica la posibilidad de aplicar dos o más reducciones diferentes, es decir, la posibilidad de activar 2 (dos) o más reglas de producción para la misma secuencia de entrada. En estos casos, Bison ejecutará la reducción que se definió primero en la gramática, pero de nuevo, sin conocer el impacto semántico que esto produce sobre la compilación final de la secuencia de entrada, motivo por el cual debe eliminarse por completo al igual que un conflicto shift/reduce.
La eliminación de conflictos de este tipo en general involucra la reescritura de parte de la gramática. Es común generar este tipo de conflictos por el uso excesivo de producciones anulables (aquellas que utilizan el símbolo λ), o por construcciones que recursivamente permiten derivar las mismas estructuras sintácticas mediante diferentes caminos (diferentes secuencias de derivación).
Tanto para conflictos shift/reduce como para reduce/reduce, Bison posee un mecanismo que produce contra-ejemplos, simplificando de este modo, no solo la detección temprana de los mismos, sino también la visualización de la estructura sintáctica particular que destraba dicho comportamiento (estructura que el mismo analizador imprime por consola).

## Testeo
Permitir la correcta ejecución del script de testing disponible en el repositorio de base, aceptando o rechazando cada uno de los casos de prueba según corresponda.
