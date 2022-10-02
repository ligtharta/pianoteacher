#include "APlayer2.h"





/******************************************************************************************************************************
* Constructor
*******************************************************************************************************************************/
Player2::Player2(LedPanel* lp, Metronome* m, MidiInterface* mi, Gloves* g) {
  _ledPanel = lp;
  _metronome = m;
  _midi = mi;
  _gloves = g;
}


/******************************************************************************************************************************
* Initialize song to play / practice
*******************************************************************************************************************************/
void Player2::startSong(Song* song, int startMeasureNr, bool withGloves, byte midiPlay, uint16_t tempoFactor, bool repeat) {
  uint32_t now = millis();
  /* Copy some basic data/pointers from the song object, for performance reasons... */
  _resolution = song->resolution;  /* ticks per quarter note */
  _totalTicks = song->totalTicks;     
  _noteCount = song->noteCount;   
  _notes = song->notes;   

  /* prepare members regarding playing the song. */
  _realtimeMode = false;        /* not yet first note played by pressing a piano key */
  _doRepeat = repeat;
  _withGloves = withGloves;
  _midiPlay = midiPlay;         /* is auto-playing (MIDI) on? If so, what volume (very low, low, normal)? */
  _songEndMillis = 0;
  curMeasureNr = startMeasureNr;
  _curNoteIdx = song->getMeasureStartIndex(startMeasureNr);  /* points to next note to process */
  _curNoteIdx2 = _curNoteIdx; /* will point to first upcoming note */

  _tempoFactor = tempoFactor;   /* factor to change normal tempo (10% - 180%)  */ 
  _tempoQPM = 0;  /* Tempo (in Quarter Notes per minute) is set later, when first measure is handled */
  _tempo    = 0;  /* Tempo (in milliseconds per Quarter Note) is set later, when first measure is handled */  

  _moveToFirstNote();           /* this will set curNoteIdx and update curMeasureNr */
  _currentTick = _notes[_curNoteIdx].atTick;     /* start tick (is zero when starting at first measure) */
  _timeAhead = 300;                         /* milliseconds to look ahead for upcoming notes to be played shortly */
  _lastMillis = now;           /* millis of last played note -> start song after '_timeAhead' milliseconds from 'now' */
  _missedMillis = 0;                        /* each time the LED-panel is updated, this will increase with 4 (compensate for disabled interrupts) */
  _startupTime = now;
   
  _upcomingArray.reset();
  _ledOffSchedule.reset();
  _updateMeasureNrSchedule.reset();
  _glovesSchedule.reset();
  //_undimRow3Schedule.reset();
  _ledPanel->clear(); /* clear all LEDs in data structure */
  _midi->clearReadBuffer();
  _gloves->reset(withGloves);
  isPlaying = true;
}


void Player2::_moveToFirstNote() {
  SongNote* note;
  SongMeasure* measure; /* we must update 'curMeasureNr' when we come accross a measure */
  while (true) {
    note = &_notes[_curNoteIdx];
    if (note->type == TYPE_NOTE) return; /* YES, note found, that's all! */
    /* note represents a measure (can be casted to SongMeasure), we must update 'curMeasureNr' */
    /* we only come here if the measure where we started did not contain any notes  */
    measure = (SongMeasure*)note;
    curMeasureNr = measure->measureNr;   /* update 'curMeasureNr' */
    _setTempo(measure->tempoQPM);
    _curNoteIdx++;    
  }
}

/******************************************************************************************************************************
* This is called constantly to handle everything that needs to be done to play / practice the song
*******************************************************************************************************************************/
void Player2::handlePlaying() {  
  uint32_t nowCorr;
  uint32_t now = millis();
  bool ledsUpdated = false;
  if (!isPlaying) return;
  if (!_realtimeMode) { /* start-up mode: wait for piano key before entering realtime mode */
    int pitch = _midi->getPressedPianoKey(); /* zero if no piano key was pressed */
    if (pitch != 0) {
      _realtimeMode = true;            /* from now on, real-time mode... */
      _startupDelayMillis = now - _startupTime; /* how long does it take before user presses first piano key? */
    }
    _timeAhead = 0;    /* during start-up mode, only look at the first note(s) to play */
    nowCorr = _startupTime;
    byte oldCount;
    do {
      oldCount = _upcomingArray.count();
      _lookAheadAndSchedule(nowCorr);  
    } while (oldCount != _upcomingArray.count());  
    ledsUpdated = handlePlaying_doStuff(nowCorr);
  }
  else { /* real-time mode */
    _timeAhead = 300;                 /* in realtime mode, look ahead in time */
    uint32_t nowCorr = now + _missedMillis - _startupDelayMillis;
    _lookAheadAndSchedule(nowCorr);
    ledsUpdated = handlePlaying_doStuff(nowCorr);
  }

  if (ledsUpdated) {                          /* anything changed so that LED panel must be re-drawn? */
    _ledPanel->writeLeds_asm();               /* Interrupts will be disabled temporarily and 'millis()' will miss a few milliseconds  */
    if (_realtimeMode) _missedMillis += MILLIS_TO_WRITE_LED_PANEL; /* correct for 'missed interrupts' due to writeLeds_asm()  */
  }
}


