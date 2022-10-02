#ifndef Player2_h
#define Player2_h

#include <Arduino.h>
#include "Templates.h"
#include "Entities.h"
#include "Metronome.h"
#include "LedPanel.h"
#include "Gloves.h"
#include "Midi.h"

#define LED_OFF_SCHEDULE_MAX     20    /* how many LED-off commands can the DelayManager hold?  */
#define UPCOMING_NOTES_MAX       50    /* how many 'upcoming' notes can be stored in CircularArray? */
#define UPCOMING_MEASURES_MAX    4     /* how many 'upcoming' measures can be stored in DelayManager? */
#define GLOVE_COMMANDS_MAX       60    /* how many glove-commands (eg.: 'finger 8 ON') can be stored in DelayManager? */

#define GLOVE_FINGER_ON          128   /* bit-7 is 1 means: turn glove-finger ON */
#define GLOVE_FINGER_OFF         0     /* bit-7 is 0 means: turn glove-finger OFF */ 
#define GLOVE_SCHEDULE_EARLY     100   /* turn on vibrating motor of glove 100ms earlier, to give motor time to start-up */



/*
*
*
*/
class Player2 {
  
  /* NESTED CLASS: item-type of '_upcomingArray' : this represents a note to be played shortly  */
  class UpcomingNote {
    public:
      SongNote* note;
      uint32_t startMillis;    /* when to start this note? */
      uint32_t durationMillis; /* how long to play this note? */
      byte column;             /* column of LED-panel (represents pitch) */
  };
  
  public:
    Player2(LedPanel* lp, Metronome* m, MidiInterface* mi, Gloves* g);

    /* playing the song */
    void startSong(Song* song, int startMeasureNr, bool withGloves, byte midiPlay, uint16_t tempoFactor, bool repeat);
    void handlePlaying();
    bool stopPlayingNow();
    bool isSongFinished();    
    /* Suspend and resume song while practicing */
    void suspendPlaying();
    void resumePlaying();

    bool isPlaying = false;
    int curMeasureNr;        /* the current measure number while practicing (1=first) */
    
  protected:

  private:
    /* basic song info (copied from Song object) */
    uint32_t  _resolution;         /* [resolution: ticks/quarter note] */
    int       _totalTicks;         /* Total length of song in ticks. */
    int       _noteCount;          /* how many of MAX_NOTES elements used? */
    SongNote* _notes;              /* notes within song */

    /* references to needed objects */
    MidiInterface* _midi;
    Metronome*     _metronome;
    LedPanel*      _ledPanel;
    Gloves*        _gloves;

    /* playing the song */
    bool _realtimeMode;     /* true if user has played first note by pressing a piano key */
    uint32_t _startupTime;  /* value of millis() at startup */
    uint32_t _startupDelayMillis; /* time (millis) between startup and first note played by user (=first pressed piano key) */
    bool _doRepeat;         /*  repeat after end of song? */
    bool _withGloves;
    byte _midiPlay;          /* 0 = off, otherwise 1,2 or 3 (higher is louder) */
    uint32_t _songEndMillis;  /* time (millis) when song is finished (only when _doRepeat = false) */
    uint32_t _currentTick;  /* when was last note played (MIDI-ticks) ? */
    uint32_t _lastMillis;   /* when was last note played (milli-seconds) ? */
    int _curNoteIdx;                /* index of current note being processed */
    int _curNoteIdx2;         /* index of note being played, and displayed on LED panel row 4 (current note) */
    uint32_t _timeAhead;        /* time (milliseconds) that note is put in _upcomingArr before it is actually played  */
    uint32_t _missedMillis;
    uint32_t _suspendMillis;    /* at what time was the song suspended (paused)? This is done using the foot pedal */
    /* tempo management */
    unsigned int _tempo;           /* [tempo: ms/quarter note. Example: 461 means ~130 beats/min (= 60.000 / 461) ] */
    uint16_t _tempoFactor;
    uint16_t _tempoQPM;            /* tempo in quarter notes per minute */
  
    CircularArray<UpcomingNote, UPCOMING_NOTES_MAX>  _upcomingArray;            /* which notes are to played shortly? */
    DelayManager<byte, LED_OFF_SCHEDULE_MAX>         _ledOffSchedule;           /* when should playing-LED be turned off */
    DelayManager<byte, UPCOMING_MEASURES_MAX>        _updateMeasureNrSchedule;  /* when should measure-nr be updated? */
    DelayManager<byte, GLOVE_COMMANDS_MAX>           _glovesSchedule;           /* when should which finger of glove turn on/off? */
    //DelayManager<byte, 3>                            _undimRow3Schedule;        /* when should LEDs on row 3 get more bright? */
    
    void _moveToFirstNote();     /* when start playing.. */
    bool handlePlaying_doStuff(uint32_t nowCorr);      /* called from handlePlaying() during both start-up and realtime mode */
    uint32_t _drawUpcomingNotes(uint32_t currNoteTick);  /* display notes that are about to be played (LED panel row 0/1/2/3) */
    void _lookAheadAndSchedule(uint32_t nowCorr);  /* process upcoming notes and measures */
    uint32_t _getMillisDuration(uint32_t ticks);
    void _setTempo(uint16_t qpm);
    
};


#endif // Player2_h
