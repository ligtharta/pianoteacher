#ifndef LedPanel_h
#define LedPanel_h

#include <Arduino.h>
#include "HardwDefs.h"

#define PANEL_COLS (MAX_UNIQUE_PITCHES)       /* LED-panel columns (=number of piano keys) */
#define PANEL_ROWS 5                          /* LED-panel rows (=number of LEDs per piano key) */
#define PANEL_LEDS (PANEL_COLS * PANEL_ROWS)  /* total number of LEDs */
#define BYTES_PER_LETTER 8                    /* bytes used to code LED-pixels of 1 character */

#define BUTTON_USER   0         /* 1st button on the LED panel: select user */
#define BUTTON_SONG   1         /* 2nd button on the LED panel: select song */
#define BUTTON_RELOAD 2         /* 3rd button on the LED panel: reload song (change L/R hand data) */
#define BUTTON_WIFI   3         /* 4th button on the LED panel: wifi connection to update song */

#define COLOR_IDX_OFF      0
#define COLOR_IDX_WHITE    8
#define COLOR_IDX_GREY     16
#define COLOR_IDX_BLUE     24
#define COLOR_IDX_GREEN    32
#define COLOR_IDX_PURPLE   40
#define COLOR_IDX_YELLOW   48
#define COLOR_IDX_RED      56

extern const uint32_t _colors[] PROGMEM;
extern const byte _letters[] PROGMEM;
extern const byte _letter_index[] PROGMEM;
extern const byte _colorIdx_users[] PROGMEM;
extern const byte _colorIdx_fingers[] PROGMEM;



/*
*
*
*/
class LedPanel {
  public:
    /**
    * Constructor.
    */
    LedPanel();
    /* Methods of IWriterLEDMatrix (interface class) */
    void clear();
    int writeText(const char* txt, int columnStart, byte clr);
    int writeText(const char* txt, int columnStart, byte clr, bool keepBG);
    int writeChar(char c, int columnStart, byte clr);
    int writeChar(char c, int columnStart, byte clr, bool keepBG);
    int getTextWidth(const char* txt);
    void setPixel(int x, int y, byte clr);
    byte getPixel(int x, int y);
    void fillRect(int x1, int y1, int x2, int y2, byte clr);
    byte getUserColorIdx(int idUser); 
    byte getFingerColorIdx(int finger);
    void setGlobalBrightness(byte brightnessId);  /*brightnessId: 0 for lowest, 3 for highest */
    void setRowDimming(int row, byte dimmingIdx);  /* 0 = normal color, 1 = slightly dimmed, ..., 7 = max dimmed */
    void SetColorInArea(int x1, int y1, int x2, int y2, byte clr);
    bool IsColumnEmpty(int x);
    
    void writeLeds_asm();
    /*  buttons: */
    uint32_t readButtons();
    inline bool isButtonDown(int btn) { return _isButton(false, btn); }
    inline bool isButtonPressed(int btn) { return _isButton(true, btn); }
    
  protected:

  private:
    /* ledstrips: */
    unsigned char _colorIdx[PANEL_LEDS];  /* every led can have own indexed color */
    uint32_t _input_for_asm[8];           /* used to pass data to assembler routine */
    /* buttons: */
    uint32_t _button_Mask[4];
    uint32_t _button_All_Mask;
    volatile uint32_t *_readBtnRegister; /* all buttons on the same Arduino PORT! */
    uint32_t _newReading, _formerReading;
    bool _isButton(bool onlyNewPress, int button);
};




#endif // LedPanel_h