bool Player2::handlePlaying_doStuff(uint32_t nowCorr) {
  bool ledsUpdated = false;
  byte velocity; /* MIDI-velocity/volume of note */
  byte* pitch;   /* MIDI-pitch of note */
  byte* mNr;     /* measure number */
  byte* finger;  /* finger-nr for gloves */
  //byte* dummy;   /* byte that signals that row 3 must be undimmed */
  _metronome->handleBeats(nowCorr);
  while ( (mNr = _updateMeasureNrSchedule.checkForRelease(nowCorr)) != NULL) {
    curMeasureNr = *mNr; /* update measure-nr, because new measure starts now...  */
  }
  UpcomingNote* upcoming;
  SongNote* note;
  while( (upcoming = _upcomingArray.getFirst()) != NULL)  {
    if (upcoming->startMillis > nowCorr) break; /* quit while loop, not yet time for this note and all thereafter... */
    note = upcoming->note;

    /* always true: note->type == TYPE_NOTE */ 
    uint32_t d = upcoming->durationMillis;
    if (_midiPlay != PLAY_WHILE_PRACTICE_OFF) {
      /* the note must be played via MIDI, there are 3 possible degrees for velocity/volume */
      velocity = note->velocity >> 3;  /*  1/8  of velocity */
      if      (_midiPlay == PLAY_WHILE_PRACTICE_VOLU_1) velocity = (note->velocity>>2) + velocity; /* 1/4 + 1/8 = 37% */
      else if (_midiPlay == PLAY_WHILE_PRACTICE_VOLU_2) velocity = (note->velocity>>1) + velocity; /* 1/2 + 1/8 = 62% */        
      else                                              velocity = note->velocity;                 /* 100% velocity  */
      _midi->playNote(note->pitch, velocity, upcoming->startMillis + d - (d/10));        
    }
    uint32_t shorten = min(d/7, 300); /* turn off LED quicker than 'official' note length: about 14% but not more than 300ms */
    _ledOffSchedule.add(note->pitch, nowCorr + d - shorten );    /* turn off LED (that indicates a playing note) at a later time */
    byte color = _colorIdx_fingers[note->finger];
    _ledPanel->setPixel(note->pitch - MIDI_PITCH_MIN, 4 /* row 4 */ , color /* color per finger */);
    ledsUpdated = true;
    _upcomingArray.removeFirst(); /* first note in circular array is now handled (not upcoming anymore), so remove it. */
  }
  if (ledsUpdated) { /* if one or more notes played, upcoming notes (LED panel row 0/1/2/3) should be updated, too */
    uint32_t currNoteTick = note->atTick; /* tick of note that is now played and visible on LED panel row 4 */
    /* below: increase _curNoteIdx2 until note found with the same 'atTick' value as currNoteTick */
    while (true) {
      note = &_notes[_curNoteIdx2];
      if (note->type == TYPE_NOTE && note->atTick == currNoteTick) break; /* YES, note found with the right tick, that's all! */
      _curNoteIdx2++;
      if (_curNoteIdx2 == _noteCount) _curNoteIdx2 = 0;
    }
    uint32_t millisToNextNote = _drawUpcomingNotes(currNoteTick);  /* draw upcoming notes on LED panel (row 0/1/2/3) */
//    if (millisToNextNote >= 225) {
//      _undimRow3Schedule.add(1, nowCorr + millisToNextNote - 200);
//    }
  }
  /* Each time a note ends, turn of corresponding LED in row 4 of LED-panel (row 4 shows notes that are currently played).  */
  while ( (pitch = _ledOffSchedule.checkForRelease(nowCorr)) != NULL) {
    _ledPanel->setPixel(*pitch - MIDI_PITCH_MIN, 4 /* row 4 */, COLOR_IDX_OFF); /* LED off*/
    ledsUpdated = true;
  }
//  while ( (dummy = _undimRow3Schedule.checkForRelease(nowCorr)) != NULL) {
//    //_ledPanel->setRowDimming(3, 0);
//    _ledPanel->setRowDimming(4, 5);
//    ledsUpdated = true;
//  } 
  _midi->handleDelays(nowCorr);

  /* is it time to turn on/off glove-fingers (vibrating motors)? */
  while ( (finger = _glovesSchedule.checkForRelease(nowCorr)) != NULL) {
    bool on = (*finger & GLOVE_FINGER_ON);
    _gloves->setFinger(*finger & 0b1111 /* remove flags */, on);
  }
  _gloves->updateGloves(); /* If finger-data changed, update status of vibrating motors in gloves. */
  
  return ledsUpdated;
}


