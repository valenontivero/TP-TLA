#include "AudioConverter.h"
#include <sndfile.h>
#include <lame/lame.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

static Logger * _logger = NULL;

static void _shutdownAudioConverterModule(void) {
	if (_logger != NULL) {
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAudioConverterModule() {
	_logger = createLogger("AudioConverter");
	return _shutdownAudioConverterModule;
}

/**
 * MIDI to WAV using fluidsynth command-line tool.
 * Then read the WAV file and return PCM samples.
 */
static int16_t * _midiToPcm(const char * midiFilename, int * sampleCount, int * sampleRate) {
	logDebugging(_logger, "Converting MIDI to PCM using fluidsynth...");

	// Create temporary WAV file
	char tempWavFile[256] = "/tmp/midi_temp_XXXXXX";
	int fd = mkstemp(tempWavFile);
	if (fd < 0) {
		logError(_logger, "Failed to create temporary file");
		return NULL;
	}
	close(fd);
	// Rename to .wav extension
	char tempWavFileFinal[256];
	snprintf(tempWavFileFinal, sizeof(tempWavFileFinal), "%s.wav", tempWavFile);
	rename(tempWavFile, tempWavFileFinal);

	// Find soundfont
	const char * soundfontPaths[] = {
		"/usr/share/sounds/sf2/FluidR3_GM.sf2",
		"/usr/share/sounds/sf2/default.sf2",
		"/usr/share/sounds/sf2/FluidR3_GS.sf2",
		"/usr/share/sounds/sf2/FluidR3_GM2-2.sf2",
		"/usr/share/sounds/sf2/TimGM6mb.sf2",
		NULL
	};

	const char * soundfont = NULL;
	for (int i = 0; soundfontPaths[i] != NULL; i++) {
		FILE * test = fopen(soundfontPaths[i], "r");
		if (test != NULL) {
			fclose(test);
			soundfont = soundfontPaths[i];
			logDebugging(_logger, "Using soundfont: %s", soundfont);
			break;
		}
	}

	if (soundfont == NULL) {
		logError(_logger, "No soundfont found. MIDI playback may be silent.");
		logError(_logger, "Please ensure a soundfont is available in /usr/share/sounds/sf2/");
		// Continue anyway
		soundfont = "/usr/share/sounds/sf2/FluidR3_GM.sf2"; // Try default
	}

	// Run fluidsynth command: fluidsynth -F output.wav soundfont.sf2 input.mid
	char command[1024];
	if (soundfont != NULL) {
		snprintf(command, sizeof(command), 
			"fluidsynth -F %s -q %s %s 2>&1",
			tempWavFileFinal, soundfont, midiFilename);
	} else {
		snprintf(command, sizeof(command),
			"fluidsynth -F %s -q %s 2>&1",
			tempWavFileFinal, midiFilename);
	}

	logDebugging(_logger, "Executing: %s", command);
	int result = system(command);
	if (result != 0) {
		logError(_logger, "fluidsynth command failed with exit code %d", result);
		unlink(tempWavFileFinal);
		return NULL;
	}

	// Read the WAV file using libsndfile
	SNDFILE * file = NULL;
	SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(sfinfo));

	file = sf_open(tempWavFileFinal, SFM_READ, &sfinfo);
	if (file == NULL) {
		logError(_logger, "Failed to open temporary WAV file: %s (%s)", tempWavFileFinal, sf_strerror(NULL));
		unlink(tempWavFileFinal);
		return NULL;
	}

	logDebugging(_logger, "WAV file info: %d Hz, %d channels, %ld frames", 
		sfinfo.samplerate, sfinfo.channels, sfinfo.frames);

	// Allocate buffer for all samples
	int totalSamples = sfinfo.frames * sfinfo.channels;
	int16_t * samples = (int16_t *) malloc(totalSamples * sizeof(int16_t));
	if (samples == NULL) {
		logError(_logger, "Failed to allocate sample buffer");
		sf_close(file);
		unlink(tempWavFile);
		return NULL;
	}

	// Read all samples
	sf_count_t framesRead = sf_readf_short(file, samples, sfinfo.frames);
	if (framesRead != sfinfo.frames) {
		logError(_logger, "Failed to read all samples: read %ld of %ld frames", framesRead, sfinfo.frames);
		free(samples);
		sf_close(file);
		unlink(tempWavFile);
		return NULL;
	}

	sf_close(file);
	unlink(tempWavFileFinal); // Clean up temporary file

	// Convert to stereo if needed
	int16_t * finalSamples = samples;
	if (sfinfo.channels == 1) {
		// Convert mono to stereo
		totalSamples = sfinfo.frames * AUDIO_CHANNELS;
		finalSamples = (int16_t *) malloc(totalSamples * sizeof(int16_t));
		if (finalSamples == NULL) {
			logError(_logger, "Failed to allocate stereo buffer");
			free(samples);
			return NULL;
		}
		for (int i = 0; i < sfinfo.frames; i++) {
			finalSamples[i * 2] = samples[i];
			finalSamples[i * 2 + 1] = samples[i];
		}
		free(samples);
	} else if (sfinfo.channels != AUDIO_CHANNELS) {
		logError(_logger, "Unsupported channel count: %d (expected %d)", sfinfo.channels, AUDIO_CHANNELS);
		free(samples);
		return NULL;
	}

	*sampleCount = totalSamples;
	*sampleRate = sfinfo.samplerate;

	logDebugging(_logger, "MIDI to PCM conversion complete: %d samples at %d Hz", totalSamples, *sampleRate);
	return finalSamples;
}

/* ============================================================================
 * WAV/FLAC OUTPUT USING LIBSNDFILE
 * ============================================================================ */

/**
 * Write PCM samples to WAV or FLAC file using libsndfile.
 */
static CompilationStatus _writeAudioFile(const char * filename, int16_t * samples, int sampleCount, int sampleRate, int format) {
	SNDFILE * file = NULL;
	SF_INFO sfinfo;

	memset(&sfinfo, 0, sizeof(sfinfo));
	sfinfo.samplerate = sampleRate;
	sfinfo.channels = AUDIO_CHANNELS;
	sfinfo.format = format;

	file = sf_open(filename, SFM_WRITE, &sfinfo);
	if (file == NULL) {
		logError(_logger, "Failed to open audio file for writing: %s (%s)", filename, sf_strerror(NULL));
		return FAILED;
	}

	sf_count_t framesWritten = sf_writef_short(file, samples, sampleCount / AUDIO_CHANNELS);
	if (framesWritten != (sampleCount / AUDIO_CHANNELS)) {
		logError(_logger, "Failed to write all samples to %s", filename);
		sf_close(file);
		return FAILED;
	}

	sf_close(file);
	logInformation(_logger, "Successfully wrote audio file: %s (%ld frames)", filename, framesWritten);
	return SUCCEEDED;
}

CompilationStatus convertMidiToWav(const char * midiFilename, const char * wavFilename) {
	if (midiFilename == NULL || wavFilename == NULL) {
		logError(_logger, "NULL filename provided");
		return FAILED;
	}

	logInformation(_logger, "Converting MIDI to WAV: %s -> %s", midiFilename, wavFilename);

	int sampleCount = 0;
	int sampleRate = 0;
	int16_t * samples = _midiToPcm(midiFilename, &sampleCount, &sampleRate);

	if (samples == NULL || sampleCount == 0) {
		logError(_logger, "Failed to convert MIDI to PCM");
		return FAILED;
	}

	// Write WAV file (SF_FORMAT_WAV | SF_FORMAT_PCM_16)
	CompilationStatus status = _writeAudioFile(wavFilename, samples, sampleCount, sampleRate,
	                                          SF_FORMAT_WAV | SF_FORMAT_PCM_16);

	free(samples);
	return status;
}

CompilationStatus convertMidiToFlac(const char * midiFilename, const char * flacFilename) {
	if (midiFilename == NULL || flacFilename == NULL) {
		logError(_logger, "NULL filename provided");
		return FAILED;
	}

	logInformation(_logger, "Converting MIDI to FLAC: %s -> %s", midiFilename, flacFilename);

	int sampleCount = 0;
	int sampleRate = 0;
	int16_t * samples = _midiToPcm(midiFilename, &sampleCount, &sampleRate);

	if (samples == NULL || sampleCount == 0) {
		logError(_logger, "Failed to convert MIDI to PCM");
		return FAILED;
	}

	// Write FLAC file (SF_FORMAT_FLAC | SF_FORMAT_PCM_16)
	CompilationStatus status = _writeAudioFile(flacFilename, samples, sampleCount, sampleRate,
	                                          SF_FORMAT_FLAC | SF_FORMAT_PCM_16);

	free(samples);
	return status;
}

/* ============================================================================
 * MP3 OUTPUT USING LAME
 * ============================================================================ */

CompilationStatus convertMidiToMp3(const char * midiFilename, const char * mp3Filename) {
	if (midiFilename == NULL || mp3Filename == NULL) {
		logError(_logger, "NULL filename provided");
		return FAILED;
	}

	logInformation(_logger, "Converting MIDI to MP3: %s -> %s", midiFilename, mp3Filename);

	int sampleCount = 0;
	int sampleRate = 0;
	int16_t * samples = _midiToPcm(midiFilename, &sampleCount, &sampleRate);

	if (samples == NULL || sampleCount == 0) {
		logError(_logger, "Failed to convert MIDI to PCM");
		return FAILED;
	}

	// Initialize LAME encoder
	lame_t lame = lame_init();
	if (lame == NULL) {
		logError(_logger, "Failed to initialize LAME encoder");
		free(samples);
		return FAILED;
	}

	// Configure LAME
	lame_set_in_samplerate(lame, sampleRate);
	lame_set_VBR(lame, vbr_default);
	lame_set_VBR_q(lame, 2);  // Quality 0-9, 2 is good quality
	lame_set_brate(lame, 192); // 192 kbps
	lame_set_mode(lame, STEREO);
	lame_set_quality(lame, 2); // Quality 0-9

	if (lame_init_params(lame) < 0) {
		logError(_logger, "Failed to initialize LAME parameters");
		lame_close(lame);
		free(samples);
		return FAILED;
	}

	// Open output file
	FILE * mp3File = fopen(mp3Filename, "wb");
	if (mp3File == NULL) {
		logError(_logger, "Failed to open MP3 file for writing: %s", mp3Filename);
		lame_close(lame);
		free(samples);
		return FAILED;
	}

	// Separate left and right channels
	int numFrames = sampleCount / AUDIO_CHANNELS;
	int16_t * leftChannel = (int16_t *) malloc(numFrames * sizeof(int16_t));
	int16_t * rightChannel = (int16_t *) malloc(numFrames * sizeof(int16_t));

	if (leftChannel == NULL || rightChannel == NULL) {
		logError(_logger, "Failed to allocate channel buffers");
		free(leftChannel);
		free(rightChannel);
		fclose(mp3File);
		lame_close(lame);
		free(samples);
		return FAILED;
	}

	// De-interleave samples
	for (int i = 0; i < numFrames; i++) {
		leftChannel[i] = samples[i * AUDIO_CHANNELS];
		rightChannel[i] = samples[i * AUDIO_CHANNELS + 1];
	}

	// Encode to MP3
	int mp3BufferSize = (int) (1.25 * numFrames + 7200);
	unsigned char * mp3Buffer = (unsigned char *) malloc(mp3BufferSize);
	if (mp3Buffer == NULL) {
		logError(_logger, "Failed to allocate MP3 buffer");
		free(leftChannel);
		free(rightChannel);
		fclose(mp3File);
		lame_close(lame);
		free(samples);
		return FAILED;
	}

	int mp3Bytes = lame_encode_buffer(lame, leftChannel, rightChannel, numFrames, mp3Buffer, mp3BufferSize);
	if (mp3Bytes < 0) {
		logError(_logger, "LAME encoding error: %d", mp3Bytes);
		free(mp3Buffer);
		free(leftChannel);
		free(rightChannel);
		fclose(mp3File);
		lame_close(lame);
		free(samples);
		return FAILED;
	}

	// Write encoded data
	if (fwrite(mp3Buffer, 1, mp3Bytes, mp3File) != (size_t) mp3Bytes) {
		logError(_logger, "Failed to write MP3 data");
		free(mp3Buffer);
		free(leftChannel);
		free(rightChannel);
		fclose(mp3File);
		lame_close(lame);
		free(samples);
		return FAILED;
	}

	// Flush encoder
	int flushBytes = lame_encode_flush(lame, mp3Buffer, mp3BufferSize);
	if (flushBytes > 0) {
		fwrite(mp3Buffer, 1, flushBytes, mp3File);
	}

	// Cleanup
	free(mp3Buffer);
	free(leftChannel);
	free(rightChannel);
	fclose(mp3File);
	lame_close(lame);
	free(samples);

	logInformation(_logger, "Successfully wrote MP3 file: %s", mp3Filename);
	return SUCCEEDED;
}
