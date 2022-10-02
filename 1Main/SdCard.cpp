#include "SdCard.h"



/******************************************************************************************************************************
*
* CLASS  :  SdCard
* 
*******************************************************************************************************************************/

// declaration of static members
char SdCard::_filename[20];  

SdCard::SdCard() {
  _scanFilenamesDone = false;
}

bool SdCard::init_SD() {
  init_SD_success = SD.begin(SDCARD_CSPIN);
  return init_SD_success;
}

bool SdCard::loadSettings() {
  Settings::getFilename(_filename);
  File entry = SD.open(_filename);
  if (!entry) { 
    Settings::setDefaultSettings();
    //Settings::resetDirty();
    return false;
  }
  Settings::parseSettings(&entry);
  entry.close();
  Settings::resetDirty();
  return true;
}

bool SdCard::saveSettingsIfDirty() {  
  if (!Settings::isDirty()) return false;
  Settings::getFilename(_filename);
  SD.remove(_filename);
  File entry = SD.open(_filename, FILE_WRITE);
  if (!entry) return false;
  Settings::writeSettings(&entry);
  entry.close();
  Settings::resetDirty();
  return true;
}

bool SdCard::loadUser(int userId) {
  User::getFilename(userId, _filename);
  File entry = SD.open(_filename);
  if (!entry) {
    User::setDefaults(userId);
    //User::resetDirty();
    return false;
  }
  User::parseUser(userId, &entry);
  entry.close();
  User::resetDirty();
  return true;
}

bool SdCard::saveUserIfDirty() {
  if (!User::isDirty()) return false;
  User::getFilename((int)User::userId, _filename);
  SD.remove(_filename);
  //File entry = SD.open(_filename, O_RDWR | O_WRITE | O_CREAT | O_TRUNC);
  File entry = SD.open(_filename, FILE_WRITE);
  if (!entry) return false;
  User::writeUser(&entry);
  entry.close();
  User::resetDirty();
  return true;
}

bool SdCard::loadSong(Song* song, int songId, byte loadFlags) {
  Song::getFilename(songId, _filename);
  File entry = SD.open(_filename);
  if (!entry) return false;
  song->parseSong(songId, &entry, loadFlags);
  entry.close();
  return true;
}


void SdCard::scanUserAndSongFiles() {
  if (_scanFilenamesDone) return;  /* already done */
  _songsBitArr = 0;          /* availability of song01 -> song60 is coded in bit 0 -> 59 */
  _usersBitArr = 0;          /* availability of user01 -> user05 is coded in bit 0 -> 4  */
  File root = SD.open("/");
  root.rewindDirectory();
  File entry = root.openNextFile();
  int id;
  while (entry) {
    if (!entry.isDirectory()) {
      char* name = entry.name();
      if (strncmp(name, "USER", 4) == 0) {
        id = _getNumberFromFileName(name, 4, 2); /* example filename: 'USER01~1.TXT' */
        if (id >= 1 && id <= MAX_USERS) {
          _usersBitArr |= (1 << (id-1));         /* set the right bit that represents the user */
        }
      }
      else if (strncmp(name, "SONG", 4) == 0) {
        id = _getNumberFromFileName(name, 4, 2); /* example filename: 'SONG01~1.TXT' */
        if (id >=1 && id <= MAX_SONGS) {         /* id is between 1 and 60 */
          _songsBitArr |= (1ULL << (id-1));      /* set the right bit that represents the user (type: uint64_t) */
        }
      }
    }
    entry.close();
    entry = root.openNextFile();
  }
  root.close();
  _scanFilenamesDone = true;
}


void SdCard::resetUserAndSongFilesScan() {
  _scanFilenamesDone = false;
}


bool SdCard::isSongAvailable(int idSong) {
  return (_songsBitArr & (1ULL << (idSong-1)));
}

bool SdCard::isUserAvailable(int idUser) {
  return (_usersBitArr & (1 << (idUser-1)));  
}


int SdCard::_getNumberFromFileName(char* filename, int iStart, int iLen) {
  int tot = 0;
  int unit = 1;             /* first the 'ones', then the 'tens', etc. */
  while (--iLen >= 0) {
    char c = filename[iStart+iLen];
    if (!isdigit(c)) return 0;
    tot += ((c - '0') * unit);
    unit *= 10;
  }
  return tot;
}
