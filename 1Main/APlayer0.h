#ifndef Player0_h
#define Player0_h

#include <Arduino.h>
#include "Templates.h"
#include "Entities.h"
#include "Metronome.h"
#include "LedPanel.h"
#include "Midi.h"


#define LED_OFF_SCHEDULE_MAX  20    /* how many LED-off commands can the DelayManager hold?  */

/*
*
*
*/
class Player0 {
  public:
    Player0(MidiInterface* mi, Metronome* m);
    Player0(MidiInterface* mi, Metronome* m, LedPanel* lp);

    /* playing the song */
    void startSong(Song* song, uint16_t tempoFactor, bool repeat);
    void handlePlaying();
    bool stopPlayingNow();
   
    bool isPlaying = false;
    
  protected:

  private:
    /* basic song info (copied from Song object) */
    uint32_t _resolution;          /* [resolution: ticks/quarter note] */
    int _totalTicks;               /* [in total ticks] */
    int _noteCount;                /* how many of MAX_NOTES elements used? */
    SongNote* _notes;              /* notes within song */

    /* references to needed objects */
    MidiInterface* _midi;
    LedPanel*      _ledPanel;
    Metronome*     _metronome;

    /* playing the song */
    uint32_t _currentTick;  /* when was last note played (MIDI-ticks) ? */
    uint32_t _lastMillis;   /* when was last note played (milli-seconds) ? */
    int _curNoteIdx;        /* index of current note */
    bool _withLEDs;         /*  show LEDs while playing ? */
    bool _doRepeat;         /*  repeat after end of song? */
    DelayManager<byte, LED_OFF_SCHEDULE_MAX>  _ledOffSchedule;   /* when should playing-LED be turned off */
    /* tempo management */
    unsigned int _tempo;           /* [tempo: ms/quarter note. Example: 461 means ~130 beats/min (= 60.000 / 461) ] */
    uint16_t _tempoFactor;
    uint16_t _tempoQPM;            /* tempo in quarter notes per minute */

    SongNote* _checkNewNote(uint32_t now);        /* is there a new note ready to be played?  */
    uint32_t _getMillisDuration(uint32_t ticks);  /* MIDI ticks -> milliseconds */
    void _setTempo(uint16_t qpm);
};



#endif // Player0_h
