#include "APlayer0.h"





/******************************************************************************************************************************
* Constructor for playing song WITHOUT LEDs
*******************************************************************************************************************************/
Player0::Player0(MidiInterface* mi, Metronome* m) : Player0(mi, m, NULL) { 
  _withLEDs  = false;
}

/******************************************************************************************************************************
* Constructor for playing song WITH LEDs
*******************************************************************************************************************************/
Player0::Player0(MidiInterface* mi, Metronome* m, LedPanel* lp) {
  _ledPanel  = lp;
  _midi      = mi;
  _metronome = m;
  _withLEDs  = true;            /* with or without auto-playing using MIDI commands */
}


/******************************************************************************************************************************
* Play the song. It's possible to change the standard tempo with a factor. Song can be played once or be repeated.
*******************************************************************************************************************************/
void Player0::startSong(Song* song, uint16_t tempoFactor, bool repeat) {
  /* Copy some basic data/pointers from the song object, for performance reasons... */
  _resolution = song->resolution;  /* ticks per quarter note */
  _totalTicks = song->totalTicks;     
  _noteCount = song->noteCount;   
  _notes = song->notes;   

  /* prepare members regarding playing the song. */
  _withLEDs =      (_ledPanel != NULL);     /* true if LED should be blinked per note on the LED-panel */
  _doRepeat = repeat;
  _currentTick = 0;
  _lastMillis = millis() + 50;              /* millis of last played note -> start song after 50 ms from now */
  _curNoteIdx = 0; 

  _tempoFactor = tempoFactor;                    /* factor to change normal tempo (10% - 180%)  */ 
  _tempoQPM = 0;  /* Tempo (in Quarter Notes per minute) is set later, when first measure is handled */
  _tempo    = 0;  /* Tempo (in milliseconds per Quarter Note) is set later, when first measure is handled */  
  
  _ledOffSchedule.reset();  /* ensure that _notesOffToDo starts EMPTY */
  isPlaying = true;

  //_midi->selectInstrument(5); /* 88 = Synth Piano on 'Yamaha P-121 Digital Piano' */
}


/******************************************************************************************************************************
* Call this non-blocking function constantly to play the song.
*******************************************************************************************************************************/
void Player0::handlePlaying() {
  if (!isPlaying) return;
  byte* pitch;     /* MIDI pitch */
  bool ledPanelDirty = false;
  uint32_t now =  millis();
  uint32_t d;      /* duration in ms */

  SongNote* note;
  SongMeasure* measure;
  while ((note = _checkNewNote(now)) != NULL) {  /* as long as there are notes ready to be played */
    switch(note->type) {
      case TYPE_MEASURE:       /* not a note, but start of new measure -> tell the metronome */
      case TYPE_MEASURE_BM:
        measure = (SongMeasure*)note;
        _setTempo(measure->tempoQPM);  /* change tempo from here?  */
        d = _getMillisDuration(measure->beatTicks);
        _metronome->startNewMeasure(measure->beatCount, d, now + 3);  /* d = beat duration in ms */
        break;
      case TYPE_NOTE:
        uint32_t d = _getMillisDuration(note->duration);
        _midi->playNote(note->pitch, note->velocity, now + d - (d/10));    /* play the note... */
        if (_withLEDs) {                                                   /* ...and possibly show it (turn LED on) */
          byte color = _colorIdx_fingers[note->finger];             /* color for the finger to play */
          for (int row=1; row <= 4; row++) { /* row 1,2,3,4 on LED panel */
            _ledPanel->setPixel(note->pitch - MIDI_PITCH_MIN, row , color);
          }
          _ledOffSchedule.add(note->pitch, now + d - (d/10) );         /* send noteOff MIDI message at a later time */
          ledPanelDirty = true;    
        }
        break;
    }
  }
  _midi->handleDelays(now);
  _metronome->handleBeats(now);
  if (_withLEDs) {
    /* check if it is time to turn off one or more LEDs  */
    while ( (pitch = _ledOffSchedule.checkForRelease(now)) != NULL) {
          for (int row=1; row <= 4; row++) { /* row 1,2,3,4 on LED panel */
            byte clr = row == 4 ? COLOR_IDX_WHITE + 7 : COLOR_IDX_OFF; /* on row 4: set color to grey, other rows: turn LED off */
            _ledPanel->setPixel(*pitch - MIDI_PITCH_MIN, row , clr); 
          }
          ledPanelDirty = true;
    }
    if (ledPanelDirty) {
      _ledPanel->writeLeds_asm(); /* display the changed LED-matrix  */
      ledPanelDirty = false;
    }  
  }
}



