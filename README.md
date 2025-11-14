# Drum Machine DSL Compiler

[![Release](https://img.shields.io/badge/Release-v2.0.0-ffb600.svg?style=for-the-badge)](https://github.com/agustin-golmar/Flex-Bison-Compiler/releases)
[![Pipeline](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml)

A complete compiler frontend for a domain-specific language (DSL) designed to create rhythm patterns for drum machines. Built with Flex (lexical analysis) and Bison (syntactic analysis).

---

## Quick Start

Verify the compiler works correctly:

```bash
./test-complete.sh
```

Expected result: `✓ ALL TESTS PASSED! (25/25)`

---

## Current Status

| Phase | Status | Description |
|-------|--------|-------------|
| **Lexical Analysis** | ✅ Complete | 24 token types recognized |
| **Syntactic Analysis** | ✅ Complete | Full AST construction, zero conflicts |
| **Backend** | 🚧 TODO | Audio/MIDI generation not implemented |

The frontend is 100% complete and tested. See [FRONTEND.md](FRONTEND.md) for complete technical documentation.

---

## Language Features

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

- **Import mechanism**: Reference external pattern files with `remember "file.dsl"`
- **Declarations**: Define `tempo` (BPM), `compasses`, and `steps` (subdivisions)
- **Patterns**: Create reusable rhythm patterns with percussion hits (`x`), silence (`.`), or musical notes (`E2`, `A#2`)
- **Operators**:
  - Concatenation (`+`): Combine rhythm expressions
  - Repetition (`* N`): Repeat a pattern N times
- **Instruments**: Map patterns to instruments with active range specifications
- **Range concatenation**: Define discontinuous active ranges (e.g., `1-4 + 6-8 + 10-12`)

---

## Requirements

- [Docker v28.3.2](https://www.docker.com/) or later

---

## Usage

### Build the Compiler

```bash
docker compose run --rm compiler bash -c "src/main/bash/build.sh"
```

### Compile a Program

```bash
docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh <program.dsl>"
```

### Run Tests

```bash
# Full test suite (recommended)
./test-complete.sh

# Lexer only
./test-lexer.sh

# Parser only
./test-parser.sh
```

---

## Configuration

Environment variables (optional):

| Name | Default | Description |
|------|---------|-------------|
| `ENVIRONMENT` | `Local` | Active environment: `Local`, `Development`, `Production` |
| `LOG_IGNORED_LEXEMES` | `true` | Log ignored lexemes at `DEBUGGING` level |
| `LOGGING_LEVEL` | `ALL` | Minimum log level: `ALL`, `DEBUGGING`, `INFORMATION`, `WARNING`, `ERROR`, `CRITICAL` |

Set these in `.env` file or pass directly to Docker Compose.

---

## Documentation

- **[FRONTEND.md](FRONTEND.md)** - Complete frontend implementation documentation (lexer, parser, AST, testing)
- **[CLAUDE.md](CLAUDE.md)** - AI assistant project guide

---

## Example Output

Running the example program above produces:

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
        Active: 1-16
    Instrument: kick
        Pattern: kickPattern
        Active: 1-4 + 6-8 + 10-16

=== End of Program ===
```

---

## Architecture

```
┌─────────────┐      ┌─────────────┐      ┌─────────────┐
│   Source    │      │    Token    │      │     AST     │
│    Code     │─────▶│   Stream    │─────▶│   (Tree)    │
│   (.dsl)    │      │             │      │             │
└─────────────┘      └─────────────┘      └─────────────┘
     Flex                  Bison               Generator
  (Lexer)               (Parser)            (Pretty-print)
```

### Components

1. **Lexical Analysis** (Flex)
   - Tokenizes input into 24 token types
   - PUSH mode operation (v2.0.0 feature)
   - Pattern order is critical for correct tokenization

2. **Syntactic Analysis** (Bison)
   - LALR(1) parser with zero conflicts
   - Context-free grammar (Type 2, Chomsky)
   - Builds complete Abstract Syntax Tree (AST)
   - 13 AST node types with memory-safe destructors

3. **Code Generation** (Stub)
   - Currently pretty-prints the AST
   - Backend for audio/MIDI generation is TODO

See [FRONTEND.md](FRONTEND.md) for detailed technical documentation.

---

## Docker Commands

### Development

```bash
# Start ephemeral container
docker compose run --rm compiler

# Build inside container
src/main/bash/build.sh

# Run program
src/main/bash/run.sh <program.dsl>

# Exit and cleanup
exit
docker compose down
```

### Cleanup

| Command | Description |
|---------|-------------|
| `docker builder prune --all` | Remove all builds and build cache |
| `docker compose --progress=plain build` | Force rebuild images |
| `docker image prune` | Remove dangling images |
| `docker network prune` | Remove unused networks |
| `docker volume prune` | Remove unused volumes |

---

## Testing

The test suite includes:

- **8 acceptance tests** - Valid programs that should compile
- **17 rejection tests** - Invalid programs that should fail
- **Token recognition** - All 24 token types verified
- **Grammar coverage** - All 23 productions tested
- **Memory management** - No leaks detected

Run `./test-complete.sh` to verify everything works.

---

## Recommended VS Code Extensions

- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
- [Yash](https://marketplace.visualstudio.com/items?itemName=daohong-emilio.yash)

---

## License

See [LICENSE.md](LICENSE.md) for details.

---

## Next Steps

The frontend is complete. Next phase:

1. **Import Resolution** - Implement file loading for `remember` statements
2. **Semantic Validation** - Verify pattern references, range validity, etc.
3. **Backend Implementation** - Generate audio/MIDI output from AST

---

## Contributing

This is an educational project demonstrating compiler construction with Flex and Bison. For technical details on implementation, see [FRONTEND.md](FRONTEND.md).
