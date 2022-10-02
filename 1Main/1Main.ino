// #define DEBUG_MODE
/**************************************************************************************************************************
* Piano Teacher, V 1.0   -  made for Arduino    -   developed by Andre Ligthart in 2018-2020
***************************************************************************************************************************
* Naming conventions:
* -  classes start with capital letter (UpperCamelCase): class FootPedal
* -  global functions and variables in lowerCamelCase: loop()
* -  public members of classes also in lowerCamelCase: writeText() 
* -  private members of classes start with underscore: _currentTick 
* -  defines in all capitals: UPCOMING_NOTES_MAX, FOOTPEDAL_LEFT_PIN
***************************************************************************************************************************
* About memory (RAM): 
* - The Arduino device has not more than 32 kB of RAM.
* - No dynamic memory allocation is used (malloc), to prevent heap fragmentation and improve robustness. 
* - Thus: objects are either declared globally or within the scope of functions (=stack).
* - The biggest object that is put on the stack is Player2 or Player3 which is about 1400 bytes.
* - A song can have no more than MAX_NOTES notes. If set too high, the stack may destroy important data in RAM.
***************************************************************************************************************************
* About Arduino device support:
*   This code has been tested with these 4 boards:   MKR 1000,   MKR WiFi 1010,   MKR Zero,   Nano 33 IoT.
*   All these boards use the same processor: SAMD21 Cortex-M0+ 32 bit.
*   They all run at 48MHz speed: this is important for driving the LED panel (timing, see writeLeds_asm() in LedPanel.cpp)
*   See file 'HardwDefs.h' for settings about hardware and Arduino boards.
**************************************************************************************************************************/

#include "Templates.h"
#include "LedPanel.h"
#include "Metronome.h"
#include "Gloves.h"
#include "FootPedal.h"
#include "SdCard.h"
#include "Midi.h"
#include "APlayer0.h"  /* used in start screen and during song selection */
#include "APlayer1.h"  /* while doing practice 1 (user controls pace/tempo/note lengths) */
#include "APlayer2.h"  /* while doing practice 2 (system sets the pace/tempo/note lengths)  */
#include "APlayer3.h"  /* while doing practice 3 (system sets the pace/tempo/note lengths)  */
#include "APlayer4.h"  /* while doing practice 4 (learn to read notes)  */
#include "APlayer5.h"  /* while doing practice 5 (learn to read notes)  */

/* When NOT in practice mode, the user can switch between 3 Start-views/screens */
#define START_VIEW_0_NONE         0
#define START_VIEW_1_SCROLL       1  /* shows user and song, with a scroll text */
#define START_VIEW_2_SETTINGS     2  /* shows settings */
#define START_VIEW_3_PREVIEW      3  /* song preview / analysis */

/* global objects */
LedPanel      ledPanel;   /* panel with 5 LEDs for each piano key, also 4 push buttons (user, song, right/left, wifi) */
Metronome     metronome;  /* optional metronome that ticks at every measure or beat */
Gloves        gloves;     /* optional special gloves with a vibrating motor on each of the 10 fingers */
FootPedal     footPedal;  /* 3-switch foot pedal, mainly to control/navigate while playing  */
SdCard        sdCard;     /* SD Card with a file for each song, also for each user, and also a general settings file */
Song          song;       /* represents the data of the loaded song */
MidiInterface midi;       /* to exchange MIDI messages with the digital piano/keyboard */

/* include for Wifi depends on chip on Arduino board */
#include <SPI.h>
#ifdef WIFI_HARDWARE_ATMEL /* ATMEL is used on Arduino MKR 1000 */ 
#include <WiFi101.h>       /* https://www.arduino.cc/reference/en/libraries/wifi101/ */
#endif
#ifdef WIFI_HARDWARE_UBLOX /* UBLOX is used on Arduino Mkr WIFI 1010  and  Arduino Nano 33 IoT */
#include <WiFiNINA.h>      /* https://www.arduino.cc/reference/en/libraries/wifinina/ */
#endif

bool setupSucceed = false;

