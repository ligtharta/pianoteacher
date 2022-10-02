#include "APlayer1.h"




/******************************************************************************************************************************
* Constructor
*******************************************************************************************************************************/
Player1::Player1(MidiInterface* mi, LedPanel* lp, Gloves* g) {
  _ledPanel  = lp;
  _midi      = mi;
  _gloves    = g;
}


/******************************************************************************************************************************
* Initialize song to play / practice
*******************************************************************************************************************************/
void Player1::startSong(Song* song, int startMeasureNr, bool withGloves, byte panelRowsUsed, bool repeat) {
  /* Copy some basic data/pointers from the song object, for performance reasons... */
  _noteCount = song->noteCount;   
  _notes = song->notes;   
  /* prepare members regarding playing the song. */
  _doRepeat = repeat;
  _panelRowsUsed  = panelRowsUsed; /* how many rows of LED panel used to display notes to play? */
  _songFinished = false;
  curMeasureNr = startMeasureNr;
  _curNoteIdx = song->getMeasureStartIndex(startMeasureNr); 
  _moveToFirstNote();                   /* let _curNoteIdx refer to first real Note (not Measure) */
  _registerPianoKeysCurrentPosition();  /* 1 bit for each key (user should press) before proceed to next position */
  _gloves->reset(withGloves);
  _drawLEDpanel();
  isPlaying = true;
}



/******************************************************************************************************************************
* This is called constantly to handle everything that needs to be done to play / practice the song
*******************************************************************************************************************************/
void Player1::handlePlaying() {
  int pitch;
  bool done = false;                  /* all neccessary piano keys pressed?  */
  do {
    pitch = _midi->getPressedPianoKey(); /* zero if no piano key was pressed */
    if (pitch >= MIDI_PITCH_MIN && pitch <= MIDI_PITCH_MAX) {
      _ledPanel->setPixel(pitch - MIDI_PITCH_MIN, 4, COLOR_IDX_OFF); /* set LED off */    
      done = _pianoKeyRemove(pitch);
    }
  } while (pitch != 0);       /* loop, 'cause more piano keys may be pressed simultaniously */
  
  if (done) {              /* all neccessary keys have been pressed: go to next step!  */
    _moveToNoteAtNewTick();
    _registerPianoKeysCurrentPosition();
    /* NOTE: Updating the LED panel (writeLeds_asm()) cannot be combined by reading piano keys (using 'Serial1' object).
     *       This is because 'writeLeds_asm()' DISABLES INTERRUPTS temporarily, while 'Serial1' NEEDS INTERRUPTS for reading data.
     *       So the LED panel is only updated (see below) after ALL neccessary piano keys have been pressed.
     *       More frequent updates (e.g. after each key) will result in missing bytes by 'Serial1' (thus missing pressed piano keys).   */
    _drawLEDpanel();
  }
}

void Player1::_moveToFirstNote() {
  SongNote* note;
  SongMeasure* measure; /* we must update 'curMeasureNr' when we come accross a measure */
  while (true) {
    note = &_notes[_curNoteIdx];
    if (note->type == TYPE_NOTE) return; /* YES, note found, that's all! */
    /* note represents a measure (can be casted to SongMeasure), we must update 'curMeasureNr' */
    /* we only come here if the measure where we started did not contain any notes  */
    measure = (SongMeasure*)note;
    curMeasureNr = measure->measureNr;   /* update 'curMeasureNr' */
    _curNoteIdx++;    
  }
}

void Player1::_moveToNoteAtNewTick() {
  SongNote* note = &_notes[_curNoteIdx];
  SongMeasure* measure;   /* we must update 'curMeasureNr' when we come accross a measure */
  uint32_t oldTickPos = note->atTick;
  uint32_t newTickPos;

  do {
    _curNoteIdx++;
    if (_curNoteIdx == _noteCount) { /* end of song reached... */
      if (_doRepeat) {
        _curNoteIdx = 0; /* repeat: start over again... */
      }
      else {
        _songFinished = true;
        return;    /* end reached with no repeat */
      }
    }
    note = &_notes[_curNoteIdx];
    newTickPos = note->atTick;
    if (note->type != TYPE_NOTE) { /* it's a measure, thus we must update 'curMeasureNr' */
      measure = (SongMeasure*)note;
      curMeasureNr = measure->measureNr;   /* update 'curMeasureNr' */
    }
  } while (newTickPos == oldTickPos || note->type != TYPE_NOTE);
}


