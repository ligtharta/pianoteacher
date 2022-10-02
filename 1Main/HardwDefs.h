#ifndef HardwareDefs_h
#define HardwareDefs_h


/******************************************************************************************************************************
*
* Hardware related defines: Device characteristics, Arduino PIN numbers, size of piano keyboard, etc..
* 
*******************************************************************************************************************************/


/******************************************************************************************************************************
*
* GENERAL INFO ABOUT HARDWARE SUPPORT
* 
*******************************************************************************************************************************
* General:
* - Arduino boards supported:  MKR 1000 WiFi,   MKR WiFi 1010,   MKR Zero,   Nano 33 IoT  (all SAMD21 Cortex-M0+ 32bit at 48MHz)
* - PCB boards:
*   - V1 (this PCB supports all experimental features, is big in size, and needs a PC power supply using a ATX connector)
*   - V2 (this PCB fits in small case, but lacks support for experimental features: metronome & vibrating gloves)
*   - V3 (same as V2, but even smaller and also a smaller Arduino form factor: Nano 33 IoT)
*******************************************************************************************************************************
* Hardware features:
* - driving a full color LED panel (61 x 5 LEDs or 73 x 5 LEDs)
* - 4 push buttons on the LED panel (select user, select song, reload song, wifi mode)
* - a foot pedal with 3 pedals (left, middle, right)
* - a micro-SD card with users, settings and songs.
* - a MIDI interface (to send and receive MIDI data to/from Digital Piano or Keyboard).
* - Wifi (to update song data) (NOT with the Arduino MKR Zero)
* - Only with PCB board V1 (experimental features):
*   - Vibrating gloves (a vibration motor for each of 10 fingers: for Haptic feedback about which finger to use)
*   - A metronome (ticks at each measure or beat, to be used when the system controls the tempo while practicing)
*******************************************************************************************************************************/


/******************************************************************************************************************************
* Compile for only 1 of the devices below, or create your own device specification
*******************************************************************************************************************************/
//#define HARDWARE_DEVICE_1    /* compile for: Arduino MKR 1000 (on PCB board V1), 73-key digital piano, all hardware features available */
//#define HARDWARE_DEVICE_2    /* compile for: Arduino MKR 1000 (on PCB board V1), 61-key keyboard, no vibrating gloves, no metronome */
//#define HARDWARE_DEVICE_3    /* compile for: Arduino MKR ZERO (on PCB board V2), 73-key digital piano, no wifi, no vibrating gloves, no metronome  */
//#define HARDWARE_DEVICE_4    /* compile for: Arduino WIFI 1010 (on PCB board V2), 73-key digital piano, wifi, no vibrating gloves, no metronome  */
#define HARDWARE_DEVICE_5    /* compile for: Arduino Nano 33 IoT (on PCB board V3), 73-key digital piano, wifi, no vibrating gloves, no metronome  */

/******************************************************************************************************************************
* Device characteristics: 
* PCB board V1, Arduino Mkr1000 (with build-in wifi), used with 73-key piano, vibrating gloves possible, metronome possible.
*******************************************************************************************************************************/
#ifdef HARDWARE_DEVICE_1
#define INSTRUMENT_73_KEYS                 /* Digital Piano - Yamaha P-121 */                
#define MIDI_PITCH_MIN                  28 /* = MIDI_PITCH_E1 */
#define MIDI_PITCH_MAX                 100 /* = MIDI_PITCH_E7 */
#define MIDI_DEFAULT_VELOCITY           80 /* MIDI velocity when the system is playing the song */
#define MILLIS_TO_WRITE_LED_PANEL        4 /* time in milliseconds to write 73 x 5 LEDs with asm function */
#define METRONOME_HARDWARE_AVAILABLE  true
#define GLOVES_HARDWARE_AVAILABLE     true
#define WIFI_HARDWARE_ATMEL                /* ATMEL chip for Wifi (Arduino MKR1000) */
#define PANEL_LEFT_MARGIN                6 /* left margin on LED panel (only info/settings screens, to center these) */
#endif // HARDWARE_DEVICE_1

/******************************************************************************************************************************
* Device characteristics: 
* PCB board V1, Arduino Mkr1000 (with build-in wifi), used with 61-key keyboard, no vibrating gloves, no metronome.
*******************************************************************************************************************************/
#ifdef HARDWARE_DEVICE_2
#define INSTRUMENT_61_KEYS                 /* Digital Keyboard - Yamaha PSR-E343 */
#define MIDI_PITCH_MIN                  36 /* = MIDI_PITCH_C2 */
#define MIDI_PITCH_MAX                  96 /* = MIDI_PITCH_C7 */
#define MIDI_DEFAULT_VELOCITY           60 /* MIDI velocity when the system is playing the song */
#define MILLIS_TO_WRITE_LED_PANEL        3 /* time in milliseconds to write 61 x 5 LEDs with asm function */
#define METRONOME_HARDWARE_AVAILABLE false
#define GLOVES_HARDWARE_AVAILABLE    false
#define WIFI_HARDWARE_ATMEL                /* ATMEL chip for Wifi (Arduino MKR1000) */
#define PANEL_LEFT_MARGIN                0 /* no left margin on LED panel (only info/settings screens, all 61 columns needed) */
#endif // HARDWARE_DEVICE_2

