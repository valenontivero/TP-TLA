#include "OutputConversion.h"
#include "../code-generation/AudioConverter.h"
#include "../../support/configuration/Environment.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"

#include <stdio.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
static void _shutdownOutputConversionModule(void) {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: OutputConversion...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeOutputConversionModule() {
	_logger = createLogger("OutputConversion");
	return _shutdownOutputConversionModule;
}

/** PUBLIC FUNCTIONS */

void executeOutputConversion(const CompilerState * compilerState) {
	logDebugging(_logger, "Starting output conversion stage...");

	if (compilerState == NULL || compilerState->abstractSyntaxtTree == NULL) {
		logDebugging(_logger, "No program available; skipping output conversion.");
		return;
	}

	/* Environment toggle for this stage. */
	const bool enabled = getBooleanOrDefault("OUTPUT_CONVERSION_ENABLED", false);
	if (!enabled) {
		logDebugging(_logger,
			"Environment variable OUTPUT_CONVERSION_ENABLED is false or undefined; "
			"skipping output conversion stage.");
		return;
	}

	const bool wantsWav  = compilerState->outputWav;
	const bool wantsFlac = compilerState->outputFlac;
	const bool wantsMp3  = compilerState->outputMp3;

	if (!wantsWav && !wantsFlac && !wantsMp3) {
		logDebugging(_logger,
			"No audio formats requested (no -fo flag, or empty formats); "
			"skipping output conversion.");
		return;
	}

	const char * midiPath = "out/output.mid";

	printf("\nConverting MIDI to audio formats (OUTPUT_CONVERSION_ENABLED=true)...\n");

	CompilationStatus status;

	if (wantsWav) {
		status = convertMidiToWav(midiPath, "out/output.wav");
		if (status == SUCCEEDED) {
			printf("✓ WAV file generated: out/output.wav\n");
		} else {
			printf("✗ WAV conversion failed\n");
		}
	}

	if (wantsFlac) {
		status = convertMidiToFlac(midiPath, "out/output.flac");
		if (status == SUCCEEDED) {
			printf("✓ FLAC file generated: out/output.flac\n");
		} else {
			printf("✗ FLAC conversion failed\n");
		}
	}

	if (wantsMp3) {
		status = convertMidiToMp3(midiPath, "out/output.mp3");
		if (status == SUCCEEDED) {
			printf("✓ MP3 file generated: out/output.mp3\n");
		} else {
			printf("✗ MP3 conversion failed\n");
		}
	}

	logDebugging(_logger, "Output conversion stage finished.");
}


