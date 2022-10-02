#ifdef DEBUG_MODE

/******************************************************************************************************************************
* Called from setup()
* When pedals are hold down during boot up, certain tests will be run.
*******************************************************************************************************************************/
void handleTestsDuringSetUp() {
  /* Handle tests (only run when pedals are pressed during start-up) */
  footPedal.readPedals();
  bool isL = footPedal.isLeftDown(); 
  bool isM = footPedal.isMiddleDown(); 
  bool isR = footPedal.isRightDown();
  if      (isL && isR) test_showLEDperSongAndUser();           /* run Test during booting */
  else if (isM && isR) test_showEachLedOfStrip1And2();         /* run Test during booting */
  else if (isL && isM) test_Metronome();                       /* run Test during booting */
  else if (isL)        test_playSongWithMetronome(1);          /* run Test during booting */
  else if (isM)        test_playSongWithMetronome(3);          /* run Test during booting */
  else if (isR)        test_playSongWithMetronome(5);          /* run Test during booting */
}


/******************************************************************************************************************************
* 
* 
* THE TESTS BELOW !!!DO NOT!!! NEED THE 'ARDUINO SERIAL MONITOR'     -    WILL RUN WHEN USER HOLDS A PEDAL WHILE BOOTING.
* 
* 
*******************************************************************************************************************************/



/******************************************************************************************************************************
* Test: scan all User- and Song-files and show 1 LED per user and 1 LED per song on the LED-panel
*******************************************************************************************************************************/
#define TEST_COLOR_USER COLOR_IDX_BLUE
#define TEST_COLOR_SONG COLOR_IDX_YELLOW

void test_showLEDperSongAndUser() {
  sdCard.scanUserAndSongFiles();
  ledPanel.clear();
  for (int i=1; i<=5; i++) { /* 1 LED per users on row 3 (max 5 users) */
    if (sdCard.isUserAvailable(i)) { ledPanel.setPixel(i-1, 3, TEST_COLOR_USER); }
  }
  for (int i=1; i<=60; i++) { /* 1 LED per song on row 4 (max 60 songs) */
    if (sdCard.isSongAvailable(i)) { ledPanel.setPixel(i-1, 4, TEST_COLOR_SONG); }
  } 
  ledPanel.writeLeds_asm();

  while(true)
  {
    footPedal.readPedals();
    if (footPedal.isAnyPressed()) return;           /* User wants to exit? */
    delay(10);
  }
}



/******************************************************************************************************************************
* Test: Show each LED of Ledstrip 1 and Ledstrip 2 (from begin to end in the way the LEDs are connected)
*******************************************************************************************************************************/

void test_showEachLedOfStrip1And2() {

  for (int col = 60; col >= 0 ; col -= 2)
  {
    for (int row = 0; row <= 4; row++)
    {
      ledPanel.clear();
      int y;
      if (col % 4 == 1 || col % 4 == 2) 
        y = 4-row; /* correct for zig-zag way of connecting LEDs */
      else
        y = row;
      ledPanel.setPixel(col, y, COLOR_IDX_RED);       /* LED strip 1 */
      ledPanel.setPixel(col-1, y, COLOR_IDX_YELLOW);  /* LED strip 2 */
      ledPanel.writeLeds_asm();
      delay(150);  
      footPedal.readPedals();
      if (footPedal.isAnyPressed()) return;           /* User wants to exit? */
    }
  }
}


/******************************************************************************************************************************
* Test: Metronome (sound and LEDs) in 4/4 time signature
*******************************************************************************************************************************/
void test_Metronome() {
  uint32_t now;
  metronome.working = METRONOME_ALL_BEATS;  
  while(true) {
    now = millis();
    metronome.startNewMeasure(4, 500, now);
    for (int i=0; i<2000; i++) {
      now = millis();
      metronome.handleBeats(now);
      footPedal.readPedals();
      if (footPedal.isAnyPressed())           /* User wants to exit? */
      {
        metronome.reset();
        return;                               /* EXIT when any footpedal is pressed */
      }
      delay(1);  
    }
  }
}



