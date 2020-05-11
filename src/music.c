#include "music.h"
static float frequency_table [128] /* midi note frequency lookup */;
static void init_frequency_table(void);
static void init_frequency_table(void) {
    int i;
    for (i = 0; i < 128; i++) {
        // Fixme: Might be exactly one octave too high!
        frequency_table [i] = 16.35159783 * pow(2.0, i / 12.0);
    }
}
double samples_to_ticks(Music * self, double samples) {
    // 1 samples = (1 / frequency) seconds
    // 1 ticks = (60 / (BPM * div)) seconds
    // 1 seconds = 1 ticks / (60 / (BPM * div))
    // 1 samples = (1 / frequency) ticks / (60 / (BPM * div)) =
    //             (BPM * div) / (60 * frequency) ticks
    //
    return samples * (self->bpm * self->midi->division) / (60.0 * self->frequency);
}
double ticks_to_samples(Music * self, double ticks) {
    // 1 samples = (1 / frequency) seconds
    // 1 ticks = (60 / (BPM * div)) seconds
    // 1 seconds = 1 samples / (1 / frequency)
    // 1 ticks = (60 / (BPM * div)) samples / (1 / frequency)
    //         = (60 * frequency) / (BPM * div) ticks
    //
    return ticks * (60.0 * self->frequency) / (self->bpm * self->midi->division);
}
Music* music_new(int frequency, int buffer_size, int fragments, int bits) {
    Music * self = calloc(1, sizeof (* self));
    init_frequency_table();
    self->frequency = frequency;
    self->buffer_size = buffer_size;
    self->fragments = fragments;
    self->bits = bits;
    self->audiostream = land_stream_new(buffer_size, fragments, frequency, bits, 2);
    self->midi_position = 0;
    self->bpm = 120;
    self->midi_speed = 0;
    self->paused = 1;
    int i;
    for (i = 0; i < 16; i++) {
        self->volume [i] = 1.0;
        self->pan [i] = 0.5;
    }
    return self;
}
void music_pause(Music * self) {
    self->paused = 1;
}
void music_unpause(Music * self) {
    self->paused = 0;
}
void music_toggle(Music * self) {
    self->paused = ! self->paused;
}
void music_del(Music * self) {
    land_stream_destroy(self->audiostream);
    free(self);
}
void music_poll(Music * self) {
    void * buffer = land_stream_buffer(self->audiostream);
    if (buffer) {
        int p;
        if (! self->midi_speed) {
            self->midi_speed = samples_to_ticks(self, 1);
        }
        for (p = 0; p < self->buffer_size; p++) {
            float vl = 0, vr = 0;
            if (! self->paused) {
                if (self->last_note == self->midi->n) {
                    self->last_note = 0;
                    self->midi_position = 0;
                    self->sample_position = 0;
                }
                while (self->last_note < self->midi->n) {
                    MidiNote * note = & self->midi->notes [self->last_note];
                    if (self->midi_position >= note->t) {
                        if (note->com == BPM) {
                            self->bpm = note->n;
                            self->midi_speed = samples_to_ticks(self, 1);
                        }
                        else if (note->com == VOL) {
                            self->volume [note->c] = note->n / 128.0;
                        }
                        else if (note->com == PAN) {
                            self->pan [note->c] = (note->n - 64) / 64.0;
                        }
                        else if (note->com == NOTE) {
                            int i;
                            self->active_voices_count++;
                            if (self->active_voices_count > self->voices_allocated) {
                                i = self->voices_allocated;
                                self->voices_allocated++;
                                self->voices = realloc(self->voices, self->voices_allocated * sizeof (* self->voices));
                            }
                            else {
                                for (i = 0; i < self->voices_allocated; i++) {
                                    if (! self->voices [i].frequency) {
                                        break;
                                    }
                                }
                            }
                            self->voices [i].sample_start = self->sample_position;
                            self->voices [i].frequency = frequency_table [note->n];
                            self->voices [i].midi_duration = note->d;
                            self->voices [i].volume = note->v / 128.0;
                            self->voices [i].volume *= self->volume [note->c];
                            //  FIXME: Pan and volume changes must affect already
                            //  playing notes..
                            if (self->pan [note->c] >= 0) {
                                self->voices [i].pan_left = 1 - self->pan [note->c];
                                self->voices [i].pan_right = 1;
                            }
                            else {
                                self->voices [i].pan_left = 1;
                                self->voices [i].pan_right = 1 + self->pan [note->c];
                            }
                        }
                        self->last_note++;
                    }
                    else {
                        break;
                    }
                }
                int i;
                for (i = 0; i < self->voices_allocated; i++) {
                    if (! self->voices [i].frequency) {
                        continue;
                    }
                    int pos = self->sample_position - self->voices [i].sample_start;
                    float x = 2.0 * self->voices [i].frequency * pos / (float) self->frequency;
                    //float s = (1 - 2 * ((int)(x) & 1))
                    //float s = (x))
                    //s -= (int)s
                    //s = 1 - 2 * s
                    float s = x / 4;
                    s -= (int) s;
                    s = 2 * (s - 0.5);
                    vl += self->voices [i].volume * (self->voices [i].pan_left) * s;
                    vr += self->voices [i].volume * (self->voices [i].pan_right) * s;
                    self->voices [i].midi_duration -= self->midi_speed;
                    if (self->voices [i].midi_duration < 0) {
                        self->voices [i].volume -= 0.001;
                        if (self->voices [i].volume < 0) {
                            self->voices [i].frequency = 0;
                            self->active_voices_count--;
                        }
                    }
                }
                self->sample_position++;
                self->midi_position += self->midi_speed;
            }
            //int pos = self->sample_position
            //vl = vr = sin(pos * 2 * LAND_PI * 440 / self->frequency)
            //vl = vr = (1 - 2 * ((int)(2.0 * 440 * pos / (float)self->frequency) & 1))
            if (self->bits == 8) {
                ((unsigned char *) buffer) [p * 2] = 127.5 + vl * 127.5;
                ((unsigned char *) buffer) [p * 2 + 1] = 127.5 + vr * 127.5;
            }
            else if (self->bits == 16) {
                vl *= 0.1;
                if (vl > 1) {
                    vl = 1;
                }
                if (vl < - 1) {
                    vl = - 1;
                }
                vr *= 0.1;
                if (vr > 1) {
                    vr = 1;
                }
                if (vr < - 1) {
                    vr = - 1;
                }
                ((int16_t *) buffer) [p * 2] = vl * 32767;
                ((int16_t *) buffer) [p * 2 + 1] = vr * 32767;
            }
        }
        land_stream_fill(self->audiostream);
    }
}
void music_simulate(Music * self) {
    /* """Simulate playing the whole music, to find the exact duration."""
     */
    double samples = 0;
    double length = 0;
    int midi_position = 0;
    int i;
    for (i = 0; i < self->midi->n; i++) {
        MidiNote * note = & self->midi->notes [i];
        int d = note->t - midi_position;
        midi_position = note->t;
        samples += ticks_to_samples(self, d);
        if (note->com == BPM) {
            self->bpm = note->n;
        }
        else if (note->com == NOTE) {
            length = samples + ticks_to_samples(self, note->d);
        }
    }
    self->bpm = 120;
    self->total_seconds = length / self->frequency;
}
