#ifndef Player1_h
#define Player1_h

#include <Arduino.h>
#include "Templates.h"
#include "Entities.h"
#include "LedPanel.h"
#include "Gloves.h"
#include "Midi.h"


/*
*
*
*/
class Player1 {
  public:
    Player1(MidiInterface* mi, LedPanel* lp, Gloves* g);

    /* playing the song */
    void startSong(Song* song, int startMeasureNr, bool withGloves, byte panelRowsUsed, bool repeat);
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
    Gloves*        _gloves;

    /* playing the song */
    int _curNoteIdx;        /* index of current note */
    byte _panelRowsUsed;    /* how many rows of LED panel used to display notes to play? Between 1 and 5 */
    bool _doRepeat;         /*  repeat after end of song? */
    bool _songFinished;

    void _moveToFirstNote();     /* when start playing.. */
    void _moveToNoteAtNewTick(); /* after the right piano keys have been pressed */
    void _registerPianoKeysCurrentPosition();
    void _drawLEDpanel();
    void _drawMeasureNr();
    
    uint32_t _bitPerPianoKey[3];
    void _pianoKeyReset();
    void _pianoKeyRegister(int pitch);
    bool _pianoKeyRemove(int pitch);
};



#endif // Player1_h
