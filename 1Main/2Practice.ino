


/******************************************************************************************************************************
* Practice 1 : practice where user sets the pace 
*******************************************************************************************************************************/
int doPractice1(int startMeasureNr) {
  Player1 player(&midi, &ledPanel, &gloves);
  player.startSong(&song, startMeasureNr, User::isGloves /* use vibrating gloves? */, User::panelRowsUsed, User::isPracticeRepeat /* repeat song? */);
  while (true) {
    player.handlePlaying();
    footPedal.readPedals(millis());
    if (footPedal.isMiddlePressed()) break;  /* quit practicing: return to loop() */
    bool leftDown = footPedal.isLeftDown();
    bool rightDown = footPedal.isRightDown();
    if (leftDown || rightDown) {
      player.stopPlayingNow();
      int newMeasureNr = doPedalNavigation(player.curMeasureNr, leftDown ? true : false); /* handle left/right pedal navigation */
      /* restart song starting at measure 'newMeasureNr' */
      player.startSong(&song, newMeasureNr, User::isGloves /* use vibrating gloves? */, User::panelRowsUsed, User::isPracticeRepeat /* repeat song? */);
    }
    if (player.isSongFinished()) { player.curMeasureNr = 1; break; }
    delay(25);    
  }
  int curMeasurNr = player.curMeasureNr;
  player.stopPlayingNow();
  return curMeasurNr; /* the measure nr where the user stopped/quit */
}


/******************************************************************************************************************************
* Practice 2 : practice where system sets the pace
*******************************************************************************************************************************/
int doPractice2(int startMeasureNr) {
  byte midiPlay = User::playWhilePractice; /* should song play via MIDI while practicing? */
  metronome.working = User::metronome;
  Player2 player(&ledPanel, &metronome, &midi, &gloves);
  player.startSong(&song, startMeasureNr, User::isGloves, midiPlay, User::tempoFactor, User::isPracticeRepeat);
  bool pReleased = false;  /* before responding to middle foot pedal, it should be released once */
  while (true) {
    player.handlePlaying();
    footPedal.readPedals(millis());
    if (!pReleased && footPedal.isMiddleReleased()) {
      pReleased = true;
    }
    if (pReleased && footPedal.isMiddleDown()) {
      player.suspendPlaying();
      bool doQuit = doHandlePause();
      if (doQuit) break;
      player.resumePlaying();
    }
    bool leftDown = footPedal.isLeftDown();
    bool rightDown = footPedal.isRightDown();
    if (leftDown || rightDown) {
      player.stopPlayingNow();
      int newMeasureNr = doPedalNavigation(player.curMeasureNr, leftDown ? true : false); /* handle left/right pedal navigation */
      /* restart song starting at measure 'newMeasureNr' */
      player.startSong(&song, newMeasureNr, User::isGloves, midiPlay, User::tempoFactor, User::isPracticeRepeat);
    }
    if (player.isSongFinished()) { player.curMeasureNr = 1; break; }
  }
  int curMeasurNr = player.curMeasureNr;
  player.stopPlayingNow();
  return curMeasurNr; /* the measure nr where the user stopped/quit */
}


