#include "MidiGenerator.h"
#include "../semantic-analysis/SymbolTable.h"
#include "../semantic-analysis/SemanticAnalyzer.h"
#include <string.h>
#include <stdlib.h>

static Logger * _logger = NULL;
static FILE * _midiFile = NULL;

static void _shutdownMidiGeneratorModule(void) {
	if (_midiFile != NULL) {
		fclose(_midiFile);
		_midiFile = NULL;
	}
	if (_logger != NULL) {
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeMidiGeneratorModule() {
	_logger = createLogger("MidiGenerator");
	return _shutdownMidiGeneratorModule;
}

/* ============================================================================
 * MIDI BINARY OUTPUT FUNCTIONS
 * ============================================================================ */

/**
 * Write a single byte to the MIDI file.
 */
static void _writeByte(uint8_t byte) {
	fwrite(&byte, 1, 1, _midiFile);
}

/**
 * Write a 16-bit big-endian integer.
 */
static void _writeInt16(uint16_t value) {
	_writeByte((value >> 8) & 0xFF);
	_writeByte(value & 0xFF);
}

/**
 * Write a 32-bit big-endian integer.
 */
static void _writeInt32(uint32_t value) {
	_writeByte((value >> 24) & 0xFF);
	_writeByte((value >> 16) & 0xFF);
	_writeByte((value >> 8) & 0xFF);
	_writeByte(value & 0xFF);
}

/**
 * Write a variable-length quantity (VLQ) - MIDI timing format.
 * Used for delta times in MIDI events.
 */
static void _writeVarLen(uint32_t value) {
	uint32_t buffer = value & 0x7F;

	while ((value >>= 7) > 0) {
		buffer <<= 8;
		buffer |= 0x80;
		buffer += (value & 0x7F);
	}

	while (1) {
		_writeByte(buffer & 0xFF);
		if (buffer & 0x80) {
			buffer >>= 8;
		} else {
			break;
		}
	}
}

/**
 * Write a string (for chunk IDs like "MThd", "MTrk").
 */
static void _writeString(const char * str, int length) {
	fwrite(str, 1, length, _midiFile);
}

/* ============================================================================
 * MIDI HEADER GENERATION
 * ============================================================================ */

/**
 * Write MIDI file header chunk (MThd).
 * Format 1 = multiple tracks (one for metadata, one per instrument).
 */
static void _writeMidiHeader(int numTracks) {
	logDebugging(_logger, "Writing MIDI header (tracks=%d, TPQN=%d)", numTracks, MIDI_TICKS_PER_QUARTER_NOTE);

	_writeString("MThd", 4);           // Chunk ID
	_writeInt32(6);                     // Chunk size
	_writeInt16(1);                     // Format 1 (multi-track)
	_writeInt16(numTracks);             // Number of tracks
	_writeInt16(MIDI_TICKS_PER_QUARTER_NOTE); // Ticks per quarter note
}

/* ============================================================================
 * MIDI EVENT GENERATION
 * ============================================================================ */

/**
 * Write a MIDI tempo meta event.
 * Tempo in microseconds per quarter note.
 */
static void _writeTempoEvent(int bpm) {
	uint32_t microsecondsPerQuarter = 60000000 / bpm;
	logDebugging(_logger, "Writing tempo event: %d BPM (%d μs/quarter)", bpm, microsecondsPerQuarter);

	_writeVarLen(0);          // Delta time = 0 (immediate)
	_writeByte(MIDI_META_EVENT);
	_writeByte(MIDI_SET_TEMPO);
	_writeByte(3);            // Length = 3 bytes
	_writeByte((microsecondsPerQuarter >> 16) & 0xFF);
	_writeByte((microsecondsPerQuarter >> 8) & 0xFF);
	_writeByte(microsecondsPerQuarter & 0xFF);
}

/**
 * Write a MIDI time signature meta event.
 */
static void _writeTimeSignatureEvent(int numerator, int denominator) {
	logDebugging(_logger, "Writing time signature: %d/%d", numerator, denominator);

	_writeVarLen(0);          // Delta time = 0
	_writeByte(MIDI_META_EVENT);
	_writeByte(MIDI_TIME_SIGNATURE);
	_writeByte(4);            // Length = 4 bytes
	_writeByte(numerator);    // Numerator
	_writeByte(2);            // Denominator (2 = quarter note, 2^2 = 4)
	_writeByte(24);           // MIDI clocks per metronome click
	_writeByte(8);            // 32nd notes per quarter note
}

/**
 * Write end-of-track meta event.
 */
static void _writeEndOfTrack() {
	_writeVarLen(0);          // Delta time = 0
	_writeByte(MIDI_META_EVENT);
	_writeByte(MIDI_END_OF_TRACK);
	_writeByte(0);            // Length = 0
}

/**
 * Write a MIDI note on event.
 */
static void _writeNoteOn(uint32_t deltaTime, uint8_t channel, uint8_t note, uint8_t velocity) {
	_writeVarLen(deltaTime);
	_writeByte(MIDI_NOTE_ON | channel);
	_writeByte(note);
	_writeByte(velocity);
}

/**
 * Write a MIDI note off event.
 */
static void _writeNoteOff(uint32_t deltaTime, uint8_t channel, uint8_t note) {
	_writeVarLen(deltaTime);
	_writeByte(MIDI_NOTE_OFF | channel);
	_writeByte(note);
	_writeByte(0);            // Velocity = 0
}

/* ============================================================================
 * RHYTHM EXPANSION
 * ============================================================================ */

/**
 * Expanded rhythm element with timing information.
 */
typedef struct {
	RhythmElementType type;
	char * noteValue;         // For ELEMENT_NOTE
	int position;             // Position in the expanded sequence
} ExpandedElement;

typedef struct {
	ExpandedElement * elements;
	int count;
	int capacity;
} ExpandedRhythm;

static ExpandedRhythm * _createExpandedRhythm() {
	ExpandedRhythm * rhythm = (ExpandedRhythm *) calloc(1, sizeof(ExpandedRhythm));
	rhythm->capacity = 16;
	rhythm->count = 0;
	rhythm->elements = (ExpandedElement *) calloc(rhythm->capacity, sizeof(ExpandedElement));
	return rhythm;
}

static void _destroyExpandedRhythm(ExpandedRhythm * rhythm) {
	if (rhythm != NULL) {
		// Free note values in elements
		for (int i = 0; i < rhythm->count; i++) {
			if (rhythm->elements[i].noteValue != NULL) {
				free(rhythm->elements[i].noteValue);
			}
		}
		free(rhythm->elements);
		free(rhythm);
	}
}

static void _addExpandedElement(ExpandedRhythm * rhythm, RhythmElementType type, const char * noteValue) {
	if (rhythm->count >= rhythm->capacity) {
		rhythm->capacity *= 2;
		rhythm->elements = (ExpandedElement *) realloc(rhythm->elements, rhythm->capacity * sizeof(ExpandedElement));
	}

	rhythm->elements[rhythm->count].type = type;
	rhythm->elements[rhythm->count].noteValue = noteValue ? strdup(noteValue) : NULL;
	rhythm->elements[rhythm->count].position = rhythm->count;
	rhythm->count++;
}

/**
 * Expand a rhythm array into a flat sequence.
 */
static void _expandRhythmArray(ExpandedRhythm * result, RhythmArray * array) {
	if (array == NULL || array->elements == NULL) {
		return;
	}

	RhythmElementList * current = array->elements;
	while (current != NULL) {
		if (current->element != NULL) {
			_addExpandedElement(result, current->element->type, current->element->noteValue);
		}
		current = current->next;
	}
}

/**
 * Recursively expand a rhythm expression (handles +, *, arrays).
 */
static void _expandRhythmExpression(ExpandedRhythm * result, RhythmExpression * expression) {
	if (expression == NULL) {
		return;
	}

	switch (expression->type) {
		case RHYTHM_ARRAY:
			_expandRhythmArray(result, expression->array);
			break;

		case RHYTHM_CONCATENATION:
			_expandRhythmExpression(result, expression->concatenation.left);
			_expandRhythmExpression(result, expression->concatenation.right);
			break;

		case RHYTHM_REPETITION: {
			int repetitions = expression->repetition.repetitions;
			for (int i = 0; i < repetitions; i++) {
				_expandRhythmArray(result, expression->repetition.array);
			}
			break;
		}
	}
}

/* ============================================================================
 * NOTE MAPPING
 * ============================================================================ */

/**
 * Convert a note string (e.g., "E2", "A#2", "Gb3") to MIDI note number.
 * Formula: noteNumber = 12 + (octave * 12) + pitchClass
 * C0=12, C#0=13, ..., C1=24, ..., A2=45, etc.
 */
static uint8_t _noteToMidiNumber(const char * note) {
	if (note == NULL || strlen(note) < 2) {
		return 60; // Default to middle C
	}

	// Parse note letter (C=0, D=2, E=4, F=5, G=7, A=9, B=11)
	int pitchClass = 0;
	switch (note[0]) {
		case 'C': pitchClass = 0; break;
		case 'D': pitchClass = 2; break;
		case 'E': pitchClass = 4; break;
		case 'F': pitchClass = 5; break;
		case 'G': pitchClass = 7; break;
		case 'A': pitchClass = 9; break;
		case 'B': pitchClass = 11; break;
		default: return 60;
	}

	// Parse accidental
	int offset = 1;
	if (note[1] == '#') {
		pitchClass++;
		offset = 2;
	} else if (note[1] == 'b') {
		pitchClass--;
		offset = 2;
	}

	// Parse octave
	int octave = atoi(&note[offset]);

	int midiNote = 12 + (octave * 12) + pitchClass;
	return (uint8_t) midiNote;
}

/* ============================================================================
 * TRACK GENERATION
 * ============================================================================ */

/**
 * Generate the metadata track (track 0).
 * Contains tempo and time signature.
 */
static long _generateMetadataTrack(Program * program) {
	logDebugging(_logger, "Generating metadata track...");

	long trackStartPos = ftell(_midiFile);
	_writeString("MTrk", 4);
	_writeInt32(0);  // Placeholder for chunk size

	long dataStartPos = ftell(_midiFile);

	// Write tempo
	_writeTempoEvent(program->declarations->tempo);

	// Write time signature (steps/4)
	_writeTimeSignatureEvent(program->declarations->steps, 4);

	// End of track
	_writeEndOfTrack();

	// Calculate and write chunk size
	long dataEndPos = ftell(_midiFile);
	long chunkSize = dataEndPos - dataStartPos;
	fseek(_midiFile, trackStartPos + 4, SEEK_SET);
	_writeInt32(chunkSize);
	fseek(_midiFile, dataEndPos, SEEK_SET);

	logDebugging(_logger, "Metadata track written (%ld bytes)", chunkSize);
	return chunkSize;
}

/**
 * Check if a compass is in the active range list.
 */
static bool _isCompassActive(int compass, ActiveRange * activeRange) {
	ActiveRange * current = activeRange;
	while (current != NULL) {
		if (compass >= current->start && compass <= current->end) {
			return true;
		}
		current = current->next;
	}
	return false;
}

/**
 * Generate a track for a single instrument.
 */
static long _generateInstrumentTrack(Program * program, Instrument * instrument, Pattern * pattern) {
	logDebugging(_logger, "Generating track for instrument '%s'...", instrument->name);

	long trackStartPos = ftell(_midiFile);
	_writeString("MTrk", 4);
	_writeInt32(0);  // Placeholder

	long dataStartPos = ftell(_midiFile);

	// Expand the pattern's rhythm
	ExpandedRhythm * rhythm = _createExpandedRhythm();
	_expandRhythmExpression(rhythm, pattern->rhythm);

	logDebugging(_logger, "Expanded rhythm: %d elements", rhythm->count);

	// Calculate timing
	int steps = program->declarations->steps;
	int compasses = program->declarations->compasses;
	// Each step = 1 quarter note (since time signature is steps/4)
	int ticksPerStep = MIDI_TICKS_PER_QUARTER_NOTE;

	// Determine channel and note
	uint8_t channel = MIDI_PERCUSSION_CHANNEL;
	bool isPercussion = (strcmp(instrument->name, "kick") == 0 ||
	                     strcmp(instrument->name, "snare") == 0 ||
	                     strcmp(instrument->name, "hihat") == 0);

	if (!isPercussion) {
		channel = MIDI_MELODIC_CHANNEL;
	}

	// Calculate how many compasses the expanded rhythm spans
	// Each compass should have 'steps' number of elements
	int totalRhythmCompasses = (rhythm->count + steps - 1) / steps;  // Ceiling division

	logDebugging(_logger, "Rhythm spans %d compasses (%d elements / %d steps)",
		totalRhythmCompasses, rhythm->count, steps);

	// Generate events for each compass
	uint32_t currentTick = 0;
	uint32_t lastEventTick = 0;
	int rhythmElementIndex = 0;  // Track position in the expanded rhythm

	for (int compass = 1; compass <= compasses; compass++) {
		if (!_isCompassActive(compass, instrument->activeRange)) {
			// Skip inactive compass - just advance time
			currentTick += steps * ticksPerStep;
			continue;
		}

		// Calculate which slice of the rhythm to play in this compass
		// The rhythm repeats/cycles through the compasses
		int rhythmCompassIndex = (compass - 1) % totalRhythmCompasses;
		int startElementIndex = rhythmCompassIndex * steps;
		int endElementIndex = startElementIndex + steps;

		// Make sure we don't go past the end of the rhythm
		if (endElementIndex > rhythm->count) {
			endElementIndex = rhythm->count;
		}

		logDebugging(_logger, "Compass %d: playing rhythm elements %d-%d (currentTick=%u)",
			compass, startElementIndex, endElementIndex - 1, currentTick);

		// Generate events for this compass (steps elements)
		for (int i = startElementIndex; i < endElementIndex; i++) {
			ExpandedElement * element = &rhythm->elements[i];

			if (element->type == ELEMENT_SILENCE) {
				// Silence: just advance time
				currentTick += ticksPerStep;
				continue;
			}

			// Determine MIDI note
			uint8_t midiNote = MIDI_BASS_DRUM_NOTE;
			if (element->type == ELEMENT_NOTE && element->noteValue != NULL) {
				midiNote = _noteToMidiNumber(element->noteValue);
			}

			// Note On
			uint32_t deltaTime = currentTick - lastEventTick;
			_writeNoteOn(deltaTime, channel, midiNote, 100);
			lastEventTick = currentTick;

			// Note Off (after one step)
			uint32_t noteOffTick = currentTick + ticksPerStep;
			deltaTime = noteOffTick - lastEventTick;
			_writeNoteOff(deltaTime, channel, midiNote);
			lastEventTick = noteOffTick;

			currentTick = noteOffTick;
		}

		// Pad the rest of the compass with silence if needed
		int elementsPlayed = endElementIndex - startElementIndex;
		if (elementsPlayed < steps) {
			int silenceTicks = (steps - elementsPlayed) * ticksPerStep;
			currentTick += silenceTicks;
		}
	}

	// End of track
	_writeEndOfTrack();

	_destroyExpandedRhythm(rhythm);

	long dataEndPos = ftell(_midiFile);
	long chunkSize = dataEndPos - dataStartPos;
	fseek(_midiFile, trackStartPos + 4, SEEK_SET);
	_writeInt32(chunkSize);
	fseek(_midiFile, dataEndPos, SEEK_SET);

	logDebugging(_logger, "Instrument track written (%ld bytes)", chunkSize);
	return chunkSize;
}

/* ============================================================================
 * MAIN GENERATION FUNCTION
 * ============================================================================ */

CompilationStatus generateMidiFile(Program * program, const char * outputFilename) {
	if (program == NULL || outputFilename == NULL) {
		logError(_logger, "NULL program or filename");
		return FAILED;
	}

	logInformation(_logger, "Generating MIDI file: %s", outputFilename);

	// Open output file
	_midiFile = fopen(outputFilename, "wb");
	if (_midiFile == NULL) {
		logError(_logger, "Failed to open output file: %s", outputFilename);
		return FAILED;
	}

	// Count tracks: 1 metadata + N instruments
	int numInstruments = 0;
	InstrumentList * instList = program->instruments;
	while (instList != NULL) {
		numInstruments++;
		instList = instList->next;
	}

	int numTracks = 1 + numInstruments;
	logInformation(_logger, "Total tracks: %d (1 metadata + %d instruments)", numTracks, numInstruments);

	// Write MIDI header
	_writeMidiHeader(numTracks);

	// Write metadata track
	_generateMetadataTrack(program);

	// Write instrument tracks
	instList = program->instruments;
	SymbolTable * symbolTable = getSymbolTable();

	while (instList != NULL && instList->instrument != NULL) {
		Instrument * instrument = instList->instrument;
		Pattern * pattern = lookupPattern(symbolTable, instrument->patternName);

		if (pattern == NULL) {
			logError(_logger, "Pattern '%s' not found for instrument '%s'",
			         instrument->patternName, instrument->name);
			fclose(_midiFile);
			_midiFile = NULL;
			return FAILED;
		}

		_generateInstrumentTrack(program, instrument, pattern);
		instList = instList->next;
	}

	fclose(_midiFile);
	_midiFile = NULL;

	logInformation(_logger, "MIDI file generated successfully: %s", outputFilename);
	return SUCCEEDED;
}
