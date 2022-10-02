#include "APlayer4.h"






/******************************************************************************************************************************
* Constructor for playing song
*******************************************************************************************************************************/
Player4::Player4(MidiInterface* mi, LedPanel* lp) {
  _ledPanel  = lp;
  _midi      = mi;
  _measuresPracticed = 0; /* increases when user completes a measure */
}


/******************************************************************************************************************************
* Start the song at the given measure
*******************************************************************************************************************************/
void Player4::startSong(Song* song, int startMeasureNr, byte midiPlay) {
  /* Copy some basic data/pointers from the song object, for performance reasons... */
  _noteCount = song->noteCount;   
  _notes = song->notes;   
  _lastMeasureNr = song->lastMeasureNr;
  _song = song;
  /* prepare members regarding playing the song. */
  _scheduledNotes.reset();
  _midiPlay = midiPlay;
  _startMeasure(startMeasureNr);
  _prepareStepAndMoveToNext();  /* 1 bit for each key (user should press) before proceed to next position */
  _sustainDown = false;
  _drawLEDpanel(true);
  isPlaying = true;
}




/******************************************************************************************************************************
* Call this non-blocking function 'very often' to play the song.
*******************************************************************************************************************************/
void Player4::handlePlaying() {
  int pitch;
  bool stepDone = false;                  /* all neccessary piano keys pressed?  */
  uint32_t now = millis();
  bool sustainPressed, sustainReleased;
  do {
    pitch = _midi->getPressedPianoKey(&sustainPressed, &sustainReleased); /* zero if no piano key was pressed */
    if (sustainPressed) { _sustainDown = true; _drawLEDpanel(false); }
    if (sustainReleased){ _sustainDown = false; _drawLEDpanel(true); }
    if (pitch != 0) {
      bool isWrongKey;
      bool changed = _pianoKeyRemove(pitch, &stepDone, &isWrongKey);
      /* not an expected piano key pressed -> register error for this step */
      if (isWrongKey) _errorPerStep[_stepIndex] = true;  
    }
  } while (pitch != 0);       /* loop, 'cause more piano keys may be pressed simultaniously */

  /* check which scheduled notes should be played now. */
  byte* scheduledPitch;
  while ( (scheduledPitch = _scheduledNotes.checkForRelease(now)) != NULL) {
    byte noteVelocity = MIDI_DEFAULT_VELOCITY; /* standard velocity */
    byte velocity = noteVelocity >> 3;  /*  1/8  of vecolity */
    if      (_midiPlay == PLAY_WHILE_PRACTICE_VOLU_1) velocity = (noteVelocity>>2) + velocity; /* 1/4 + 1/8 = 37% */
    else if (_midiPlay == PLAY_WHILE_PRACTICE_VOLU_2) velocity = (noteVelocity>>1) + velocity; /* 1/2 + 1/8 = 62% */        
    else                                              velocity = noteVelocity;                 /* 100% velocity  */
    _midi->playNote(*scheduledPitch, velocity, now + 2000);    /* note-off after 2000ms */
  }

  _midi->handleDelays(now);

  if (stepDone) {
    _prepareStepAndMoveToNext();
    _stepIndex++;
    if (_stepIndex == _stepCount) {
      /* measure completed: user has gone through all steps */
      _measureCompleted(); /* paint a green or red pixel to show accomplishments */
      /* now start a new measure... */
      if (curMeasureNr == _lastMeasureNr) curMeasureNr = 1; else curMeasureNr++;  /* go to next or start over. */
      _startMeasure(curMeasureNr);
      _prepareStepAndMoveToNext();
      _drawLEDpanel(true);        
    }
    else
    {
      _drawLEDpanel(false);      
    }
  }
}


/******************************************************************************************************************************
* Immediately stops playing current song.
*******************************************************************************************************************************/
bool Player4::stopPlayingNow() {
  if (!isPlaying) return false;
  _scheduledNotes.reset();
  _midi->handleAllDelaysImmediately();
  isPlaying = false;
  return true;
}