/******************************************************************************************************************************
* Arduino setup()
*******************************************************************************************************************************/
void setup() {
#ifdef DEBUG_MODE
  Serial.begin(9600);
  // while (!Serial) { ; } /* wait for serial port to connect. Needed for native USB port only */
#endif
  bool success = sdCard.init_SD();
  if (!success) { 
    displayError("SD ERROR");
    return; 
  }
  midi.init_MIDI();
  sdCard.loadSettings();
  sdCard.loadUser(Settings::lastUser);
  sdCard.loadSong(&song, User::lastSong, getSongLoadingFlags());

#ifdef DEBUG_MODE
  handleTestsDuringSetUp();
  //  test_CircularArray();
  //  test_DumpSongData();
  //  test_FootPedals2();
#endif
  setupSucceed = true;
}


/******************************************************************************************************************************
* Arduino loop()
*******************************************************************************************************************************/
void loop() {
  static int startView = START_VIEW_1_SCROLL;
  static int startMeasureNr = 1;
  if (!setupSucceed) return;

  bool resetStartMeasureNr;
  byte practiceId = doStartViews(startView, &resetStartMeasureNr);
  if (resetStartMeasureNr) startMeasureNr = 1;
  
  // play & practice here
  switch(practiceId) {
    case PRACTICE_1:
      startMeasureNr = doPractice1(startMeasureNr);  /* practice where user sets the pace */
      break;
    case PRACTICE_2:
      startMeasureNr = doPractice2(startMeasureNr);  /* practice where system sets the pace */
      break;
    case PRACTICE_3:
      startMeasureNr = doPractice3(startMeasureNr);  /* practice where system sets the pace, LEDs fly down */
      break;
    case PRACTICE_4:
      startMeasureNr = doPractice4(startMeasureNr);  /* learn to read music notes */
      break;
    case PRACTICE_5:
      startMeasureNr = doPractice5(startMeasureNr);  /* learn to read music notes */
      break;
  } 
  startView = START_VIEW_2_SETTINGS;
}



