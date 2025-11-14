#ifndef MIDI_GENERATOR_HEADER
#define MIDI_GENERATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdio.h>
#include <stdint.h>

/**
 * MIDI File Generator for Drum Machine DSL.
 *
 * Generates Standard MIDI Format (SMF) files from the validated AST.
 *
 * MIDI File Structure:
 * - Header chunk (MThd): Format, number of tracks, ticks per quarter note
 * - Track chunks (MTrk): One per instrument + one for tempo/time signature
 *
 * Output: self-contained .mid file that can be played by any MIDI player.
 */

/** Initialize module's internal state. */
ModuleDestructor initializeMidiGeneratorModule();

/**
 * Generate a MIDI file from the program AST.
 *
 * @param program The validated program AST
 * @param outputFilename The output .mid filename
 * @return SUCCEEDED if MIDI file generated successfully, FAILED otherwise
 */
CompilationStatus generateMidiFile(Program * program, const char * outputFilename);

/**
 * MIDI Constants
 */
#define MIDI_TICKS_PER_QUARTER_NOTE 480
#define MIDI_PERCUSSION_CHANNEL 9  // Channel 10 (0-indexed = 9)
#define MIDI_MELODIC_CHANNEL 0     // Channel 1 (0-indexed = 0)
#define MIDI_BASS_DRUM_NOTE 36     // General MIDI bass drum

/**
 * MIDI Event Types
 */
#define MIDI_NOTE_OFF 0x80
#define MIDI_NOTE_ON 0x90
#define MIDI_META_EVENT 0xFF
#define MIDI_SET_TEMPO 0x51
#define MIDI_TIME_SIGNATURE 0x58
#define MIDI_END_OF_TRACK 0x2F

#endif
