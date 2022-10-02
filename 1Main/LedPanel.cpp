
#include "LedPanel.h"


LedPanel::LedPanel() {

  // ledstrips:
  pinMode(LEDPANEL_STRIP1_PIN, OUTPUT);        /* LEDstrip line 1 */
  digitalWrite(LEDPANEL_STRIP1_PIN, LOW);
  pinMode(LEDPANEL_STRIP2_PIN, OUTPUT);        /* LEDstrip line 2 */
  digitalWrite(LEDPANEL_STRIP2_PIN, LOW);
  
  _input_for_asm[0] = 0ul;                                                 /* bitmask: none            */
  _input_for_asm[1] = 1ul << g_APinDescription[LEDPANEL_STRIP1_PIN].ulPin; /* bitmask: only line 1     */
  _input_for_asm[2] = 1ul << g_APinDescription[LEDPANEL_STRIP2_PIN].ulPin; /* bitmask: only line 2     */
  _input_for_asm[3] = _input_for_asm[1] | _input_for_asm[2];               /* bitmask: line 1 + line 2 */
  
  _input_for_asm[4] =  ((uint32_t)(void*)_colorIdx);    /* ref. to LED-array (61 x 5, or 73 x 5) */
  setGlobalBrightness(0);   /* this sets _input_for_asm[5]  (pointer to array of colors used)  */

  byte port = g_APinDescription[LEDPANEL_STRIP1_PIN].ulPort;
  _input_for_asm[6] = (uint32_t)&(PORT->Group[port].OUTCLR.reg); /* store 'Clear' address */
  _input_for_asm[7] = (uint32_t)&(PORT->Group[port].OUTSET.reg); /* store 'Set' address   */

  clear(); writeLeds_asm();          /* turn all LEDs OFF  */

  // buttons:
  pinMode(LEDPANEL_BUTTON1_PIN, INPUT_PULLUP);   /* push button on LED panel*/
  pinMode(LEDPANEL_BUTTON2_PIN, INPUT_PULLUP);
  pinMode(LEDPANEL_BUTTON3_PIN, INPUT_PULLUP);
  pinMode(LEDPANEL_BUTTON4_PIN, INPUT_PULLUP);

  _button_Mask[0]  = 1ul << g_APinDescription[LEDPANEL_BUTTON1_PIN].ulPin;
  _button_Mask[1]  = 1ul << g_APinDescription[LEDPANEL_BUTTON2_PIN].ulPin;
  _button_Mask[2]  = 1ul << g_APinDescription[LEDPANEL_BUTTON3_PIN].ulPin;
  _button_Mask[3]  = 1ul << g_APinDescription[LEDPANEL_BUTTON4_PIN].ulPin;
  _button_All_Mask = _button_Mask[0] | _button_Mask[1] | _button_Mask[2] | _button_Mask[3];

  _readBtnRegister = &PORT->Group[g_APinDescription[LEDPANEL_BUTTON1_PIN].ulPort].IN.reg; /* all buttons on same PORT!! */
  _newReading = _formerReading = _button_All_Mask;  /* all buttons OFF (1 = OFF, 0 = ON)  */
}

void LedPanel::setGlobalBrightness(byte brightnessId)
{
  if (brightnessId > 3) brightnessId = 3; /* between 0 and 3 (higher means more brightness) */
  uint32_t pointerOffset = (brightnessId*256);
  _input_for_asm[5] =  ((uint32_t)(void*)_colors) + pointerOffset;      /* ref. to colors array       */
}

/******************************************************************************************************************************
*
* CODE FOR CONTROLLING LED STRIPS BELOW
* 
*******************************************************************************************************************************/

void LedPanel::clear() {
  for (int i=0; i < PANEL_LEDS; i++) { _colorIdx[i] = 0; }
}

int LedPanel::writeChar(char c, int xStart, byte clr) {
  return writeChar(c, xStart, clr, false);
}

