#include "APlayer5.h"




/******************************************************************************************************************************
* Constructor
*******************************************************************************************************************************/
Player5::Player5(MidiInterface* mi, LedPanel* lp) {
  _ledPanel  = lp;
  _midi      = mi;
}


/******************************************************************************************************************************
* Initialize song to play / practice
*******************************************************************************************************************************/
void Player5::startSong(Song* song, int startMeasureNr, byte midiPlay, bool repeat) {
  /* Copy some basic data/pointers from the song object, for performance reasons... */
  _noteCount = song->noteCount;   
  _notes = song->notes;   
  /* prepare members regarding playing the song. */
  _doRepeat = repeat;
  _scheduledNotes.reset();
  _midiPlay = midiPlay;
  _songFinished = false;
  curMeasureNr = startMeasureNr;
  _curNoteIdx = song->getMeasureStartIndex(startMeasureNr); 
  _moveToFirstNote();                   /* let _curNoteIdx refer to first real Note (not Measure) */
  _registerPianoKeysCurrentPosition();  /* 1 bit for each key (user should press) before proceed to next position */
  
  _drawNoteLetters();
  isPlaying = true;
}


/******************************************************************************************************************************
* This is called constantly to handle everything that needs to be done to play / practice the song
*******************************************************************************************************************************/
void Player5::handlePlaying() {
  int pitch;
  bool done = false;                  /* all neccessary piano keys pressed?  */
  uint32_t now = millis();
  
  do {
    pitch = _midi->getPressedPianoKey(); /* zero if no piano key was pressed */
    if (pitch >= MIDI_PITCH_MIN && pitch <= MIDI_PITCH_MAX) {
      _ledPanel->setPixel(pitch - MIDI_PITCH_MIN, 4, COLOR_IDX_OFF); /* set LED off */    
      done = _pianoKeyRemove(pitch);
    }
  } while (pitch != 0);       /* loop, 'cause more piano keys may be pressed simultaniously */

  /* check which scheduled notes should be played now. */
  byte* scheduledPitch;
  while ( (scheduledPitch = _scheduledNotes.checkForRelease(now)) != NULL) {
    byte noteVelocity = MIDI_DEFAULT_VELOCITY; /* standard velocity */
    byte velocity = noteVelocity >> 3;  /*  1/8  of velocity */
    if      (_midiPlay == PLAY_WHILE_PRACTICE_VOLU_1) velocity = (noteVelocity>>2) + velocity; /* 1/4 + 1/8 = 37% */
    else if (_midiPlay == PLAY_WHILE_PRACTICE_VOLU_2) velocity = (noteVelocity>>1) + velocity; /* 1/2 + 1/8 = 62% */        
    else                                              velocity = noteVelocity;                 /* 100% velocity  */
    _midi->playNote(*scheduledPitch, velocity, now + 2000);    /* note-off after 2000ms */
  }

  _midi->handleDelays(now);
  
  if (done) {              /* all neccessary keys have been pressed: go to next step!  */
    _moveToNoteAtNewTick();
    _registerPianoKeysCurrentPosition();
    /* NOTE: Updating the LED panel (writeLeds_asm()) cannot be combined by reading piano keys (using 'Serial1' object).
     *       This is because 'writeLeds_asm()' DISABLES INTERRUPTS temporarily, while 'Serial1' NEEDS INTERRUPTS for reading data.
     *       So the LED panel is only updated (see below) after ALL neccessary piano keys have been pressed.
     *       More frequent updates (e.g. after each key) will result in missing bytes by 'Serial1' (thus missing pressed piano keys).   */
    _drawNoteLetters();
  }
}

