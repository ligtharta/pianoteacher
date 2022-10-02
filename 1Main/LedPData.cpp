#include <Arduino.h>
#include "LedPanel.h"

/* REASON WHY KEYWORD 'extern' IS USED AT BOTH LOCATIONS: 
   C++ behaves differently from 'C'.  Const declarations default to internal linkage in C++, external in 'C'.  
   You'll have to put "extern" in front of the declaration to force external linkage.                       */


extern const uint32_t _colors[] PROGMEM =
{ //GGRRBB00
/******************************************************************************************************************************
* The 64 (8 x 8) colors below have a MAXIMUM BRIGHTNESS of 32 out of 255 
*                                    (except for Blue and Red: these have higher MAXIMUM brightness)
*            To use these colors, see: ledPanel.setGlobalBrightness(0) 
*******************************************************************************************************************************/
/* color: COLOR_IDX_OFF  */
  0x00000000,  // index 0, RGB=(0,0,0)
  0x00000000,  // index 1, RGB=(0,0,0)
  0x00000000,  // index 2, RGB=(0,0,0)
  0x00000000,  // index 3, RGB=(0,0,0)
  0x00000000,  // index 4, RGB=(0,0,0)
  0x00000000,  // index 5, RGB=(0,0,0)
  0x00000000,  // index 6, RGB=(0,0,0)
  0x00000000,  // index 7, RGB=(0,0,0)
/* color: COLOR_IDX_WHITE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x20202000,  // index 8, RGB=(32,32,32)
  0x1C1C1C00,  // index 9, RGB=(28,28,28)
  0x18181800,  // index 10, RGB=(24,24,24)
  0x14141400,  // index 11, RGB=(20,20,20)
  0x10101000,  // index 12, RGB=(16,16,16)
  0x0C0C0C00,  // index 13, RGB=(12,12,12)
  0x08080800,  // index 14, RGB=(8,8,8)
  0x04040400,  // index 15, RGB=(4,4,4)
/* color: COLOR_IDX_GREY  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x10101000,  // index 16, RGB=(16,16,16)
  0x0E0E0E00,  // index 17, RGB=(14,14,14)
  0x0C0C0C00,  // index 18, RGB=(12,12,12)
  0x0A0A0A00,  // index 19, RGB=(10,10,10)
  0x08080800,  // index 20, RGB=(8,8,8)
  0x04040400,  // index 21, RGB=(4,4,4)
  0x02020200,  // index 22, RGB=(2,2,2)
  0x01010100,  // index 23, RGB=(1,1,1)
/* color: COLOR_IDX_BLUE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00002A00,  // index 24, RGB=(0,0,42)
  0x00002500,  // index 25, RGB=(0,0,37)
  0x00001F00,  // index 26, RGB=(0,0,31)
  0x00001A00,  // index 27, RGB=(0,0,26)
  0x00001500,  // index 28, RGB=(0,0,21)
  0x00000F00,  // index 29, RGB=(0,0,15)
  0x00000A00,  // index 30, RGB=(0,0,10)
  0x00000500,  // index 31, RGB=(0,0,5)
/* color: COLOR_IDX_GREEN  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x20000000,  // index 32, RGB=(0,32,0)
  0x1C000000,  // index 33, RGB=(0,28,0)
  0x18000000,  // index 34, RGB=(0,24,0)
  0x14000000,  // index 35, RGB=(0,20,0)
  0x10000000,  // index 36, RGB=(0,16,0)
  0x0C000000,  // index 37, RGB=(0,12,0)
  0x08000000,  // index 38, RGB=(0,8,0)
  0x04000000,  // index 39, RGB=(0,4,0)
/* color: COLOR_IDX_PURPLE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00202000,  // index 40, RGB=(32,0,32)
  0x001C1C00,  // index 41, RGB=(28,0,28)
  0x00181800,  // index 42, RGB=(24,0,24)
  0x00141400,  // index 43, RGB=(20,0,20)
  0x00101000,  // index 44, RGB=(16,0,16)
  0x000C0C00,  // index 45, RGB=(12,0,12)
  0x00080800,  // index 46, RGB=(8,0,8)
  0x00040400,  // index 47, RGB=(4,0,4)
/* color: COLOR_IDX_YELLOW  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x20200000,  // index 48, RGB=(32,32,0)
  0x1C1C0000,  // index 49, RGB=(28,28,0)
  0x18180000,  // index 50, RGB=(24,24,0)
  0x14140000,  // index 51, RGB=(20,20,0)
  0x10100000,  // index 52, RGB=(16,16,0)
  0x0C0C0000,  // index 53, RGB=(12,12,0)
  0x08080000,  // index 54, RGB=(8,8,0)
  0x04040000,  // index 55, RGB=(4,4,0)
/* color: COLOR_IDX_RED  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00260000,  // index 56, RGB=(38,0,0)
  0x00210000,  // index 57, RGB=(33,0,0)
  0x001C0000,  // index 58, RGB=(28,0,0)
  0x00180000,  // index 59, RGB=(24,0,0)
  0x00130000,  // index 60, RGB=(19,0,0)
  0x000E0000,  // index 61, RGB=(14,0,0)
  0x00090000,  // index 62, RGB=(9,0,0)
  0x00040000,  // index 63, RGB=(4,0,0)
/******************************************************************************************************************************
* The 64 (8 x 8) colors below have a MAXIMUM BRIGHTNESS of 64 out of 255
*                                    (except for Blue and Red: these have higher MAXIMUM brightness)
*            To use these colors, see: ledPanel.setGlobalBrightness(1) 
*******************************************************************************************************************************/
/* color: COLOR_IDX_OFF   */
  0x00000000,  // index 0, RGB=(0,0,0)
  0x00000000,  // index 1, RGB=(0,0,0)
  0x00000000,  // index 2, RGB=(0,0,0)
  0x00000000,  // index 3, RGB=(0,0,0)
  0x00000000,  // index 4, RGB=(0,0,0)
  0x00000000,  // index 5, RGB=(0,0,0)
  0x00000000,  // index 6, RGB=(0,0,0)
  0x00000000,  // index 7, RGB=(0,0,0)
/* color: COLOR_IDX_WHITE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x40404000,  // index 8, RGB=(64,64,64)
  0x38383800,  // index 9, RGB=(56,56,56)
  0x30303000,  // index 10, RGB=(48,48,48)
  0x28282800,  // index 11, RGB=(40,40,40)
  0x20202000,  // index 12, RGB=(32,32,32)
  0x18181800,  // index 13, RGB=(24,24,24)
  0x10101000,  // index 14, RGB=(16,16,16)
  0x08080800,  // index 15, RGB=(8,8,8)
/* color: COLOR_IDX_GREY  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x20202000,  // index 16, RGB=(32,32,32)
  0x1C1C1C00,  // index 17, RGB=(28,28,28)
  0x18181800,  // index 18, RGB=(24,24,24)
  0x14141400,  // index 19, RGB=(20,20,20)
  0x10101000,  // index 20, RGB=(16,16,16)
  0x0C0C0C00,  // index 21, RGB=(12,12,12)
  0x08080800,  // index 22, RGB=(8,8,8)
  0x04040400,  // index 23, RGB=(4,4,4)
/* color: COLOR_IDX_BLUE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00005500,  // index 24, RGB=(0,0,85)
  0x00004A00,  // index 25, RGB=(0,0,74)
  0x00003F00,  // index 26, RGB=(0,0,63)
  0x00003500,  // index 27, RGB=(0,0,53)
  0x00002A00,  // index 28, RGB=(0,0,42)
  0x00001F00,  // index 29, RGB=(0,0,31)
  0x00001500,  // index 30, RGB=(0,0,21)
  0x00000A00,  // index 31, RGB=(0,0,10)
/* color: COLOR_IDX_GREEN  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x40000000,  // index 32, RGB=(0,64,0)
  0x38000000,  // index 33, RGB=(0,56,0)
  0x30000000,  // index 34, RGB=(0,48,0)
  0x28000000,  // index 35, RGB=(0,40,0)
  0x20000000,  // index 36, RGB=(0,32,0)
  0x18000000,  // index 37, RGB=(0,24,0)
  0x10000000,  // index 38, RGB=(0,16,0)
  0x08000000,  // index 39, RGB=(0,8,0)
/* color: COLOR_IDX_PURPLE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00404000,  // index 40, RGB=(64,0,64)
  0x00383800,  // index 41, RGB=(56,0,56)
  0x00303000,  // index 42, RGB=(48,0,48)
  0x00282800,  // index 43, RGB=(40,0,40)
  0x00202000,  // index 44, RGB=(32,0,32)
  0x00181800,  // index 45, RGB=(24,0,24)
  0x00101000,  // index 46, RGB=(16,0,16)
  0x00080800,  // index 47, RGB=(8,0,8)
/* color: COLOR_IDX_YELLOW  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x40400000,  // index 48, RGB=(64,64,0)
  0x38380000,  // index 49, RGB=(56,56,0)
  0x30300000,  // index 50, RGB=(48,48,0)
  0x28280000,  // index 51, RGB=(40,40,0)
  0x20200000,  // index 52, RGB=(32,32,0)
  0x18180000,  // index 53, RGB=(24,24,0)
  0x10100000,  // index 54, RGB=(16,16,0)
  0x08080000,  // index 55, RGB=(8,8,0)
/* color: COLOR_IDX_RED  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x004C0000,  // index 56, RGB=(76,0,0)
  0x00430000,  // index 57, RGB=(67,0,0)
  0x00390000,  // index 58, RGB=(57,0,0)
  0x00300000,  // index 59, RGB=(48,0,0)
  0x00260000,  // index 60, RGB=(38,0,0)
  0x001C0000,  // index 61, RGB=(28,0,0)
  0x00130000,  // index 62, RGB=(19,0,0)
  0x00090000,  // index 63, RGB=(9,0,0)
/******************************************************************************************************************************
* The 64 (8 x 8) colors below have a MAXIMUM BRIGHTNESS of 128 out of 255
*                                    (except for Blue and Red: these have higher MAXIMUM brightness)
*            To use these colors, see: ledPanel.setGlobalBrightness(2)
*******************************************************************************************************************************/
/* color: COLOR_IDX_OFF  */
  0x00000000,  // index 0, RGB=(0,0,0)
  0x00000000,  // index 1, RGB=(0,0,0)
  0x00000000,  // index 2, RGB=(0,0,0)
  0x00000000,  // index 3, RGB=(0,0,0)
  0x00000000,  // index 4, RGB=(0,0,0)
  0x00000000,  // index 5, RGB=(0,0,0)
  0x00000000,  // index 6, RGB=(0,0,0)
  0x00000000,  // index 7, RGB=(0,0,0)
/* color: COLOR_IDX_WHITE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x80808000,  // index 8, RGB=(128,128,128)
  0x70707000,  // index 9, RGB=(112,112,112)
  0x60606000,  // index 10, RGB=(96,96,96)
  0x50505000,  // index 11, RGB=(80,80,80)
  0x40404000,  // index 12, RGB=(64,64,64)
  0x30303000,  // index 13, RGB=(48,48,48)
  0x20202000,  // index 14, RGB=(32,32,32)
  0x10101000,  // index 15, RGB=(16,16,16)
/* color: COLOR_IDX_GREY  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x40404000,  // index 16, RGB=(64,64,64)
  0x38383800,  // index 17, RGB=(56,56,56)
  0x30303000,  // index 18, RGB=(48,48,48)
  0x28282800,  // index 19, RGB=(40,40,40)
  0x20202000,  // index 20, RGB=(32,32,32)
  0x18181800,  // index 21, RGB=(24,24,24)
  0x10101000,  // index 22, RGB=(16,16,16)
  0x08080800,  // index 23, RGB=(8,8,8)
/* color: COLOR_IDX_BLUE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x0000AA00,  // index 24, RGB=(0,0,170)
  0x00009400,  // index 25, RGB=(0,0,148)
  0x00007F00,  // index 26, RGB=(0,0,127)
  0x00006A00,  // index 27, RGB=(0,0,106)
  0x00005500,  // index 28, RGB=(0,0,85)
  0x00003F00,  // index 29, RGB=(0,0,63)
  0x00002A00,  // index 30, RGB=(0,0,42)
  0x00001500,  // index 31, RGB=(0,0,21)
/* color: COLOR_IDX_GREEN  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x80000000,  // index 32, RGB=(0,128,0)
  0x70000000,  // index 33, RGB=(0,112,0)
  0x60000000,  // index 34, RGB=(0,96,0)
  0x50000000,  // index 35, RGB=(0,80,0)
  0x40000000,  // index 36, RGB=(0,64,0)
  0x30000000,  // index 37, RGB=(0,48,0)
  0x20000000,  // index 38, RGB=(0,32,0)
  0x10000000,  // index 39, RGB=(0,16,0)
/* color: COLOR_IDX_PURPLE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00808000,  // index 40, RGB=(128,0,128)
  0x00707000,  // index 41, RGB=(112,0,112)
  0x00606000,  // index 42, RGB=(96,0,96)
  0x00505000,  // index 43, RGB=(80,0,80)
  0x00404000,  // index 44, RGB=(64,0,64)
  0x00303000,  // index 45, RGB=(48,0,48)
  0x00202000,  // index 46, RGB=(32,0,32)
  0x00101000,  // index 47, RGB=(16,0,16)
/* color: COLOR_IDX_YELLOW  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x80800000,  // index 48, RGB=(128,128,0)
  0x70700000,  // index 49, RGB=(112,112,0)
  0x60600000,  // index 50, RGB=(96,96,0)
  0x50500000,  // index 51, RGB=(80,80,0)
  0x40400000,  // index 52, RGB=(64,64,0)
  0x30300000,  // index 53, RGB=(48,48,0)
  0x20200000,  // index 54, RGB=(32,32,0)
  0x10100000,  // index 55, RGB=(16,16,0)
/* color: COLOR_IDX_RED  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00990000,  // index 56, RGB=(153,0,0)
  0x00860000,  // index 57, RGB=(134,0,0)
  0x00730000,  // index 58, RGB=(115,0,0)
  0x00600000,  // index 59, RGB=(96,0,0)
  0x004C0000,  // index 60, RGB=(76,0,0)
  0x00390000,  // index 61, RGB=(57,0,0)
  0x00260000,  // index 62, RGB=(38,0,0)
  0x00130000,  // index 63, RGB=(19,0,0)
/******************************************************************************************************************************
* The 64 (8 x 8) colors below have a MAXIMUM BRIGHTNESS of 192 out of 255
*                                    (except for Blue and Red: these have higher MAXIMUM brightness)
*            To use these colors, see: ledPanel.setGlobalBrightness(3) 
*******************************************************************************************************************************/
/* color: COLOR_IDX_OFF  */
  0x00000000,  // index 0, RGB=(0,0,0)
  0x00000000,  // index 1, RGB=(0,0,0)
  0x00000000,  // index 2, RGB=(0,0,0)
  0x00000000,  // index 3, RGB=(0,0,0)
  0x00000000,  // index 4, RGB=(0,0,0)
  0x00000000,  // index 5, RGB=(0,0,0)
  0x00000000,  // index 6, RGB=(0,0,0)
  0x00000000,  // index 7, RGB=(0,0,0)
/* color: COLOR_IDX_WHITE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0xC0C0C000,  // index 8, RGB=(192,192,192)
  0xA8A8A800,  // index 9, RGB=(168,168,168)
  0x90909000,  // index 10, RGB=(144,144,144)
  0x78787800,  // index 11, RGB=(120,120,120)
  0x60606000,  // index 12, RGB=(96,96,96)
  0x48484800,  // index 13, RGB=(72,72,72)
  0x30303000,  // index 14, RGB=(48,48,48)
  0x18181800,  // index 15, RGB=(24,24,24)
/* color: COLOR_IDX_GREY  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x60606000,  // index 16, RGB=(96,96,96)
  0x54545400,  // index 17, RGB=(84,84,84)
  0x48484800,  // index 18, RGB=(72,72,72)
  0x3C3C3C00,  // index 19, RGB=(60,60,60)
  0x30303000,  // index 20, RGB=(48,48,48)
  0x24242400,  // index 21, RGB=(36,36,36)
  0x18181800,  // index 22, RGB=(24,24,24)
  0x0C0C0C00,  // index 23, RGB=(12,12,12)
/* color: COLOR_IDX_BLUE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x0000FF00,  // index 24, RGB=(0,0,255)
  0x0000DF00,  // index 25, RGB=(0,0,223)
  0x0000BF00,  // index 26, RGB=(0,0,191)
  0x00009F00,  // index 27, RGB=(0,0,159)
  0x00007F00,  // index 28, RGB=(0,0,127)
  0x00005F00,  // index 29, RGB=(0,0,95)
  0x00003F00,  // index 30, RGB=(0,0,63)
  0x00001F00,  // index 31, RGB=(0,0,31)
/* color: COLOR_IDX_GREEN  (add 1/2/3/4/5/6/7 for less brightness)  */
  0xC0000000,  // index 32, RGB=(0,192,0)
  0xA8000000,  // index 33, RGB=(0,168,0)
  0x90000000,  // index 34, RGB=(0,144,0)
  0x78000000,  // index 35, RGB=(0,120,0)
  0x60000000,  // index 36, RGB=(0,96,0)
  0x48000000,  // index 37, RGB=(0,72,0)
  0x30000000,  // index 38, RGB=(0,48,0)
  0x18000000,  // index 39, RGB=(0,24,0)
/* color: COLOR_IDX_PURPLE  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00C0C000,  // index 40, RGB=(192,0,192)
  0x00A8A800,  // index 41, RGB=(168,0,168)
  0x00909000,  // index 42, RGB=(144,0,144)
  0x00787800,  // index 43, RGB=(120,0,120)
  0x00606000,  // index 44, RGB=(96,0,96)
  0x00484800,  // index 45, RGB=(72,0,72)
  0x00303000,  // index 46, RGB=(48,0,48)
  0x00181800,  // index 47, RGB=(24,0,24)
/* color: COLOR_IDX_YELLOW  (add 1/2/3/4/5/6/7 for less brightness)  */
  0xC0C00000,  // index 48, RGB=(192,192,0)
  0xA8A80000,  // index 49, RGB=(168,168,0)
  0x90900000,  // index 50, RGB=(144,144,0)
  0x78780000,  // index 51, RGB=(120,120,0)
  0x60600000,  // index 52, RGB=(96,96,0)
  0x48480000,  // index 53, RGB=(72,72,0)
  0x30300000,  // index 54, RGB=(48,48,0)
  0x18180000,  // index 55, RGB=(24,24,0)
/* color: COLOR_IDX_RED  (add 1/2/3/4/5/6/7 for less brightness)  */
  0x00E60000,  // index 56, RGB=(230,0,0)
  0x00C90000,  // index 57, RGB=(201,0,0)
  0x00AC0000,  // index 58, RGB=(172,0,0)
  0x00900000,  // index 59, RGB=(144,0,0)
  0x00730000,  // index 60, RGB=(115,0,0)
  0x00560000,  // index 61, RGB=(86,0,0)
  0x00390000,  // index 62, RGB=(57,0,0)
  0x001C0000,  // index 63, RGB=(28,0,0)
};

