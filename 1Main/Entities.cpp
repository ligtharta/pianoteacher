#include "Entities.h"
#include "MidiDefs.h"


/******************************************************************************************************************************
*
* CLASS  :  StorageEntityBase
* 
*******************************************************************************************************************************/


// declaration of static members
char StorageEntityBase::row_Name[20];
char StorageEntityBase::row_Value[50];
const char* StorageEntityBase::_delimiter = ",#";    /* delimiters, when parsing multi-part values */

/* Read colon-seperated name/value pair and copy to row_Name[] and row_Value[] buffers */
bool StorageEntityBase::readNameValueRow(File* file) {
  int bufSize, n;
  char* buf;
  char ch;
  /* read 'name' part */
  buf = row_Name; bufSize = 20; n = 0;
  while ((n + 1) < bufSize && file->read(&ch, 1) == 1 && ch != ':') {
    if (ch == '\n') { continue; }              /* ignore newline */
    if (ch >= 'A' && ch <= 'Z') { ch += 32; }  /* make lower case */
    buf[n++] = ch;
  }
  if (ch != ':') { return false; }
  buf[n] = '\0';
  /* read 'value' part */
  buf = row_Value; bufSize = 50; n = 0;
  while ((n + 1) < bufSize && file->read(&ch, 1) == 1 && ch != '\r') {
    buf[n++] = ch;
  }
  if (ch != '\r' && file->available() > 0) { return false; }
  buf[n] = '\0';
  return true;
}

char* StorageEntityBase::readFirstValuePart() {
  return strtok(row_Value, _delimiter); 
}

char* StorageEntityBase::readNextValuePart() {
  return strtok(NULL, _delimiter);
}

void StorageEntityBase::toUpper(char *sPtr) {
  while ( *sPtr != '\0' ) {
    *sPtr = toupper ( (char)*sPtr );
    ++sPtr;
  }
}


/******************************************************************************************************************************
*
* CLASS  :  User
* 
*******************************************************************************************************************************/


// declaration of static members
byte User::userId;
char User::userName[15];
byte User::lastSong = 0;
int User::tempoFactor = 0;
byte User::metronome = 0;
bool User::isGloves = 0;
byte User::practice = PRACTICE_1;
bool User::isPracticeRepeat = true;
byte User::animationSpeed = DEFAULT_ANIMATION_SPD;
byte User::playWhilePractice = PLAY_WHILE_PRACTICE_OFF; /* not persisted on SD card */
byte User::globalBrightness = 0;                        /* not persisted on SD card */
byte User::panelRowsUsed = 5;                           /* not persisted on SD card */
bool User::leftHandCue = false;                         /* not persisted on SD card */

char User::_userName_cpy[15];
byte User::_lastSong_cpy = 0;
int  User::_tempoFactor_cpy = 0;
byte User::_metronome_cpy = 0;
bool User::_isGloves_cpy = 0;
byte User::_practice_cpy = PRACTICE_1;
bool User::_isPracticeRepeat_cpy = true;
byte User::_animationSpeed_cpy = DEFAULT_ANIMATION_SPD;


User::User() {
}

void User::getFilename(int userId, char* charBuffer) {
  sprintf (charBuffer, "user%02d.txt", userId);  
}

void User::parseUser(int id, File* file) {
  userId = id;
  while(readNameValueRow(file))
  {
    if (strcmp(row_Name, "name") == 0) {
      strcpy(userName, row_Value);
      toUpper(userName);
    }
    else if (strcmp(row_Name, "song") == 0)
      lastSong =  (byte)atoi(row_Value);
    else if (strcmp(row_Name, "tempo") == 0)
      tempoFactor = atoi(row_Value);
    else if (strcmp(row_Name, "gloves") == 0)
    {
      if (GLOVES_HARDWARE_AVAILABLE) isGloves = (bool)atoi(row_Value);
    }
    else if (strcmp(row_Name, "metronome") == 0)
    {
      if (METRONOME_HARDWARE_AVAILABLE) metronome = (byte)atoi(row_Value);
    }
    else if (strcmp(row_Name, "practice") == 0) {
      isPracticeRepeat = (row_Value[1] == 'R');
      row_Value[1] = '\0';
      practice = (byte)atoi(row_Value);
    }
    else if (strcmp(row_Name, "animation") == 0)
      animationSpeed = (byte)atoi(row_Value);
  }
}

void User::writeUser(File* file) {
  file->print("name:");
  file->println(userName);
  file->print("song:");
  file->println(lastSong);
  file->print("tempo:");
  file->println(tempoFactor);
  file->print("gloves:");
  file->println(isGloves ? 1 : 0);
  file->print("metronome:");
  file->println(metronome);
  file->print("practice:");
  file->print(practice);
  file->println(isPracticeRepeat ? "R" : "");
  file->print("animation:");
  file->println(animationSpeed);
}

void User::resetDirty() {
  strcpy(_userName_cpy, userName);
  _lastSong_cpy = lastSong;
  _tempoFactor_cpy = tempoFactor;
  _metronome_cpy = metronome;
  _isGloves_cpy = isGloves;
  _practice_cpy = practice;
  _isPracticeRepeat_cpy = isPracticeRepeat;
  _animationSpeed_cpy = animationSpeed;
}
  