void Player5::_moveToFirstNote() {
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


void Player5::_moveToNoteAtNewTick() {
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



void Player5::_registerPianoKeysCurrentPosition() {
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
      _pianoKeyRegister(note->pitch, note->finger);
      if (_midiPlay != PLAY_WHILE_PRACTICE_OFF) {
        _scheduledNotes.add(note->pitch, millis() + 1000); /* play note after 1000ms */    
      }      
    }
    noteIdx++;
    if (noteIdx == _noteCount) noteIdx = 0; /* repeat: start over again... */
    note = &_notes[noteIdx];
  } while (!tickPosFound || note->atTick == tickPos);
}



/******************************************************************************************************************************
* Draw notes on LED panel (like: D F#) in right finger-colors
*******************************************************************************************************************************/
void Player5::_drawNoteLetters() {
  static char notes[]  = "C C#D D#E F F#G G#A A#B ";
  static char widths[] = "445544444555"; /* widths in pixels of letters CCDDEFFGGAAB */
  const uint8_t PHASE_MEASURE = 0; /* firstly, only measure size in pixels/LEDcolumns */
  const uint8_t PHASE_DRAW = 1;    /* secondly, do the drawing, so it can be centered */
  const uint8_t PHASE_FINISHED = 2;
  
  _ledPanel->clear();

  int x = 0;
  int totalWidth; /* measured in PHASE_MEASURE, used in PHASE_DRAW */
  uint8_t phase = PHASE_MEASURE;
  bool isOnlyLeft = false;  /* are there only notes for left hand? */
  bool isOnlyRight = false; /* are there only notes for right hand? */
  while (phase != PHASE_FINISHED) { /* within this 'while': PHASE_MEASURE or PHASE_DRAW */
    bool isAnyLeft = false;
    bool isAnyRight = false;
    bool isAnyUnknown = false;
    int formerFinger = -1;
    if (phase == PHASE_DRAW) {
      /* PHASE_MEASURE is done, so calculate the x to start drawing */
      int unusedCols = PANEL_COLS - totalWidth;
      if (isOnlyLeft || isOnlyRight) totalWidth += 7; /* little arrow + spacing */
      x = (PANEL_COLS - totalWidth ) / 2; /* this is where drawing will start, to align (center) the text */
      if (isOnlyRight) {
        /* when there is only right hand notes, draw a little arrow first */
        _ledPanel->writeChar(')', x, COLOR_IDX_GREY + 5 ); /* '(' = little arrow,  + 5 means: less brightness of this color grey. */
        x += 7;        
      }
    }
    /* there is a maximum of MAX_KEYS_IN_STEP registered 'current' piano keys */
    for (int i = MAX_KEYS_IN_STEP-1; i >= 0; i--) /* loop from low pitch keys to high pitch keys */
    {
      int val = _currentPianoKeys[i];
      if (val == 0) continue;  /* empty slot in array: nothing registered here */
      uint8_t pitch = val & 0xFF;        /* pitch data in low byte */
      uint8_t finger = (val >> 8) & 0xF; /* finger (0-10) in bit 8-11 */    
      if (phase == PHASE_MEASURE) {
        if (SongNote::isFingerLeft(finger)) isAnyLeft = true;
        else if (SongNote::isFingerRight(finger)) isAnyRight = true;
        else isAnyUnknown = true;
      }
      uint8_t color = _ledPanel->getFingerColorIdx(finger);
      pitch %= 12; /* pitch is now between 0 and 11. 0=C, 1=C#, 2=D, 3=D#, ... ,11=B */ 
  
      if (formerFinger != -1) {
        x++;
        if (!_isSameHand(finger, formerFinger) ) {
          if (phase == PHASE_DRAW) {
            _ledPanel->writeChar('-', x, COLOR_IDX_GREY + 5 ); /* '-' indicates switch of hand */
          }          
          x += 5; /* different hand? -> extra spacing */
        }
      }
      if (phase == PHASE_DRAW) {
        _ledPanel->writeChar(notes[pitch*2], x, color ); /* Note letter, like C,D,E,F,G,A,B */
      }
      uint8_t w = widths[pitch] - '0';
      x += (w + 1); /* width of letter + 1 pixel spacing */
      if (notes[pitch*2+1] == '#')
      {
        if (phase == PHASE_DRAW) {
          _ledPanel->writeChar('#', x, color ); /* sharp symbol (#) to indicate black piano key */
        }
        x += (5 + 1); /* width of '#' symbol + 1 pixel spacing */
      }
      formerFinger = finger;
    }
    if (phase == PHASE_MEASURE) {
      if (!isAnyUnknown) {
        if (isAnyLeft && !isAnyRight) isOnlyLeft = true;
        if (isAnyRight && !isAnyLeft) isOnlyRight = true;
      }
      totalWidth = x - 1; /* end of measure phase: now we know total width (LED columns) of all text */
    }
    if (phase == PHASE_DRAW) {
      if (isOnlyLeft) {
        x += 2;
        _ledPanel->writeChar('(', x, COLOR_IDX_GREY + 5 ); /* ( = little arrow,  + 5 means: less brightness of this color grey. */
      }
    }
    phase++;
  }
  _ledPanel->writeLeds_asm(); /* display the changed LED-matrix  */
}


