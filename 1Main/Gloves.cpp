#include "Gloves.h"

#if (GLOVES_HARDWARE_AVAILABLE == true)


Gloves::Gloves() { 
  pinMode(GLOVES_DATA_SERIAL_PIN,   OUTPUT);
  pinMode(GLOVES_STORE_REG_CLK_PIN, OUTPUT);
  pinMode(GLOVES_SHIFT_REG_CLK_PIN, OUTPUT);
  pinMode(GLOVES_MASTER_RESET_PIN,  OUTPUT);

  digitalWrite(GLOVES_MASTER_RESET_PIN, HIGH);
  reset(false);
}


void Gloves::reset(bool glovesActive) {
  clearAllFingers();
  _sendDataToGloves();
  _glovesActive = glovesActive;
  _dataDirty = false;
}


void Gloves::clearAllFingers() {
  for(int i = 0; i<=MAX_FINGER_INDEX; i++){
     _fingers[i] = LOW;
  }    
}

void Gloves::setFinger(int fingerIdx, bool on) {
  _fingers[fingerIdx] = on ? HIGH : LOW;
  _dataDirty = true;
}

void Gloves::updateGloves() {
  if (!_glovesActive) return; /* gloves not active */
  if (!_dataDirty) return;    /* no update needed */
  _sendDataToGloves();
  _dataDirty = false;
}



void Gloves::_sendDataToGloves() {
  digitalWrite(GLOVES_STORE_REG_CLK_PIN, LOW);
  /* Send 16 bits to 75HC595 SHIFT registers...*/
  /* Only 10 bits contain data, so there are 6 dummy bits to do */
//  doOutputDummy(2);
  /* First 2 dummy bits.. */
  for (int i=0; i<2; i++) {
    digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, LOW);
    digitalWrite(GLOVES_DATA_SERIAL_PIN, LOW);
    delayMicroseconds(1);
    digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, HIGH);    
  }
  /* Then 5 bits for left hand (thumb -> little finger) */  
  for(int i = 1; i <= 5; i++){   /* left hand */
    digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, LOW);
    int val = _fingers[i];
    digitalWrite(GLOVES_DATA_SERIAL_PIN, val);
    digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, HIGH);
  }
  //doOutputDummy(3);
  /* Then 3 dummy bits */  
  for (int i=0; i<3; i++) {
    digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, LOW);
    digitalWrite(GLOVES_DATA_SERIAL_PIN, LOW);
    delayMicroseconds(1);
    digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, HIGH);    
  }
  /* Then 5 bits for right hand (little finger -> thumb) */  
  for(int i = 10; i >=  6; i--){  /* right hand */
    digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, LOW);
    int val = _fingers[i];
    digitalWrite(GLOVES_DATA_SERIAL_PIN, val);
    digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, HIGH);
  } 
//  doOutputDummy(1);
  /* And finally 1 dummy bit */
  digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, LOW);
  digitalWrite(GLOVES_DATA_SERIAL_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(GLOVES_SHIFT_REG_CLK_PIN, HIGH);    
  
  digitalWrite(GLOVES_STORE_REG_CLK_PIN, HIGH);
}


#else  // Gloves hardware not available: empty implementation...


Gloves::Gloves() {}
void Gloves::reset(bool glovesActive) {}
void Gloves::clearAllFingers() {}
void Gloves::setFinger(int fingerIdx, bool on) {}
void Gloves::updateGloves() {}
void Gloves::_sendDataToGloves() {}


#endif // (GLOVES_HARDWARE_AVAILABLE == true)
