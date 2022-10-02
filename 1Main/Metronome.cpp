#include "Metronome.h"


#if (METRONOME_HARDWARE_AVAILABLE == true)

const uint32_t _mColors[] PROGMEM =
{ //GGRRBB00
  0x00000000,     // color: off
  0x00800000,     // color: red
  0x20000000,     // color: green
  0x0000C000,     // color: blue
};

#define MCOLOR_OFF   0
#define MCOLOR_RED   1
#define MCOLOR_GREEN 2
#define MCOLOR_BLUE  3

#define MAUDIO_PULSE 128  /* bit-mask: is audio pulse playing? */
#define MLED         64   /* bit-mask: is LED on? */

Metronome::Metronome() {
  _setup_PWM();                                              // PWM (audio pulses) through A3
  
  pinMode(METRONOME_LEDSTRIP_PIN, OUTPUT);
  digitalWrite(METRONOME_LEDSTRIP_PIN, LOW);

  _input_for_asm[0] = 0ul;
  _input_for_asm[1] = 1ul << g_APinDescription[METRONOME_LEDSTRIP_PIN].ulPin;
  _input_for_asm[2] = _input_for_asm[3] = _input_for_asm[1];  // same, because only 1 LED strip
  
  _input_for_asm[4] =  ((uint32_t)(void*)_colorIdx);           // ref. to LED array (4 LEDs)
  _input_for_asm[5] =  ((uint32_t)(void*)_mColors);            // ref. to color array

  byte port = g_APinDescription[METRONOME_LEDSTRIP_PIN].ulPort;
  _input_for_asm[6] = (uint32_t)&(PORT->Group[port].OUTCLR.reg);  // store Clr address
  _input_for_asm[7] = (uint32_t)&(PORT->Group[port].OUTSET.reg);  // store Set address

  for (int i=0 ; i < 4 ; i++) _colorIdx[i]=0; _writeLeds_asm();        // clear all metronome LEDs
  working = METRONOME_OFF;
}


void Metronome::_setup_PWM(){
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1) |          // Divide the 48MHz clock source by divisor 1: 48MHz/1=48MHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Use A3 for PWM:
  // ---------------
  // Enable the port multiplexer for the TCC0 PWM channel 2 (digital pin D6), SAMD21 pin PA20
  PORT->Group[g_APinDescription[METRONOME_PWM_PIN].ulPort].PINCFG[g_APinDescription[METRONOME_PWM_PIN].ulPin].bit.PMUXEN = 1;
  // Connect the TCC0 timer to the port outputs - port pins are paired odd PMUO and even PMUXE
  // F & E specify the timers: TCC0, TCC1 and TCC2
  PORT->Group[g_APinDescription[METRONOME_PWM_PIN].ulPort].PMUX[g_APinDescription[METRONOME_PWM_PIN].ulPin >> 1].bit.PMUXE = PORT_PMUX_PMUXE_E_Val;

  // Feed GCLK4 to TCC0 and TCC1
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TCC0 and TCC1
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TCC0_TCC1;   // Feed GCLK4 to TCC0 and TCC1
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Normal (single slope) PWM operation: timer countinuouslys count up to PER register value and then is reset to 0
  REG_TCC0_WAVE |= TCC_WAVE_WAVEGEN_NPWM;         // Setup single slope PWM on TCC0
  while (TCC0->SYNCBUSY.bit.WAVE);                // Wait for synchronization
  
  // Each timer counts up to a maximum or TOP value set by the PER (period) register,
  // this determines the frequency of the PWM operation:
  // 1919 = 25kHz
  REG_TCC0_PER = 1919;      // Set the frequency of the PWM on TCC0 to 25kHz
  while(TCC0->SYNCBUSY.bit.PER);

  // The CCBx register value determines the duty cycle
  REG_TCC0_CCB0 = 959;       // TCC0 CCB2 - duty cycle
  while(TCC0->SYNCBUSY.bit.CCB0);

  // Divide the 48MHz signal by 1 giving 48MHz (20.8ns) TCC0 timer tick and enable the outputs
  REG_TCC0_CTRLA |= TCC_CTRLA_PRESCALER_DIV16;
  while (TCC0->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
}



void Metronome::startNewMeasure(byte beats, uint32_t millisPerBeat, uint32_t now) {
  if (working == METRONOME_OFF) return;
  if (working == METRONOME_MEASURE_ONLY) beats = 1; 
  for (int i=0; i<beats; i++) {
    _beatsToDo.add(i, now + 2);
    now += millisPerBeat;
  }
}

