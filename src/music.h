#ifndef _MUSIC_
#define _MUSIC_
#include <stdio.h>
#include <land/land.h>
#include <math.h>
typedef struct Voice Voice;
typedef struct Music Music;
#include "midi.h"
struct Voice {
    int sample_start;
    double midi_duration;
    float frequency;
    float pan_left, pan_right;
    float volume;
};
struct Music {
    LandStream * audiostream;
    int sample_position /* song position in samples */;
    double midi_position /* song position in midi ticks */;
    double midi_speed /* how fast midi_position proceeds per sample */;
    double bpm;
    int frequency;
    int buffer_size, fragments;
    int bits;
    float volume [16];
    float pan [16];
    int paused;
    MidiFile * midi;
    int last_note;
    int active_voices_count;
    int voices_allocated;
    Voice * voices;
    int total_seconds;
};
double samples_to_ticks(Music * self, double samples);
double ticks_to_samples(Music * self, double ticks);
Music* music_new(int frequency, int buffer_size, int fragments, int bits);
void music_pause(Music * self);
void music_unpause(Music * self);
void music_toggle(Music * self);
void music_del(Music * self);
void music_poll(Music * self);
void music_simulate(Music * self);
#endif