byte doStartViews(int startView, bool* resetStartMeasureNr) {
  *resetStartMeasureNr = false;
  Player0   player(&midi, &metronome, &ledPanel);        /* Simple player: play song via MIDI and show LEDs in simple way */
  int pKey; /* pressed piano key */
  int x; /* column on LED panel */
  int view = startView;
  int formerView = START_VIEW_0_NONE;       /* force 'needInit' to true */
  uint32_t now;
  midi.clearReadBuffer();
  while (true) {
    now = millis();
    pKey = midi.getPressedPianoKey(); /* zero if no piano key was pressed */
    x = pKey == 0 ? -1 : pKey - MIDI_PITCH_MIN - PANEL_LEFT_MARGIN; /* x is between 0 and 60 (for 61 key instrument) */
    bool needInit = (view != formerView); /* just switched to another screen? */
    /* below which settings are valid in which practice modes? */
    bool canAniSpd = (User::practice == PRACTICE_3); /* can the animation speed of LEDs (that fall down) be set */
    bool canTempo = (User::practice == PRACTICE_3 || User::practice == PRACTICE_2); /* can tempo/pace be set */
    bool canMetro = canTempo; /* is the metronome possible? */
    bool canGloves = (canMetro || User::practice == PRACTICE_1); /* are gloves (with vibration motors) possible? */
    bool canMidiOut = (User::practice != PRACTICE_1);  /* can the system play notes using MIDI out? If so, user can control volume */
    bool canRepeat = (User::practice != PRACTICE_4); /* is repeat (after song is finished) possible? */
    bool canLimitPanelRows = (User::practice == PRACTICE_1); /* can user limit the amount of steps that can be previewed on the LED panel? */
    bool canLeftHandCue = (User::practice == PRACTICE_1); /* is a cue (grey LED) above left hand notes possible? */
    bool settingsDirty = User::isDirty() || Settings::isDirty(); /* has any (persistable) setting been changed by user?  */
    switch(view) {
      /* this screen shows username + songId + songName */
      case START_VIEW_1_SCROLL: 
        displayUserAndSongScroll(needInit, settingsDirty);
        if (settingsDirty && pKey == MIDI_PITCH_MIN) saveDirtySettingsAndUser(); /* save changed Settings to SD Card */
        delay(25);
        break;
      /* screen to show and modify settings: practice type, metronome, gloves, tempo, repeat, etc. */
      case START_VIEW_2_SETTINGS:
        displaySettingsOnLEDpanel();
        if (              x >=  1 && x <=  6) if (++User::practice > PRACTICE_5) User::practice = PRACTICE_1 ; /* practice type is 1, 2, 3 or 4 */
        if (canRepeat  && x >=  7 && x <= 10) User::isPracticeRepeat = !User::isPracticeRepeat; /* On or Off: repeat song in practice mode */
        if (canMidiOut && x >= 11 && x <= 14) if (++User::playWhilePractice > PLAY_WHILE_PRACTICE_VOLU_3) User::playWhilePractice = PLAY_WHILE_PRACTICE_OFF;
        if (canAniSpd  && x >= 16 && x <= 19) if (++User::animationSpeed > 3) User::animationSpeed = 0; /* speed of falling LEDs on LED-panel (practice 3 only) */
        if (METRONOME_HARDWARE_AVAILABLE)
          if (canMetro && x >= 21 && x <= 28) if (++User::metronome > METRONOME_ALL_BEATS) User::metronome = METRONOME_OFF; /* metronome type is 0, 1 or 2 (0=off) */
        if (GLOVES_HARDWARE_AVAILABLE)
          if (canGloves && x >= 29 && x <= 35) User::isGloves = !User::isGloves;       /* toggle vibrating gloves ON/OFF */
        if (canTempo  && x >= 36 && x <= 47) User::tempoFactor -= (x <= 41 ? 5 : 1); /* decrease tempo-factor with 1% or 5% */
        if (canTempo  && x >= 48 && x <= 58) User::tempoFactor += (x >= 53 ? 5 : 1); /* increase tempo-factor with 1% or 5% */
        if (User::tempoFactor > 180) User::tempoFactor = 180;    /* max tempo-factor in percentage */
        if (User::tempoFactor < 10)  User::tempoFactor = 10;     /* min tempo-factor in percentage */
        if (canLimitPanelRows && x >= 42 && x <= 53) User::panelRowsUsed ^= 4; /* toggle between 5 and 1 (1 XOR 4 = 5, 5 XOR 4 = 1) */
        if (canLeftHandCue && x == 40) User::leftHandCue = !User::leftHandCue; /* toggle left hand cue (grey LED) above notes on/off */
        if (x == 60) { if (++User::globalBrightness > 3) User::globalBrightness = 0; ledPanel.setGlobalBrightness(User::globalBrightness); }
        delay(50);
        break;
      /* screen to show song analysis and to play the song  */
      case START_VIEW_3_PREVIEW: 
        if (needInit) { 
          displaySongAnalysis(); 
          metronome.working = User::metronome;
          player.startSong(&song, User::tempoFactor, true /* repeat song */);
        }
        if (player.isPlaying) player.handlePlaying();
        break;
    }
    formerView = view;
    /* read foot pedals */
    footPedal.readPedals(millis());
    bool isPedalR = footPedal.isRightPressed();
    bool isPedalL = footPedal.isLeftPressed();
    bool isPedalM = footPedal.isMiddlePressed();
    bool isPedalRdown = footPedal.isRightDown();
    bool isPedalLdown = footPedal.isLeftDown();
    /* middle pedal pressed while (left or right) pedal down: go into LOCKED mode */
    if ( isPedalM && (isPedalRdown || isPedalLdown) ) {
      player.stopPlayingNow();
      HandleLockedMode();
      view = START_VIEW_1_SCROLL;
      continue; /* go to top of the main while loop */
    }
    /* only middle pedal pressed: quit and start practicing */
    if (isPedalM) {
      player.stopPlayingNow(); 
      break;  /* return to loop() , from there, start practicing  */
    }
    /* read LED panel buttons */
    ledPanel.readButtons();
    bool isBtnUser   = ledPanel.isButtonPressed(BUTTON_USER);   /* select and load user */
    bool isBtnSong   = ledPanel.isButtonPressed(BUTTON_SONG);   /* select and load song */
    bool isBtnReload = ledPanel.isButtonPressed(BUTTON_RELOAD); /* reload song with restricted data like only right hand data */
    bool isBtnWifi   = ledPanel.isButtonPressed(BUTTON_WIFI);   /* receive new song files/versions wirelessly */
    /* nav to next/former screen */
    if ((isPedalR || isPedalL)) {
      if (isPedalR) { view = (view == START_VIEW_3_PREVIEW ? START_VIEW_1_SCROLL  : view + 1); } /* next screen */
      if (isPedalL) { view = (view == START_VIEW_1_SCROLL  ? START_VIEW_3_PREVIEW : view - 1); } /* former screen */
      *resetStartMeasureNr = true; /* when same song is practiced again, always start at measure 1 */
      player.stopPlayingNow(); /* stop playing song when switching view */
    }
    /* handle buttons on LED panel: navigate to user, song, left-right or wifi */
    bool gotoSelection = (isBtnUser || isBtnSong || isBtnReload || isBtnWifi);
    if (gotoSelection) {
      delay(75);                           /* wait a bit, to deal with possible button bounce effect */
      *resetStartMeasureNr = true;         /* when same song is practiced again, always start at measure 1 */
      player.stopPlayingNow();             /* to selection subscreen: turn off all notes immediately */
      if (isBtnUser) selectUser();         /* select another user */
      else if (isBtnSong) selectSong();    /* select another song */
      else if (isBtnReload) reloadSong();  /* reload song (optionally omit left or right hand data using foot pedal) */
      else if (isBtnWifi) handleWifi();    /* start web server, and wait for incoming data */
      formerView = START_VIEW_0_NONE;      /* force 'needInit' to true */
      delay(75);                           /* wait a bit, to deal with possible button bounce effect */
    }
  }
  return User::practice;  /* type of practice the user chooses */
}


