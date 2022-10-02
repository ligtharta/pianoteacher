#include "APlayer3.h"


/******************************************************************************************************************************
* Player3 : The player used for 'Practice 3' (playing where the system sets the pace, and where LEDs are flying down)
*******************************************************************************************************************************
* Special feature: every note of the song falls down on the LED panel AT THE SAME SPEED. This speed can be set by the user.
* So, when this speed is set slower, there is more time for the note (represented by a LED on the panel) to fall down.
*******************************************************************************************************************************
* Hardware that is controlled by this player: 
*  - Metronome: only when metronome is ENABLED
*  - Gloves with vibrating motors: only when Gloves are ENABLED
*  - MIDI output: the play the notes if ENABLED (user should play notes, but system can optionally play them at the correct time)
*  - LED panel: to show which notes to play (they fall down from upper row to lower row, in 5 steps)
*******************************************************************************************************************************
* Most important public methods: 
*  void startSong(...);   -> initialize song to play/practice
*  bool handlePlaying();  -> is called constantly from outside to handle everyting the player has to do.
*******************************************************************************************************************************
* Some details about the working:
* This player has to look ahead for 'upcoming' notes to play, because these notes need to be visible on the LED panel before
* they should be played, and fall down on the LED panel from row 0 to row 3 (one above lowest row). These upcoming notes are 
* added to a special array (CircularArray) named _upcomingArray. Every few milliseconds this array is read to update the LED 
* panel. The more 'near' the note is (in terms of 'ready to play'), the more down (higher row-nr) it will be drawn on the 
* LED panel. As soon as the first entry of _upcomingArray is ready to be played:
*  - take the note out of the array
*  - turn on the corresponding LED on on the lowest row (row index 4) of the LED panel.
*  - optionally play the note using MIDI out.
*  - add an entry to a special timing object (DelayManager) that helps to time when the LED should be turned off.
* There are also timing objects (type: DelayManager) to exactly time when to update the measure-nr, 
* and when to turn on/off vibrating motors of the gloves.
*******************************************************************************************************************************/




/******************************************************************************************************************************
* Constructor
*******************************************************************************************************************************/
Player3::Player3(LedPanel* lp, Metronome* m, MidiInterface* mi, Gloves* g) {
  _ledPanel = lp;
  _metronome = m;
  _midi = mi;
  _gloves = g;
}


/******************************************************************************************************************************
* Initialize song to play / practice
*******************************************************************************************************************************/
void Player3::startSong(Song* song, int startMeasureNr, bool withGloves, byte midiPlay, uint16_t tempoFactor, byte animationSpeed, bool repeat) {
  uint32_t now = millis();
  /* Copy some basic data/pointers from the song object, for performance reasons... */
  _resolution = song->resolution;  /* ticks per quarter note */
  _totalTicks = song->totalTicks;     
  _noteCount = song->noteCount;   
  _notes = song->notes;   

  /* prepare members regarding playing the song. */
  _doRepeat = repeat;
  _withGloves = withGloves;
  _midiPlay = midiPlay;             /* is auto-playing (MIDI) on? If so, what volume (very low, low, normal)? */
  _songEndMillis = 0;
  curMeasureNr = startMeasureNr;
  _curNoteIdx = song->getMeasureStartIndex(startMeasureNr); 

  _tempoFactor = tempoFactor;                    /* factor to change normal tempo (10% - 180%)  */ 
  _tempoQPM = 0;  /* Tempo (in Quarter Notes per minute) is set later, when first measure is handled */
  _tempo    = 0;  /* Tempo (in milliseconds per Quarter Note) is set later, when first measure is handled */  

  _currentTick = _notes[_curNoteIdx].atTick;     /* start tick (is zero when starting at first measure) */

  switch (animationSpeed) { /* how fast should the LEDs fall down on the LED panel? */
    case 0:  /* slow speed: 300ms per led */
      _ledAnimationData[0] = 1800;  /* row 0 */
      _ledAnimationData[1] = 900;  /* row 1 */
      _ledAnimationData[2] = 600;  /* row 2 */
      _ledAnimationData[3] = 300;  /* row 3 */
      break;
    case 1:  /* normaal speed: 200ms per led */
      _ledAnimationData[0] = 1200;  /* row 0 */
      _ledAnimationData[1] = 600;  /* row 1 */
      _ledAnimationData[2] = 400;  /* row 2 */
      _ledAnimationData[3] = 200;  /* row 3 */
      break;
    case 2:  /* medium fast speed: 150ms per led */
      _ledAnimationData[0] = 900;  /* row 0 */
      _ledAnimationData[1] = 450;  /* row 1 */
      _ledAnimationData[2] = 300;  /* row 2 */
      _ledAnimationData[3] = 150;  /* row 3 */
      break;
    case 3:  /* fast speed: 100ms per led */
      _ledAnimationData[0] = 600;  /* row 0 */
      _ledAnimationData[1] = 300;  /* row 1 */
      _ledAnimationData[2] = 200;  /* row 2 */
      _ledAnimationData[3] = 100;  /* row 3 */
      break;
  }
  _timeAhead = _ledAnimationData[0] + 300;  /* milliseconds to look ahead for upcoming notes to be played shortly */
  _lastMillis = now + _timeAhead;           /* millis of last played note -> start song after '_timeAhead' milliseconds from 'now' */
  _missedMillis = 0;                        /* each time the LED-panel is updated, this will increase with 4 (compensate for disabled interrupts) */

  _upcomingArray.reset();
  _ledOffSchedule.reset();
  _updateMeasureNrSchedule.reset();
  _glovesSchedule.reset();
  _ledPanel->clear(); /* clear all LEDs in data structure */
  _gloves->reset(withGloves);
  isPlaying = true;
}


