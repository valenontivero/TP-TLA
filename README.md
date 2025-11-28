# Drum Machine DSL Compiler

[![Release](https://img.shields.io/badge/Release-v2.0.0-ffb600.svg?style=for-the-badge)](https://github.com/agustin-golmar/Flex-Bison-Compiler/releases)
[![Pipeline](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/agustin-golmar/Flex-Bison-Compiler/actions/workflows/pipeline.yaml)

A complete compiler for 'DrumLang', a domain-specific language (DSL) designed to create rhythm patterns for drum machines. Built with Flex (lexical analysis) and Bison (syntactic analysis).

---

## Usage

### Build the Compiler

```bash
docker compose run --rm compiler bash -c "src/main/bash/build.sh"
```

### Compile a Program

```bash
docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
  "src/main/bash/run.sh <program.dsl> -fo=wav,flac,mp3"
```
the option -fo is optional. If not provided, the program will only generate the MIDI file. If provided, the program will generate the MIDI file and the audio file in either WAV, FLAC or MP3 format.

### How To Write a Program

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


## Docker Commands

### Development

```bash
# Start ephemeral container
docker compose run --rm compiler

# Build inside container
src/main/bash/build.sh

# Run program
src/main/bash/run.sh <program.dsl> [-fo=wav,flac,mp3]

# Exit and cleanup
exit
docker compose down
```