/******************************************************************************************************************************
* Test: play the song (idSong) using the metronome and MIDI-out
*******************************************************************************************************************************/
void test_playSongWithMetronome(int idSong) {
  uint32_t now;
  metronome.working = METRONOME_ALL_BEATS;
  Player0   player(&midi, &metronome);           /* Simple player: just play song via MIDI (no metronome, no LEDs)  */
  sdCard.loadSong(&song, idSong, LOAD_FLAG_NONE);
  
  for (int i=0; i< 2; i++) {      /* play song 2 times */
    player.startSong(&song, 100, false);   /* tempo 100%, no repeat*/
    while (player.isPlaying)
    {
      player.handlePlaying();
      footPedal.readPedals();
      if (footPedal.isAnyPressed())           /* User wants to exit? */
      {
        player.stopPlayingNow();
        return;                               /* EXIT when any footpedal is pressed */
      }
      delay(1);
    }    
  }
}



/******************************************************************************************************************************
* 
* 
* THE TESTS BELOW !!!DO!!! NEED THE 'ARDUINO SERIAL MONITOR'     -    ONLY TO BE USED BY DEVELOPER TO TEST LOGIC.
* 
* 
*******************************************************************************************************************************/



/******************************************************************************************************************************
* Test: print song data to the Serial Monitor
* beware: Song must be loaded before calling this function!
*******************************************************************************************************************************/
void test_DumpSongData() {
  Serial.println("----- START SONG DUMP -----");
  Serial.print("songName: "); Serial.println(song.songName);
  Serial.print("resolution: "); Serial.println(song.resolution);
  Serial.print("totalTicks: "); Serial.println(song.totalTicks);
  Serial.print("lastMeasureNr: "); Serial.println(song.lastMeasureNr);
  for (int i=0; i<song.noteCount; i++) {
    SongNote* n = &song.notes[i];
    if (n->type == TYPE_MEASURE || n->type == TYPE_MEASURE_BM) /* measure or measure with bookmark flag */
    {
      SongMeasure* m = (SongMeasure*)n;
      Serial.print("Measure (");    Serial.print(m->measureNr);
      Serial.print("): at ");       Serial.print(m->atTick); 
      Serial.print(", beatCount "); Serial.print(m->beatCount); 
      Serial.print(", beatTicks "); Serial.print(m->beatTicks); 
      Serial.print(", tempoQPM ");  Serial.print(m->tempoQPM);     
      if (n->type == TYPE_MEASURE_BM)
        Serial.println(" (bookmarked measure)");
      else
        Serial.println();
    }
    else {
      Serial.print("Note   : at "); Serial.print(n->atTick); 
      Serial.print(", pitch "); Serial.print(n->pitch); 
      Serial.print(", velocity "); Serial.print(n->velocity);
      Serial.print(", finger "); Serial.print(n->finger); 
      Serial.print(", duration "); Serial.println(n->duration);
    }
  }
  Serial.println("----- END SONG DUMP -----");
}


/******************************************************************************************************************************
* SongNote and SongMeasure are dependent in terms of alignment.
* Test if object data can be read (as it should) after pointer casting.
*******************************************************************************************************************************/
void test_CheckAlignmentAfterCasting() {
  uint32_t v1 = 12345678;
  uint16_t v2 = 511;
  uint16_t v3 = 1025;  
  byte v4 = 13;
  byte v5 = 78;
  byte v6 = 221;
  byte v7 = 90;

  SongNote n;
  n.atTick = v1;
  n.duration = (v3 << 16) + v2; /* two 16-bit values combined */
  n.finger = v4;
  n.pitch = v5;
  n.velocity = v6;
  n.type = v7;

  SongMeasure* m = (SongMeasure*)&n;

  bool ok = m->atTick    == v1 &&
            m->beatTicks == v2 &&
            m->tempoQPM  == v3 &&
            m->beatCount == v4 &&
            m->measureNr == v5 &&
            m->unused    == v6 &&
            m->type      == v7;
  Serial.print("Are SongNote and SongMeasure aligned correctly? ");
  Serial.println(ok ? "YES" : "NO");
  Serial.print("sizeof(SongNote) : "); Serial.println(sizeof(SongNote));
  Serial.print("sizeof(SongMeasure) : "); Serial.println(sizeof(SongMeasure));  
}


