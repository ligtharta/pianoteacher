/*
*
*
*/

#ifndef Gloves_h
#define Gloves_h

#include <Arduino.h>
#include "HardwDefs.h"

#define MAX_FINGER_INDEX 10  /* 0=uknown, 1-5=leftHand, 6-10=rightHand */
/*
*
*
*/
class Gloves {
  public:
    /**
    * Constructor.
    */
    Gloves();
    void reset(bool glovesActive);
    void setFinger(int fingerIdx, bool on);
    void clearAllFingers();
    void updateGloves();
    
  protected:

  private:
    /*
    * index:   finger:    |  index:     finger:          |   index:      finger:
    *     0    UNKNOWN    |      1      left thumb       |       6       right thumb
    *                     |      2      left index       |       7       right index
    *                     |      3      left middle      |       8       right middle
    *                     |      4      left ring        |       9       right ring
    *                     |      5      left little      |      10       right little
    */
    bool _glovesActive;                 /* if false, gloves will stay off/silent */
    byte _fingers[MAX_FINGER_INDEX+1];  /* see above (0=unknown,   1/2/3/4/5=left hand,   6/7/8/9/10=right hand */
    bool _dataDirty;
    void _sendDataToGloves();
};



#endif // Gloves_h
