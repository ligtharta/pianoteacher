#ifndef SdCard_h
#define SdCard_h

#include <Arduino.h>
#include <SD.h>
#include "HardwDefs.h"
#include "Entities.h"

#define MAX_SONGS 60
#define MAX_USERS 5

/******************************************************************************************************************************
*
* CLASS  :  SdCard
* 
* Handles SD-Card related functionality. 
*
*******************************************************************************************************************************/
class SdCard {
  public:
    /**
    * Constructor.
    */
    SdCard();
    bool init_SD();
    bool init_SD_success;
    /* Settings file */
    bool loadSettings();
    bool saveSettingsIfDirty();
    /* User file (1 per user) */
    bool loadUser(int userId);
    bool saveUserIfDirty();
    /* Song file (load only) */
    bool loadSong(Song* song, int songId, byte loadFlags);
    /* Scan files to see which song files (60 possible) and user files (5 possible) are available */
    void scanUserAndSongFiles();
    void resetUserAndSongFilesScan();
    bool isSongAvailable(int idSong);
    bool isUserAvailable(int idUser);
    
  protected:

  private:
    static char _filename[20];  

    bool _scanFilenamesDone; /* if true, then bit-arrays below are valid */
    uint64_t _songsBitArr;   /* availability of song01 -> song60 is coded in bit 0 -> 59 */
    int _usersBitArr;        /* availability of user01 -> user05 is coded in bit 0 -> 4  */

    int _getNumberFromFileName(char* filename, int iStart, int iLen);

};






#endif // SdCard_h