void Metronome::handleBeats(uint32_t now) {
  byte* b; /* bit 0-5 is LED color index,  bit 6+7 have special meaning, see MAUDIO_PULSE, MLED */
  uint32_t actTime;
  
  b = _beatsToDo.peekFirst(actTime);          /* actTime is set (by ref) */ 
  if (b == NULL) return;                      /* nothing to do, so quit... */
  if (now >= actTime) {                       /* time to process this beat */
    if ((*b & (MAUDIO_PULSE | MLED)) == 0) {  /* beat not yet started (i.e. LED & audio pulse both OFF) */
      _setLEDs(*b, true);                     /* turn metronome LED on! */
      _setPWM(*b, true);                      /* start audio pulse using PWM */
      *b += (MAUDIO_PULSE | MLED);
    }
    else if ((*b & (MAUDIO_PULSE | MLED)) 
                == (MAUDIO_PULSE | MLED)) {   /* next step: turn audio pulse off */
      actTime += 2;                           /* time (ms) that audio pulse is on */
      if (now >= actTime) {                   /* time to turn OFF audio pulse? */
        _setPWM(*b, false);                   /* stop audio pulse OFF */
        *b -= MAUDIO_PULSE;
      }
    }
    else if ((*b & (MAUDIO_PULSE | MLED)) 
                 == MLED ) {                  /* next step: turn LED off */
      actTime += 40;                          /* time (ms) that LED pulse is on */
      if (now >= actTime) {                   /* time to turn off LED? */
        _setLEDs(*b - MAUDIO_PULSE, false);   /* turn metronome LED off */
        _beatsToDo.removeFirst();             /* beat complete: remove from delay-manager */
      }
    }
  }
}


void Metronome::reset() {
  _beatsToDo.reset();
  _setPWM(0, false);
  _setLEDs(0, false);
}



void Metronome::_setLEDs(byte ledIdx, bool turnOn) {
  if (ledIdx > 3) ledIdx = 3; /* If there are more than 4 beats in a measure, use the fourth LED for beat 4,5,6, etc. */
  for (int j=0; j<4; j++)
  { /* first LED indicates new Measure and has another color. Only 'ledIdx' LED on, other LEDs off */
    _colorIdx[j] = ((j==ledIdx && turnOn) ? (ledIdx==0 ? MCOLOR_BLUE : MCOLOR_GREEN) : MCOLOR_OFF);
  }
  _writeLeds_asm();
}

