#include "Generator.h"
#include "MidiGenerator.h"
#include "AudioConverter.h"

/* MODULE INTERNAL STATE */

const char _indentationCharacter = ' ';
const char _indentationSize = 4;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Generator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeGeneratorModule() {
	_logger = createLogger("Generator");
	return _shutdownGeneratorModule;
}

/** PRIVATE FUNCTIONS */

static char * _indentation(const unsigned int indentationLevel);
static void _generateProgram(Program * program);
static void _generateImportList(ImportList * importList);
static void _generateDeclarations(Declarations * declarations);
static void _generatePatternList(PatternList * patternList);
static void _generateInstrumentList(InstrumentList * instrumentList);
static void _output(const unsigned int indentationLevel, const char * const format, ...);

/**
 * Generates the output of import list.
 */
static void _generateImportList(ImportList * importList) {
	if (importList != NULL) {
		_output(0, "Imports:\n");
		ImportList * current = importList;
		while (current != NULL) {
			if (current->import != NULL) {
				_output(1, "Import: \"%s\"\n", current->import->filePath);
			}
			current = current->next;
		}
		_output(0, "\n");
	}
}

/**
 * Generates the output of declarations.
 */
static void _generateDeclarations(Declarations * declarations) {
	if (declarations != NULL) {
		_output(0, "Tempo: %d BPM\n", declarations->tempo);
		_output(0, "Compasses: %d\n", declarations->compasses);
		_output(0, "Steps: %d\n", declarations->steps);
	}
}

/**
 * Generates the output of pattern list.
 */
static void _generatePatternList(PatternList * patternList) {
	if (patternList != NULL) {
		_output(0, "\nPatterns:\n");
		PatternList * current = patternList;
		while (current != NULL) {
			if (current->pattern != NULL) {
				_output(1, "Pattern: %s\n", current->pattern->name);
			}
			current = current->next;
		}
	}
}

/**
 * Generates the output of instrument list.
 */
static void _generateInstrumentList(InstrumentList * instrumentList) {
	if (instrumentList != NULL) {
		_output(0, "\nInstruments:\n");
		InstrumentList * current = instrumentList;
		while (current != NULL) {
			if (current->instrument != NULL) {
				_output(1, "Instrument: %s\n", current->instrument->name);
				_output(2, "Pattern: %s\n", current->instrument->patternName);
				if (current->instrument->activeRange != NULL) {
                    _output(2, "Active: ");
                    ActiveRange * range = current->instrument->activeRange;
                    bool first = true;
                    while (range != NULL) {
                        if (!first) {
                            printf(" + ");
                        }
                        printf("%d-%d", range->start, range->end);
                        first = false;
                        range = range->next;
                    }
                    printf("\n");
                }
			}
			current = current->next;
		}
	}
}

/**
 * Generates the output of the program.
 */
static void _generateProgram(Program * program) {
	if (program != NULL) {
		_output(0, "=== Drum Machine Program ===\n\n");
		_generateImportList(program->imports);
		_generateDeclarations(program->declarations);
		_generatePatternList(program->patterns);
		_generateInstrumentList(program->instruments);
		_output(0, "\n=== End of Program ===\n");
	}
}

/**
 * Generates an indentation string for the specified level.
 */
static char * _indentation(const unsigned int level) {
	return indentation(_indentationCharacter, level, _indentationSize);
}

/**
 * Outputs a formatted string to standard output. The "fflush" instruction
 * allows to see the output even close to a failure, because it drops the
 * buffering.
 */
static void _output(const unsigned int indentationLevel, const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	char * indentation = _indentation(indentationLevel);
	char * effectiveFormat = concatenate(2, indentation, format);
	vfprintf(stdout, effectiveFormat, arguments);
	fflush(stdout);
	free(effectiveFormat);
	free(indentation);
	va_end(arguments);
}

/** PUBLIC FUNCTIONS */

void executeGenerator(CompilerState * compilerState) {
	logDebugging(_logger, "Generating final output...");

	// Generate text output (summary)
	_generateProgram(compilerState->abstractSyntaxtTree);

	// Generate MIDI file (only if we have declarations and instruments)
	Program * program = compilerState->abstractSyntaxtTree;
	if (program != NULL && program->declarations != NULL && program->instruments != NULL) {
		CompilationStatus status = generateMidiFile(program, "out/output.mid");
		if (status == SUCCEEDED) {
			printf("\n✓ MIDI file generated: out/output.mid\n");

			// Convert MIDI to audio formats
			printf("\nConverting MIDI to audio formats...\n");

			// Convert to WAV
			status = convertMidiToWav("out/output.mid", "out/output.wav");
			if (status == SUCCEEDED) {
				printf("✓ WAV file generated: out/output.wav\n");
			} else {
				printf("✗ WAV conversion failed\n");
			}

			// Convert to FLAC
			status = convertMidiToFlac("out/output.mid", "out/output.flac");
			if (status == SUCCEEDED) {
				printf("✓ FLAC file generated: out/output.flac\n");
			} else {
				printf("✗ FLAC conversion failed\n");
			}

			// Convert to MP3
			status = convertMidiToMp3("out/output.mid", "out/output.mp3");
			if (status == SUCCEEDED) {
				printf("✓ MP3 file generated: out/output.mp3\n");
			} else {
				printf("✗ MP3 conversion failed\n");
			}
		} else {
			printf("\n✗ MIDI generation failed\n");
		}
	} else {
		logDebugging(_logger, "Skipping MIDI generation (pattern library or incomplete program)");
	}

	logDebugging(_logger, "Generation is done.");
}
