#ifndef Metronome_h
#define Metronome_h

#include <Arduino.h>
#include "HardwDefs.h"
#include "Templates.h"

#define METRONOME_OFF          0
#define METRONOME_MEASURE_ONLY 1
#define METRONOME_ALL_BEATS    2

/*
*
*
*/
class Metronome {
  public:
    Metronome();
    void startNewMeasure(byte beats, uint32_t millisPerBeat, uint32_t now);
    void handleBeats(uint32_t now);
    void reset();
    byte working;
  protected:

  private:
    void _setup_PWM();
    void _setLEDs(byte ledIdx, bool turnOn);
    void _setPWM(byte ledIdx, bool turnOn);
  
    unsigned char _colorIdx[4];              /* colors (indexes) of each LED on metronome */
    void _writeLeds_asm();
    uint32_t _input_for_asm[8];              /* 8-byte structure to pass data to asm routine */

    DelayManager<byte, 32> _beatsToDo;       /* metronome beats to process in the future. */
};



#endif // Metronome_h