/******************************************************************************************************************************
* Practice 3 : practice where system sets the pace, LEDs fly down
*******************************************************************************************************************************/
int doPractice3(int startMeasureNr) {
  byte midiPlay = User::playWhilePractice; /* should song play via MIDI while practicing? */
  metronome.working = User::metronome;
  Player3 player(&ledPanel, &metronome, &midi, &gloves);
  player.startSong(&song, startMeasureNr, User::isGloves, midiPlay, User::tempoFactor, User::animationSpeed, User::isPracticeRepeat);
  bool pReleased = false;  /* before responding to middle foot pedal, it should be released once */
  while (true) {
    player.handlePlaying();
    footPedal.readPedals(millis());
    if (!pReleased && footPedal.isMiddleReleased()) {
      pReleased = true;
    }
    if (pReleased && footPedal.isMiddleDown()) {
      player.suspendPlaying();
      bool doQuit = doHandlePause();
      if (doQuit) break;
      player.resumePlaying();
    }
    bool leftDown = footPedal.isLeftDown();
    bool rightDown = footPedal.isRightDown();
    if (leftDown || rightDown) {
      player.stopPlayingNow();
      int newMeasureNr = doPedalNavigation(player.curMeasureNr, leftDown ? true : false); /* handle left/right pedal navigation */
      /* restart song starting at measure 'newMeasureNr' */
      player.startSong(&song, newMeasureNr, User::isGloves, midiPlay, User::tempoFactor, User::animationSpeed, User::isPracticeRepeat);
    }
    if (player.isSongFinished()) { player.curMeasureNr = 1; break; }
  }
  int curMeasurNr = player.curMeasureNr;
  player.stopPlayingNow();
  return curMeasurNr; /* the measure nr where the user stopped/quit */
}


/******************************************************************************************************************************
* Practice 4 : learn to read music notes
*******************************************************************************************************************************/
int doPractice4(int startMeasureNr) {
  byte midiPlay = User::playWhilePractice; /* should song play via MIDI while practicing? */
  Player4 player(&midi, &ledPanel);
  player.startSong(&song, startMeasureNr, midiPlay);
  while (true) {
    player.handlePlaying();
    footPedal.readPedals(millis());
    if (footPedal.isMiddlePressed()) break;  /* quit practicing: return to loop() */
    bool leftDown = footPedal.isLeftDown();
    bool rightDown = footPedal.isRightDown();
    if (leftDown || rightDown) {
      player.stopPlayingNow();
      int newMeasureNr = doPedalNavigation(player.curMeasureNr, leftDown ? true : false); /* handle left/right pedal navigation */
      /* restart song starting at measure 'newMeasureNr' */
      player.startSong(&song, newMeasureNr, midiPlay);
    }
    delay(25);    
  }
  int curMeasurNr = player.curMeasureNr;
  player.stopPlayingNow();
  return curMeasurNr; /* the measure nr where the user stopped/quit */
}

/******************************************************************************************************************************
* Practice 5 : learn to play by reading music notes (letters) on LED display
*******************************************************************************************************************************/
int doPractice5(int startMeasureNr) {
  byte midiPlay = User::playWhilePractice; /* should song play via MIDI while practicing? */
  Player5 player(&midi, &ledPanel);
  player.startSong(&song, startMeasureNr, midiPlay, User::isPracticeRepeat /* repeat song? */);
  while (true) {
    player.handlePlaying();
    footPedal.readPedals(millis());
    if (footPedal.isMiddlePressed()) break;  /* quit practicing: return to loop() */
    bool leftDown = footPedal.isLeftDown();
    bool rightDown = footPedal.isRightDown();
    if (leftDown || rightDown) {
      player.stopPlayingNow();
      int newMeasureNr = doPedalNavigation(player.curMeasureNr, leftDown ? true : false); /* handle left/right pedal navigation */
      /* restart song starting at measure 'newMeasureNr' */
      player.startSong(&song, newMeasureNr, midiPlay, User::isPracticeRepeat /* repeat song? */);
    }
    if (player.isSongFinished()) { player.curMeasureNr = 1; break; }
    delay(25);    
  }
  int curMeasurNr = player.curMeasureNr;
  player.stopPlayingNow();
  return curMeasurNr; /* the measure nr where the user stopped/quit */
}


bool doHandlePause() {
  bool isReleased = false;
  uint32_t now, down = 0;
  delay(50); /* wait 50 ms, to deal with possible switch bounce of foot pedal */
  while(!isReleased) { /* as long as pedal is DOWN */
    now = millis();
    footPedal.readPedals(now);
    isReleased = footPedal.isMiddleReleased();
    if (!isReleased) {
      down = footPedal.getMiddletDownTime(now);
    }
    delay(9); /* loop about 100 times/sec */    
  }
  delay(50); /* wait 50 ms, to deal with possible switch bounce of foot pedal */
  if (down > 400) return false;   /* resume practicing */
  return true;                    /* stop practicing, go to start screen */  
}