int LedPanel::writeChar(char c, int xStart, byte clr, bool keepBG) {
  static char txt[2];
  txt[0] = c;
  txt[1] = '\0';
  return writeText(txt, xStart, clr, keepBG);
}

int LedPanel::writeText(const char* txt, int xStart, byte clr) {
  return writeText(txt, xStart, clr, false);
}

int LedPanel::writeText(const char* txt, int xStart, byte clr, bool keepBG)
{
  int tIdx=0;
  int x = xStart;
  xStart = max(0, xStart);
  if (x >= PANEL_COLS) { return 0; }   /* LED panel has 'PANEL_COLS' columns */
  while (txt[tIdx] != 0)
  {
    int idx = (txt[tIdx]-32);
    idx = _letter_index[idx] * BYTES_PER_LETTER;  /* each character's pixel-pattern is coded in 8 bytes */
    
    while (_letters[idx] != 0) /* 0 means all columns of current char written */
    {
      byte b = _letters[idx];
      for (int y=0; y < PANEL_ROWS; y++) /* write a column, 5 pixels (top -> down) */
      {
        if (x >=0 ) { 
          if (b % 2 == 1) {            
            _colorIdx[x*PANEL_ROWS + y] = clr; 
          }
          else if (!keepBG) { /* if keep background, than do not set black (=LED off) pixels */
            _colorIdx[x*PANEL_ROWS + y] = COLOR_IDX_OFF; 
          }
        }
        b = b >> 1; /* next higher bit in next iteration */
      }
      x++;
      if (x >= PANEL_COLS) { return PANEL_COLS - xStart; }
      idx++;
    }
    tIdx++;
    if (txt[tIdx] != 0) {
      x += 2;
      if (x >= PANEL_COLS) { return PANEL_COLS - xStart; }
    }
  }
  return x - xStart;
}

int LedPanel::getTextWidth(const char* txt) {
  int tIdx = 0;
  int width = 0;
  while (txt[tIdx] != 0)
  {
    int idx = (txt[tIdx] - ' ');
    idx = _letter_index[idx] * BYTES_PER_LETTER;  /* each character's pixel-pattern is coded in 8 bytes */    
    while (_letters[idx++] != 0)  /* 0 means all columns of current char written */
      width++;
    tIdx++;
    if (txt[tIdx] != 0) width += 2;
  }
  return width;  
}

void LedPanel::setPixel(int x, int y, byte clr) {
  _colorIdx[x*PANEL_ROWS + y] = clr;
}

byte LedPanel::getPixel(int x, int y) {
  return _colorIdx[x*PANEL_ROWS + y];
}


void LedPanel::fillRect(int x1, int y1, int x2, int y2, byte clr) {
  int v;
  if (x1 > x2) { v = x1; x1 = x2; x2 = v; }
  if (y1 > y2) { v = y1; y1 = y2; y2 = v; }
  for (int x = x1; x <= x2; x++) {
    for (int y = y1; y <= y2; y++) {
      _colorIdx[x*PANEL_ROWS + y] = clr;
    }
  }
}

/* 0 = normal color, 1 = slightly dimmed, ..., 7 = max dimmed */
void LedPanel::setRowDimming(int row, byte dimmingIdx) {
  for (int x = 0; x < PANEL_COLS; x++) {
    byte clr = _colorIdx[x*PANEL_ROWS + row];
    _colorIdx[x*PANEL_ROWS + row] = (clr & 0b11111000) + dimmingIdx;
  }
}


void LedPanel::SetColorInArea(int x1, int y1, int x2, int y2, byte clr) {
  int v, idx;
  if (x1 > x2) { v = x1; x1 = x2; x2 = v; }
  if (y1 > y2) { v = y1; y1 = y2; y2 = v; }
  for (int x = x1; x <= x2; x++) {
    for (int y = y1; y <= y2; y++) {
      idx = x*PANEL_ROWS + y;
      if (_colorIdx[idx] != COLOR_IDX_OFF) _colorIdx[idx] = clr;
    }
  }  
}