/******************************************************************************************************************************
* If the given measure does not contain notes, increase curMeasureNr until the measure does have at least a note inside
* Analyse how many steps are in this measure (sets of notes that start at the same time/tick): 
*                - fill _stepsInMeasure[] array -> how many notes per step?
*                - set _stepCount               -> how many steps?
* Ensure that _curNoteIdx points to the first note within the measure to start (curMeasureNr)
*******************************************************************************************************************************/
void Player4::_startMeasure(int measureNr) {
  bool emptyMeasure, emptyLastMeasure;
  curMeasureNr = measureNr;
  _stepCount = 0;
  _stepIndex = 0;
  do {
    _curNoteIdx = _song->getMeasureStartIndex(curMeasureNr);
    _curNoteIdx++; /* index points to 1st note in measure (or to next measure if not notes in measure) */
    emptyMeasure = emptyLastMeasure = (_curNoteIdx == _noteCount); /* end of song -> last measure has no notes */
    if (!emptyMeasure) emptyMeasure = (_notes[_curNoteIdx].type != TYPE_NOTE); /* no notes, but new measure immediately  */
    if (emptyLastMeasure) {
      curMeasureNr = 1; /* repeat, start over at first measure */
    }
    else if (emptyMeasure) {
      curMeasureNr++;
    }
  } while(emptyMeasure); /* next measure until the measure does have notes inside */
  /* now we know curMeasureNr is a measure with at least a note inside */
  int noteIdx = _curNoteIdx;
  uint32_t tick = _notes[noteIdx].atTick;
  _notesPerStep[0] = 0;
  
  while (true) {
    if (noteIdx == _noteCount) break; /* no more notes, because end of last measure */
    SongNote* note = &_notes[noteIdx];
    if (note->type != TYPE_NOTE) break;  /* no more notes, because start of new measure*/
    if (note->atTick == tick) {
      _notesPerStep[_stepCount]++;
    }
    else {
      tick = note->atTick;
      _stepCount++;
      _notesPerStep[_stepCount] = 1;
    }
    noteIdx++;
  }
  _stepCount++;
  for (int i=0; i<_stepCount; i++) _errorPerStep[i] = false;
}

/******************************************************************************************************************************
* _pianoKeyRegister(pitch) is called for each note within current step
* Ensure that _curNoteIdx points to next step or next measure
*******************************************************************************************************************************/
void Player4::_prepareStepAndMoveToNext() {
  _pianoKeyReset();
  SongNote* note = &_notes[_curNoteIdx];
  bool tickPosFound = false;
  uint32_t tickPos; 
  byte velocity; /* MIDI: velocity/volume of note */

  while (true) {
    if (_curNoteIdx == _noteCount) return; /* end reached, no more notes */
    note = &_notes[_curNoteIdx];
    if (note->type != TYPE_NOTE) return; /* new measure starts here */
    if (!tickPosFound) { 
      tickPos = note->atTick; 
      tickPosFound = true; 
    }
    else {
      if (note->atTick != tickPos) return; /* notes from here are for next step */
    }
    _pianoKeyRegister(note->pitch, note->finger);
    if (_midiPlay != PLAY_WHILE_PRACTICE_OFF) {
      _scheduledNotes.add(note->pitch, millis() + 1000); /* play note after 1000ms */    
    }
    _curNoteIdx++;
  }
}
 

/******************************************************************************************************************************
* User completed a measure. 
* Register in _practiceResults[] wheter red or green pixel is earned, and increase _measuresPracticed 
*******************************************************************************************************************************/
void Player4::_measureCompleted() {
  bool donePerfect = true;
  for(int i = 0; i < _stepCount; i++) {
    if (_errorPerStep[i]) { donePerfect = false; break; }
  }
  _practiceResults[_measuresPracticed] = donePerfect;
  _measuresPracticed++;
  if (_measuresPracticed == PRACTICE_RESULTS_MAX) _measuresPracticed = 0;
}