/******************************************************************************************************************************
* Save changed User settings to SD Card.
*******************************************************************************************************************************/
void saveDirtySettingsAndUser() {
  sdCard.saveUserIfDirty();
  sdCard.saveSettingsIfDirty();
  
  ledPanel.clear(); /* clear all LEDs in data structure */
  displayMsg("SAVED", COLOR_IDX_GREEN);
  ledPanel.writeLeds_asm();
  delay(800);    
}

/******************************************************************************************************************************
* User wants to select another user (Button 1 on the LED panel was pressed)
* Each user is represented by colored filled circle. Each user has a unique color. Max 5 user to choose from.
* The selected user will be persisted/saved on SD Card (NOT ANYMORE, NOW EXPLICIT ACTION BY USER).
*******************************************************************************************************************************/
void selectUser() {
  int pKey; /* pressed piano key */
  int selected = 0;
  ledPanel.clear(); /* clear all LEDs in data structure */
  ledPanel.writeLeds_asm();
  sdCard.scanUserAndSongFiles();
  while (selected == 0) {
    for (int i=1; i<=5; i++) {
      if (sdCard.isUserAvailable(i)) {
        byte clrIdx = ledPanel.getUserColorIdx(i); /* each user has own color */
        ledPanel.writeChar('*', (i - 1) * 12 + 5 +  + PANEL_LEFT_MARGIN, clrIdx);
      }
    }
    ledPanel.writeLeds_asm();
    while ( (pKey = midi.getPressedPianoKey()) == 0 )
    {
      ledPanel.readButtons();
      if (ledPanel.isButtonPressed(BUTTON_USER)) return; /* Abort USER selection: go back to start screen */
      delay(5); /* wait for piano key to be pressed */
    }
    /* convert pressed piano Key to userId, eg. user 1 is selected when pKey between 41 and 47 */
    pKey -= 29;
    int id = pKey / 12;
    if ( (pKey % 12 <= 6) && id >= 1 && id <= 5) {
      if (sdCard.isUserAvailable(id)) { selected = id; } /* selected!... */
    }
  }
  /* load this user and exit.... */
  Settings::lastUser = selected;
  // sdCard.saveSettingsIfDirty();
  sdCard.loadUser(Settings::lastUser);
  sdCard.loadSong(&song, User::lastSong, getSongLoadingFlags());
}



