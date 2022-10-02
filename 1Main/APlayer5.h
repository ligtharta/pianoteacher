#ifndef Player5_h
#define Player5_h

#include <Arduino.h>
#include "Templates.h"
#include "Entities.h"
#include "LedPanel.h"
#include "Gloves.h"
#include "Midi.h"



#define TO_PLAY_BIT_MASK (1 << 12)     /* bit that tracks whether a piano key still needs to be played  */
#define MAX_KEYS_IN_STEP       10      /* max piano keys in 1 step (that should be played simultaniously) */
#define SCHEDULED_NOTES_MAX    20      /* max notes that can be scheduled to be played by MIDI */


/*
*
*
*/
class Player5 {
  public:
    Player5(MidiInterface* mi, LedPanel* lp);

    /* playing the song */
    void startSong(Song* song, int startMeasureNr, byte midiPlay, bool repeat);
    void handlePlaying();
    bool stopPlayingNow();
    bool isSongFinished();
    
    bool isPlaying = false;
    int  curMeasureNr;      /* the current measure number while practicing (1=first) */
    
  protected:

  private:
    /* basic song info (copied from Song object) */
    int       _noteCount;          /* how many of MAX_NOTES elements used? */
    SongNote* _notes;              /* notes within song */

    /* references to needed objects */
    MidiInterface* _midi;
    LedPanel*      _ledPanel;

    /* playing the song */
    int _curNoteIdx;        /* index of current note */
    bool _doRepeat;         /*  repeat after end of song? */
    bool _songFinished;
    byte _midiPlay;         /* 0 = off, otherwise 1,2 or 3 (higher is louder) */
    DelayManager<byte, SCHEDULED_NOTES_MAX> _scheduledNotes;

    void _moveToFirstNote();     /* when start playing.. */
    void _moveToNoteAtNewTick(); /* after the right piano keys have been pressed */
    void _registerPianoKeysCurrentPosition();
    void _drawNoteLetters();
    bool _isSameHand(uint8_t finger1, uint8_t finger2);
    
    uint16_t _currentPianoKeys[MAX_KEYS_IN_STEP];
    void _pianoKeyReset();
    void _pianoKeyRegister(int pitch, int finger);
    bool _pianoKeyRemove(int pitch);
    
};



#endif // Player5_h
