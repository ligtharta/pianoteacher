#ifndef MidiDefs_h
#define MidiDefs_h


// -----------------------------------------------------------------------------
// MIDI tutorial: http://www.music-software-development.com/midi-tutorial.html
// -----------------------------------------------------------------------------

/*! Enumeration of MIDI types */
enum MidiType : byte
{
    InvalidType           = 0x00,    ///< For notifying errors
    NoteOff               = 0x80,    ///< Note Off
    NoteOn                = 0x90,    ///< Note On
    AfterTouchPoly        = 0xA0,    ///< Polyphonic AfterTouch
    ControlChange         = 0xB0,    ///< Control Change / Channel Mode
    ProgramChange         = 0xC0,    ///< Program Change
    AfterTouchChannel     = 0xD0,    ///< Channel (monophonic) AfterTouch
    PitchBend             = 0xE0,    ///< Pitch Bend
    SystemExclusive       = 0xF0,    ///< System Exclusive
    TimeCodeQuarterFrame  = 0xF1,    ///< System Common - MIDI Time Code Quarter Frame
    SongPosition          = 0xF2,    ///< System Common - Song Position Pointer
    SongSelect            = 0xF3,    ///< System Common - Song Select
    TuneRequest           = 0xF6,    ///< System Common - Tune Request
    Clock                 = 0xF8,    ///< System Real Time - Timing Clock
    Start                 = 0xFA,    ///< System Real Time - Start
    Continue              = 0xFB,    ///< System Real Time - Continue
    Stop                  = 0xFC,    ///< System Real Time - Stop
    ActiveSensing         = 0xFE,    ///< System Real Time - Active Sensing
    SystemReset           = 0xFF,    ///< System Real Time - System Reset
};

// -----------------------------------------------------------------------------


enum MidiControlChange : byte
{
    SustainPedal        = 0x40,
};


// midi commands
#define MIDI_CMD_NOTE_OFF 0x80
#define MIDI_CMD_NOTE_ON 0x90
#define MIDI_CMD_KEY_PRESSURE 0xA0
#define MIDI_CMD_CONTROLLER_CHANGE 0xB0
#define MIDI_CMD_PROGRAM_CHANGE 0xC0
#define MIDI_CMD_CHANNEL_PRESSURE 0xD0
#define MIDI_CMD_PITCH_BEND 0xE0

#define MIDI_PITCH_A0         21    /* lowest pitch for 88-key instrument */
#define MIDI_PITCH_ASHARP0    22
#define MIDI_PITCH_B0         23

#define MIDI_PITCH_C1         24
#define MIDI_PITCH_CSHARP1    25
#define MIDI_PITCH_D1         26
#define MIDI_PITCH_DSHARP1    27
#define MIDI_PITCH_E1         28   /* lowest pitch for 73-key instrument */
#define MIDI_PITCH_F1         29
#define MIDI_PITCH_FSHARP1    30
#define MIDI_PITCH_G1         31
#define MIDI_PITCH_GSHARP1    32
#define MIDI_PITCH_A1         33
#define MIDI_PITCH_ASHARP1    34
#define MIDI_PITCH_B1         35

#define MIDI_PITCH_C2         36    /* lowest pitch for 61-key instrument */
#define MIDI_PITCH_CSHARP2    37
#define MIDI_PITCH_D2         38
#define MIDI_PITCH_DSHARP2    39
#define MIDI_PITCH_E2         40
#define MIDI_PITCH_F2         41
#define MIDI_PITCH_FSHARP2    42
#define MIDI_PITCH_G2         43
#define MIDI_PITCH_GSHARP2    44
#define MIDI_PITCH_A2         45
#define MIDI_PITCH_ASHARP2    46
#define MIDI_PITCH_B2         47

#define MIDI_PITCH_C3         48
#define MIDI_PITCH_CSHARP3    49
#define MIDI_PITCH_D3         50
#define MIDI_PITCH_DSHARP3    51
#define MIDI_PITCH_E3         52
#define MIDI_PITCH_F3         53
#define MIDI_PITCH_FSHARP3    54
#define MIDI_PITCH_G3         55
#define MIDI_PITCH_GSHARP3    56
#define MIDI_PITCH_A3         57
#define MIDI_PITCH_ASHARP3    58
#define MIDI_PITCH_B3         59

#define MIDI_PITCH_C4         60  // Middle-C
#define MIDI_PITCH_CSHARP4    61
#define MIDI_PITCH_D4         62
#define MIDI_PITCH_DSHARP4    63
#define MIDI_PITCH_E4         64
#define MIDI_PITCH_F4         65
#define MIDI_PITCH_FSHARP4    66
#define MIDI_PITCH_G4         67
#define MIDI_PITCH_GSHARP4    68
#define MIDI_PITCH_A4         69
#define MIDI_PITCH_ASHARP4    70
#define MIDI_PITCH_B4         71