/******************************************************************************************************************************
* Just determine whether 2 given finger numbers are of the same hand (left / right)
*******************************************************************************************************************************/
bool Player5::_isSameHand(uint8_t finger1, uint8_t finger2) {
  if (SongNote::isFingerLeft(finger1) && SongNote::isFingerLeft(finger2) ) return true;   /* both left hand */
  if (SongNote::isFingerRight(finger1) && SongNote::isFingerRight(finger2) ) return true; /* both right hand */
  if (finger1 == 0 && finger2 == 0) return true; /* both unknown finger/hand */
  return false; /* finger1 is NOT on same hand as finger2 */
}


/******************************************************************************************************************************
* delete array of current piano keys to play
*******************************************************************************************************************************/
void Player5::_pianoKeyReset() {
  for (int i=0; i < MAX_KEYS_IN_STEP; i++)
  {
    _currentPianoKeys[i] = 0;
  }
}


/******************************************************************************************************************************
* add/register piano key to play
*******************************************************************************************************************************/
void Player5::_pianoKeyRegister(int pitch, int finger) {
  int i = 0;
  do
  {
    if (_currentPianoKeys[i] == 0) /* empty slot found */
    {
      /* combine pitch/finger/to-play-bit all in one 16-bit value */
      _currentPianoKeys[i] = pitch + (finger << 8) + TO_PLAY_BIT_MASK;
      break;
    }
  }
  while (++i < MAX_KEYS_IN_STEP);
}


/******************************************************************************************************************************
* remove the to-play-bit (TO_PLAY_BIT_MASK) of registered piano key with given pitch
*******************************************************************************************************************************/
bool Player5::_pianoKeyRemove(int pitch) {

  bool anyToPlay = false;
  for (int i=0; i < MAX_KEYS_IN_STEP; i++)
  {
    uint16_t val = _currentPianoKeys[i];
    if ( (val & 0xFF) == pitch && (val & TO_PLAY_BIT_MASK) )
    {
      val ^= TO_PLAY_BIT_MASK; /* toggle to-play-bit -> becomes zero */
      _currentPianoKeys[i] = val;
    }
    anyToPlay |= (val & TO_PLAY_BIT_MASK);
  }
  return (!anyToPlay); /* if true, then all piano keys are played -> can go to next step */
}


/******************************************************************************************************************************
* Only return true if the song is finished. When repeat is ON, then always return false.
*******************************************************************************************************************************/
bool Player5::isSongFinished() {
  return _songFinished;
}


/******************************************************************************************************************************
* Immediately stops playing current song.
*******************************************************************************************************************************/
bool Player5::stopPlayingNow() {
  if (!isPlaying) return false;
  _scheduledNotes.reset();
  _midi->handleAllDelaysImmediately();
  isPlaying = false;
  return true;
}