/******************************************************************************************************************************
* Device characteristics: 
* PCB board V2, Arduino MkrZero (with build-in uSD card reader), used with 73-key piano, no vibrating gloves, no metronome, no wifi
*******************************************************************************************************************************/
#ifdef HARDWARE_DEVICE_3
#define INSTRUMENT_61_KEYS                 /* Digital Piano - Yamaha P-121 */                
#define MIDI_PITCH_MIN                  36 /* = MIDI_PITCH_E1 */
#define MIDI_PITCH_MAX                  96 /* = MIDI_PITCH_E7 */
#define MIDI_DEFAULT_VELOCITY           80 /* MIDI velocity when the system is playing the song */
#define MILLIS_TO_WRITE_LED_PANEL        3 /* time in milliseconds to write 73 x 5 LEDs with asm function */
#define METRONOME_HARDWARE_AVAILABLE  false
#define GLOVES_HARDWARE_AVAILABLE     false
#define PANEL_LEFT_MARGIN                0 /* left margin on LED panel (only info/settings screens, to center these) */
#define USE_BUILTIN_SD                     /* Use built-in SD card reader (the 'Arduino Mkr Zero' has one) */
#endif // HARDWARE_DEVICE_3

/******************************************************************************************************************************
* Device characteristics: 
* PCB board V2, Arduino MKR WIFI 1010, used with 73-key piano, no vibrating gloves, no metronome
*******************************************************************************************************************************/
#ifdef HARDWARE_DEVICE_4
#define INSTRUMENT_73_KEYS                 /* Digital Piano - Yamaha P-121 */                
#define MIDI_PITCH_MIN                  28 /* = MIDI_PITCH_E1 */
#define MIDI_PITCH_MAX                 100 /* = MIDI_PITCH_E7 */
#define MIDI_DEFAULT_VELOCITY           80 /* MIDI velocity when the system is playing the song */
#define MILLIS_TO_WRITE_LED_PANEL        4 /* time in milliseconds to write 73 x 5 LEDs with asm function */
#define METRONOME_HARDWARE_AVAILABLE  false
#define GLOVES_HARDWARE_AVAILABLE     false
#define WIFI_HARDWARE_UBLOX                /* UBLOX chip for Wifi (Arduino MKR WiFi 1010, Nano 33 IoT) */
#define PANEL_LEFT_MARGIN                6 /* left margin on LED panel (only info/settings screens, to center these) */
#endif // HARDWARE_DEVICE_4

/******************************************************************************************************************************
* Device characteristics: 
* PCB board V3, Arduino Nano 33 IoT (with build-in wifi), used with 73-key piano, no vibrating gloves, no metronome
*******************************************************************************************************************************/
#ifdef HARDWARE_DEVICE_5
#define NANO_33_IOT_BOARD                  /* With "Arduino Nano 33 IoT" some PIN numbers are different, see below */
#define INSTRUMENT_73_KEYS                 /* Digital Piano - Yamaha P-121 */                
#define MIDI_PITCH_MIN                  28 /* = MIDI_PITCH_E1 */
#define MIDI_PITCH_MAX                 100 /* = MIDI_PITCH_E7 */
#define MIDI_DEFAULT_VELOCITY           80 /* MIDI velocity when the system is playing the song */
#define MILLIS_TO_WRITE_LED_PANEL        4 /* time in milliseconds to write 73 x 5 LEDs with asm function */
#define METRONOME_HARDWARE_AVAILABLE  false
#define GLOVES_HARDWARE_AVAILABLE     false
#define WIFI_HARDWARE_UBLOX                /* UBLOX chip for Wifi (Arduino MKR WiFi 1010, Nano 33 IoT) */
#define PANEL_LEFT_MARGIN                6 /* left margin on LED panel (only info/settings screens, to center these) */
#endif // HARDWARE_DEVICE_5



#define MAX_UNIQUE_PITCHES        (MIDI_PITCH_MAX - MIDI_PITCH_MIN + 1)    /* max used MIDI-pitches */


/******************************************************************************************************************************
* Wifi available?
*******************************************************************************************************************************/
#if defined(WIFI_HARDWARE_ATMEL) || defined(WIFI_HARDWARE_UBLOX)
#define WIFI_HARDWARE_AVAILABLE       true
#else
#define WIFI_HARDWARE_AVAILABLE       false
#endif



