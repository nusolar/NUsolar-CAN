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

#define VERIFY true // Optional argument to both CAN_IO::Send() forcing them to verify the data loaded on the MCP2515 is correct before sending, and fail if it isn't.

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

	CANFilterOpt& setRB0(uint16_t m0, uint16_t f0, uint16_t f1)
	{
	  RXM0 = m0;
	  RXF0 = f0;
	  RXF1 = f1;
	  return *(this); //Allow chaining
	}
	
	CANFilterOpt& setRB1(uint16_t m1, uint16_t f2, uint16_t f3, uint16_t f4, uint16_t f5)
	{
	  RXM1 = m1;
	  RXF2 = f2;
	  RXF3 = f3;
	  RXF4 = f4;
	  RXF5 = f5;
	  return *(this); //Allow chaining
	}
};


/*
 * Define CAN errors
 * Some of these only work if FetchErrors() is called periodically.
 * See error data variables for details.
 */
 	#define CANERR_RX0FULL_OCCURED	0x0001 // RX0 buffer received valid message but RX0IF was set (this is cleared automatically, but the flag persists in errors until cleared by the user)
  #define CANERR_RX1FULL_OCCURED	0x0002 // RX1 buffer received valid message but RX1IF was set (this is cleared automatically, but the flag persists in errors until cleared by the user)
  #define CANERR_EMPTY_INTERRUPT	0x0004 // Interrupt buffer came in empty (probably means SPI is not working, if it is constantly set).
  #define CANERR_SETUP_BAUDFAIL   0x0100 // Failed to set baud rate properly during setup (can mean that SPI is wired incorrectly)
  #define CANERR_SETUP_MODEFAIL   0x0200 // Failed to switch modes (can mean that SPI is wired incorrectly)
  #define CANERR_RXBUFFER_FULL    0x0400 // Local buffer is full
  #define CANERR_MESSAGE_ERROR	  0x1000 // Message transmission error 
  #define CANERR_BUSOFF_MODE	  	0x2000 // MCP2515 has entered Bus Off mode
  #define CANERR_HIGH_ERROR_COUNT	0x4000 // Triggered when TEC or REC exceeds 96

/*
 * Class for handling CAN I/O operations using the
 * MCP2515 CAN controller.
 */
class CAN_IO {
public:
	/*
	 * Constructor. Creates a MCP2515 object using
	 * the given pins.
	 */
	CAN_IO(byte CS_pin, byte INT_pin, int baud, byte freq); // Constructor for using interrupts
	CAN_IO(byte CS_pin, int baud, byte freq);								// Constructor if interrupts are not used

	/*
	 * Initializes the CAN controller to desired settings,
	 * including read masks/filters. All types of interrupt
	 * are enabled.
	 */
	void Setup(const CANFilterOpt& filters, byte interrupts = RX0IE | RX1IE | TX1IE | TX2IE | TX0IE );

	//inline void AbortTransmissions(byte timeout = 10); not being used right now

	/*
	 * Methods to put the controller to sleep or wake it up again.
	 */
	bool Sleep();
	bool Wake();

	void ResetController();
	
	/* Reconfigure the interrupts that are enabled on the MCP2515 */
	bool ConfigureInterrupts(byte interrupts);

	/*
	 * Invoked when the interrupt pin is pulled low. Handles
	 * errors or reads messages, determined by the type of interrupt.
	 */
	void Fetch();

	/*
	 * Get and parse the error flag bit from the MCP2515 into my variables errors, tec, and rec.
	 */
	void FetchErrors();

	/*
	 * Get the contents of CANSTAT register, which can be used to tell the operational state of the device.
	 */
	void FetchStatus();

	/*
	 * Sends messages to the CAN bus via the controller.
	 */
	bool Send(const Layout& layout, uint8_t buffer, bool verify = false);
	bool Send(const Frame& frame, uint8_t buffer, bool verify = false);
	
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
   	RX_Queue<16> RXbuffer; //A queue for holding incoming messages

    // Status data
    volatile uint8_t  canstat_register;
    
    // Error data 
    // Note that these are only updated when FetchErrors() is called.
    // You can make this automatic by enabling the ERRIE interrupt when setting up can
    // but this can cause arduino to freeze, so you do so at your own risk.
		volatile uint32_t	errors;
		volatile uint32_t	tec;
		volatile uint32_t rec;
		volatile long int_counter; // increments when an interrupt happens (always updated)
		volatile uint8_t  last_interrupt;
	
private:
  byte    INT_pin;
	int 	  bus_speed;
	byte	  bus_freq;
	volatile byte 		tx_open;	// Tracks which TX buffers are open.

	//store filters and interrupts for reset
	CANFilterOpt my_filters;
	byte my_interrupts;
	bool enable_interrupts; // Sets (by constructor) whether interrupts are used to fetch messages.

	/*
	 * Helper function for configuring the RX masks/filters.
	 * If first is true, sets the mask/filter for the first buffer;
	 * otherwise sets the second.
	 */
	void write_rx_filter(uint8_t address, uint16_t);
	
	inline void init_controller();

	/*
	 * Helper function to select a TX buffer
	 */
	inline uint8_t select_open_buffer();
};

/*
 * Declare a pointer to the main can_io instance. We need this because interrupt functions
 * can't have arguments. We can't assign CAN_IO::Fetch() to the ISR because it has an implicit
 * this* pointer which goes to the specific instance. If you want multiple can controllers,
 * you will need to set up your own interrupts for them.
 */
extern CAN_IO* main_CAN;
#endif