bool LedPanel::IsColumnEmpty(int x) {
  int idx = x*PANEL_ROWS;
  for (int y=0; y < 5; y++)
  {
    if (_colorIdx[idx++] != COLOR_IDX_OFF) return false;
  }
  return true;
}





byte LedPanel::getUserColorIdx(int idUser) {
  return _colorIdx_users[idUser - 1];
}

byte LedPanel::getFingerColorIdx(int finger) {
  return _colorIdx_fingers[finger];
}



/**************************************************************************************************************************
* IMPORTANT INFO ABOUT ASSEMBLER CODE BELOW (implemented within 'writeLeds_asm()' function).
***************************************************************************************************************************
* See here how the two WS2812B LED strips are connected. The 2 LED strips make zig-zag paths and are intertwined:
* LED strip 1: Piano key 61, 59, 57, 55, 53, .. ,5, 3, 1     in a down/up/down/up/etc. manner [*]
* LED strip 2: Piano key 60, 58, 56, 54, 52, .. ,4, 2        in a down/up/down/up/etc. manner [*]
* How to read the 'drawing' below:
*  The letters O and X represent the LEDs of LED strip 1 and 2 respectively (5 LEDs for each piano key).
*  The symbols |-+ describe how all is connected (where + means: the lines cross but are NOT connected)
*  The symbol < describes the flow/direction of data (the flow is: down-left-up-left-down-left-up-left-down, etc)
*  O! and X! are the very last LEDs of LED strip 1 and 2 respectively.
* (only the DATA line of each LED strip is shown; all LEDs are also connected to VCC and GND, of course)
***************************************************************************************************************************
*
*   1   2   3  (4 ... 45)    46  47  48  49  50  51  52  53  54  55  56  57  58  59  60  61   (piano key number, 1=lowest pitch) [*]
*
*   |---<---|          etc ---<---|       |---<---|       |---<---|       |---<---|
*   |       |                     |       |       |       |       |       |       |   |-----<-- LED-strip 2 data line
*   |       |      etc ---<---|   |   |---+---|   |   |---+---|   |   |---+---|   |   |
*   |       |                 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |-<-- LED-strip 1 data line
*   |       |                 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
*   O   X!  O                 X   O   X   O   X   O   X   O   X   O   X   O   X   O   X   O  
*   |   |   |                 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
*   O   X   O                 X   O   X   O   X   O   X   O   X   O   X   O   X   O   X   O
*   |   |   |                 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
*   O   X   O                 X   O   X   O   X   O   X   O   X   O   X   O   X   O   X   O
*   |   |   |                 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
*   O   X   O                 X   O   X   O   X   O   X   O   X   O   X   O   X   O   X   O
*   |   |   |                 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
*   O!  X   O                 X   O   X   O   X   O   X   O   X   O   X   O   X   O   X   O
*       |   |                 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
*       |   |---<---          |   |---+---|   |   |---+---|   |   |---+---|   |   |---+---|
*       |                     |       |       |       |       |       |       |       |
*       |---<---              |---<---|       |---<---|       |---<---|       |---<---|
*
***************************************************************************************************************************
* [*] Piano key numbers for 61-key instrument. A 73-key instrument is also supported (12 more piano keys).
***************************************************************************************************************************
* Some design decisions explained:
* Why do LED strips make a zig-zag path? Because it's easier to connect/assemble the physical LED-panel that way.
* Why 2 LED-strips instead of one? Because all LEDs can be set in half the time (3 ms instead of 6 ms, for 305 LEDs).
* Why are the LED strips intertwined? The limited ARM registers (that point to data structures) are better utilized this way. 
* Why indexed colors? Because the whole LED panel needs much less RAM this way. There's only 32kB for everything including song data.
* Why assembly code? Because all above together is impossible to implement in C (speed, timing, data look-ups, etc).
***************************************************************************************************************************/





