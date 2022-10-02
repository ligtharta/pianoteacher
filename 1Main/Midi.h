#ifndef Midi_h
#define Midi_h

#include <Arduino.h>
#include "MidiDefs.h"
#include "Templates.h"


#define MIDI_SEND_CHANNEL      2    /* MIDI channel used for playing notes (low nibble of NoteOn/NoteOff MIDI messages)  */
#define MIDI_MAX_DELAYED_MSG  20    /* max amount of MIDI messages (noteOff) that the system should be able to delay */

/******************************************************************************************************************************
*
* CLASS  :  MidiInterface
* 
* Handles communication with MIDI keyboard
* (writing and reading MIDI messages 'note_on' and 'note_off')
*
*******************************************************************************************************************************/
class MidiInterface {
  public:
    /**
    * Constructor.
    */
    MidiInterface();
    void init_MIDI();
    void playNote(byte pitch, byte velocity, uint32_t noteOffTime);
    void handleDelays(uint32_t now);
    void handleAllDelaysImmediately();
    void selectInstrument(byte instrument);

    /* reading MIDI messages */
    int getPressedPianoKey();
    int getPressedPianoKey(bool *sustainPressed, bool* sustainReleased);
    void clearReadBuffer();

  private:
    DelayManager<byte, MIDI_MAX_DELAYED_MSG> _noteOffs_todo;       /* MIDI noteOff messages to be sent in the future */

    void _noteOn(byte pitch, byte velocity);
    void _noteOff(byte pitch);
    
};




#endif // Midi_h
