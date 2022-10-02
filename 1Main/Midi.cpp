#include "Midi.h"





/******************************************************************************************************************************
*
* CLASS  :  MidiInterface
* 
*******************************************************************************************************************************/


MidiInterface::MidiInterface() {
}

void MidiInterface::init_MIDI() {
  Serial1.begin(31250);  
}

/******************************************************************************************************************************
*
* CODE TO SEND MIDI MESSAGES (PLAY NOTES)
* 
*******************************************************************************************************************************/


/* play a MIDI note: ensure that NoteOff message will be send later  */
void MidiInterface::playNote(byte pitch, byte velocity, uint32_t noteOffTime) {
  _noteOn(pitch, velocity);
  _noteOffs_todo.add(pitch, noteOffTime);    /* send noteOff MIDI message at a later time */
}

/* Must be called every several milliseconds to ensure delayed NoteOff MIDI messages will be sent */
void MidiInterface::handleDelays(uint32_t now) {
  byte* pitch;
  while ( (pitch = _noteOffs_todo.checkForRelease(now)) != NULL) {
    _noteOff(*pitch);
  };
}

/* When song is stopped, do all NoteOff MIDI messages immediately */
void MidiInterface::handleAllDelaysImmediately() {
  uint32_t tOut; /* unused output */
  byte* pitch;
  while ( (pitch = _noteOffs_todo.peekFirst(tOut)) != NULL) {
    _noteOff(*pitch); 
    _noteOffs_todo.removeFirst();
  }
}

/* Send ProgramChange MIDI message to Piano to change the instrument (using the USB Host Controller connected to Serial1) */
void MidiInterface::selectInstrument(byte instrument) {
  Serial1.write(MidiType::ProgramChange + MIDI_SEND_CHANNEL);
  Serial1.write(instrument & 0b01111111);
}

/* Send NoteOn MIDI message to Piano (using the USB Host Controller connected to Serial1) */
void MidiInterface::_noteOn(byte pitch, byte velocity) {
  Serial1.write(MidiType::NoteOn + MIDI_SEND_CHANNEL);
  Serial1.write((byte)pitch);
  Serial1.write((byte)velocity);
}

/* Send NoteOff MIDI message to Piano (using the USB Host Controller connected to Serial1) */
void MidiInterface::_noteOff(byte pitch) {
  Serial1.write(MidiType::NoteOff + MIDI_SEND_CHANNEL);
  Serial1.write((byte)pitch);
  Serial1.write((byte)0);  
}


/******************************************************************************************************************************
*
* CODE TO RECEIVE MIDI MESSAGES (READ PIANO KEYS PRESSED)
* 
*******************************************************************************************************************************/

int MidiInterface::getPressedPianoKey() {
  static int waitForBytes = 3;  /* example MIDI message: 0x90-0x60-0x040 (NoteOn-pitchC2-velocity64) */
  static int pitch;
  while (Serial1.available() > 0) 
  {
    int b = Serial1.read();
    if (b == MidiType::NoteOn) { waitForBytes = 2; }          /* note-on detected, now wait for 2 more bytes */
    else if (waitForBytes == 2) { pitch = b; waitForBytes--; }/* piano key (pitch) stored, now wait for velocity */
    else if (waitForBytes == 1) {  /* third byte must be >0, otherwise it is 'note-off' message */
      waitForBytes = 3;
      if (b > 0) {
        return pitch;     /* yes, velocity is >0, now we know the piano key (pitch) that was pressed */
      }
    }
    else { waitForBytes = 3; }
  }
  return 0;      /* zero means: no piano key pressed */
}

int MidiInterface::getPressedPianoKey(bool *sustainPressed, bool* sustainReleased) {
  static int waitForBytes = 3;  /* example MIDI message: 0x90-0x60-0x040 (NoteOn-pitchC2-velocity64) */
  static int pitch;
  static int bFirst;
  *sustainPressed = *sustainReleased = false;
  while (Serial1.available() > 0) 
  {
    int b = Serial1.read();
    if (b == MidiType::NoteOn || b == MidiType::ControlChange) { bFirst = b; waitForBytes = 2; }          /* note-on detected, now wait for 2 more bytes */
    else if (waitForBytes == 2)
    {
      if (bFirst == MidiType::NoteOn) 
      {
        pitch = b; waitForBytes--; /* piano key (pitch) stored, now wait for velocity */
      }
      else if (bFirst == MidiType::ControlChange) 
      {
        if (b == MidiControlChange::SustainPedal /* 0x40 */) { waitForBytes--; } else { waitForBytes = 3; }
      }
    }
    else if (waitForBytes == 1)
    {
      waitForBytes = 3;
      if (bFirst == MidiType::NoteOn) 
      { 
        if (b > 0) return pitch;
      }
      else if (bFirst == MidiType::ControlChange) 
      {
        if (b >= 64) *sustainPressed = true; else *sustainReleased = true;
      }
    }
    else { waitForBytes = 3; }
  }
  return 0;      /* zero means: no piano key pressed */
}



void MidiInterface::clearReadBuffer() {
  while (Serial1.available() > 0) Serial1.read();
}