void Metronome::_setPWM(byte ledIdx, bool turnOn) {
  if (turnOn) {
    REG_TCC0_PER = (ledIdx == 0 ? 2150 : 1919);      // Set the frequency of the PWM on TCC0
    while(TCC0->SYNCBUSY.bit.PER);
    
    REG_TCC0_CTRLA |= TCC_CTRLA_ENABLE;             // Enable the TCC0 output
    while (TCC0->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
  } 
  else {
    if (REG_TCC0_CTRLA & TCC_CTRLA_ENABLE) {
      REG_TCC0_CTRLA ^= TCC_CTRLA_ENABLE;             // Disable the TCC0 output
    }
    while (TCC0->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
  }
}


void Metronome::_writeLeds_asm()
{ 
    __disable_irq();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.

      asm volatile(
    "LDR  R5, [%[data], #16]   \n"          // R5 = pointer to indexed colors (5 for each piano key), see _input_for_asm[4]
    
    "LDR  R1, [%[data], #24]   \n"
    "MOV  R10, R1              \n"          // R10 = I/O address to drive lines LOW (addrClr), see _input_for_asm[6]

    "LDR  R1, [%[data], #28]   \n"
    "MOV  R6, R1               \n"          // R6 = I/O address to drive lines HIGH (addrSet), see _input_for_asm[7]

    "LDR  R1, [%[data], #20]   \n"
    "MOV  R8, R1               \n"          // R8 = pointer to colors-list, see _input_for_asm[5]
    
    
"mainLoop%=:\n"
    "LDRB R1, [R5, #0]         \n"          // R1=R2 : this way 'writeLED' subroutine is the same as for piano LED panel
    "MOV  R2, R1               \n"
    "BL   writeLED%=           \n"          // write LED 1
    "LDRB R1, [R5, #1]         \n"
    "MOV  R2, R1               \n"
    "BL   writeLED%=           \n"          // write LED 2   
    "LDRB R1, [R5, #2]         \n"
    "MOV  R2, R1               \n"
    "BL   writeLED%=           \n"          // write LED 3
    "LDRB R1, [R5, #3]         \n"
    "MOV  R2, R1               \n"
    "BL   writeLED%=           \n"          // write LED 4
    
    "B   finished%=            \n"   
    
"writeLED%=:\n"                        // looks up two RGB values and writes each of the 24 bits of both to two seperate WS2812B-lines
    "LSL  R1, R1, #2    \n"            // (line 1) R1 = offset in color table (0, 4, 8, etc)
    "ADD  R1, R1, R8    \n"            // (line 1) R1 = pointer to the right color (4-byte value)
    "LDR  R1, [R1]      \n"            // (line 1) R1 is now 24-bit color (coded as 0xGGRRBB00)
    "LSL  R2, R2, #2    \n"            // (line 2) R2 = offset in color table (0, 4, 8, etc)
    "ADD  R2, R2, R8    \n"            // (line 2) R2 = pointer to the right color (4-byte value)
    "LDR  R2, [R2]      \n"            // (line 2) R2 is now 24-bit color (coded as 0xGGRRBB00)
    
    "MOV  R0, #24       \n"            // set counter to 24 (for each bit of RGB-color)  
                                       // (ABOVE LINE DEVIATES FROM THE CORRESPONDING SUBROUTINE TO DRIVE THE PIANO LED PANEL)
"writeBit%=:\n"  
    "LDR  R3, [%[data], #12]   \n"     // R3 is pinMask for both lines
    "STR  R3, [R6]             \n"     // drive both lines HIGH

    "nop\n"                            /* almost no NOPs needed, because of code below */ 
   
    ".syntax unified         \n"
    "LSLS R1, R1, #1         \n"       // shift bit out of R1 (line 1) into carry flag
    ".syntax divided         \n"
    "SBC  R4, R4,R4          \n"       // R4=0xFFFFFFFF (if carry==0) or R4=0 (if carry==1)
    "MOV  R3, #4             \n"
    "AND  R3, R3, R4         \n"       // R3 is 0 (shifted bit was 1) or 4 (shifted bit was 0)
    "MOV  R9, R3             \n"
    
    ".syntax unified         \n"
    "LSLS R2, R2, #1         \n"       // shift bit out of R2 (line 2) into carry flag
    ".syntax divided         \n"
    "SBC  R4, R4, R4         \n"       // R4=0xFFFFFFFF (if carry==0) or R4=0 (if carry==1)
    "MOV  R3, #8             \n"
    "AND  R3, R3, R4         \n"       // R3 is 0 (shifted bit was 1) or 8 (shifted bit was 0)
    "ADD  R3, R3, R9         \n"       // R3 is offset 0, 4, 8 or 12 (to get the right pinMask)

    "LDR  R3, [%[data], R3]  \n"       // R3 = pinMask, for the lines who send 0-bit (must be driven LOW after 0.4us)

    "MOV  R4, R10            \n"       // R4 = addrClr (I/O address to drive lines low)
    "STR  R3, [R4]           \n"       // drive lines (only lines who send 0-bit) LOW
    
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"     /*  NOPs here to fill up approx. 0.8us */ 
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
    "nop\n" "nop\n"
   
    "LDR  R3, [%[data], #12] \n"       // R3 is pinMask for both lines
    "STR  R3, [R4]           \n"       // drive all lines low (also lines who send 1-bit, this is after approx. 0.8us)

    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"     /*  NOPs here to fill up some time */ 
    "nop\n" "nop\n"
     
    "MOV  R3, #31                   \n"     // mask for counter, because R3 has double counting function
    "SUB R0, R0, #1                 \n"     // decrease bit-counter by 1
    ".syntax unified                \n"
    "ANDS R3, R0, R3                \n"     // are lower 5-bits of counter all zero?
    ".syntax divided\n"

    "BNE   writeBit%=               \n"     // write another bit while bit-counter >= 1
    "MOV   PC, LR                   \n"     // return to caller

"finished%=:\n"    
    :
    : [data] "r" (_input_for_asm)
    : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "cc"  /* clobbers */
    );

    __enable_irq();         // Re-enable interrupts now that we are done.
    //delayMicroseconds(50);  // Hold the line low for 50 microseconds to send the reset signal. 
}


// 0 coderen: hoog 0.4us, laag 0.85us
// 1 coderen: hoog 0.8us, laag 0.45us
// RESET coderen: laag 50us of meer


#else      // Metronome hardware not available: empty implementation...


Metronome::Metronome() {}
void Metronome::_setup_PWM(){}
void Metronome::startNewMeasure(byte beats, uint32_t millisPerBeat, uint32_t now) {}
void Metronome::handleBeats(uint32_t now) {}
void Metronome::reset() {}
void Metronome::_setLEDs(byte ledIdx, bool turnOn) {}
void Metronome::_setPWM(byte ledIdx, bool turnOn) {}
void Metronome::_writeLeds_asm() {}


#endif // (METRONOME_HARDWARE_AVAILABLE == true)