/******************************************************************************************************************************
* Update data structure that represents the LED panel and write data to LED panel
*******************************************************************************************************************************/
void Player4::_drawLEDpanel(bool newMeasure) {
  char charBuf[5];            /* to store measure-nr in text */

  if (_sustainDown) /* when sustain is down -> show the notes to play (peek) */
  {
    _ledPanel->clear();

    for (int i=0; i < MAX_KEYS_IN_STEP; i++) {
      uint16_t pitch = _keysCurrStepCopy[i]; 
      if (pitch == 0) continue; /* 0 = empty slot in array / no pitch */
      /* pitch (16-bit) also contains finger number (higher order byte) */
      byte finger = pitch >> 8; /* get finger number by shifting high byte to low byte */
      pitch = (pitch & 0xff);  /* remove finger number from pitch */
      uint16_t x = pitch - MIDI_PITCH_MIN; /* convert MIDI pitch to LED panel column, so that pitch 0 is the very left piano key */
      byte color = _colorIdx_fingers[finger];     /* color for the finger to play */
      _ledPanel->fillRect(x, 1, x, 4, color);
      if (SongNote::isFingerLeft(finger)) { /* is this note to played with left hand? */
        _ledPanel->setPixel(x, 0, COLOR_IDX_GREY + 7);  /* grey LED on top means: should be played with left hand */
      }

    }
    _ledPanel->writeLeds_asm(); /* display the changed LED-matrix  */
    return;  /* while sustain pedal is down, only the above code is needed */
  }
  
  if (newMeasure) {
    _ledPanel->clear();
    /* initial drawing at the start of a new measure */
    itoa(curMeasureNr, charBuf, 10);
    _ledPanel->writeText(charBuf, 25 + PANEL_LEFT_MARGIN, COLOR_IDX_YELLOW); /* write measure-nr */
    /* Steps on the X-axis, notes per step on the Y-axis */
    for (int s=0; s < _stepCount; s++) {
      for (int y=0; y< _notesPerStep[s]; y++) {
        if (y >= 5) continue; /* LED-panel has only 5 rows of LEDs*/
        int x = LED_PANEL_X_FIRST_STEP + s;
        _ledPanel->setPixel(x + PANEL_LEFT_MARGIN, y /* row */ , COLOR_IDX_WHITE + 5 /* dimmed grey */);
      }
    }
  }
  for (int s=0; s <= _stepIndex; s++) {   /* steps from first to current step (within measure)... */
    for (int y=0; y< _notesPerStep[s]; y++) { /* each step can have more notes to play  */
      if (y >= 5) continue; /* LED-panel has only 5 rows of LEDs*/
      int x = LED_PANEL_X_FIRST_STEP + s;
      byte color =  (s < _stepIndex ? COLOR_IDX_GREEN : COLOR_IDX_YELLOW);
      if (s < _stepIndex && _errorPerStep[s]) color = COLOR_IDX_RED;
      _ledPanel->setPixel(x + PANEL_LEFT_MARGIN, y /* row */ , color);
    }
  }
  /* draw accomplishments (red or green pixel per completed measure) */
  for (int i = 0; i< _measuresPracticed; i++) {
    int y = i % 5; /* LED panel row (0,1,2,3,4) */
    int x = i / 5; /* LED panel column */
    _ledPanel->setPixel(x + PANEL_LEFT_MARGIN, y, _practiceResults[i] ? COLOR_IDX_GREEN : COLOR_IDX_RED);  
  } 
  _ledPanel->writeLeds_asm(); /* display the changed LED-matrix  */
}




/******************************************************************************************************************************
* Clear arrays (one slot represents a piano key to be played in current step) so it does not contain piano keys to be played
*******************************************************************************************************************************/
void Player4::_pianoKeyReset() {
  for (int i=0; i < MAX_KEYS_IN_STEP; i++)
    _keysCurrStep[i] = _keysCurrStepCopy[i] = 0;
  _currStepNotesPlayed = 0;
}


/******************************************************************************************************************************
* Register a piano key to be played in current step. Search for empty slot and set pitch value and finger number in a 16-bit value
*******************************************************************************************************************************/
void Player4::_pianoKeyRegister(int pitch, byte finger) {  
  for (int i=0; i < MAX_KEYS_IN_STEP; i++) {
    if (_keysCurrStep[i] == 0) {
      _keysCurrStep[i] = _keysCurrStepCopy[i] = pitch + (finger << 8); /* finger info in higher order byte of the 16-bit int */
      break;
    }
  }
}




/******************************************************************************************************************************
* Remove the piano key (given by pitch parameter) from the array.
* returns true if the piano key needed to be played in current step.
* allDone is set true when all piano keys of current step are played, so we can proceed to next step.
* isWrong is set if the pitch was wrong, that means: not registered with _pianoKeyRegister(pitch, finger)
*******************************************************************************************************************************/
bool Player4::_pianoKeyRemove(int pitch, bool* allDone, bool* isWrong) {
  *allDone = true;
  *isWrong = true;
  bool changed = false;
  int p; /* pitch as stored in array */
  for (int i=0; i < MAX_KEYS_IN_STEP; i++) {
    p = _keysCurrStepCopy[i] & 0xff; /* get the pitch, clear higher order byte to remove finger data */
    if ( p == pitch) { 
      *isWrong = false;
      if (_keysCurrStep[i] != 0) changed = true;
      _keysCurrStep[i] = 0;
    }
    if (_keysCurrStep[i] != 0) *allDone = false; /* allDone will be true only when all array-elements are zero */
  }
  return changed;
}
