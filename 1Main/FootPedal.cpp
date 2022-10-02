#include "FootPedal.h"


FootPedal::FootPedal() {
  
  pinMode(FOOTPEDAL_LEFT_PIN,   INPUT_PULLUP);
  pinMode(FOOTPEDAL_MIDDLE_PIN, INPUT_PULLUP);
  pinMode(FOOTPEDAL_RIGHT_PIN,  INPUT_PULLUP);

  _left_Pedal_Mask    = 1ul << g_APinDescription[FOOTPEDAL_LEFT_PIN].ulPin;
  _middle_Pedal_Mask  = 1ul << g_APinDescription[FOOTPEDAL_MIDDLE_PIN].ulPin;
  _right_Pedal_Mask   = 1ul << g_APinDescription[FOOTPEDAL_RIGHT_PIN].ulPin;
  _all_Pedals_Mask    = _left_Pedal_Mask | _middle_Pedal_Mask | _right_Pedal_Mask;
  /* all pedals on the same Arduino PORT. */
  _readPedalReg = &PORT->Group[g_APinDescription[FOOTPEDAL_LEFT_PIN].ulPort].IN.reg;
  clear();
}

void FootPedal::clear() {
  _newReading = _formerReading = _all_Pedals_Mask;                  /* all pedals OFF (1 = OFF, 0 = ON) */
  for (int i=0; i < PEDAL_CNT; i++) {     /* remove recordings of pedal presses/releases */
    _pressTimes[i * 4] = 0;
    _pressIndexes[i] = 1;
    _releaseTimes[i * 4] = 0;
    _releaseIndexes[i] = 1;
  }
}

uint32_t FootPedal::readPedals(uint32_t now) {
  _formerReading = _newReading;
  _newReading = (*_readPedalReg) & _all_Pedals_Mask;    /* read digital state of pedals */

  uint32_t bReleases = (_formerReading ^ _all_Pedals_Mask) & _newReading;   /* determine just-released status of each pedal */
  if (bReleases & _left_Pedal_Mask) _recordRelease(0, now);
  if (bReleases & _middle_Pedal_Mask) _recordRelease(1, now);
  if (bReleases & _right_Pedal_Mask) _recordRelease(2, now);

  uint32_t bPresses = _formerReading & (_newReading ^ _all_Pedals_Mask);   /* determine just-pressed status of each pedal */
  if (bPresses & _left_Pedal_Mask) _recordPress(0, now);
  if (bPresses & _middle_Pedal_Mask) _recordPress(1, now);
  if (bPresses & _right_Pedal_Mask) _recordPress(2, now);

  return _newReading;
}


bool FootPedal::_isPedalDown(bool onlyNewPress, uint32_t mask) {
  if ((_newReading & mask) == mask) return false; /* if bit is 1, then pedal is UP */
  if (!onlyNewPress) return true;
  return ((_formerReading & mask) == mask);
}

bool FootPedal::_isPedalUp(bool onlyNewRelease, uint32_t mask) {
  if ((_newReading & mask) == 0) return false; /* if bit is 0, then pedal is DOWN */
  if (!onlyNewRelease) return true;
  return ((_formerReading & mask) == 0);
}

void FootPedal::_recordRelease(int pedalIdx, uint32_t now) {
  int idx = _releaseIndexes[pedalIdx];                  /* idx always 0,1,2 or 3 */
  _releaseTimes[pedalIdx * 4 + idx] = now;
  idx = (idx+1) & 3;
  _releaseTimes[pedalIdx * 4 + idx] = 0;
  _releaseIndexes[pedalIdx] = idx;
}

int FootPedal::_getReleases(int pedalIdx, uint32_t now) {
  int idx = _releaseIndexes[pedalIdx];                 /* idx always 0,1,2 or 3 */
  uint32_t millis;
  int cnt = 0;
  do {
    idx = (idx-1) & 3;
    millis = _releaseTimes[pedalIdx * 4 + idx];
    if (now - millis <= 400) { cnt++; now = millis; }         /* 400 ms */
  } while (millis != 0);
  return cnt;         /* cnt is now number of quick subsequential pedal releases */
}


void FootPedal::_recordPress(int pedalIdx, uint32_t now) {
  int idx = _pressIndexes[pedalIdx];                  /* idx always 0,1,2 or 3 */
  _pressTimes[pedalIdx * 4 + idx] = now;
  idx = (idx+1) & 3;
  _pressTimes[pedalIdx * 4 + idx] = 0;
  _pressIndexes[pedalIdx] = idx;
}

int FootPedal::_getPresses(int pedalIdx, uint32_t now) {
  int idx = _pressIndexes[pedalIdx];                 /* idx always 0,1,2 or 3 */
  uint32_t millis;
  int cnt = 0;
  do {
    idx = (idx-1) & 3;
    millis = _pressTimes[pedalIdx * 4 + idx];
    if (now - millis <= 400) { cnt++; now = millis; }         /* 400 ms */
  } while (millis != 0);
  return cnt;         /* cnt is now number of quick subsequential pedal presses */
}

uint32_t FootPedal::_getDownTime(int pedalIdx, uint32_t now) {
  int idx = _pressIndexes[pedalIdx];                 /* idx always 0,1,2 or 3 */
  idx = (idx-1) & 3;
  uint32_t millis = _pressTimes[pedalIdx * 4 + idx];
  if (millis == 0) return 0;
  return (now - millis);
}