/******************************************************************************************************************************
* This is called constantly to handle everything that needs to be done to play / practice the song
*******************************************************************************************************************************/
void Player3::handlePlaying() {
  static uint32_t millisLastLEDsUpdate = 0;
  if (!isPlaying) return;
  byte velocity; /* MIDI-velocity/volume of note */
  byte* pitch;   /* MIDI-pitch of note */
  byte* mNr;     /* measure number */
  byte* finger;  /* finger-nr for gloves */
  uint32_t now = millis();
  uint32_t nowCorr = now + _missedMillis;
  _lookAheadAndSchedule(nowCorr);
  _metronome->handleBeats(nowCorr);
  while ( (mNr = _updateMeasureNrSchedule.checkForRelease(nowCorr)) != NULL) {
    curMeasureNr = *mNr; /* update measure-nr, because new measure starts now...  */
  }
  UpcomingNote* upcoming;
  while( (upcoming = _upcomingArray.getFirst()) != NULL)  {
    if (upcoming->startMillis > nowCorr) break; /* quit while loop, not yet time for this note and all thereafter... */
    SongNote* note = upcoming->note;
    /* always true: note->type == TYPE_NOTE */ 
    uint32_t d = upcoming->durationMillis;
    if (_midiPlay != PLAY_WHILE_PRACTICE_OFF) {
      /* the note must be played via MIDI, there are 3 possible degrees for velocity/volume */
      velocity = note->velocity >> 3;  /*  1/8  of vecolity */
      if      (_midiPlay == PLAY_WHILE_PRACTICE_VOLU_1) velocity = (note->velocity>>2) + velocity; /* 1/4 + 1/8 = 37% */
      else if (_midiPlay == PLAY_WHILE_PRACTICE_VOLU_2) velocity = (note->velocity>>1) + velocity; /* 1/2 + 1/8 = 62% */        
      else                                              velocity = note->velocity;                 /* 100% velocity  */
      _midi->playNote(note->pitch, velocity, upcoming->startMillis + d - (d/10));        
    }
    _ledOffSchedule.add(note->pitch, nowCorr + d - (d/10) );    /* turn off LED (that indicates a playing note) at a later time */
    byte color = _colorIdx_fingers[note->finger];
    _ledPanel->setPixel(note->pitch - MIDI_PITCH_MIN, 4 /* row 4 */ , color /* color per finger */);
    _upcomingArray.removeFirst(); /* first note in circular array is now handled (not upcoming anymore), so remove it. */
  }
  /* Each time a note ends, turn of corresponding LED in row 4 of LED-panel (row 4 shows notes that are currently played).  */
  while ( (pitch = _ledOffSchedule.checkForRelease(nowCorr)) != NULL) {
    _ledPanel->setPixel(*pitch - MIDI_PITCH_MIN, 4 /* row 4 */, COLOR_IDX_OFF); /* LED off*/
  }
  _displayUpcomingNotes(nowCorr); /* displays LEDs in LED-panel row 0,1,2,3 */
  _midi->handleDelays(nowCorr);

  /* is it time to turn on/off glove-fingers (vibrating motors)? */
  while ( (finger = _glovesSchedule.checkForRelease(nowCorr)) != NULL) {
    bool on = (*finger & GLOVE_FINGER_ON);
    _gloves->setFinger(*finger & 0b1111 /* remove flags */, on);
  }
  _gloves->updateGloves(); /* If finger-data changed, update status of vibrating motors in gloves. */
  
  if (now - millisLastLEDsUpdate >= 20) {  /* update LED panel 50 times per second */
    _ledPanel->writeLeds_asm();          /* Interrupts will be disabled temporarily and 'millis()' will miss a few milliseconds  */
    _missedMillis += MILLIS_TO_WRITE_LED_PANEL;  /* correct for 'missed interrupts' due to writeLeds_asm()  */
    _displayUpcomingNotes(nowCorr + 3);  /* extra call, because this needs to be called every 3ms */
    millisLastLEDsUpdate = now;
  }
}

