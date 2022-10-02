

void displayError(const char* err)
{
  displayMsg(err, COLOR_IDX_RED);
}

int displayMsg(const char* msg, byte color) {
  ledPanel.clear();
  int width = ledPanel.getTextWidth(msg);
  int x = (PANEL_COLS - width) / 2;
  ledPanel.writeText(msg, x, color);
  ledPanel.writeLeds_asm();
  return width;
}


/******************************************************************************************************************************
* Scroll text-info on the LED-panel: NAME USER + ID SONG + NAME SONG
* bool init: call once with 'TRUE' to initialize the scroll.
*            call repeatedly with 'FALSE' to write text to LED panel and move a LED-pixel each time.
*******************************************************************************************************************************/
void displayUserAndSongScroll(bool init, bool settingsDirty) {
  static char charBuf[4]; /* to store songId in text */
  static int wUser, wSong, wSongId; /* widths in LEDs */
  static int xUser, xSong, xSongId, x;    /* x (=column) of LED panel */
  if (init) {
    itoa(song.songId, charBuf, 10);
    wUser = ledPanel.getTextWidth(User::userName) + 7; /* width of user name text */
    wSong = ledPanel.getTextWidth(song.songName) + 7;  /* width of song name text */
    wSongId = ledPanel.getTextWidth(charBuf) + 7;      /* width of song-id text */
    xUser = PANEL_COLS - 1;              /* start scrolling from the very right, column 60 */
    xSongId = xUser + wUser;
    xSong = xSongId + wSongId;
  }
  int wAll = wUser + wSongId + wSong;
  ledPanel.clear(); /* clear all LEDs in data structure */
  x = xUser;
  byte clrUser = ledPanel.getUserColorIdx(User::userId); /* each user has own color */
  while (x < PANEL_COLS) { ledPanel.writeText(User::userName, x, clrUser); x += wAll; }
  x = xSongId;
  while (x < PANEL_COLS) { ledPanel.writeText(charBuf, x, COLOR_IDX_WHITE); x += wAll; }
  x = xSong;
  while (x < PANEL_COLS) { ledPanel.writeText(song.songName, x, COLOR_IDX_YELLOW); x += wAll; }
  if (settingsDirty) ledPanel.setPixel(0, 4, COLOR_IDX_RED); /* set RED pixel to reveal dirty state to user */
  ledPanel.writeLeds_asm();
  xUser--;  xSongId--; xSong--;    /* scroll to left when called next time */
  if (xUser < -wUser && xUser < xSong) xUser = xSong + wSong; /* user-name scrolled out of the LED panel? */
  if (xSongId < -wSongId && xSongId < xUser) xSongId = xUser + wUser; /* song-id scrolled out of the LED panel? */
  if (xSong < -wSong && xSong < xSongId) xSong = xSongId + wSongId; /* song-name scrolled out of the LED panel? */
}