bool User::isDirty() {
  if (strcmp(_userName_cpy, userName) != 0) return true;
  return (_lastSong_cpy         != lastSong ||
          _tempoFactor_cpy      != tempoFactor ||
          _metronome_cpy        != metronome ||
          _isGloves_cpy         != isGloves ||
          _practice_cpy         != practice ||
          _isPracticeRepeat_cpy != isPracticeRepeat ||
          _animationSpeed_cpy   != animationSpeed
          );
}

void User::setDefaults(int id) {
  userId = id;
  strcpy(userName,"USER");
  lastSong = DEFAULT_SONG_ID;
  tempoFactor = 100;
  metronome = 0;
  isGloves = 0;
  practice = PRACTICE_1;
  isPracticeRepeat = false;
  animationSpeed = DEFAULT_ANIMATION_SPD;
}



/******************************************************************************************************************************
*
* CLASS  :  Settings
* 
*******************************************************************************************************************************/


// declaration of static members
int Settings::lastUser = 0;
char Settings::wifiSSID[MAX_LEN_SSID + 1];
char Settings::wifiPassword[MAX_LEN_PASSW + 1];
int Settings::_lastUser_cpy = 0;


Settings::Settings() { 
}

void Settings::getFilename(char* charBuffer) { 
  sprintf (charBuffer, "settings.txt");
}

void Settings::parseSettings(File* file) {
  while(readNameValueRow(file))
  {
    if (strcmp(row_Name, "lastuser") == 0)
      lastUser =  atoi(row_Value);
    else if (strcmp(row_Name, "ssid") == 0) {
      row_Value[MAX_LEN_SSID] = 0;  /* prevent buffer overrun in 'wifiSSID' */
      strcpy(wifiSSID, row_Value);
    }
    else if (strcmp(row_Name, "password") == 0) {
      row_Value[MAX_LEN_PASSW] = 0;  /* prevent buffer overrun in 'wifiPassword' */
      strcpy(wifiPassword, row_Value);
    }      
  }
}

void Settings::writeSettings(File* file) {
  file->print("LastUser:");
  file->println(lastUser);
  file->print("SSID:");
  file->println(wifiSSID);
  file->print("Password:");
  file->println(wifiPassword);
}

void Settings::resetDirty() {
  _lastUser_cpy = lastUser;
}
  
bool Settings::isDirty() {
  return (_lastUser_cpy != lastUser);
}

void Settings::setDefaultSettings() {
  lastUser = DEFAULT_USER_ID;
  strcpy(wifiSSID, "");
  strcpy(wifiPassword, "");
}



/******************************************************************************************************************************
*
* CLASS  :  SongNote
* 
*******************************************************************************************************************************/



bool SongNote::isFingerLeft() {
  return (finger >= 1 && finger <= 5);
}

bool SongNote::isFingerRight(){
  return (finger >= 6 && finger <= 10);  
}


bool SongNote::isFingerLeft(byte f) {
  return (f >= 1 && f <= 5);
}

bool SongNote::isFingerRight(byte f){
  return (f >= 6 && f <= 10);  
}


/******************************************************************************************************************************
*
* CLASS  :  Song
* 
*******************************************************************************************************************************/



Song::Song() {
  
}


void Song::getFilename(int songId, char* charBuffer) {
  sprintf (charBuffer, "song%02d~1.txt", songId);  
}