/******************************************************************************************************************************
* Immediately stops playing current song.
*******************************************************************************************************************************/
bool Player0::stopPlayingNow() {
  if (!isPlaying) return false;
  isPlaying = false;
  _metronome->reset();
  _midi->handleAllDelaysImmediately();
  uint32_t now =  millis();
  byte* pitch;
  while ( (pitch = _ledOffSchedule.checkForRelease(now + 60000 /* 1 minute in future */ )) != NULL) {
    for (int row=1; row <= 4; row++) { /* row 1,2,3,4 on LED panel */
      byte clr = row == 4 ? COLOR_IDX_WHITE + 7 : COLOR_IDX_OFF; /* on row 4: set color to grey, other rows: turn LED off */
      _ledPanel->setPixel(*pitch - MIDI_PITCH_MIN, row , clr); 
    }
  }
  return true;
}



/******************************************************************************************************************************
* Returns a SongNote when time is ready for that note.
* When 'type' of SongNote is TYPE_MEASURE, the SongNote* should be casted to SongMeasure*  
*                                          (this trick is needed for performance reasons)
*******************************************************************************************************************************/
SongNote* Player0::_checkNewNote(uint32_t now) {
  if (!isPlaying) { return NULL; }
  uint32_t dTicks;
  SongNote* note;
  bool newNoteAvailable = (_curNoteIdx < _noteCount);
  if (newNoteAvailable) {
    note = &_notes[_curNoteIdx];
    dTicks = note->atTick - _currentTick;  /* MIDI-ticks between former note and this new note ( >= 0 ) */
  }
  else {  /* no new notes  */
    dTicks = _totalTicks - _currentTick;       /* MIDI-ticks between former (also last) note and the end of song ( >= 0 ) */
  }
  uint32_t dMillis = _getMillisDuration(dTicks);   /* convert ticks to milliseconds  */
  uint32_t elapsed = now - _lastMillis;            /* milliseconds passed since former note */
  if (elapsed > 0x80000000) return NULL;          /* this means overflow (thus: now < lastMillis), can happen at start of song */
  if (elapsed >= dMillis) {     /* time to play new note? or to end song? */
    _currentTick += dTicks;
    _lastMillis += dMillis;
    if (newNoteAvailable) { _curNoteIdx++; return note; }
    /* End of song reached. Repeat song, yes or no? */
    if (_doRepeat) { _currentTick = 0; _curNoteIdx = 0; }
    isPlaying = _doRepeat;   
  }
  return NULL;    /* not yet time to play next note, or song has just finished. */
}


/******************************************************************************************************************************
* Tempo is set when first measure is handled and also when tempo must change (tempo change is possible per measure, not per note!)
*******************************************************************************************************************************/
void Player0::_setTempo(uint16_t qpm) {
  if (qpm == _tempoQPM) return; /* tempo not changed: do nothing */
  _tempoQPM = qpm;  /* in Quarter Notes per minute */
  _tempo = 6000000 / (_tempoQPM * _tempoFactor); /* in Milliseconds per Quarter Note */    
}

/******************************************************************************************************************************
* Convert duration from MIDI-ticks to Milliseconds  :    ticks * (ms/QN) / (ticks/QN) = ms
*******************************************************************************************************************************/
uint32_t Player0::_getMillisDuration(uint32_t ticks) {
  return ((uint32_t)ticks * _tempo / _resolution);
}