/******************************************************************************************************************************
* Display settings. Example: '2.. M H [120]'
*                   This means: practice type (1/2), Metronome (off/partial/on), Gloves (off/on), Tempo-factor (10-180%)
*******************************************************************************************************************************/
void displaySettingsOnLEDpanel() {
  static char charBuf[4]; /* for tempo */
  byte practice = User::practice;
  byte colors[3] = { COLOR_IDX_RED, COLOR_IDX_BLUE, COLOR_IDX_GREEN }; /* color per metronome type */
  ledPanel.clear(); /* clear all LEDs in data structure */
  for (int i = 0; i <= User::globalBrightness; i++) {
    ledPanel.setPixel(60 + PANEL_LEFT_MARGIN, 4-i, COLOR_IDX_YELLOW);
  }
  ledPanel.writeChar(practice + '0', 2 + PANEL_LEFT_MARGIN, COLOR_IDX_WHITE);    /* type of practice the user wants to do */
  if (User::isPracticeRepeat || practice == PRACTICE_4) {  /* practice 4: repeat is always on */
    ledPanel.setPixel(7 + PANEL_LEFT_MARGIN, 4, COLOR_IDX_WHITE);
    ledPanel.setPixel(9 + PANEL_LEFT_MARGIN, 4, COLOR_IDX_WHITE);
  }
  if (practice != PRACTICE_1) {  /* settings used for practice 2, 3, 4 and 5 -> where MIDI out is used */
    /* auto-play via MIDI while practicing? */
    bool midiPlay = User::playWhilePractice != PLAY_WHILE_PRACTICE_OFF;
    /* characters ';', '<', '=' and '>' are used for midiPlay symbols */
    ledPanel.writeChar(';' + User::playWhilePractice, 11 + PANEL_LEFT_MARGIN, midiPlay ? COLOR_IDX_GREEN : (COLOR_IDX_RED + 4));
  }
  if (practice == PRACTICE_3) {  /* settings only used for practice 3 -> where LEDs fall down at certain speed */  
    /* animation speed setting */
    int spd = User::animationSpeed;
    ledPanel.fillRect(17 + PANEL_LEFT_MARGIN, 0, 17 + PANEL_LEFT_MARGIN, spd + 1, COLOR_IDX_YELLOW); /* draw little arrow pointing downwards */
    ledPanel.setPixel(16 + PANEL_LEFT_MARGIN, spd, COLOR_IDX_YELLOW);
    ledPanel.setPixel(18 + PANEL_LEFT_MARGIN, spd, COLOR_IDX_YELLOW);
  }
  if (practice == PRACTICE_2 || practice == PRACTICE_3) {  /* settings only used for practice 2 and 3 -> where pace is controlled by the system */  
    /* metronome setting */
    if (METRONOME_HARDWARE_AVAILABLE)
    {
      ledPanel.writeChar('M', 22 + PANEL_LEFT_MARGIN, colors[User::metronome]);
    }
    /* tempo setting */
    ledPanel.writeChar('[', 37 + PANEL_LEFT_MARGIN, COLOR_IDX_BLUE + 4);  /* +4 means that the color blue is dimmed (4 steps) */
    ledPanel.writeChar(']', 56 + PANEL_LEFT_MARGIN, COLOR_IDX_BLUE + 4);
    sprintf(charBuf, "%d", User::tempoFactor);
    int w = ledPanel.getTextWidth(charBuf); 
    ledPanel.writeText(charBuf, (16-w)/2 + 40 + PANEL_LEFT_MARGIN, COLOR_IDX_YELLOW);
  }
  if (practice == PRACTICE_1) {  /* settings only used for practice 1 */  
    ledPanel.writeText(User::panelRowsUsed == 5 ? "P5" : "P1", 43 + PANEL_LEFT_MARGIN, COLOR_IDX_YELLOW); /* preview 1 or 5 steps while playing? */
    ledPanel.setPixel(40 + PANEL_LEFT_MARGIN, 4, COLOR_IDX_BLUE);
    if (User::leftHandCue) /* should left hand notes be cued with a grey pixel above? */
      ledPanel.setPixel(40 + PANEL_LEFT_MARGIN, 3, COLOR_IDX_GREY + 7);  /* +7 means that the color grey is dimmed (7 steps) */
  }
  if (practice != PRACTICE_4) { /* no gloves support in practice 4 */
    if (GLOVES_HARDWARE_AVAILABLE)
    {
      ledPanel.writeChar('H', 30 + PANEL_LEFT_MARGIN, User::isGloves ? COLOR_IDX_GREEN : COLOR_IDX_RED);
    }
  }
  ledPanel.writeLeds_asm();
}



void displaySongAnalysis()
{
  bool* pitchArray = song.getSongAnalysis();
  ledPanel.clear();

  for (int i = 0; i < PANEL_COLS; i++) {
    if (pitchArray[i]) { 
      ledPanel.setPixel(i,0, COLOR_IDX_GREY); 
    }
  }
  ledPanel.writeLeds_asm();
}