/******************************************************************************************************************************
* Colors used on 'User selection screen'
*******************************************************************************************************************************/
extern const byte _colorIdx_users[] PROGMEM = { COLOR_IDX_BLUE,   /* user 1 */
                                                COLOR_IDX_GREEN,  /* user 2 */
                                                COLOR_IDX_PURPLE, /* user 3 */
                                                COLOR_IDX_RED,    /* user 4 */
                                                COLOR_IDX_RED };  /* user 5 */

/******************************************************************************************************************************
* Colors assigned to each finger, so that user can see which finger should play the specific piano key.
*******************************************************************************************************************************/
extern const byte _colorIdx_fingers[] PROGMEM =                          { COLOR_IDX_WHITE,    /* index 0 means: finger UNKNOWN -> use WHITE  */
  COLOR_IDX_BLUE,  COLOR_IDX_GREEN,  COLOR_IDX_PURPLE,  COLOR_IDX_YELLOW,  COLOR_IDX_RED,      /* left hand:  thumb -> little finger */
  COLOR_IDX_BLUE,  COLOR_IDX_GREEN,  COLOR_IDX_PURPLE,  COLOR_IDX_YELLOW,  COLOR_IDX_RED       /* right hand: thumb -> little finger  */
};

/******************************************************************************************************************************
* Array below maps ASCII characters (starting with ASCII 32 = space) to the _letters array below.
*******************************************************************************************************************************/
extern const byte _letter_index[] PROGMEM = 
{ 
  37,0,0,48,0,0,0,0,               // [space] !"#$%&'
  50,51,40,47,0,39,38,0,           // ()*+,-./          * = [filled circle for user selection] ( = [small arrow to left]  ) = [small arrow to right]
  1,2,3,4,5,6,7,8,9,10,            // 0 t/m 9
  49,41,42,43,44,0,0,              // :;<=>?@           ;<=>  = [ (MIDI)play-symbols ]
  11,12,13,14,15,16,17,18,19,20,   // A - J
  21,22,23,24,25,26,27,28,29,30,   // K - T
  31,32,33,34,35,36,               // U - Z
  45,0,46                          // [\] 
};