/******************************************************************************************************************************
* LED panel
*******************************************************************************************************************************/
#ifdef NANO_33_IOT_BOARD
#define LEDPANEL_STRIP1_PIN       2  /* STRIP1 and STRIP2 must be on the same PORT! */
#define LEDPANEL_STRIP2_PIN       3  /* STRIP1 and STRIP2 must be on the same PORT! */
#define LEDPANEL_BUTTON1_PIN      A3 /* all 4 buttons on same PORT!! */
#define LEDPANEL_BUTTON2_PIN      A2 /* all 4 buttons on same PORT!! */
#define LEDPANEL_BUTTON3_PIN      A0 /* all 4 buttons on same PORT!! */
#define LEDPANEL_BUTTON4_PIN      A6 /* all 4 buttons on same PORT!! */
#else
#define LEDPANEL_STRIP1_PIN       5  /* STRIP1 and STRIP2 must be on the same PORT! */
#define LEDPANEL_STRIP2_PIN       4  /* STRIP1 and STRIP2 must be on the same PORT! */
#define LEDPANEL_BUTTON1_PIN      2  /* all 4 buttons on same PORT!! */
#define LEDPANEL_BUTTON2_PIN      3  /* all 4 buttons on same PORT!! */
#define LEDPANEL_BUTTON3_PIN      0  /* all 4 buttons on same PORT!! */
#define LEDPANEL_BUTTON4_PIN      1  /* all 4 buttons on same PORT!! */
#endif

/******************************************************************************************************************************
* Foot Pedal
*******************************************************************************************************************************/
#ifdef NANO_33_IOT_BOARD
#define FOOTPEDAL_LEFT_PIN         7 /* all 3 footpedal switches on the same PORT! */
#define FOOTPEDAL_MIDDLE_PIN       6 /* all 3 footpedal switches on the same PORT! */
#define FOOTPEDAL_RIGHT_PIN        5 /* all 3 footpedal switches on the same PORT! */
#else
#define FOOTPEDAL_LEFT_PIN        12 /* all 3 footpedal switches on the same PORT! */
#define FOOTPEDAL_MIDDLE_PIN      11 /* all 3 footpedal switches on the same PORT! */
#define FOOTPEDAL_RIGHT_PIN        7 /* all 3 footpedal switches on the same PORT! */
#endif


/******************************************************************************************************************************
* SD Card
*******************************************************************************************************************************/
#ifdef USE_BUILTIN_SD
#define SDCARD_CSPIN  SDCARD_SS_PIN  /* use built-in SD Card Reader/Writer (Arduino Mkr Zero)  */
#else
# ifdef NANO_33_IOT_BOARD
# define SDCARD_CSPIN             9  /* CS pin of the SD Card Reader/Writer (Arduino Nano 33 IoT) */
# else
# define SDCARD_CSPIN             6  /* CS pin of the SD Card Reader/Writer (Arduino Mkr boards)  */
# endif
#endif // USE_BUILTIN_SD

/******************************************************************************************************************************
* Metronome (only supported with PCB board V1)
*******************************************************************************************************************************/
#if (METRONOME_HARDWARE_AVAILABLE == true)
#define METRONOME_LEDSTRIP_PIN    A4
#define METRONOME_PWM_PIN         A3
#endif

/******************************************************************************************************************************
* Vibrating Gloves (only supported with PCB board V1)
*******************************************************************************************************************************/
#if (GLOVES_HARDWARE_AVAILABLE == true)
#define GLOVES_DATA_SERIAL_PIN    A1 /* see: 75HC595, pin 14 */
#define GLOVES_STORE_REG_CLK_PIN  A2 /* see: 75HC595, pin 12 */
#define GLOVES_SHIFT_REG_CLK_PIN  A5 /* see: 75HC595, pin 11 */
#define GLOVES_MASTER_RESET_PIN   A6 /* see: 75HC595, pin 10 */
#endif


/******************************************************************************************************************************
* Arduino MKR1000 / MKR ZERO / MKR WIFI 1010
* --------------------------------------------
* pin,port,bit, bitmask,    addrSet,  addrClear
*   0,   0, 22, 4194304, 1090536472, 1090536468
*   1,   0, 23, 8388608, 1090536472, 1090536468
*   2,   0, 10,    1024, 1090536472, 1090536468
*   3,   0, 11,    2048, 1090536472, 1090536468
*   4,   1, 10,    1024, 1090536600, 1090536596
*   5,   1, 11,    2048, 1090536600, 1090536596
*   6,   0, 20, 1048576, 1090536472, 1090536468
*   7,   0, 21, 2097152, 1090536472, 1090536468
*   8,   0, 16,   65536, 1090536472, 1090536468
*   9,   0, 17,  131072, 1090536472, 1090536468
*  10,   0, 19,  524288, 1090536472, 1090536468
*  11,   0,  8,     256, 1090536472, 1090536468
*  12,   0,  9,     512, 1090536472, 1090536468
*  13,   1, 23, 8388608, 1090536600, 1090536596
*  14,   1, 22, 4194304, 1090536600, 1090536596
*******************************************************************************************************************************/


#endif // HardwareDefs_h
