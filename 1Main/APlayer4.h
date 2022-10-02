#ifndef Player4_h
#define Player4_h

#include <Arduino.h>
#include "Templates.h"
#include "Entities.h"
#include "LedPanel.h"
#include "Midi.h"


#define LED_PANEL_X_FIRST_STEP 41      /* column/x-position on LED-panel from where steps are drawn */
#define MAX_STEPS_IN_MEASURE   30      /* max amount of steps (a discrete time when a note starts) in one measure */
#define MAX_KEYS_IN_STEP       10      /* max piano keys in 1 step (that should be played simultaniously) */
#define PRACTICE_RESULTS_MAX   (24*5)  /* how many red/green accomplishment LEDs before start over? */
#define SCHEDULED_NOTES_MAX    20      /* max notes that can be scheduled to be played by MIDI */


/*
*
*
*/
class Player4 {
  public:
    Player4(MidiInterface* mi, LedPanel* lp);

    /* playing the song */
    void startSong(Song* song, int startMeasureNr, byte midiPlay);
    void handlePlaying();
    bool stopPlayingNow();
   
    bool isPlaying = false;
    int  curMeasureNr;      /* the current measure number while practicing (1=first) */
    
  protected:

  private:
    /* basic song info (copied from Song object) */
    int _noteCount;                /* how many of MAX_NOTES elements used? */
    Song* _song;
    SongNote* _notes;              /* notes within song */
    int _lastMeasureNr;            /* Number/Id of the very last measure  */

    /* references to needed objects */
    MidiInterface* _midi;
    LedPanel*      _ledPanel;

    /* playing the song */
    int _curNoteIdx;        /* index of current note */
    int _measuresPracticed;                      /* how many measures have been practiced by the user? */
    byte _midiPlay;                              /* 0 = off, otherwise 1,2 or 3 (higher is louder) */
    DelayManager<byte, SCHEDULED_NOTES_MAX> _scheduledNotes;
    byte _practiceResults[PRACTICE_RESULTS_MAX]; /* per practiced measure: red or green pixel earned? */
    bool _sustainDown;                           /* is sustain pedal down? */
    /* playing the song: steps (measure is divided in steps)  */
    byte _notesPerStep[MAX_STEPS_IN_MEASURE]; /* per step: how many notes in that particular step? */
    bool _errorPerStep[MAX_STEPS_IN_MEASURE];   /* per step: did the user play all notes correctly? */
    byte _stepCount;                            /* how many steps in current measure? */
    byte _stepIndex;                            /* current step within current measure */
    byte _currStepNotesPlayed;                  /* how many notes of current step have been played? */
    /* piano keys that needs to be played within one step (mostly 1, 2 or a few more) */
    uint16_t _keysCurrStep[MAX_KEYS_IN_STEP];     /* max 10 simultanious keys per step */
    uint16_t _keysCurrStepCopy[MAX_KEYS_IN_STEP];
    void _pianoKeyReset();                        /* clear all piano keys for new step */
    void _pianoKeyRegister(int pitch, byte finger);                /* set piano key in current step */
    bool _pianoKeyRemove(int pitch, bool* allDone, bool* isWrong); /* clear piano key that has been played in current step */
    
    void _startMeasure(int measureNr);
    void _prepareStepAndMoveToNext();
    void _drawLEDpanel(bool newMeasure);
    void _measureCompleted();           /* set a red or green pixel for each completed measure */
};



#endif // Player4_h