/******************************************************************************************************************************
* Test: print Wifi status details to the Serial Monitor 
*******************************************************************************************************************************/
void test_dumpWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  ip = WiFi.gatewayIP();
  Serial.print("Gateway IP: ");
  Serial.println(ip);

  ip = WiFi.subnetMask();
  Serial.print("Subnet mask: ");
  Serial.println(ip); 

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  String fv = WiFi.firmwareVersion();
  Serial.print("Firmware version:");
  Serial.println(fv);
  
}


/******************************************************************************************************************************
* Test MIDI-input: the pitch of each piano key pressed is printed
*******************************************************************************************************************************/
void test_ReadPressedPianoKeys() {
  midi.clearReadBuffer();
  while (true) {
    int pitch;
    while ((pitch = midi.getPressedPianoKey()) > 0) { Serial.println(pitch); }
    delay(25);
  }
}


/******************************************************************************************************************************
* Test the 4 Buttons on the LED-panel (DOWN, just PRESSED)
*******************************************************************************************************************************/
void test_LedPanelButtons() {
  Serial.println("down  pressed");
  while(true)
  {
    ledPanel.readButtons();
    bool a = ledPanel.isButtonDown(BUTTON_USER);  // down
    bool b = ledPanel.isButtonDown(BUTTON_SONG);
    bool c = ledPanel.isButtonDown(BUTTON_RELOAD);
    bool d = ledPanel.isButtonDown(BUTTON_WIFI);
    
    bool e = ledPanel.isButtonPressed(BUTTON_USER);  // pressed
    bool f = ledPanel.isButtonPressed(BUTTON_SONG);
    bool g = ledPanel.isButtonPressed(BUTTON_RELOAD);
    bool h = ledPanel.isButtonPressed(BUTTON_WIFI);

    if (a || b || c || d || e || f || g || h) {
      Serial.print(a ? "1" : "0");
      Serial.print(b ? "1" : "0");
      Serial.print(c ? "1" : "0");
      Serial.print(d ? "1" : "0");
      Serial.print(e ? "   1" : "   0");
      Serial.print(f ? "1" : "0");
      Serial.print(g ? "1" : "0");
      Serial.println(h ? "1" : "0");
    }
    delay(100);
  }
}


/******************************************************************************************************************************
* Test the Foot Pedals (DOWN, just PRESSED, just RELEASED)
*******************************************************************************************************************************/
void test_FootPedals1() {
  uint32_t now;
  Serial.println("down  pressed   released");
  while(true)
  {
    now = millis();
    footPedal.readPedals(now);
    bool a = footPedal.isLeftDown();  // down
    bool b = footPedal.isMiddleDown();
    bool c = footPedal.isRightDown();    
    bool d = footPedal.isLeftPressed(); // pressed
    bool e = footPedal.isMiddlePressed();
    bool f = footPedal.isRightPressed();
    bool g = footPedal.isLeftReleased(); // released
    bool h = footPedal.isMiddleReleased();
    bool i = footPedal.isRightReleased();

    if (a || b || c || d || e || f || g || h || i) {
      Serial.print(a ? " 1" : " 0");
      Serial.print(b ? "1" : "0");
      Serial.print(c ? "1" : "0");
      Serial.print(d ? "    1" : "    0");
      Serial.print(e ? "1" : "0");
      Serial.print(f ? "1" : "0");
      Serial.print(g ? "     1" : "     0");
      Serial.print(h ? "1" : "0");
      Serial.println(i ? "1" : "0");
    }
    delay(100);
  }
}


