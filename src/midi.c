#include "midi.h"
static MidiNote active [128 * 16];
static int compar(const void * va, const void * vb);
MidiFile* midifile_new(void) {
    MidiFile * self = calloc(1, sizeof (* self));
    return self;
}
void midifile_del(MidiFile * self) {
    free(self);
}
void midifile_addbpm(MidiFile * self, int bpm, int pos) {
    int i = self->n;
    self->n++;
    self->notes = realloc(self->notes, self->n * sizeof (* self->notes));
    self->notes [i] = (MidiNote) {BPM, bpm, 0, pos, 0, 0};
}
void midifile_addvol(MidiFile * self, int v, int channel, int pos) {
    int i = self->n;
    self->n++;
    self->notes = realloc(self->notes, self->n * sizeof (* self->notes));
    self->notes [i] = (MidiNote) {VOL, v, 0, pos, 0, channel};
}
void midifile_addpan(MidiFile * self, int v, int channel, int pos) {
    int i = self->n;
    self->n++;
    self->notes = realloc(self->notes, self->n * sizeof (* self->notes));
    self->notes [i] = (MidiNote) {PAN, v, 0, pos, 0, channel};
}
void midifile_addnote(MidiFile * self, int note, int channel, int v, int pos) {
    if (v) {
        active [channel * 128 + note] = (MidiNote) {NOTE, note, v, pos, 0, channel};
    }
    else {
        int d = pos - active [channel * 128 + note].t;
        int i = self->n;
        self->n++;
        self->notes = realloc(self->notes, self->n * sizeof (* self->notes));
        self->notes [i] = active [channel * 128 + note];
        self->notes [i].d = d;
    }
}
static int compar(const void * va, const void * vb) {
    MidiNote const * a = va;
    MidiNote const * b = vb;
    if (a->t > b->t) {
        return 1;
    }
    if (a->t < b->t) {
        return - 1;
    }
    return 0;
}
void midifile_sort_notes(MidiFile * self) {
    qsort(self->notes, self->n, sizeof (MidiNote), compar);
}
MidiFile* midifile_load(str name) {
    int M, T, h, d, r, k;
    int hl, t, tn;
    int div1, div2;
    int com = 0;
    int midipos;
    int it;
    LandFile * notesfile;
    int length;
    int i, j;
    MidiFile * midi = midifile_new();
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 128; j++) {
            active [i * 128 + j] = (MidiNote) {NOTE, 0, 0, 0, 0, 0};
        }
    }
    notesfile = land_file_new(name, "r");
    if (! notesfile) {
        goto error;
    }
    M = land_file_getc(notesfile);
    T = land_file_getc(notesfile);
    h = land_file_getc(notesfile);
    d = land_file_getc(notesfile);
    // check for .rm
    if (M == 'R' && T == 'I' && h == 'F' && d == 'F') {
        // Note: RIFF files are little endian (but midi is still big endian)
        land_file_get32le(notesfile);
        while (! land_file_eof(notesfile)) {
            M = land_file_getc(notesfile);
            T = land_file_getc(notesfile);
            h = land_file_getc(notesfile);
            d = land_file_getc(notesfile);
            if (M == 'R' && T == 'M' && h == 'I' && d == 'D') {
                land_file_get32le(notesfile);
                land_file_get32le(notesfile);
                break;
            }
            else {
                // skip to next chunk
                land_file_skip(notesfile, land_file_get32le(notesfile));
            }
        }
        M = land_file_getc(notesfile);
        T = land_file_getc(notesfile);
        h = land_file_getc(notesfile);
        d = land_file_getc(notesfile);
    }
    hl = land_file_getc(notesfile) << 24;
    hl += land_file_getc(notesfile) << 16;
    hl += land_file_getc(notesfile) << 8;
    hl += land_file_getc(notesfile);
    t = land_file_getc(notesfile) << 8;
    t += land_file_getc(notesfile);
    tn = land_file_getc(notesfile) << 8;
    tn += land_file_getc(notesfile);
    // how much midi clocks in one quarter note
    div1 = land_file_getc(notesfile);
    div2 = land_file_getc(notesfile);
    // Alternatively, if the first byte has highest bit set, the time is given
    // in another format: The first byte is the negative FPS, normally -24, -25,
    // -29 or -30; the second byte then is the sub-resolution, usually 4, 8, 10,
    // 80 or 1000.
    if (div1 < 128) {
        // i.e., first byte is positive :
        midi->division = (div1 * 256 + div2);
        printf("%d midi clocks per quarter note.\n", midi->division);
    }
    else {
        // TODO: how does this work? What does BPM do when midi times are second
        // based?
        // Here we just assume 120 BPM to get a divider value. If the music is
        // then played as 120 BPM, times are second based as specified (but not
        // if the BPM change).
        midi->division = (div1 - 256) * div2;
        printf("%d midi clocks per second.", midi->division);
        midi->division /= 2 /* With 120 BPM, one quarter note is 0.5 seconds */;
    }
    if (M != 'M' || T != 'T' || h != 'h' || d != 'd' || hl != 6) {
        goto error;
    }
    if (t != 0 && t != 1) {
        goto error;
    }
    if (t == 0) {
        tn = 1;
    }
    // Read all tracks.
    for (t = 0; t < tn; t++) {
        printf("Track %d\n", t);
        midipos = 0;
        M = land_file_getc(notesfile);
        T = land_file_getc(notesfile);
        r = land_file_getc(notesfile);
        k = land_file_getc(notesfile);
        if (M != 'M' || T != 'T' || r != 'r' || k != 'k') {
            goto error;
        }
        length = land_file_getc(notesfile) << 24;
        length += land_file_getc(notesfile) << 16;
        length += land_file_getc(notesfile) << 8;
        length += land_file_getc(notesfile);
        // Read the track.
        for (it = 0; it < length; it++) {
            int c, tt, next;
            int vl;
            if (land_file_eof(notesfile)) {
                break;
            }
            tt = 0;
            for (vl = 0; vl < 4; vl++) {
                int b;
                b = land_file_getc(notesfile);
                tt += b & 127;
                if (b > 127) {
                    tt *= 128;
                }
                else {
                    break;
                }
            }
            midipos += tt;
            c = land_file_getc(notesfile);
            if (c & 0x80) {
                com = c;
                next = land_file_getc(notesfile);
            }
            else {
                // running status
                next = c;
                c = com;
            }
            if (c >= 0x80 && c <= 0x8f) {
                // note off
                int n = next;
                int v = land_file_getc(notesfile);
                (void) v;
                int channel = c - 0x80;
                // printf("%d - Note Off: %d: %d %d\n", midipos, channel, n, v)
                midifile_addnote(midi, n, channel, 0, midipos);
            }
            else if (c >= 0x90 && c <= 0x9f) {
                // note on
                int n = next;
                int v = land_file_getc(notesfile);
                int channel = c - 0x90;
                // printf("%d - Note On: %d: %d %d\n", midipos, channel, n, v)
                midifile_addnote(midi, n, channel, v, midipos);
            }
            else if (c >= 0xa0 && c <= 0xaf) {
                // aftertouch
                int n = next;
                int v = land_file_getc(notesfile);
                int channel = c - 0xa0;
                printf("Aftertouch: %d: %d %d\n", channel, n, v);
            }
            else if (c >= 0xb0 && c <= 0xbf) {
                // controller :
                char const * controllers [] = {"Bank Select", "Modulation Wheel", "Breath controller", "?(3)", "Foot Pedal", "Portamento Time", "Data Entry", "Volume", "Balance", "?(9)", "Pan position", /* 10 */ "Expression", "Effect Control 1", "Effect Control 2", "?(14)", "?(15)", "General Purpose Slider 1", "General Purpose Slider 2", "General Purpose Slider 3", "General Purpose Slider 4", "?", /* 20 */ "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", /* 30 */ "?", "Bank Select (fine)", /* 32 */ "Modulation Wheel (fine)", "Breath controller (fine)", "?(35)", "Foot Pedal (fine)", "Portamento Time (fine)", "Data Entry (fine)", "Volume (fine)", "Balance (fine)", /* 40 */ "?(9)", "Pan position (fine)", /* 42 */ "Expression (fine)", "Effect Control 1 (fine)", "Effect Control 2 (fine)", "?", "?", "?", "?", "?""?" /* 50 */, "?", "?", "?", "?", "?", "?", "?", "?", "?", /* 60 */ "?", "?", "?", "Hold Pedal", /* 64 */ "Portamento (on/off)", "Sustenuto Pedal", "Soft Pedal", "Legato Pedal", "Hold 2 Pedal", "Sound Variation", /* 70 */ "Sound Timbre", "Sound Release Time", "Sound Attack Time", "Sound Brightness", "Sound Control 6", "Sound Control 7", "Sound Control 8", "Sound Control 9", "Sound Control 10", "General Purpose Button 1", /* 80 */ "General Purpose Button 2", "General Purpose Button 3", "General Purpose Button 4", "?(84)", "?(85)", "?(86)", "?(87)", "?(88)", "?(89)", "?(90)", /* 90 */ "Effects Level", "Tremulo Level", "Chorus Level", "Celeste Level", "Phaser Level", "Data Button increment", "Data Button decrement", "Non-registered Parameter (fine)", "Non-registered Parameter", "Registered Parameter (fine)", /* 100 */ "Registered Parameter", "?", "?", "?", "?", "?", "?", "?", "?", "?", /* 110 */ "?", "?", "?", "?", "?", "?", "?", "?", "?", "All Sound Off", /* 120 */ "All Controllers Off", "Local Keyboard (on/off)", "All Notes Off", "Omni Mode Off", "Omni Mode On", "Mono Operation", "Poly Operation"};
                int n = next;
                int v = land_file_getc(notesfile);
                int channel = c - 0xb0;
                if (n == 7) {
                    // Volume :
                    midifile_addvol(midi, v, channel, midipos);
                }
                else if (n == 8) {
                    // Balance :
                    // FIXME: what if balance and pan at the same time?
                    midifile_addpan(midi, v, channel, midipos);
                }
                else if (n == 10) {
                    // Pan :
                    // FIXME: what if balance and pan at the same time?
                    midifile_addpan(midi, v, channel, midipos);
                }
                else {
                    printf("%d - Controller: %d: %s %d\n", midipos, channel, controllers [n], v);
                }
            }
            else if (c >= 0xc0 && c <= 0xcf) {
                // program change :
                int program = next;
                int channel = c - 0xc0;
                printf("%d - Program: %d %d\n", midipos, channel, program);
            }
            else if (c >= 0xd0 && c <= 0xdf) {
                // channel pressure :
                int n = next;
                int channel = c - 0xd0;
                printf("Pressure: %d %d\n", channel, n);
            }
            else if (c >= 0xe0 && c <= 0xef) {
                // pitch wheel :
                int n = next;
                int v = land_file_getc(notesfile);
                int channel = c - 0xe0;
                printf("Pitch wheel: %d: %d %d\n", channel, n, v);
            }
            else if (c == 0xf0 || c == 0xf7) {
                // sysex :
                int l;
                l = 0;
                for (vl = 0; vl < 4; vl++) {
                    int b;
                    b = vl ? land_file_getc(notesfile) : next;
                    l += b & 127;
                    if (b > 127) {
                        l *= 128;
                    }
                    else {
                        break;
                    }
                }
                printf("Sysex: ");
                for (i = 0; i < l; i++) {
                    int x = land_file_getc(notesfile);
                    if (x >= 32 && x <= 127) {
                        printf("%c", x);
                    }
                    else {
                        printf("[%02x]", x);
                    }
                }
                printf("\n");
            }
            else if (c == 0xff) {
                // meta :
                int t2, l;
                t2 = next;
                l = 0;
                for (vl = 0; vl < 4; vl++) {
                    int b;
                    b = land_file_getc(notesfile);
                    l += (b & 127);
                    if (b > 127) {
                        l *= 128;
                    }
                    else {
                        break;
                    }
                }
                if (t2 == 0x2f) {
                    printf("Escape\n");
                    break;
                }
                else if (t2 == 0x51) {
                    int qnt;
                    int bpm;
                    qnt = land_file_getc(notesfile) << 16;
                    qnt += land_file_getc(notesfile) << 8;
                    qnt += land_file_getc(notesfile);
                    // qnt is length of a quarter note in µs
                    // BPM is how many quarter notes in one minute
                    bpm = 60000000 / qnt;
                    // printf ("%d - BPM: %d\n", midipos, bpm)
                    midifile_addbpm(midi, bpm, midipos);
                }
                else if (t2 >= 1 && t2 <= 9) {
                    char const * meta_type []={"?", "Text", "Copyright", "Name", "Instrument", "Lyric", "Marker", "Cue Point", "Program name", "Device",};
                    printf("%d - %s:", midipos, meta_type [t2]);
                    for (i = 0; i < l; i++) {
                        int x = land_file_getc(notesfile);
                        if (x >= 32 && x <= 127) {
                            printf("%c", x);
                        }
                        else {
                            printf("[%02x]", x);
                        }
                    }
                    printf("\n");
                }
                else {
                    printf("%d - Meta %02x:", midipos, t2);
                    for (i = 0; i < l; i++) {
                        int x = land_file_getc(notesfile);
                        if (x >= 32 && x <= 127) {
                            printf("%c", x);
                        }
                        else {
                            printf("[%02x]", x);
                        }
                    }
                    printf("\n");
                }
            }
        }
    }
    land_file_destroy(notesfile);
    midifile_sort_notes(midi);
    return midi;
    error:;
    if (notesfile) {
        land_file_destroy(notesfile);
    }
    if (midi) {
        midifile_del(midi);
    }
    return NULL;
}
