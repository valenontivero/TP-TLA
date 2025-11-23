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

/** Initialize module's internal state. */
ModuleDestructor initializeAudioConverterModule();

/**
 * Convert a MIDI file to WAV format.
 *
 * @param midiFilename Input MIDI file path
 * @param wavFilename Output WAV file path
 * @return SUCCEEDED if conversion successful, FAILED otherwise
 */
CompilationStatus convertMidiToWav(const char * midiFilename, const char * wavFilename);

/**
 * Convert a MIDI file to FLAC format.
 *
 * @param midiFilename Input MIDI file path
 * @param flacFilename Output FLAC file path
 * @return SUCCEEDED if conversion successful, FAILED otherwise
 */
CompilationStatus convertMidiToFlac(const char * midiFilename, const char * flacFilename);

/**
 * Convert a MIDI file to MP3 format.
 *
 * @param midiFilename Input MIDI file path
 * @param mp3Filename Output MP3 file path
 * @return SUCCEEDED if conversion successful, FAILED otherwise
 */
CompilationStatus convertMidiToMp3(const char * midiFilename, const char * mp3Filename);

/**
 * Audio Configuration Constants
 */
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_CHANNELS 2  // Stereo
#define AUDIO_BITS_PER_SAMPLE 16

#endif