/******************************************************************************************************************************
* Display options for reloading song: [left]   OK   [right]
* [left] can be colored 'L', white 'L', or red 'x'
* [right] can be colored 'R', white 'R, or red 'x'
*    colored 'L' or 'R' : load left/right channel normaly (with finger data)
*    white 'L' or 'R'   : load left/right channel without finger data (all white)
*    'x'                : do not load this channel (left or right)
*******************************************************************************************************************************/
void displayReloadSongOptions(int counter, int flagLeft, int flagRight)
{
  byte colors[] = {COLOR_IDX_YELLOW, COLOR_IDX_RED, COLOR_IDX_GREEN, COLOR_IDX_BLUE, COLOR_IDX_PURPLE };
  ledPanel.clear(); /* clear all LEDs in data structure */
  if (flagLeft & LOAD_FLAG_LEFT_COLOR)
    ledPanel.writeChar('L', 6 + PANEL_LEFT_MARGIN, COLOR_IDX_YELLOW);
  if (flagRight & LOAD_FLAG_RIGHT_COLOR)
    ledPanel.writeChar('R', 50 + PANEL_LEFT_MARGIN, COLOR_IDX_YELLOW);
  
  /* below: animate colors */
  for (int y=0; y<5; y++) { /* each row of LED panel */
    byte clr1 = colors[(counter + y) % 5];
    byte clr2 = colors[(counter + y + 2) % 5];
    ledPanel.SetColorInArea(0 + PANEL_LEFT_MARGIN, y, PANEL_COLS-1, y, clr1);
    ledPanel.SetColorInArea(15 + PANEL_LEFT_MARGIN, y, 20, y, clr2);
    ledPanel.SetColorInArea(41 + PANEL_LEFT_MARGIN, y, 46, y, clr2);
  }

  if (flagLeft & LOAD_FLAG_LEFT_WHITE)
    ledPanel.writeChar('L', 6 + PANEL_LEFT_MARGIN, COLOR_IDX_WHITE);
  else if (flagLeft == 0 )
    ledPanel.writeChar(';', 7 + PANEL_LEFT_MARGIN, COLOR_IDX_RED);
  
  if (flagRight & LOAD_FLAG_RIGHT_WHITE)
    ledPanel.writeChar('R', 50 + PANEL_LEFT_MARGIN, COLOR_IDX_WHITE);
  else if (flagRight == 0 )
    ledPanel.writeChar(';', 51 + PANEL_LEFT_MARGIN, COLOR_IDX_RED);

  int clr = (flagLeft | flagRight) == LOAD_FLAG_NONE ? COLOR_IDX_RED : COLOR_IDX_GREEN;
  ledPanel.writeChar('O', 25 + PANEL_LEFT_MARGIN, clr);
  ledPanel.writeChar('K', 31 + PANEL_LEFT_MARGIN, clr);

  ledPanel.writeLeds_asm();
}

#if (WIFI_HARDWARE_AVAILABLE == true)
void displayWifiConnectionIP(bool init) {
  char bufMsg[] = "CONNECTED";
  static char bufIP[16];    /* e.g.: '192.168.100.200' , max 15 chars */
  static int wMsg, wIP;     /* widths in LEDs/pixels */
  static int xMsg, xIP, x;  /* x-positions (column-index from left) on LED-panel */
  IPAddress ip = WiFi.localIP();
  if (init) {    
    sprintf(bufIP, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    wMsg = ledPanel.getTextWidth(bufMsg) + 6;      /* width of 'CONNECTED' message */
    wIP  = ledPanel.getTextWidth(bufIP) + 7;       /* width of IP address in text */
    xMsg = PANEL_COLS - 1;              /* start scrolling from the very right, column 60 */
    xIP = xMsg + wMsg;
  }
  ledPanel.clear(); /* clear all LEDs in data structure */
  int wAll = wMsg + wIP;
  x = xMsg;
  while (x < PANEL_COLS) { ledPanel.writeText(bufMsg, x, COLOR_IDX_GREEN); x += wAll; }
  x = xIP;
  while (x < PANEL_COLS) { ledPanel.writeText(bufIP, x, COLOR_IDX_WHITE); x += wAll; }
  ledPanel.writeLeds_asm();
  xMsg--;  xIP--;    /* scroll to left when called next time */
  if (xMsg < -wMsg) xMsg = xIP + wIP; /* 'CONNECTED' scrolled out of the LED panel? */
  if (xIP < -wIP) xIP = xMsg + wMsg; /* IP address scrolled out of the LED panel? */
}
#endif // (WIFI_HARDWARE_AVAILABLE == true) 
