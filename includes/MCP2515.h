/*
  MCP2515.h - Library for Microchip MCP2515 CAN Controller
  
  Author: David Harding
  
  Created: 11/08/2010
  
  For further information see:
  
  http://ww1.microchip.com/downloads/en/DeviceDoc/21801e.pdf
  http://en.wikipedia.org/wiki/CAN_bus
*/

#ifndef MCP2515_h
#define MCP2515_h

#include "Arduino.h"
#include "MCP2515_defs.h"

String frameToString(const Frame&);

class MCP2515
{
  public:
      // Constructor defining which pins to use for CS and INT
    MCP2515(byte CS_Pin, byte INT_Pin);
      
      // Overloaded initialization function
      int Init(int baud, byte freq);
      int Init(int baud, byte freq, byte sjw);
      
      // Basic MCP2515 SPI Command Set
    void Reset();
    byte Read(byte address);
    void Read(byte address, byte data[], byte bytes);
      byte CheckBuffers();
      Frame ReadBuffer(byte buffer);
      void Write(byte address, byte data);
      void Write(byte address, byte data[], byte bytes);
      bool LoadBuffer(byte buffer, Frame message, bool verify = false);
      void SendBuffer(byte buffers);
      byte Status();
      byte RXStatus();
      void BitModify(byte address, byte mask, byte data);

      // Extra functions
      bool Interrupt(); // Expose state of INT pin
      byte GetInterrupt(); // Returns CANINTF Register
      bool ResetInterrupt(byte intSelect); // Resets the interrupt flags specified (use ORed combination of CANINTF flags)
      bool Mode(byte mode); // Returns TRUE if mode change successful
      bool AbortTransmissions(byte timeout = 10); // Aborts any pending transmissions (may experience slight delay due to SPI). Returns false if it times out after timeout ms.
      
  private:
      bool _init(int baud, byte freq, byte sjw, bool autoBaud);
    // Pin variables
      byte _CS;
      byte _INT;
};

#endif
