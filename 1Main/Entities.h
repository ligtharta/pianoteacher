#ifndef Entities_h
#define Entities_h

#include <Arduino.h>
#include <SD.h>
#include "HardwDefs.h"


/******************************************************************************************************************************
*
* CLASS  :  StorageEntityBase
* 
* Base class for entities on SD-Card like 'User', 'Song', 'Settings'.
* This class helps with parsing from text and writing to text.
*
*******************************************************************************************************************************/
class StorageEntityBase {
  protected:
    static bool readNameValueRow(File* file);
    static char* readFirstValuePart();
    static char* readNextValuePart();
    static void toUpper(char *sPtr);
    static char row_Name[20];
    static char row_Value[50];
  private:
    static const char* _delimiter;
};


#define PLAY_WHILE_PRACTICE_OFF     0  /* do not play song via MIDI while practicing */
#define PLAY_WHILE_PRACTICE_VOLU_1  1  /* play song via MIDI while practicing: very low volume/velocity */
#define PLAY_WHILE_PRACTICE_VOLU_2  2  /*       "                    "       : low volume/velocity  */
#define PLAY_WHILE_PRACTICE_VOLU_3  3  /*       "                    "       : normal volume/velocity  */
#define PRACTICE_1       1        /* to practice step-by-step (no real time) */
#define PRACTICE_2       2        /* to practice song in real-time mode with set tempo-factor (LEDs flow in at fixed speed) */
#define PRACTICE_3       3        /* to practice song in real-time mode with set tempo-factor (LED presentation same as practice 1)  */
#define PRACTICE_4       4        /* practice reading notes from paper and press the right piano key for each note */
#define PRACTICE_5       5        /* practice playing by reading note letters (like C, D, E, F#) on the LED display */
#define DEFAULT_ANIMATION_SPD 1        /* normal speed of LED animation while doing practice 2 (see Player3 class)  */

#define DEFAULT_USER_ID       1   /* if 'lastUser' in SETTINGS.TXT not found, then use this uderId */
#define DEFAULT_SONG_ID       1   /* if 'songId' not found in USERxx.TXT, then use this songId */

/******************************************************************************************************************************
*
* CLASS  :  User
* 
* Represents the user/player who is playing the piano
* Besides the name, this class holds user-specific settings
*
*******************************************************************************************************************************/
class User : StorageEntityBase {
  public:
    /**
    * Constructor.
    */
    User();
    static void getFilename(int userId, char* charBuffer);
    static void parseUser(int userId, File* file);
    static void writeUser(File* file);
    static bool isDirty();
    static void resetDirty();
    static void setDefaults(int id);

    static char userName[15];
    static byte userId;
    static byte lastSong;
    static int tempoFactor;
    static byte metronome;
    static bool isGloves;
    static byte practice;
    static bool isPracticeRepeat;
    static byte animationSpeed;
    static byte playWhilePractice;  /* not persisted */
    static byte globalBrightness;   /* not persisted */
    static byte panelRowsUsed;      /* not persisted */
    static bool leftHandCue;        /* not persisted */
    
  private:
    static char _userName_cpy[15];
    static byte _lastSong_cpy;
    static int _tempoFactor_cpy;
    static byte _metronome_cpy;
    static bool _isGloves_cpy;
    static byte _practice_cpy;
    static bool _isPracticeRepeat_cpy;
    static byte _animationSpeed_cpy;
};



#define MAX_LEN_SSID  25
#define MAX_LEN_PASSW 25
/******************************************************************************************************************************
*
* CLASS  :  Settings
* 
* Represents the global settings
*
*******************************************************************************************************************************/
class Settings : StorageEntityBase {
  public:
    /**
    * Constructor.
    */
    Settings();
    static void getFilename(char* charBuffer);
    static void parseSettings(File* file);
    static void writeSettings(File* file);
    static void setDefaultSettings();
    static bool isDirty();
    static void resetDirty();

    static int lastUser;
    static char wifiSSID[MAX_LEN_SSID + 1];       /* only for reading */
    static char wifiPassword[MAX_LEN_PASSW + 1];  /* only for reading */
    
  private:
    static int _lastUser_cpy;
};

/******************************************************************************************************************************
*
* CLASS  :  SongNote
* 
* Represents a musical Note within a song.
* 
* BEWARE: SongNote and SongMeasure are dependent in terms of alignment, see also 'test_CheckAlignmentAfterCasting()'
*         sizeof(SongNote) = sizeof(SongMeasure) = 12 bytes
*
* See this array in Song object: SongNote notes[MAX_NOTES]    (!this array will contain both SongNote and SongMeasure objects!)
*******************************************************************************************************************************/
class SongNote {
  public:
    bool isFingerLeft();
    bool isFingerRight();
    static bool isFingerLeft(byte f);
    static bool isFingerRight(byte f);