#define MIDI_PITCH_C5         72
#define MIDI_PITCH_CSHARP5    73
#define MIDI_PITCH_D5         74
#define MIDI_PITCH_DSHARP5    75
#define MIDI_PITCH_E5         76
#define MIDI_PITCH_F5         77
#define MIDI_PITCH_FSHARP5    78
#define MIDI_PITCH_G5         79
#define MIDI_PITCH_GSHARP5    80
#define MIDI_PITCH_A5         81
#define MIDI_PITCH_ASHARP5    82
#define MIDI_PITCH_B5         83

#define MIDI_PITCH_C6         84
#define MIDI_PITCH_CSHARP6    85
#define MIDI_PITCH_D6         86
#define MIDI_PITCH_DSHARP6    87
#define MIDI_PITCH_E6         88
#define MIDI_PITCH_F6         89
#define MIDI_PITCH_FSHARP6    90
#define MIDI_PITCH_G6         91
#define MIDI_PITCH_GSHARP6    92
#define MIDI_PITCH_A6         93
#define MIDI_PITCH_ASHARP6    94
#define MIDI_PITCH_B6         95

#define MIDI_PITCH_C7         96    /* highest pitch for 61-key instrument */
#define MIDI_PITCH_CSHARP7    97
#define MIDI_PITCH_D7         98
#define MIDI_PITCH_DSHARP7    99
#define MIDI_PITCH_E7         100   /* highest pitch for 73-key instrument */
#define MIDI_PITCH_F7         101
#define MIDI_PITCH_FSHARP7    102
#define MIDI_PITCH_G7         103
#define MIDI_PITCH_GSHARP7    104
#define MIDI_PITCH_A7         105
#define MIDI_PITCH_ASHARP7    106
#define MIDI_PITCH_B7         107
#define MIDI_PITCH_C8         108   /* highest pitch for 88-key instrument */



/*

--------------------------------------------------------------------------------
  PITCH  /  FREQ /  MIDI     PITCH  /  FREQ /  MIDI     PITCH  /  FREQ /  MIDI
--------------------------------------------------------------------------------
    C0       -       12        C4    261.626    60*MC     C8   4186.009   108*PH
   C#0       -       13       C#4    277.183    61       C#8   4434.922   109
    D0       -       14        D4    293.665    62        D8   4698.637   110
   D#0       -       15       D#4    311.127    63       D#8   4978.032   111
    E0       -       16        E4    329.628    64        E8   5274.042   112
    F0       -       17        F4    349.228    65        F8   5587.652   113
   F#0       -       18       F#4    369.994    66       F#8   5919.912   114
    G0       -       19        G4    391.995    67        G8   6271.928   115
   G#0       -       20       G#4    415.305    68       G#8   6644.876   116
    A0     27.500    21*PL     A4    440.000    69        A8   7040.000   117
   A#0     29.135    22       A#4    466.164    70       A#8   7458.620   118
    B0     30.868    23        B4    493.883    71        B8   7902.133   119

    C1     32.703    24        C5    523.251    72        C9   8372.019   120
   C#1     34.648    25       C#5    554.365    73       C#9   8869.845   121
    D1     36.708    26        D5    587.330    74        D9   9397.273   122
   D#1     38.891    27       D#5    622.254    75       D#9   9956.064   123
    E1     41.203    28        E5    659.255    76        E9  10548.083   124
    F1     43.654    29        F5    698.457    77        F9  11175.305   125
   F#1     46.249    30       F#5    739.989    78       F#9  11839.823   126
    G1     48.999    31        G5    783.991    79        G9  12543.855   127
   G#1     51.913    32       G#5    830.609    80       G#9  13289.752    -
    A1     55.000    33        A5    880.000    81        A9       -       -
   A#1     58.270    34       A#5    932.328    82       A#9       -       -
    B1     61.735    35        B5    987.767    83        B9       -       -

    C2     65.406    36        C6   1046.502    84
   C#2     69.296    37       C#6   1108.731    85
    D2     73.416    38        D6   1174.659    86
   D#2     77.782    39       D#6   1244.508    87
    E2     82.407    40        E6   1318.510    88
    F2     87.307    41        F6   1396.913    89
   F#2     92.499    42       F#6   1479.978    90
    G2     97.999    43        G6   1567.982    91
   G#2    103.826    44       G#6   1661.219    92
    A2    110.000    45        A6   1760.000    93
   A#2    116.541    46       A#6   1864.655    94
    B2    123.471    47        B6   1975.533    95

    C3    130.813    48        C7   2093.005    96
   C#3    138.591    49       C#7   2217.461    97
    D3    146.832    50        D7   2349.318    98
   D#3    155.564    51       D#7   2489.016    99
    E3    164.814    52        E7   2637.021   100
    F3    174.614    53        F7   2793.826   101
   F#3    184.997    54       F#7   2959.956   102
    G3    195.998    55        G7   3135.964   103
   G#3    207.652    56       G#7   3322.438   104
    A3    220.000    57        A7   3520.000   105 
   A#3    233.082    58       A#7   3729.310   106
    B3    246.942    59        B7   3951.066   107

*PL, *PH = the (standard) piano's lowest and highest notes.  *MC = middle 

*/

#endif // MidiDefs_h
