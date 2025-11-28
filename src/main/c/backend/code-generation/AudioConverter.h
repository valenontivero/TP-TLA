#ifndef AUDIO_CONVERTER_HEADER
#define AUDIO_CONVERTER_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdint.h>

/**
 * Audio Converter for MIDI to Audio Format Conversion.
 *
 * Converts MIDI files to WAV, FLAC, and MP3 formats using:
 * - FluidSynth: MIDI to PCM synthesis
 * - libsndfile: WAV/FLAC output
 * - LAME: MP3 encoding
 */

ModuleDestructor initializeAudioConverterModule();

CompilationStatus convertMidiToWav(const char * midiFilename, const char * wavFilename);

CompilationStatus convertMidiToFlac(const char * midiFilename, const char * flacFilename);

CompilationStatus convertMidiToMp3(const char * midiFilename, const char * mp3Filename);

/**
 * Audio Configuration Constants
 */
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_CHANNELS 2  // Stereo
#define AUDIO_BITS_PER_SAMPLE 16

#endif

