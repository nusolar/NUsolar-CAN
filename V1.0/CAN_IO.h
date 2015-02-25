/*
 * CAN_IO.h
 * Contains class definition for CAN_IO library.
 */

#ifndef CAN_IO_h
#define CAN_IO_h

#include <stdint.h>
#include "MCP2515.h"
#include "MCP2515_defs.h"
#include "Layouts.h"
#include "RX_Queue.h"

/* 
 *Struct containing the filter info for the rx buffers.
 * Can specify mask and 2 filters for RXB0, and mask and
 * 5 filters for RXB1.
 */
struct CANFilterOpt {
	uint16_t RXM0; // mask 0 (RXB0)
	uint16_t RXM1; // mask 1 (RXB1)
	uint16_t RXF0; // filter 0 (RXB0)
	uint16_t RXF1; // filter 1 (RXB0)
	uint16_t RXF2; // filter 2 (RXB1)
	uint16_t RXF3; // filter 3 (RXB1)
	uint16_t RXF4; // filter 4 (RXB1)
	uint16_t RXF5; // filter 5 (RXB1)

	CANFilterOpt() : RXM0(0), RXM1(0) {} // Initialize to no masking

	void setRB0(uint16_t m0, uint16_t f0, uint16_t f1)
	{
	  RXM0 = m0;
	  RXF0 = f0;
	  RXF1 = f1;
	}
	
	void setRB1(uint16_t m1, uint16_t f2, uint16_t f3, uint16_t f4, uint16_t f5)
	{
	  RXM1 = m1;
	  RXF2 = f2;
	  RXF3 = f3;
	  RXF4 = f4;
	  RXF5 = f5;
	}
};

/*
 * Mask ID that specifically work with our SIDs
 * (packet ID's for f0-f5 can be found in Layouts.h)
 */
#define MASK_NONE			0x000
#define MASK_Sx00			0x700
#define MASK_Sxx0			0x7F0
#define MASK_Sxxx			0x7FF
#define MASK_EID			0x7FFFF


/*
 * Define Extra can errors besides those defined in MCP2515_defs.
  #define RX0IF                  0x01
  #define RX1IF                  0x02
  #define TX0IF                  0x04
  #define TX1IF                  0x08
  #define TX2IF                  0x10
  #define ERRIF                  0x20
  #define WAKIF                  0x40
  #define MERRF                  0x80 
 */
  #define CANERR_SETUP_BAUDFAIL   0x0100 // Failed to set baud rate properly during setup
  #define CANERR_SETUP_MODEFAIL   0x0200 // Failed to switch modes
  #define CANERR_BUFFER_FULL      0x0400 // Local buffer is full
  #define CANERR_MCPBUF_FULL      0x0800 // MCP2515 is reporting buffer overflow errors
  #define CANERR_MESSAGE_ERROR	  0x1000 // Message transmission error 
  #define CANERR_BUSOFF_MODE	  0x2000 // MCP2515 has entered Bus Off mode

/*
 * Class for handling CAN I/O operations using the
 * MCP2515 CAN controller.
 */
class CAN_IO {
public:
	RX_Queue<16> RXbuffer;

	/*
	 * Constructor. Creates a MCP2515 object using
	 * the given pins.
	 */
	CAN_IO(byte CS_pin, byte INT_pin, int baud, byte freq);

	/*
	 * Initializes the CAN controller to desired settings,
	 * including read masks/filters. All types of interrupt
	 * are enabled.
	 */
	void Setup(const CANFilterOpt& filters, uint16_t* errorflags, byte interrupts);
	
	/* Reconfigure the interrupts that are enabled on the MCP2515 */
	bool ConfigureInterrupts(byte interrupts);

	/*
	 * Invoked when the interrupt pin is pulled low. Handles
	 * errors or reads messages, determined by the type of interrupt.
	 */
	void Fetch();

	/*
	 * Sends messages to the CAN bus via the controller.
	 */
	void Send(const Layout& layout, uint8_t buffer);
	void Send(const Frame& frame, uint8_t buffer);
	
	/*
	 * Returns a reference to the next available frame on the buffer
	 */
	inline Frame& Read()
	{
		return RXbuffer.dequeue();
	}
      
	/*
	 * Returns true if the RX buffer is not empty.
	 */
	inline bool Available()
	{
		return !RXbuffer.is_empty();
	}

	

        
    MCP2515 controller; // The MCP2515 object
	
private:
    byte      INT_pin;
	int 	  bus_speed;
	byte	  bus_freq;

	/*
	 * Pointer to a memory space in which we will store errors
	 */
	public: uint16_t* errptr;

	/*
	 * Helper function for configuring the RX masks/filters.
	 * If first is true, sets the mask/filter for the first buffer;
	 * otherwise sets the second.
	 */
	void write_rx_filter(uint8_t address, uint16_t);
};

/*
 * Declare a pointer to the main can_io instance
 */
extern CAN_IO* main_CAN;
#endif