/******************************************************************************************************************************
* Test the Foot Pedals (quick repeating presses/releases, and time that pedal is kept down)
*******************************************************************************************************************************/
void test_FootPedals2() {
  uint32_t now;
  Serial.println("Left pedal  : quick repeating PRESSES (max 3).");
  Serial.println("Middle pedal: quick repeating RELEASES (max 3).");
  Serial.println("Right pedal : how long KEPT DOWN (milliseconds).");
  while(true)
  {
    now = millis();
    footPedal.readPedals(now);
    int cnt;
    if (footPedal.isLeftPressed()) {
      cnt = footPedal.leftPresses(now);
      Serial.print("Left presses: ");
      Serial.println(cnt);
    }
    if (footPedal.isMiddleReleased()) {
      cnt = footPedal.middleReleases(now);
      Serial.print("Middle releases: ");
      Serial.println(cnt);
    }
    if (footPedal.isRightDown())
    {
      uint32_t t = footPedal.getRightDownTime(now);
      Serial.print("Right down: ");
      Serial.println(t);      
    }
    delay(25);
  }
}


/******************************************************************************************************************************
* Test the DelayManager
*******************************************************************************************************************************/
void test_DelayManager() {
  Serial.println("\nSTART OF TEST");
  byte* b;
  DelayManager<byte, 12> delayMgr;
  int arr_ms[]   = {400,600,100,200,800,1000,50,300,5000,500 }; /* milliseconds to wait for release */
  byte arr_val[] = {  1,  2,  3,  4,  5,   6, 7,  8,   9, 10 }; /* test data */
  /* fill DelayManager first with data */
  uint32_t now = millis();
  for (int i=0; i<10; i++) {
    delayMgr.add(arr_val[i], now + arr_ms[i]);
  }
  delayMgr.dump();
  /* wait for data to be released in the right order */
  while (delayMgr.count() > 0)
  {
    delay(5);
    now = millis();
    b = delayMgr.checkForRelease(now);
    if (b != NULL) {
      Serial.print("Released: ");
      Serial.println(*b);
      delayMgr.dump();
    }
  }  
  Serial.println("END OF TEST\n");
}


/******************************************************************************************************************************
* Test the CircularArray
*******************************************************************************************************************************/
void test_CircularArray() {
  Serial.println("\nSTART OF TEST");
  byte* b;
  CircularArray<byte, 7> arr;
  byte arr_val[] = {  1,  2,  3,  4,  5,   6, 7,  8,   9, 10 }; /* test data */
  /* fill DelayManager first with data */
  for (int i=0; i<6; i++) *(arr.add()) = arr_val[i];
  Serial.println("Items 1 to 6 in next dump?");  
  arr.dump();
  arr.removeFirst();
  arr.removeFirst();
  arr.removeFirst();
  arr.removeFirst();
  arr.removeFirst();
  Serial.println("Just item 6 in next dump?");  
  arr.dump();
  for (int i=6; i<10; i++) *(arr.add()) = arr_val[i];
  Serial.println("Items 6 to 10 in next dump?");    
  arr.dump();
  Serial.println("First and last item (6 and 10?):");  
  Serial.println(*(arr.getFirst()));
  Serial.println(*(arr.getLast()));
  Serial.println("Iterate forward (6 to 10?):");  
  arr.iterateInit(true);
  while ( (b = arr.iterate() ) != NULL) {
    Serial.print(*b);
    Serial.print(",");
  }
  Serial.println();
  Serial.println("Remove last, then iterate backwards (9 to 6?):");  
  arr.removeLast();
  arr.iterateInit(false);
  while ( (b = arr.iterate() ) != NULL) {
    Serial.print(*b);
    Serial.print(",");
  }
  Serial.println();
  Serial.println("END OF TEST\n");
}


#endif // DEBUG_MODE