/******************************************************************************************************************************
* Navigate to a measure-nr using the foot pedal.
* When entering this function, the left OR right pedal has just been pressed down.
* when released quickly, go one measure down (left pedal) or up (right pedal)
* when hold down longer, go more measures up/down (at accelerating speed)
* when middle pedal is pressed WHILE left OR right pedal is kept down: go next/former bookmarked measure
*******************************************************************************************************************************/
int doPedalNavigation(int curMeasureNr, bool leftPedal ) {  /* !leftPedal = rightPedal */
  char charBuf[5];            /* to store measure-nr in text */
  int maxMeasureNr = song.lastMeasureNr;
  int newMeasureNr;           /* the measure that we will navigate to */
  bool pedalLongDown = false; /* becomes true when pedal is down for more than 400ms */
  bool bookmarkMode = false;  /* when middle pedal is pressed, switch to bookmark navigation */
  int dMeasures = 0; /* how many measures up or down? (only used when pedalLongDown=true) */
  uint32_t firstDown = millis();
  uint32_t now, down;
  bool isReleased = false;
  while(!isReleased) { /* as long as pedal is DOWN */
    now = millis();
    down = now - firstDown;
    pedalLongDown = (down >= 300); /* after 300ms, enter pedalLongDown mode */
    footPedal.readPedals(now);
    isReleased = (leftPedal ? footPedal.isLeftReleased() : footPedal.isRightReleased() );

    bool doDisplayNr = false; 

    if (footPedal.isMiddlePressed()) { /* middle pedal press while left or right pedal down: start bookmark mode */
      if (!bookmarkMode) newMeasureNr = curMeasureNr;
      bookmarkMode = true;
      /* navigate to measure that is bookmarked */
      newMeasureNr = song.getNextBookmarkMeasureNr(newMeasureNr, !leftPedal /* true=forward, false=backward */); 
      doDisplayNr = true;
    }
    if (!bookmarkMode) {
      if (!pedalLongDown && isReleased) { /* pedal released after short press */
        newMeasureNr = curMeasureNr + (leftPedal ? -1 : 1); /* just 1 measure up or down */
        doDisplayNr = true;
      }
      if (isReleased && pedalLongDown) { /* measure navigation */
        /* nothing to do here.. */
      }
      if (!isReleased && pedalLongDown) { /* pedal is more than 400ms DOWN */
        down -= 400;
        dMeasures = 0;  
        while (down < 0x800000) { /* while not overflow (below zero) */
          if      (down > 2400) { down -= 100; dMeasures++; } /* after 2400ms down: very quick navigation (100ms / measure) */
          else if (down > 1200) { down -= 200; dMeasures++; } /* between 1200ms and 2400ms down: quick navigation (200ms / measure) */
          else                  { down -= 300; dMeasures++; } /* between 0 and 1200ms: slow navigation (300ms / measure)  */
        }
        newMeasureNr = curMeasureNr + (leftPedal ? -dMeasures : dMeasures);
        doDisplayNr = true;
      }
    }
    if (newMeasureNr < 1) newMeasureNr = 1;                        /* correct possible overflow */
    if (newMeasureNr > maxMeasureNr) newMeasureNr = maxMeasureNr;  /* correct possible overflow */
    if (doDisplayNr) {
      itoa(newMeasureNr, charBuf, 10);
      ledPanel.clear(); /* clear all LEDs in data structure */
      ledPanel.writeText(charBuf, 25 + PANEL_LEFT_MARGIN, COLOR_IDX_WHITE); /* write measure-nr to LED panel */
      ledPanel.writeLeds_asm();
      if (bookmarkMode) doDisplayNr = false; /* in bookmarkMode: only display after NEW middle pedal press  */
    }
    delay(8); /* loop about 100 times/sec */    
  }
  delay(200);  /* wait 200ms to after pedal is released */
  return newMeasureNr;
}