void Song::parseSong(int id, File* file, byte loadFlags) {
  songId = id;
  char *token;
  noteCount = 0;
  uint16_t tempoQPM = 0;
  int measureNr = 0;
 
  while(readNameValueRow(file))
  {
    if (strcmp(row_Name, "name") == 0) {                             /* example:   Name:Mijn ome Bill */
      strcpy(songName, row_Value);
      toUpper(songName);
    }
    else if (strcmp(row_Name, "resolution") == 0) {                  /* example:   Resolution:24 */
      resolution =  atoi(row_Value);
    }
    else if (strcmp(row_Name, "tempo") == 0) {                       /* example:   Tempo:130 */
      tempoQPM = atoi(row_Value);                                    /* in Quarter Notes Per Minute, for example: 130 */
    }
    else if (strcmp(row_Name, "endtick") == 0) {                     /* example:   EndTick:2880 */
      totalTicks = atoi(row_Value);
    }
    else {
      /* multi-part (comma-seperated) row_Value from here (except for 'Bookmark') */
      int i = 0;    
      token = readFirstValuePart();
      if (isdigit(row_Name[0])) {             /* numeric row_Name means MIDI-tick, e.g.: "240:R1,60,100,120" */
        uint32_t tick = atol(row_Name);
        if (strncasecmp(token, "measure", 7) == 0) {   /* parse Measure (e.g.: "1080:Measure1,3,120"  or: "1080:Measure1,3,120,T130" when tempo is set) */
          if (noteCount < MAX_NOTES) {  
            bool hasBookmark = (strncasecmp(token, "measureBM", 9) == 0);  /* or: "1080:MeasureBM1,3,120"  when measure is bookmarked */
            SongMeasure* measure = (SongMeasure*)(&notes[noteCount]);
            measure->atTick = tick;
            measure->beatCount = (byte)atoi(readNextValuePart());  /* e.g.: 3 */
            measure->beatTicks = atol(readNextValuePart());        /* e.g.: 120 */
            token = readNextValuePart();               /* only when tempo is set (e.g.: T130)  */
            if (token != NULL && token[0] == 'T') tempoQPM = atoi(token + 1); /* change tempo of song from here!!! */
            measure->tempoQPM = tempoQPM;
            measure->measureNr = ++measureNr;     /* just counting up for each measure */
            measure->unused = 0;
            measure->type = hasBookmark ? TYPE_MEASURE_BM : TYPE_MEASURE;  /* bookmarked measure or normal measure */
            noteCount++;
          }
        }
        else if (token[0] == 'R' || token[0] == 'L' || token[0] == '_') {    /* parse Note (e.g.: "240:R1,60,100,120"  */
          if (noteCount < MAX_NOTES) {
            SongNote* note = &notes[noteCount];
            note->atTick = tick;
            if (token[0] == '_') {
              note->finger = 0;                                /* '_' means 'Unknown' finger */
            }
            else {
              note->finger = token[1] - '0';                   /* finger that should play this note, range 1-5 */
              note->finger += (token[0] == 'R' ? 5 : 0);       /* fingers right hand range 6-10 */
            }
            note->pitch = (byte)atoi(readNextValuePart());     /* MIDI-pitch of note */
            note->velocity = (byte)atoi(readNextValuePart());  /* MIDI-velocity of note -> this is discarded, see line below! */
            note->velocity = MIDI_DEFAULT_VELOCITY;            /* MIDI-velocity set to fixed value, dependent on device */
            note->duration = atoi(readNextValuePart());        /* duration of note in MIDI-ticks */
            note->type = TYPE_NOTE;
            /* pitch must be within supported range (for 61 key-instrument: between C2 en C7) */
            if (note->pitch >= MIDI_PITCH_MIN && note->pitch <= MIDI_PITCH_MAX) { 

              if (note->isFingerLeft()) /* this note is for left hand */
              {
                if (loadFlags & LOAD_FLAG_LEFT_MASK)    /* must left hand data be loaded? */
                {
                  noteCount++;
                }
                if (loadFlags & LOAD_FLAG_LEFT_WHITE)
                {
                  note->finger = 0; /* remove finger data for left hand (all white LEDs) */
                }
              }
              else if (note->isFingerRight()) /* this note is for right hand */
              {
                if (loadFlags & LOAD_FLAG_RIGHT_MASK)    /* must right hand data be loaded? */
                {
                  noteCount++;
                }
                if (loadFlags & LOAD_FLAG_RIGHT_WHITE)
                {
                  note->finger = 0; /* remove finger data for right hand (all white LEDs) */
                }
              }
              else /*not left, not right -> unknown finger */
              {
                  noteCount++;
              }
            }
          }
        }
      } 
      else {
        // ERROR ERROR ERROR!!! UNKNOWN row_NAME !!!!!
      }
    }
  }
  lastMeasureNr = measureNr; /* keep this number as part of Song object */
  _analyseSong();
}




void Song::_analyseSong() {
  SongNote* note;
  int pitch;
  /* set all pitches as 'unused' (=false) first... */
  for (int i=0; i<MAX_UNIQUE_PITCHES; i++) { _songAnalysis[i] = false; }

  /* scan all notes of current song... */
  for (int i=0; i < noteCount; i++) {
    note = &notes[i];
    if (note->type != TYPE_NOTE) continue; /* skip if not a note, but a measure */
    pitch = note->pitch - MIDI_PITCH_MIN;
    if (pitch < 0 || pitch >= MAX_UNIQUE_PITCHES) { continue; }
    /* pitch is now indexer: between 0 and (MAX_UNIQUE_PITCHES-1) */
    _songAnalysis[pitch] = true;
  }
}


bool* Song::getSongAnalysis() {
  return _songAnalysis;
}


int Song::getMeasureStartIndex(int measureNr) {
  SongMeasure* measure;
  for (int i=0; i<noteCount; i++) {
    measure =(SongMeasure*)&notes[i];
    if (measure->type == TYPE_NOTE) continue; /* only look at measures, not notes */
    if (measure->measureNr == measureNr) return i;
  }
  return 0; /* not found, start at first measure */
}

int Song::getNextBookmarkMeasureNr(int curMeasureNr, bool forward) {
  if (!forward && curMeasureNr == 1) curMeasureNr = lastMeasureNr;
  int i = getMeasureStartIndex(curMeasureNr); /* first look-up index of current measure */
  SongMeasure* measure;
  while (true) {
    i += (forward ? 1 : -1);
    if (i < 0)         return 1; /* not found, start at first measure */    
    if (i > noteCount) return 1; /* not found, start at first measure */    
    measure =(SongMeasure*)&notes[i];
    if (measure->type == TYPE_MEASURE_BM) {
      /* Yes, bookmark found! */
      return measure->measureNr;
    }
  }
}
