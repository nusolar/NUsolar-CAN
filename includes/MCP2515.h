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

/** \brief Converts a frame object to a string
 **
 ** Deprecated since frame objects now have their own built in Frame::toString() method.
 */
String frameToString(const Frame&);

/** \brief a class which acts as an interface to the MCP2515 chip.
 **
 ** This class handles all SPI communication with the MCP2515 chip and has methods to both read and load 
 ** the transmit and receive buffers with Frame objects, among other features. 
 **
 ** \todo Merge this class with the CAN_IO class in order to simplify and optimize the library. Requires more thorough testing.
 ** \todo Rename GetInterrupt() to readInterrupts() or something like that.
 */
class MCP2515
{
  public:
    /** \brief Constructor.
        \param CS_pin     The Arduino pin attached to the MCP2515 CS pin.
        \param INT_pin    The Arduino pin attached to the MCP2515 INT pin.
     */
    MCP2515(byte CS_pin, byte INT_pin);
      
    /** \brief Initialization function
     ** \param baud       Baud rate of the CAN bus in kbps. On SC6, this is 1Mbps, or 1000.
     ** \param freq       Frequency of the MCP2515 external oscillator. On NUsolar boards, this is 16Mhz
     */
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