/******************************************************************************************************************************
* User wants to select/load a song (Button 2 on the LED panel was pressed)
* Each song is represented by a LED on the LED panel. Selection is done using piano keys which are read via MIDI.
* When a new song is selected: play the song and make corresponding LED green.
* After a selected (green) song is selected again (confirmed): load the song and quit.
*******************************************************************************************************************************/
void selectSong() {
  metronome.working = METRONOME_OFF;
  Player0   player(&midi, &metronome);           /* Simple player: just play song via MIDI (no metronome, no LEDs)  */
  int pKey; /* pressed piano key */
  bool songLoaded = false;     /* cannot cancel song selection with button after song has been loaded for preview listening */
  int selected = song.songId;  /* currently loaded songId */
  bool confirmed = false;      /* true when the same song is selected again (confirm selection) */
  byte loadingFlags = LOAD_FLAG_NONE; /* which data to load? (all, only left hand, only right hand) */
  uint32_t canCancelAfter = millis() + 750; /* cannot cancel with button for some millis, to prevent button bounce effect */ 
  ledPanel.clear();       /* clear all LEDs in data structure */
  for (int x = 0; x < MAX_SONGS; x++)
  { /* draw auxiliary LEDs for easy counting (helps user to see which song Id is choosen) */
    int unit = (x%10); /* between 0 and 9 */
    byte clr = unit < 5 ? COLOR_IDX_BLUE : COLOR_IDX_GREY;
    ledPanel.setPixel(x + PANEL_LEFT_MARGIN, 0, clr); /* 1->5 blue, 6->10 white, 11->15 blue, etc */
    if (unit == 4 || unit == 9) 
      ledPanel.setPixel(x + PANEL_LEFT_MARGIN, 1, clr); /* extra pixels at 5, 10, 15, 20, etc. */
  }
  ledPanel.writeLeds_asm();
  sdCard.scanUserAndSongFiles();
  byte color;
  while (!confirmed) {
    for (int id = 1; id <= MAX_SONGS; id++) { /* max 60 songs, filename 'SONG01~1.TXT' -> 'SONG60~1.TXT' */
      color = (id == selected) ? COLOR_IDX_GREEN : COLOR_IDX_RED;    /* RED: song exists, GREEN: song selected */
      if (sdCard.isSongAvailable(id)) { ledPanel.setPixel(id-1 + PANEL_LEFT_MARGIN, 4, color ); }
    }
    ledPanel.writeLeds_asm();
    while ( (pKey = midi.getPressedPianoKey()) == 0 ) {  /* loop until piano key is pressed or user wants to quit (Button2) */
      ledPanel.readButtons();
      if (ledPanel.isButtonPressed(BUTTON_SONG) && !songLoaded) {
        if (millis() > canCancelAfter) {
          player.stopPlayingNow();
          return; /* Abort SONG selection: go back to start screen */
        }
      }
      if (player.isPlaying) player.handlePlaying();
      delay(1);
    }
    int id = pKey - MIDI_PITCH_MIN + 1 - PANEL_LEFT_MARGIN; /* first piano key == songId 1, etc */
    if (sdCard.isSongAvailable(id)) {
      if (id != selected || loadingFlags != getSongLoadingFlags() ) { 
        loadingFlags = getSongLoadingFlags(); /* flags that determine which data to load (all, left hand, right hand) */
        selected = id; /* song selected: make LED green and play the song */
        player.stopPlayingNow();  /* turn off all notes immediately */ 
        sdCard.loadSong(&song, id, loadingFlags); /* load song... */
        songLoaded = true;
        player.startSong(&song, 100 /* tempo-factor */ ,  false /* NO repeat */);        /* ... and play */
      }
      else { 
        confirmed = true; /* song re-selected (confirmed), exit while loop ... */
      }
    }
  }
  player.stopPlayingNow(); /* turn off all notes immediately */ 
  User::lastSong = selected;
  User::tempoFactor = 100; /* default tempo-factor is 100% */
  User::animationSpeed = DEFAULT_ANIMATION_SPD; /* new song loaded: reset to normal speed for LED animation */
}


