[![Release](https://img.shields.io/badge/Release-v2.0.0-ffb600.svg?style=for-the-badge)](https://github.com/agustin-golmar/Flex-Bison-Compiler/releases)

[![Pipeline](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml)

# Drum Machine DSL Compiler

A complete compiler for a domain-specific language (DSL) for drum machine rhythm pattern creation.

## Quick Start

Run the complete test suite to verify everything works:

```bash
./test-complete.sh
```

Expected result: All 25 tests passing

## Documentation

- **[QUICKSTART.md](QUICKSTART.md)** - Quick usage guide
- **[TESTING.md](TESTING.md)** - Complete testing guide
- **[PARSER.md](PARSER.md)** - Parser documentation
- **[LEXER.md](LEXER.md)** - Lexer documentation
- **[CLAUDE.md](CLAUDE.md)** - AI assistant project guide

---

## Current Status

### Lexical Analysis: Complete

The lexical analyzer recognizes 24 token types for the Drum Machine DSL.

Test: `./test-lexer.sh`

### Syntactic Analysis: Complete

The parser builds a complete AST for the Drum Machine DSL.

Implemented features:
- Declarations: tempo, compasses, steps
- Patterns with rhythm expressions
- Rhythm arrays: `[x,.,x,.]`
- Rhythm concatenation: `+`
- Rhythm repetition: `* N`
- Instruments with pattern references
- Active ranges with concatenation: `active 1-4 + 6-8 + 10-16`

Test: `./test-complete.sh`

This script runs 25 comprehensive tests verifying the entire compiler.

### Backend: In Progress

The backend currently generates a text representation of the AST. Audio/MIDI generation is not yet implemented.

---

## Language Example

```
remember "lib/patterns.dsl"

tempo 120
compasses 16
steps 4

pattern kickPattern {
    rhythm [x,.,x,.] * 4
}

pattern bassPattern {
    rhythm [E2,.,A2,.] + [E2,G2,A2,.]
}

instruments {
    kick {
        pattern kickPattern
        active 1-4 + 6-8 + 10-16
    }
    bass {
        pattern bassPattern
        active 1-16
    }
}
```

### Key Language Features

- **Universal silence**: `.` for all silence (percussion and melody)
- **Range separator**: `-` is used only for ranges (e.g., `1-4`), not as a rhythm element
- **Range concatenation**: Active ranges can be concatenated with `+` (e.g., `active 1-4 + 6-8 + 10-12`)
- **Musical notes**: Support for notes with accidentals (e.g., `E2`, `A#2`, `Bb3`)
- **Operators**: Concatenation `+` and repetition `*` for rhythm expressions

---

## Requirements

* [Docker v28.3.2](https://www.docker.com/)

## Configuration

You could set the following environment variables to control the compiler behavior (it is not necessary though):

| Name                  | Default | Description                                                                                                                                                           |
| :-------------------- | :-----: | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ENVIRONMENT`         | `Local` | The active environment name. Available environments: `Local`, `Development`, `Production`.                                                                            |
| `LOG_IGNORED_LEXEMES` | `true`  | When `true`, logs all ignored lexemes found with Flex at `DEBUGGING` level. Set to `false` to remove these logs from console output.                                  |
| `LOGGING_LEVEL`       | `ALL`   | The minimum level to log in console output. From lower to higher: `ALL`, `DEBUGGING`, `INFORMATION`, `WARNING`, `ERROR`, `CRITICAL`.                                 |

Docker Compose can read variables from an `.env` file (see `compose.yaml` file).

## Commands

### Start

Start an ephemeral container for development:

```bash
docker compose run --rm compiler
```

### Build

Build or rebuild the entire compiler:

```bash
src/main/bash/build.sh
```

### Run

Compile a program:

```bash
src/main/bash/run.sh <program>
```

where `<program>` is the path to the file representing the program entry-point.

### Test

Execute all available unit tests under `src/test/c` folder:

```bash
src/main/bash/test.sh
```

### Stop

Logout, destroy ephemeral containers and shutdown the cluster:

```bash
exit
docker compose down
```

### Docker

| Command                                 | Description                                             |
| :-------------------------------------- | :------------------------------------------------------ |
| `docker builder prune --all`            | Removes all builds and complete build cache.            |
| `docker compose --progress=plain build` | Forces a build or rebuild of the images in the cluster. |
| `docker image prune`                    | Removes all dangling images from Docker.                |
| `docker network prune`                  | Removes unused networks from Docker.                    |
| `docker volume prune`                   | Removes unused volumes from Docker.                     |


## Recommended Extensions

* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* [Yash](https://marketplace.visualstudio.com/items?itemName=daohong-emilio.yash)