/******************************************************************************************************************************
* How ASCII characters are drawn on the LED panel (each character has a height of 5 pixels/LEDs).
* Each character is drawn from left-to-right. Zero (0) means: end-of-character, to allow for variable character width.
*******************************************************************************************************************************/
extern const byte _letters[] PROGMEM =
{
0b10001,   // X
0b01010,
0b00100,
0b01010,
0b10001, 0,0,0,

0b01110,   // 0
0b10001,
0b10001,
0b01110,
0b00000, 0,0,0,

0b10010,   // 1
0b11111,
0b10000,
0b00000,
0b00000, 0,0,0,

0b10010,   // 2
0b11001,
0b10101,
0b10010,
0b00000, 0,0,0,

0b10001,   // 3
0b10101,
0b10101,
0b01010,
0b00000, 0,0,0,

0b01100,   // 4
0b01010,
0b01001,
0b11111,
0b00000, 0,0,0,

0b10111,   // 5
0b10101,
0b10101,
0b01101,
0b00000, 0,0,0,

0b01110,   // 6
0b10101,
0b10101,
0b01000,
0b00000, 0,0,0,

0b00001,   // 7
0b00001,
0b11101,
0b00011,
0b00000, 0,0,0,

0b01010,   // 8
0b10101,
0b10101,
0b01010,
0b00000, 0,0,0,

0b00010,   // 9
0b10101,
0b10101,
0b01110,
0b00000, 0,0,0,
  
0b11110,   // A
0b00101,
0b00101,
0b00101,
0b11110, 0,0,0,

0b11111,   // B
0b10101,
0b10101,
0b10101,
0b01010, 0,0,0,

0b01110,   // C
0b10001,
0b10001,
0b10001,
0b00000, 0,0,0,

0b11111,   // D
0b10001,
0b10001,
0b10001,
0b01110, 0,0,0,

0b11111,   // E
0b10101,
0b10101,
0b10001,
0b00000, 0,0,0,

0b11111,   // F
0b00101,
0b00101,
0b00001,
0b00000, 0,0,0,

0b01110,   // G
0b10001,
0b10101,
0b11101,
0b00000, 0,0,0,

0b11111,   // H
0b00100,
0b00100,
0b11111,
0b00000, 0,0,0,

0b10001,   // I
0b11111,
0b10001,
0b00000,
0b00000, 0,0,0,

0b01000,   // J
0b10000,
0b10000,
0b01111,
0b00000, 0,0,0,

0b11111,   // K
0b00100,
0b01010,
0b10001,
0b00000, 0,0,0,

0b11111,   // L
0b10000,
0b10000,
0b10000,
0b00000, 0,0,0,

0b11111,   // M
0b00010,
0b00100,
0b00010,
0b11111, 0,0,0,

0b11111,   // N
0b00010,
0b00100,
0b01000,
0b11111, 0,0,0,

0b01110,   // O
0b10001,
0b10001,
0b01110,
0b00000, 0,0,0,

0b11111,   // P
0b00101,
0b00101,
0b00010,
0b00000, 0,0,0,

0b01110,   // Q
0b10001,
0b11001,
0b11110,
0b00000, 0,0,0,

0b11111,   // R
0b00101,
0b00101,
0b00101,
0b11010, 0,0,0,

0b10010,   // S
0b10101,
0b10101,
0b10101,
0b01001, 0,0,0,

0b00001,   // T
0b00001,
0b11111,
0b00001,
0b00001, 0,0,0,

0b01111,   // U
0b10000,
0b10000,
0b01111,
0b00000, 0,0,0,

0b00011,   // V
0b01100,
0b10000,
0b01100,
0b00011, 0,0,0,

0b00111,   // W
0b11000,
0b00110,
0b11000,
0b00111, 0,0,0,

0b10001,   // X
0b01010,
0b00100,
0b01010,
0b10001, 0,0,0,

0b00011,   // Y
0b00100,
0b11100,
0b00100,
0b00011, 0,0,0,

0b10001,   // Z
0b11001,
0b10101,
0b10011,
0b10001, 0,0,0,

128 + 0b00000,   // [space]    // without 7th bit set, it would be seen as 'end of character'.
128 + 0b00000,                 // idem.
      0b00000,                 // zero (as always) means end of character.
      0b00000, 0,0,0,0,

      0b10000,   // . [dot]
128 + 0b00000,                 // without 7th bit set, it would be seen as 'end of character'.
      0b00000,                 // zero (as always) means end of character.
      0b00000, 0,0,0,0,

0b00100,   // -
0b00100,
0b00100,
0b00000,
0b00000, 0,0,0,

0b01110,   // * [filled circle for user selection]
0b11111,
0b11111,
0b11111,
0b11111,
0b11111,
0b01110, 0,

0b01010,   //  ; [tiny cross symbol]
0b00100,
0b01010,
0b00000,
0b00000, 0,0,0,

0b01110,   //  < [filled triangle (small) to inidicate song will play via MIDI]
0b00100,
0b00000,
0b00000,
0b00000, 0,0,0,

0b11111,   //  = [filled triangle (medium) to inidicate song will play via MIDI]
0b01110,
0b00100,
0b00000,
0b00000, 0,0,0,

0b11111,   //  > [filled triangle (big) to inidicate song will play via MIDI]
0b11111,
0b01110,
0b00100,
0b00000, 0,0,0,

0b11111,   //  [
0b10001,
0b00000,
0b00000,
0b00000, 0,0,0,

0b10001,   //  ]
0b11111,
0b00000,
0b00000,
0b00000, 0,0,0,

0b00100,   //  +
0b00100,
0b11111,
0b00100,
0b00100, 0,0,0,

0b01010,   //  #
0b11111,
0b01010,
0b11111,
0b01010, 0,0,0,

      0b10010,   //  :
128 + 0b00000,   // without 7th bit set, it would be seen as 'end of character'.
      0b00000,
      0b00000,
      0b00000, 0,0,0,

0b00100,   //  small arrow left
0b01110,
0b00100,
0b00100,
0b00000, 0,0,0,

0b00100,   //  small arrow right
0b00100,
0b01110,
0b00100,
0b00000, 0,0,0,


};