/******************************************************************************************************************************
* User wants to reload the song (Button 3 on the LED panel was pressed)
* User can choose between: 
*    colored 'L' or 'R' : load left/right channel normaly (with finger data)
*    white 'L' or 'R'   : load left/right channel without finger data (all white)
*    'x'                : do not load this channel at all (left or right)
*******************************************************************************************************************************/
void reloadSong() {
  int pKey; /* pressed piano key */
  int x; /* column on LED panel */
  int c1=0, c2=0; /* counters */
  int flagsLeft[3] =  { LOAD_FLAG_NONE, LOAD_FLAG_LEFT_WHITE,  LOAD_FLAG_LEFT_COLOR  }; /* options for left hand data */
  int flagsRight[3] = { LOAD_FLAG_NONE, LOAD_FLAG_RIGHT_WHITE, LOAD_FLAG_RIGHT_COLOR }; /* options for right hand data */
  int left = 2, right = 2; /* indexers in arrays above. This means: both left and right hand data WITH color data */
  bool chosen = false;
  midi.clearReadBuffer();
  /* user can choose interactively, see while loop below */
  while (!chosen) {
    if ((++c1 % 60) == 2) displayReloadSongOptions(c2, flagsLeft[left], flagsRight[right]); /* use c2++ to enable color animation */
    ledPanel.readButtons();
    footPedal.readPedals(millis());
    if (ledPanel.isButtonPressed(BUTTON_RELOAD)) return; /* abort song reload -> quit */
    delay(5);
    pKey = midi.getPressedPianoKey(); /* zero if no piano key was pressed */
    x = pKey - MIDI_PITCH_MIN - PANEL_LEFT_MARGIN; /* x is between 0 and 60 (61 columns on lED-panel) */
    /* foot padel left or right: this is used for quick selection */
    if (footPedal.isLeftPressed() || footPedal.isRightPressed() ) {
      if (footPedal.isLeftPressed() ) { left = 2; right = 0; } /* left pedal:  load only left hand data (with colors) */
      else                            { left = 0; right = 2; } /* right pedal: load only right hand data (with colors) */
      displayReloadSongOptions(c2, flagsLeft[left], flagsRight[right]); /* display this selection for a short while, 0.4 sec ....*/
      delay(400); 
      chosen = true;
      continue;  /* ...and quit, so the selection is activated immediately. */
    }
    if (x >=  0 && x <= 18) { left =  (left  + 1) % 3; } /* toggle left between: colored, white, none */
    if (x >= 41 && x <= 60) { right = (right + 1) % 3; } /* toggle right between: colored, white, none */
    /* if both left and right are 'none', then 'OK' will be printed in red (invalid state): */
    if (flagsLeft[left] == LOAD_FLAG_NONE && flagsRight[right] == LOAD_FLAG_NONE) continue; 
    if (footPedal.isMiddlePressed() || (x >= 22 && x <= 38)) chosen = true; /* user activated 'OK', now quit loop and do the reload... */
  }
  int loadingFlags = flagsLeft[left] | flagsRight[right];
  sdCard.loadSong(&song, song.songId, loadingFlags);  /* reload the current song using the selected loadingFlags */
  displayMsg("DONE", COLOR_IDX_GREEN);
  delay(650); 
}


/******************************************************************************************************************************
* User wants to upload a new (version of a) song, using a wireless connection (Wifi).
* Start a web server, and wait for incoming data, see file '4Wifi.ino' for more...
*******************************************************************************************************************************/
void handleWifi() {
#if (WIFI_HARDWARE_AVAILABLE == true)
  webServer();                       /* start web server and wait for requests  */
  sdCard.resetUserAndSongFilesScan();/* scan user and song files again when song list is needed */    
#else
  /* if no Wifi hardware available: only display an error message */
  displayError("NO WIFI");
  delay(1500);
#endif // (WIFI_HARDWARE_AVAILABLE == true) 
}



/******************************************************************************************************************************
* When song is loaded (at start up, when user is selected, or when song is selected), the user can hold  
* the left or right pedal, which causes only the left or right hand channel to be loaded.
*******************************************************************************************************************************/
byte getSongLoadingFlags() {
  footPedal.readPedals(millis());
  if (footPedal.isRightDown()) return LOAD_FLAG_RIGHT_COLOR;
  if (footPedal.isLeftDown())  return LOAD_FLAG_LEFT_COLOR;
  return LOAD_FLAG_RIGHT_COLOR + LOAD_FLAG_LEFT_COLOR;
}




/******************************************************************************************************************************
* LOCK state entered: display 'LOCK' message on LED panel, then clear LED panel and wait for pedal input to exit.
* The goal of LOCK state is that the user can practice without using this sytem (empty LED panel), while leaving the system turned ON.
* the LED panel will be cleared and the system will ONLY respond to pedals, NOT to piano key input.
* This LOCK state was entered as follows: keep RIGHT or LEFT pedal pressed, then also press MIDDLE pedal.
*******************************************************************************************************************************/
void HandleLockedMode() {
  for (int i=0; i< 3; i++) {
    displayMsg("LOCK", COLOR_IDX_GREEN);
    delay(300);
    ledPanel.clear(); /* clear all LEDs in data structure */
    ledPanel.writeLeds_asm();
    delay(150);
  }
  ledPanel.clear(); /* clear all LEDs in data structure */
  ledPanel.writeLeds_asm();

  while (true) {
    footPedal.readPedals(millis());
    if (footPedal.isAnyPressed()) return;
    delay(10);
  }
}