    uint32_t atTick;     /* Time (absolute MIDI-tick) at which note must be processed. */
    uint32_t duration;   /* duration of note in ticks */
    byte finger;         /* finger that should play the note ( 0=unknown / 1,2,3,4,5=left hand / 6,7,8,9,10=right hand ) */
    byte pitch;             
    byte velocity; 
    byte type;           /* TYPE_NOTE, TYPE_MEASURE, TYPE_MEASURE_BM */
  private:
};


/******************************************************************************************************************************
*
* CLASS  :  SongMeasure
* 
* Represents a Measure within the song
* 
* BEWARE: SongNote and SongMeasure are dependent in terms of alignment, see also 'test_CheckAlignmentAfterCasting()'
*         sizeof(SongMeasure) = sizeof(SongNote) 12 bytes
*
* See this array in Song object: SongNote notes[MAX_NOTES]    (!this array will contain both SongNote and SongMeasure objects!)
*******************************************************************************************************************************/
class SongMeasure {
  public:
    uint32_t atTick;       /* Time (absolute MIDI-tick) at which this measure must be processed. */
    uint16_t beatTicks;    /* MIDI-ticks per beat.  example: 120 */ 
    uint16_t tempoQPM;     /* tempo in 'quarter notes per beat'  */
    byte beatCount;        /* number of beats in measure (example: 3 or 4) */
    byte measureNr;        /* 1 for first measure, 2 for second, etc. */
    byte unused;           /* unused filling so that SongMeasure and SongNote are both 12 bytes in size, and aligned correctly */
    byte type;             /* TYPE_NOTE, TYPE_MEASURE, TYPE_MEASURE_BM */
  private:
};



#define MAX_NOTES     1200    /* max amount of song notes  */
#define TYPE_NOTE        0    /* Note             (SongNote object represents a real note like C#, E, F, G#)  */
#define TYPE_MEASURE     1    /* Measure          (SongNote object represents a measure, therefore SongNote* can be casted to SongMeasure*)  */
#define TYPE_MEASURE_BM  2    /* Measure/BookMark (same as TYPE_MEASURE, but this measure has an implicit bookmark flag)  */


/* flags used while loading a song from SD Card into RAM */
#define LOAD_FLAG_NONE 0
#define LOAD_FLAG_LEFT_COLOR 1      /* left hand data with finger colors */
#define LOAD_FLAG_LEFT_WHITE 2      /* left hand data without finger colors -> all white */ 
#define LOAD_FLAG_LEFT_MASK (1+2)   /* left hand data, with or without finger colors */ 
#define LOAD_FLAG_RIGHT_COLOR 4     /* right hand data with finger colors */
#define LOAD_FLAG_RIGHT_WHITE 8     /* right hand data without finger colors -> all white */ 
#define LOAD_FLAG_RIGHT_MASK (4+8)  /* right hand data, with or without finger colors */ 


/******************************************************************************************************************************
*
* CLASS  :  Song
* 
* Represents the currently loaded piano song
*
*******************************************************************************************************************************/
class Song : StorageEntityBase {
  public:
    /**
    * Constructor.
    */
    Song();
    static void getFilename(int songId, char* charBuffer);
    void parseSong(int songId, File* file, byte loadFlags);
    bool* getSongAnalysis();
    int getNextBookmarkMeasureNr(int curMeasureNr, bool forward);
    int getMeasureStartIndex(int measureNr);

    /* immutable song properties, read from SD Card song-file */
    char songName[100];           /* name of the song */
    int songId;                   /* id of the song */
    int resolution;               /* [resolution: ticks/quarter note] */
    int totalTicks;               /* [in total ticks] */

    /* immutable song data (notes, measures, bookmarks), read from SD Card song-file */
    SongNote notes[MAX_NOTES];     /* notes within song */
    int noteCount;                 /* how many of MAX_NOTES elements used (this includes measures!)? */
    int lastMeasureNr;             /* Number/Id of the very last measure  */
       
  private:
    /* song analysis */
    void _analyseSong();
    bool _songAnalysis[MAX_UNIQUE_PITCHES]; /* for each piano key: true if pitch is used in song, */
};


#endif // Entities_h
