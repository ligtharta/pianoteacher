/*
*
*
*/

#ifndef FootPedal_h
#define FootPedal_h

#include <Arduino.h>
#include "HardwDefs.h"

#define PEDAL_CNT 3
/*
*
*
*/
class FootPedal {
  public:
    /**
    * Constructor.
    */
    FootPedal();
    uint32_t readPedals(uint32_t now = 0);
    void clear();
    /* TRUE if pedal is currently DOWN */
    inline bool isLeftDown() { return _isPedalDown(false, _left_Pedal_Mask); }
    inline bool isMiddleDown() { return _isPedalDown(false, _middle_Pedal_Mask); }
    inline bool isRightDown() { return _isPedalDown(false, _right_Pedal_Mask); }
    inline bool isAnyDown() { return _isPedalDown(false, _all_Pedals_Mask); }
    /* TRUE if pedal has JUST pushed DOWN (UP->DOWN) */
    inline bool isLeftPressed() { return _isPedalDown(true, _left_Pedal_Mask); }
    inline bool isMiddlePressed() { return _isPedalDown(true, _middle_Pedal_Mask); }
    inline bool isRightPressed() { return _isPedalDown(true, _right_Pedal_Mask); }
    inline bool isAnyPressed() { return _isPedalDown(true, _all_Pedals_Mask); }
    /* TRUE if pedal has JUST been RELEASED (DOWN->UP) */
    inline bool isLeftReleased() { return _isPedalUp(true, _left_Pedal_Mask); }
    inline bool isMiddleReleased() { return _isPedalUp(true, _middle_Pedal_Mask); }
    inline bool isRightReleased() { return _isPedalUp(true, _right_Pedal_Mask); }
    /* get amount of quick releases (within 400 ms) */
    inline int leftReleases(uint32_t now) { return _getReleases(0, now); }
    inline int middleReleases(uint32_t now) { return _getReleases(1, now); }
    inline int rightReleases(uint32_t now) { return _getReleases(2, now); }
    /* get amount of quick presses (within 400 ms) */
    inline int leftPresses(uint32_t now) { return _getPresses(0, now); }
    inline int middlePresses(uint32_t now) { return _getPresses(1, now); }
    inline int rightPresses(uint32_t now) { return _getPresses(2, now); }
    /* get time (ms) that pedal is down */
    inline uint32_t getLeftDownTime(uint32_t now) { return _getDownTime(0, now); }
    inline uint32_t getMiddletDownTime(uint32_t now) { return _getDownTime(1, now); }
    inline uint32_t getRightDownTime(uint32_t now) { return _getDownTime(2, now); }
  protected:

  private:
    uint32_t _left_Pedal_Mask, _middle_Pedal_Mask, _right_Pedal_Mask; /* bit-mask per pedal  */
    uint32_t _all_Pedals_Mask;                                        /* bit-mask ALL pedals */
    volatile uint32_t *_readPedalReg;       /* all pedals on the same Arduino PORT. */
    uint32_t _newReading, _formerReading;
    bool _isPedalDown(bool onlyNewPress, uint32_t mask);
    bool _isPedalUp(bool onlyNewPress, uint32_t mask);
    void _recordRelease(int pedalIdx, uint32_t now);
    int _getReleases(int pedalIdx, uint32_t now);
    void _recordPress(int pedalIdx, uint32_t now);
    int _getPresses(int pedalIdx, uint32_t now);
    uint32_t _getDownTime(int pedalIdx, uint32_t now);
    
    uint32_t _releaseTimes[4 * PEDAL_CNT]; /* record time of last 3 releases (+ one extra '0' for padding) of each pedal */
    int _releaseIndexes[PEDAL_CNT];        /* index (0,1,2,3) for each padel release recording. */
    uint32_t _pressTimes[4 * PEDAL_CNT];   /* record time of last 3 presses (+ one extra '0' for padding) of each pedal */
    int _pressIndexes[PEDAL_CNT];          /* index (0,1,2,3) for each padel press recording. */
};



#endif // FootPedal_h