uint32_t Player2::_drawUpcomingNotes(uint32_t currNoteTick) {
  uint32_t millisToNextNote = 0; /* how many milliseconds until next note (here on LED panel row 3) to be played? */
  _ledPanel->fillRect(0, 0, PANEL_COLS-1, 3, COLOR_IDX_OFF);
  int noteIdx = _curNoteIdx2;
  if (noteIdx < _noteCount) {  /* not yet end reached ? */
    for (int row = 4; row >= 0; row--) { /* notes on row 4 of LED-panel must be played first, row 3 thereafter, etc.  */
      SongNote* note = &_notes[noteIdx];
      uint32_t tickPos = note->atTick;
      bool tickPosValid = (note->type == TYPE_NOTE); /* if not a note, then overwrite 'tickPos' in do-loop below */
      byte noteType;
      do {
        noteType = note->type;
        switch(noteType) {
          case TYPE_MEASURE:
          case TYPE_MEASURE_BM:
            break;
          case TYPE_NOTE:
            if (!tickPosValid) {
              /* current tickPos is of measure (not a first note at a new tick), so correct this... */
              tickPos = note->atTick;
              tickPosValid = true;
            }
            if (row == 4) break; /* current notes on row 4 are not drawn in this routine */
            byte color = _colorIdx_fingers[note->finger];     /* color for the finger to play */
            byte dimmed = 4; /* how many steps is the color dimmed/weaker? */
            if (row == 3) { /* special for row 3 */
              if (millisToNextNote == 0) { /* milliseconds to next note not yet calculated? */
                uint32_t ticksToNext = note->atTick - currNoteTick; /* after how many ticks will notes on row 3 be played? */
                millisToNextNote = _getMillisDuration(ticksToNext); /* ...and after how many milliseconds? */
              }
              //if (millisToNextNote < 225) dimmed = 0; /* full brightness on row 3 if note will be played within X milliseconds */
            }
            _ledPanel->setPixel(note->pitch - MIDI_PITCH_MIN, row, color + dimmed); /* set LED on */                  
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
  return millisToNextNote; /* return value: how many millis until note on row 3 will be played? */
}




void Player2::_lookAheadAndSchedule(uint32_t nowCorr) {

  bool stillNewNotes = (_curNoteIdx < _noteCount);
  uint32_t dTicks, dMillis;
  if (!stillNewNotes) {
    /* below: what to do after last note was already processed? */
    dTicks = _totalTicks - _currentTick;       /* MIDI-ticks between last note and the end of song ( >= 0 ) */    

    dMillis = _getMillisDuration(dTicks); /* convert ticks to milliseconds  */
    uint32_t endMillis = _lastMillis + dMillis;

    if ((nowCorr + _timeAhead) < endMillis) return;

    if (_doRepeat) {
      _currentTick = 0;
      _lastMillis = endMillis;
      _curNoteIdx = 0;    
      return;
    }
    else { /* no repeat */
      if (_songEndMillis != 0) return; /* do nothing if _finishMillis already set */
      _songEndMillis = endMillis + 100; /* quit practicing 100 ms after the 'real' endtime of the song */ 
      return;
    }
  }
  SongNote* note;
  SongMeasure* measure;
  
  note = &_notes[_curNoteIdx];
  dTicks = note->atTick - _currentTick;  /* MIDI-ticks between former note and this new note ( >= 0 ) */
  dMillis = _getMillisDuration(dTicks); /* convert ticks to milliseconds */
  uint32_t playMillis = _lastMillis + dMillis;

  if ((nowCorr + _timeAhead) < playMillis) return;
  
  switch(note->type) {
    case TYPE_MEASURE:
    case TYPE_MEASURE_BM:
      /* it's a measure: set tempo (might be changed) and plan the metronome beats */
      measure = (SongMeasure*)note;
      _updateMeasureNrSchedule.add(measure->measureNr, playMillis); /* set measure-nr later, when measure really starts */
      _setTempo(measure->tempoQPM);
      dMillis = _getMillisDuration(measure->beatTicks); /* calc duration of 1 metronome beat */
      _metronome->startNewMeasure(measure->beatCount, dMillis, playMillis - 10);  /* metronome is activated 10 milliseconds early... */
      break;
    case TYPE_NOTE:
      /* it's a note: add to _upcomingArray along with some extra data like play-time and duration in milliseconds */
      UpcomingNote* upcoming;
      upcoming = _upcomingArray.add();
      upcoming->note = note;
      upcoming->startMillis = playMillis;
      upcoming->durationMillis = _getMillisDuration(note->duration);
      upcoming->column = note->pitch - MIDI_PITCH_MIN;
      /* Schedule when to turn on / off glove finger vibrating motor */
      uint32_t gloveMillis = playMillis - GLOVE_SCHEDULE_EARLY; /* glove motors will be turned on a little more earlier */
      _glovesSchedule.add(note->finger + GLOVE_FINGER_ON, gloveMillis); /* schedule to turn ON glove-finger */
      _glovesSchedule.add(note->finger + GLOVE_FINGER_OFF, gloveMillis + upcoming->durationMillis - 5); /* schedule to turn OFF glove-finger */


      
      break;
  }
  _currentTick = note->atTick;
  _lastMillis = playMillis;
  _curNoteIdx++;
}


bool Player2::stopPlayingNow() {
  if (!isPlaying) return false;
  isPlaying = false;
  _metronome->reset();
  _midi->handleAllDelaysImmediately();
  _gloves->reset(false);
  _upcomingArray.reset();
  uint32_t nowCorr =  millis() + _missedMillis;
  byte* pitch;
  while ( (pitch = _ledOffSchedule.checkForRelease(nowCorr + 60000 /* 1 minute in future */ )) != NULL) {
        _ledPanel->setPixel(*pitch - MIDI_PITCH_MIN, 4 /* row 4 on LED panel*/, COLOR_IDX_OFF); /* LED off*/
  }
  return true;
}




/******************************************************************************************************************************
* Tempo is set when first measure is handled and also when tempo must change (possible per measure, not per note!)
*******************************************************************************************************************************/
void Player2::_setTempo(uint16_t qpm) {
  if (qpm == _tempoQPM) return; /* tempo not changed: do nothing */
  _tempoQPM = qpm;  /* in Quarter Notes per minute */
  _tempo = 6000000 / (_tempoQPM * _tempoFactor); /* in Milliseconds per Quarter Note */    
}

/******************************************************************************************************************************
* Convert duration from MIDI-ticks to Milliseconds  :    ticks * (ms/QN) / (ticks/QN) = ms
*******************************************************************************************************************************/
uint32_t Player2::_getMillisDuration(uint32_t ticks) {
  return ((uint32_t)ticks * _tempo / _resolution);
}


/******************************************************************************************************************************
* Only return true if the song is finished. When repeat is ON, then always return false.
*******************************************************************************************************************************/
bool Player2::isSongFinished() {
  /* _songEndMillis is set when the last note of the song is processed, only when repeat-mode is OFF. */
  if (_songEndMillis == 0) return false;
  uint32_t nowCorr = millis() + _missedMillis;
  return (_songEndMillis < nowCorr );
}


void Player2::suspendPlaying() {
  _suspendMillis = millis();
}


void Player2::resumePlaying() {
  uint32_t dMillis = millis() - _suspendMillis; /* how long was the song suspended/paused? */
  dMillis += 100;      /* 100ms extra  */
  _lastMillis += dMillis;  
  if (_songEndMillis != 0) _songEndMillis += dMillis;
  _ledOffSchedule.addSuspendedMillis(dMillis);
  _glovesSchedule.addSuspendedMillis(dMillis);
  //_undimRow3Schedule.addSuspendedMillis(dMillis);
  UpcomingNote* upcoming;
  _upcomingArray.iterateInit(true);
  while( (upcoming = _upcomingArray.iterate() ) != NULL) {
    upcoming->startMillis += dMillis;
  }
}
