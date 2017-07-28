/** \file CAN_IO.h
 *  \brief Contains class definition for CAN_IO library.
 */

#ifndef CAN_IO_h
#define CAN_IO_h

#include <stdint.h>
#include "includes/MCP2515.h"
#include "includes/MCP2515_defs.h"
#include "includes/Layouts.h"
#include "includes/RX_Queue.h"

/* 
 * \brief Struct containing the filter info for the rx buffers.
 *
 * The user can specify 1 mask and 2 filters for RXB0, and 1 mask and
 * 5 filters for RXB1.
 *
 * Any message which matches any of the filters on all of the bits where their respective mask contains 
 * a 1 will be accepted. e.g. If 
 * 		RXM0 = 010b
 *		RXF0 = 000b
 * Then messages with IDs matching x0xb will be accepted, where x is either a 0 or a 1.
 */
struct CANFilterOpt {
	uint16_t RXM0; /** mask 0 (RXB0) */
	uint16_t RXM1; /** mask 1 (RXB1) */
	uint16_t RXF0; /** filter 0 (RXB0) */
	uint16_t RXF1; /** filter 1 (RXB0) */
	uint16_t RXF2; /** filter 2 (RXB1) */
	uint16_t RXF3; /** filter 3 (RXB1) */
	uint16_t RXF4; /** filter 4 (RXB1) */
	uint16_t RXF5; /** filter 5 (RXB1) */

	/** Default Constructor. Initialize to No Masking or Filtering */
	CANFilterOpt() : RXM0(0), RXM1(0) {}

	/** Specify the masks and filters for the first read buffer (RB0) 
	 *  \param m0 - binary mask to use
	 *  \param f0 - 1st binary filter to check against
	 *  \param f1 - 2nd binary filter to check against
	 *  \return a reference to the calling object, allowing the setRB0 and setRB1 commands to be chained.
	 */
	CANFilterOpt& setRB0(uint16_t m0, uint16_t f0, uint16_t f1)
	{
	  RXM0 = m0;
	  RXF0 = f0;
	  RXF1 = f1;
	  return *(this); //Allow chaining
	}
	
	/** Specify the masks and filters for the second read buffer (RB1) 
	 *  \param m1 - binary mask to use
	 *  \param f2 - 1st binary filter to check against
	 *  \param f3 - 2nd binary filter to check against
	 *  \param f4 - 3rd binary filter to check against
	 *  \param f5 - 4th binary filter to check against
	 *  \param f6 - 5th binary filter to check against
	 *  \return a reference to the calling object, allowing the setRB0 and setRB1 commands to be chained.
	 */
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


/** \defgroup CANERRORS_GROUP Standard CAN errors 
 ** Some of these only work if FetchErrors() is called periodically.
 ** \TODO where are these found
 **  
 **  @{
 */
  #define CANERR_RX0FULL_OCCURED	0x0001 //!< RX0 buffer received valid message but RX0IF was set (this is cleared automatically, but the flag persists in errors until cleared by the user)
  #define CANERR_RX1FULL_OCCURED	0x0002 //!< RX1 buffer received valid message but RX1IF was set (this is cleared automatically, but the flag persists in errors until cleared by the user)
  #define CANERR_EMPTY_INTERRUPT	0x0004 //!< Interrupt buffer came in empty (probably means SPI is not working, if it is constantly set).
  #define CANERR_SETUP_BAUDFAIL   0x0100 //!< Failed to set baud rate properly during setup (can mean that SPI is wired incorrectly)
  #define CANERR_SETUP_MODEFAIL   0x0200 //!< Failed to switch modes (can mean that SPI is wired incorrectly)
  #define CANERR_RXBUFFER_FULL    0x0400 //!< Local buffer is full
  #define CANERR_MESSAGE_ERROR	  0x1000 //!< Message transmission error 
  #define CANERR_BUSOFF_MODE	  	0x2000 //!< MCP2515 has entered Bus Off mode
  #define CANERR_HIGH_ERROR_COUNT	0x4000 //!< Triggered when TEC or REC exceeds 96
/** @} */

/**
 * \brief Class for handling CAN I/O operations using the MCP2515 CAN controller.
 * 
 * You must create an instance of this object within your Arduino Setup() function, and then 
 */
class CAN_IO {
public:
	/*
	 * Main Constructor. Creates 
	 */
    CAN_IO(byte CS_pin, int baud, byte freq);			    // Constructor if interrupts are not used
	CAN_IO(byte CS_pin, byte INT_pin, int baud, byte freq); // Constructor for using interrupts


	/*
	 * Initializes the CAN controller to desired settings,
	 * including read masks/filters. All types of interrupt
	 * are enabled.
	 */
	void Setup(byte interrupts = RX0IE | RX1IE | TX1IE | TX2IE | TX0IE );

	//inline void AbortTransmissions(byte timeout = 10); not being used right now

	/*
	 * Methods to put the controller to sleep or wake it up again.
	 */
	bool Sleep();
	bool Wake();

	void ResetController();
	
	/* 
	 * Reconfigure the interrupts that are enabled on the MCP2515 
	 * Arguments: - interrupts (OR'd list of INTE flags)
	 */
	bool ConfigureInterrupts(byte interrupts);

	/*
	 * Attaches or detatches the automatic fetch interrupt (not recommended)
	 * Arguments: set (true = attach interrupt pin to the CAN_ISR routine, false = detatch interrupt from the interrupt pin [default])
	 */
	void setAutoFetch(bool set);

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
	bool Send(const Layout& layout, uint8_t buffer);
	bool Send(const Frame& frame, uint8_t buffer);
	bool SendVerified(const Layout& layout, uint8_t buffer);
	bool SendVerified(const Frame& frame, uint8_t buffer);
	
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

	//store filters
	CANFilterOpt filters;
	
private:
  byte    INT_pin;
	int 	  bus_speed;
	byte	  bus_freq;
	volatile byte 		tx_open;	// Tracks which TX buffers are open.

	// Store interrupts in case we have to reset
	byte my_interrupts;

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

/**
 ** Declare a pointer to the main can_io instance. We need this because interrupt functions
 ** can't have arguments. We can't assign CAN_IO::Fetch() to the ISR because it has an implicit
 ** this* pointer which goes to the specific instance. If you want multiple can controllers,
 ** you wll need to set up your own interrupts for them.
 */
extern CAN_IO* main_CAN;
#endif