/******************************************************************************************************************************
* Writes 305 or 365 indexed colors to two LED-strips (WS2812B)              WS2812B:  to send LOW/0  : high 0.4us, low 0.85us
*                                                                                     to send HIGH/1 : high 0.8us, low 0.45us
*                                                                                     to send RESET  : low for 50us (minimum)
* input: _input_for_asm[] array (8 values of type uint32_t):
*            _input_for_asm[0]: always 0                                              binary: 00000000000000000000000000000000
*            _input_for_asm[1]: bit-mask of data-pin of LED strip 1         binary (example): 00000000000000000000100000000000
*            _input_for_asm[2]: bit-mask of data-pin of LED strip 2         binary (example): 00000000000000000000010000000000
*            _input_for_asm[3]: bit-mask of data-pin of both LED strips     binary (example): 00000000000000000000110000000000
*            _input_for_asm[4]: pointer to _colorIdx : a byte-array with color-index per LED (305 bytes for 61-key piano)
*            _input_for_asm[5]: pointer to _colors : an array with all used colors
*            _input_for_asm[6]: I/O address of port to set the LED strip DATA lines LOW (OUTCLR)
*            _input_for_asm[7]: I/O address of port to set the LED strip DATA lines HIGH (OUTSET)
* (to correct timing to compensate for missing interrupts: 4 milliseconds for INSTRUMENT_73_KEYS, 3 ms for INSTRUMENT_61_KEYS )
*******************************************************************************************************************************/
void LedPanel::writeLeds_asm()
{ 
    __disable_irq();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.

      asm volatile(
#ifdef INSTRUMENT_61_KEYS                   // 15 x 4 keys = 60 keys, one key left to do outside the loop
    "MOV  R0, #15              \n"          // set counter R0 to 15 (each iteration writes 4x5 LEDs), that means 15x20=300 LEDs within iteration.
#endif
#ifdef  INSTRUMENT_73_KEYS                  // 18 x 4 keys = 72 keys, one key left to do outside the loop
    "MOV  R0, #18              \n"          // set counter R0 to 18 (each iteration writes 4x5 LEDs), that means 18x20=360 LEDs within iteration.
#endif
    "LSL  R0, R0, #5           \n"
    "NEG  R0, R0               \n"          // counter R0 is now negative and shifted (R0 = R0 * -32) 
    
    "LDR  R5, [%[data], #16]   \n"          // see _input_for_asm[4]. R5 = pointer to array of indexed colors, 1 byte per LED (5 for each piano key)
    "ADD  R5, R5, #200         \n"
#ifdef INSTRUMENT_61_KEYS
    "ADD  R5, R5, #85          \n"          // (R5+19) now points to last LED. Total LEDs = 305 (= 61 piano keys * 5 LEDs)
#endif
#ifdef  INSTRUMENT_73_KEYS
    "ADD  R5, R5, #145         \n"          // (R5+19) now points to last LED. Total LEDs = 365 (= 73 piano keys * 5 LEDs)
#endif
    "LDR  R1, [%[data], #24]   \n"          // see _input_for_asm[6]
    "MOV  R10, R1              \n"          // R10 = I/O address to drive lines LOW (addrClr)

    "LDR  R1, [%[data], #28]   \n"          // see _input_for_asm[7]
    "MOV  R6, R1               \n"          // R6 = I/O address to drive lines HIGH (addrSet)

    "LDR  R1, [%[data], #20]   \n"          // see _input_for_asm[5]
    "MOV  R8, R1               \n"          // R8 = pointer to colors-array (4 bytes for each color)
    
"mainLoop%=:\n"
    "LDRB R1, [R5, #15]        \n"          // write LEDs for 'zig' part of zig-zag path (counted right-to-left) of both LED strips (2 * 5 LEDs)
    "LDRB R2, [R5, #10]        \n"          // The 'zig' part means: arrow of LED strip is pointing DOWN.
    "BL   writeLED%=           \n"          // In case of 61-key instrument:
    "LDRB R1, [R5, #16]        \n"          // - LED strip 1 is doing the LEDS for these 15 piano key numbers: 61, 57, 53, 49, .. ,9, 5 (*)
    "LDRB R2, [R5, #11]        \n"          // - LED strip 2 is doing the LEDs for these 15 paino key numbers: 60, 56, 52, 48, .. ,8, 4 
    "BL   writeLED%=           \n"          //                                       (*) the last piano key 1 is done outside this mainLoop
    "LDRB R1, [R5, #17]        \n"
    "LDRB R2, [R5, #12]        \n"
    "BL   writeLED%=           \n"   
    "LDRB R1, [R5, #18]        \n"
    "LDRB R2, [R5, #13]        \n"
    "BL   writeLED%=           \n"   
    "LDRB R1, [R5, #19]        \n"
    "LDRB R2, [R5, #14]        \n"
    "BL    writeLED%=          \n"   
    
    "LDRB R1, [R5, #9]         \n"          // write LEDs for 'zag' part of zig-zag path (counted right-to-left) of both LED strips (2 * 5 LEDs)
    "LDRB R2, [R5, #4]         \n"          // The 'zag' part means: arrow of LED strip is pointing UP.
    "BL   writeLED%=           \n"          // In case of 61-key instrument:
    "LDRB R1, [R5, #8]         \n"          // - LED strip 1 is doing the LEDS for these 15 piano key numbers: 59, 55, 51, 47, .. ,7, 3
    "LDRB R2, [R5, #3]         \n"          // - LED strip 2 is doing the LEDs for these 15 paino key numbers: 58, 54, 50, 46, .. ,6, 2 
    "BL   writeLED%=           \n"   
    "LDRB R1, [R5, #7]         \n"
    "LDRB R2, [R5, #2]         \n"
    "BL   writeLED%=           \n"   
    "LDRB R1, [R5, #6]         \n"
    "LDRB R2, [R5, #1]         \n"
    "BL   writeLED%=           \n"   
    "SUB  R5, R5, #20          \n"          // step 20 bytes backwards in indexed colors table (to prepare for next iteration)
    "LDRB R1, [R5, #25]        \n"
    "LDRB R2, [R5, #20]        \n"
    "BL   writeLED%=           \n"   

    ".syntax unified           \n"
    "ADDS R0, R0, #32          \n"          // increase counter (low 5 bits have other counting function). Total 15 iterations for 300 LEDs
    ".syntax divided           \n"

    "BNE   mainLoop%=          \n"          // if counter <> 0, write another series of 20 LEDs

    "LDRB R1, [R5, #15]        \n"          // write the last 5 LEDs (one piano key left to handle outside mainLoop)
    "MOV  R2, #0               \n"          // all piano keys of line 2 (R2) already written: write color index '0': will do nothing, because there are no more LEDs
    "BL   writeLED%=           \n"   
    "LDRB R1, [R5, #16]        \n"
    "MOV  R2, #0               \n"
    "BL   writeLED%=           \n"   
    "LDRB R1, [R5, #17]        \n"
    "MOV  R2, #0               \n"
    "BL   writeLED%=           \n"   
    "LDRB R1, [R5, #18]        \n"
    "MOV  R2, #0               \n"
    "BL   writeLED%=           \n"   
    "LDRB R1, [R5, #19]        \n"
    "MOV  R2, #0               \n"
    "BL   writeLED%=           \n"   
    
    "B   finished%=            \n"     // Finished: all LEDs written! (305 for 61-key instrument, or 365 for 73-key instrument).
    
"writeLED%=:\n"                        // looks up two RGB values and writes each of the 24 bits of both to two separate WS2812B-lines
    "LSL  R1, R1, #2    \n"            // (line 1) R1 = offset in color table (0, 4, 8, etc)
    "ADD  R1, R1, R8    \n"            // (line 1) R1 = pointer to the right color (4-byte value)
    "LDR  R1, [R1]      \n"            // (line 1) R1 is now 24-bit color (coded as 0xGGRRBB00)
    "LSL  R2, R2, #2    \n"            // (line 2) R2 = offset in color table (0, 4, 8, etc)
    "ADD  R2, R2, R8    \n"            // (line 2) R2 = pointer to the right color (4-byte value)
    "LDR  R2, [R2]      \n"            // (line 2) R2 is now 24-bit color (coded as 0xGGRRBB00)
    
    "ADD R0, R0, #24    \n"            // set counter to 24 (for each bit of RGB-color)
"writeBit%=:\n"  
    "LDR  R3, [%[data], #12]   \n"     // see _input_for_asm[3]. R3 is pinMask for both lines
    "STR  R3, [R6]             \n"     // drive both lines HIGH

//    "nop\n"                            /* almost no NOPs needed, because of code below */ 
   
    ".syntax unified         \n"
    "LSLS R1, R1, #1         \n"       // shift bit out of R1 (line 1) into carry flag
    ".syntax divided         \n"
    "SBC  R4, R4,R4          \n"       // R4=0xFFFFFFFF (if carry=0) or R4=0 (if carry=1)
    "MOV  R3, #4             \n"
    "AND  R3, R3, R4         \n"       // R3 is 0 (shifted bit was 1) or 4 (shifted bit was 0)
    "MOV  R9, R3             \n"
    
    ".syntax unified         \n"
    "LSLS R2, R2, #1         \n"       // shift bit out of R2 (line 2) into carry flag
    ".syntax divided         \n"
    "SBC  R4, R4, R4         \n"       // R4=0xFFFFFFFF (if carry=0) or R4=0 (if carry=1)
    "MOV  R3, #8             \n"
    "AND  R3, R3, R4         \n"       // R3 is 0 (shifted bit was 1) or 8 (shifted bit was 0)
    "ADD  R3, R3, R9         \n"       // R3 is offset 0, 4, 8 or 12 (to get the right pinMask)

    "LDR  R3, [%[data], R3]  \n"       // R3 = pinMask, for the lines that send 0-bit (must be driven LOW after 0.4us)
	                                   // see _input_for_asm[x] where x = 0/1/2/3

    "MOV  R4, R10            \n"       // R4 = addrClr (I/O address to drive lines low)
    "STR  R3, [R4]           \n"       // drive lines (only lines that send 0-bit) LOW
    
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"     /*  NOPs here to fill up approx. 0.8us */ 
    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
 //   "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
 //   "nop\n" "nop\n"
   
    "LDR  R3, [%[data], #12] \n"       // R3 is pinMask for both lines
    "STR  R3, [R4]           \n"       // drive all lines low (also lines that send 1-bit, this is after approx. 0.8us)

    "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"     /*  NOPs here to fill up some time */ 
 //   "nop\n" "nop\n"
     
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
    : "r0", "r1", "r2", "r3", "r4", "r5","r6", "r8", "r9", "r10", "cc"  /* clobbers */
    );

    delayMicroseconds(100);  // Hold the line low for 50 microseconds to send the reset signal. 
    __enable_irq();         // Re-enable interrupts now that we are done.
}


/******************************************************************************************************************************
*
* CODE FOR CONTROLLING BUTTONS (ON LEDPANEL) BELOW
* 
*******************************************************************************************************************************/


uint32_t LedPanel::readButtons() {
  _formerReading = _newReading;
  _newReading = (*_readBtnRegister) & _button_All_Mask;
  return _newReading;
}

bool LedPanel::_isButton(bool onlyNewPress, int button) {
  if ((_newReading & _button_Mask[button]) != 0) return false;
  if (!onlyNewPress) return true;
  return (_formerReading & _button_Mask[button]);
}


