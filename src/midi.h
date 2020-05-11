#ifndef _MIDI_
#define _MIDI_
#include <land/land.h>
#include <string.h>
#include <stdio.h>
typedef enum NoteType NoteType;
typedef struct MidiNote MidiNote;
typedef struct MidiFile MidiFile;
enum NoteType {
    NOTE=0,
    BPM=1,
    VOL=2,
    PAN=3
};
struct MidiNote {
    NoteType com;
    int n /* data 1 */;
    int v /* data 2 */;
    int t /* Position in MIDI ticks */;
    int d;
    int c /* MIDI channel (0 - 15) */;
};
struct MidiFile {
    int n;
    int division /* How many midi clocks in a minute. */;
    MidiNote * notes;
};
#include "midi.h"
MidiFile* midifile_new(void);
void midifile_del(MidiFile * self);
void midifile_addbpm(MidiFile * self, int bpm, int pos);
void midifile_addvol(MidiFile * self, int v, int channel, int pos);
void midifile_addpan(MidiFile * self, int v, int channel, int pos);
void midifile_addnote(MidiFile * self, int note, int channel, int v, int pos);
void midifile_sort_notes(MidiFile * self);
MidiFile* midifile_load(str name);
#endif