void Player3::_lookAheadAndSchedule(uint32_t nowCorr) {

  bool stillNewNotes = (_curNoteIdx < _noteCount);
  uint32_t dTicks, dMillis;
  if (!stillNewNotes) { /* handle end of song within this if */
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
      _metronome->startNewMeasure(measure->beatCount, dMillis, playMillis - 15);  /* metronome is activated 15 milliseconds early... */
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



void Player3::_displayUpcomingNotes(uint32_t nowCorr) {
  int col;
  static byte seqNr = 255; /* sequence number, increases each time this function is called */
  if (++seqNr == 0) { memset(_perKey, 0, sizeof(_perKey) ); /* clear array: 1 byte per piano key */ seqNr++; }
  /* seqNr is between 1 and 255, but NEVER 0 ! */
  
  UpcomingNote* upcoming;
  SongNote*     note;
  _upcomingArray.iterateInit(false);  /* iterate backwards: from future to (almost) present time */
  while( (upcoming = _upcomingArray.iterate() ) != NULL) {
    note = upcoming->note;
//    if (note->type != TYPE_NOTE) continue; /* not a real note to be played [this CANNOT happen] */
    col = upcoming->column;  /* this value (LED panel column) is between 0 and 60 */
    if (_perKey[col] != seqNr) { /* first time this piano-key / LED-column (while in this loop)? */
      /* clear LED panel column, row indexes 0 to 3 (these rows can display upcoming notes) */
      _ledPanel->setPixel(col, 0, COLOR_IDX_OFF); 
      _ledPanel->setPixel(col, 1, COLOR_IDX_OFF); 
      _ledPanel->setPixel(col, 2, COLOR_IDX_OFF); 
      _ledPanel->setPixel(col, 3, COLOR_IDX_OFF); 
      _perKey[col] = seqNr;  /* prevent this column clear will be done again -> there might be a 2nd upcoming note in this colomn */
    }    
    uint32_t dMillis = upcoming->startMillis - nowCorr;
    byte color = _colorIdx_fingers[note->finger];
    if (dMillis > _ledAnimationData[0]) continue;
    if (dMillis < 4) continue;
    if (dMillis <= _ledAnimationData[3]) {
      _ledPanel->setPixel(col, 3, color);
    }
    else if (dMillis <= _ledAnimationData[2]) {
      _ledPanel->setPixel(col, 2, color);
    }
    else if (dMillis <= _ledAnimationData[1]) {
      _ledPanel->setPixel(col, 1, color);
    }
    else {
      _ledPanel->setPixel(col, 0, color);
    }
  }
}



bool Player3::stopPlayingNow() {
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
void Player3::_setTempo(uint16_t qpm) {
  if (qpm == _tempoQPM) return; /* tempo not changed: do nothing */
  _tempoQPM = qpm;  /* in Quarter Notes per minute */
  _tempo = 6000000 / (_tempoQPM * _tempoFactor); /* in Milliseconds per Quarter Note */    
}

/******************************************************************************************************************************
* Convert duration from MIDI-ticks to Milliseconds  :    ticks * (ms/QN) / (ticks/QN) = ms
*******************************************************************************************************************************/
uint32_t Player3::_getMillisDuration(uint32_t ticks) {
  return ((uint32_t)ticks * _tempo / _resolution);
}


/******************************************************************************************************************************
* Only return true if the song is finished. When repeat is ON, then always return false.
*******************************************************************************************************************************/
bool Player3::isSongFinished() {
  /* _songEndMillis is set when the last note of the song is processed, only when repeat-mode is OFF. */
  if (_songEndMillis == 0) return false; 
  uint32_t nowCorr = millis() + _missedMillis;
  return (_songEndMillis < nowCorr );
}


void Player3::suspendPlaying() {
  _suspendMillis = millis();
}


void Player3::resumePlaying() {
  uint32_t dMillis = millis() - _suspendMillis; /* how long was the song suspended/paused? */
  dMillis += 100;      /* 100ms extra  */
  _lastMillis += dMillis;  
  if (_songEndMillis != 0) _songEndMillis += dMillis;
  _ledOffSchedule.addSuspendedMillis(dMillis);
  _glovesSchedule.addSuspendedMillis(dMillis);
  UpcomingNote* upcoming;
  _upcomingArray.iterateInit(true);
  while( (upcoming = _upcomingArray.iterate() ) != NULL) {
    upcoming->startMillis += dMillis;
  }
}