void Player1::_drawLEDpanel() {
  _ledPanel->clear();
  _gloves->clearAllFingers();
  int noteIdx = _curNoteIdx;
  if (noteIdx < _noteCount) {  /* not yet end reached ? */
    for (int row = 4; row >= 0; row--) { /* notes on row 4 of LED-panel must be played first, row 3 thereafter, etc.  */
      SongNote* note = &_notes[noteIdx];
      SongMeasure* measure;
      uint32_t tickPos = note->atTick;
      bool tickPosValid = (note->type == TYPE_NOTE); /* if not a note, then overwrite 'tickPos' in do-loop below */
      byte noteType;
      do {
        noteType = note->type;
        switch(noteType) {
          case TYPE_MEASURE:
          case TYPE_MEASURE_BM:
            if (row == 4) { /* a measure here? Then actually starts with the next measure.. */
              measure = (SongMeasure*)note;
              curMeasureNr = measure->measureNr; /* curMeasureNr is needed when user navigates through song with foot pedals */
            }
            break;
          case TYPE_NOTE:
            if (!tickPosValid) {
              /* current tickPos is of measure (not a first note at a new tick), so correct this... */
              tickPos = note->atTick;
              tickPosValid = true;
            }
            byte color = _colorIdx_fingers[note->finger];     /* color for the finger to play */

            if ( _panelRowsUsed >= (5 - row) ) { /* bottom row is always used (displays what to play NOW), other rows depend on setting  */
              _ledPanel->setPixel(note->pitch - MIDI_PITCH_MIN, row, color); /* set LED on */                
              if (row > 0 && User::leftHandCue && note->isFingerLeft()) { /* should indication for left hand note be shown? */
                _ledPanel->setPixel(note->pitch - MIDI_PITCH_MIN, row - 1, COLOR_IDX_GREY + 7);  /* grey LED above means: should be played with left hand */
              }
            }
            if (row == 4) { /* row 4 represents notes that should be played NOW */
              _gloves->setFinger(note->finger, true);
            }
            break;
        }
        noteIdx++;
        if (noteIdx == _noteCount) {
          if (_doRepeat)
            noteIdx = 0; /* repeat: start over again... */
          else
            break; /* end reached with no repeat  */
        }
        note = &_notes[noteIdx];
      } while (note->atTick == tickPos || noteType != TYPE_NOTE); /* skip/ignore measures */
      if (noteIdx == _noteCount) break;   /* end reached with no repeat  */
    }
  }
  // _drawMeasureNr(); /* uncomment to display current measureNr on LED panel (experimental feature!) */ 
  _ledPanel->writeLeds_asm(); /* display the changed LED-matrix  */
  _gloves->updateGloves();
}


/******************************************************************************************************************************
* Display current measureNr on the LED panel (experimental feature)
*******************************************************************************************************************************/
void Player1::_drawMeasureNr() {
  static bool lastWasLeft = true;  /* try left side first, keep at choosen side for as long as possible */
  static char charBuf[4];          /* for measureNr in text */
  itoa(curMeasureNr, charBuf, 10); /* convert measureNr to text */ 
  int w = _ledPanel->getTextWidth(charBuf); /* the width of the text in pixels/LEDs  */
  int x2 = PANEL_COLS - 1; /* the very left column of the LED panel */
  bool canLeft = true;  /* possible to put measureNr at left side? */
  bool canRight = true; /* possible to put measureNr at right side? */
  /* below: check if there is space on the LED panel at the left and right side... */
  for (int x = 0; x <= w; x++) { /* check for w+1 columns (1 column margin) */
    canLeft  &= _ledPanel->IsColumnEmpty(x);
    canRight &= _ledPanel->IsColumnEmpty(x2--);
  }
  if (!(canLeft || canRight)) return; /* there is no place to put measureNr on LED panel! */
  if (lastWasLeft  && !canLeft ) lastWasLeft = false; /* switch placement to right side */
  if (!lastWasLeft && !canRight) lastWasLeft = true;  /* switch placement to left side */
  /* write measure-nr to LED panel:  */
  _ledPanel->writeText(charBuf, lastWasLeft ? 0 : (PANEL_COLS - w), COLOR_IDX_YELLOW);
}





void Player1::_registerPianoKeysCurrentPosition() {
  _pianoKeyReset();
  if (_curNoteIdx == _noteCount) return; /* end reached, no repeat */
  int noteIdx = _curNoteIdx;
  SongNote* note = &_notes[noteIdx];
  bool tickPosFound = false;
  uint32_t tickPos; // = note->atTick;
  do {
    if (note->type == TYPE_NOTE) {
      if (!tickPosFound) { 
        tickPos = note->atTick; 
        tickPosFound = true; 
      }
      _pianoKeyRegister(note->pitch);
    }
    noteIdx++;
    if (noteIdx == _noteCount) noteIdx = 0; /* repeat: start over again... */
    note = &_notes[noteIdx];
  } while (!tickPosFound || note->atTick == tickPos);
}





void Player1::_pianoKeyReset() {
  _bitPerPianoKey[0] = 0;
  _bitPerPianoKey[1] = 0;
  _bitPerPianoKey[2] = 0;
}


void Player1::_pianoKeyRegister(int pitch) {
  pitch -= MIDI_PITCH_MIN;
  if (pitch < 32)
    _bitPerPianoKey[0] |= (1UL << (pitch));
  else if (pitch < 64)
    _bitPerPianoKey[1] |= (1UL << (pitch-32));
  else
    _bitPerPianoKey[2] |= (1UL << (pitch-64));
}

bool Player1::_pianoKeyRemove(int pitch) {
  pitch -= MIDI_PITCH_MIN;
  if (pitch < 32)
    _bitPerPianoKey[0] &= ~(1UL << (pitch));
  else if (pitch < 64)
    _bitPerPianoKey[1] &= ~(1UL << (pitch-32));
  else
    _bitPerPianoKey[2] &= ~(1UL << (pitch-64));
  if (_bitPerPianoKey[0] != 0) return false;
  if (_bitPerPianoKey[1] != 0) return false;
  if (_bitPerPianoKey[2] != 0) return false;
  return true;
}


/******************************************************************************************************************************
* Only return true if the song is finished. When repeat is ON, then always return false.
*******************************************************************************************************************************/
bool Player1::isSongFinished() {
  return _songFinished;
}


/******************************************************************************************************************************
* Immediately stops playing current song.
*******************************************************************************************************************************/
bool Player1::stopPlayingNow() {
  if (!isPlaying) return false;
  _gloves->reset(false);
  isPlaying = false;
  return true;
}
