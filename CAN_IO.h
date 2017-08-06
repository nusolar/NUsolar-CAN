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

/**
 ** \brief A special struct to hold filter info for the MCP2515 \ref MCP2515_defs.h "RX buffers".
 **
 ** The user can specify 1 mask and 2 filters for RXB0, and 1 mask and
 ** 5 filters for RXB1.
 **
 ** Any message which matches any of the filters on all of the bits where their respective mask contains 
 ** a 1 will be accepted. e.g. If 
 ** 		RXM0 = 010b
 **		RXF0 = 000b
 ** Then messages with IDs matching x0xb will be accepted, where x is either a 0 or a 1.
 */
 /// \todo (Code) Incorporate the 8 filter and mask variables directly into the CAN_IO object, or at least to 
 ///		      an annonymous struct within CAN_IO, and move the setRB0 and setRB1 functions to within the CAN_IO 
 ///		      class as well. Then the user can do the following \n 
 ///			     CAN_IO CanControl(args); \n
 ///			 	 CanControl.setRB0(args); \n
 ///				 CanControl.setRB1(args); \n
 ///				 CanControl.Setup(args); \n
 ///		 	  or they can call CAN_IO::Setup() first and then set the filters and call some new set_filters() method.

struct CANFilterOpt {
	uint16_t RXM0; //!< mask 0 (applies to RXB0)
	uint16_t RXM1; //!< mask 1 (applies to RXB1)
	uint16_t RXF0; //!< filter 0 (applies to RXB0)
	uint16_t RXF1; //!< filter 1 (applies to RXB0)
	uint16_t RXF2; //!< filter 2 (applies to RXB1)
	uint16_t RXF3; //!< filter 3 (applies to RXB1)
	uint16_t RXF4; //!< filter 4 (applies to RXB1)
	uint16_t RXF5; //!< filter 5 (applies to RXB1)

	/** Default Constructor. Initialize to accept all packets (no filtering). */
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
 ** \todo where are these found
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
 ** \brief Class for handling CAN I/O operations using the MCP2515 CAN controller.
 ** 
 ** You must create an instance of this object within your Arduino Setup() function, and then call its \ref CAN_IO::Setup() "Setup()" function to
 ** initialize the MCP2515 controller to the proper settings.
 */
class CAN_IO {
public:
	/**
	 ** \brief Constructor
	 ** \param[in] CS_pin 	The arduino pin # to which the MCP2515 chip select pin is attached.
	 ** \param[in] INT_pin	The arduino pin # to which the MCP2515 interrupt pin is attached. This should be specified
	 ** 				even if \ref _____ "AutoFetch" is disabled, as the CAN_IO::Fetch() routine uses this pin to check whether
	 **					there are incoming messages.)
	 ** \param[in] baud 	Baud rate of the CAN bus, in kbps. SC6 runs at 1mbps, or baud = 1000.
	 ** \param[in] freq 	Frequency of the MCP2515 external resonator, in Mhz. For NUsolar boards on SC6, this is 16Mhz or freq = 16.
	 **/
	CAN_IO(byte CS_pin, byte INT_pin, int baud, byte freq); // Constructor


	/** \brief Initialization method for the can controller
	 **
	 ** Initializes the CAN controller to desired settings,
	 ** including read masks/filters. All types of interrupt
	 ** are enabled by default.
	 **
	 ** \param[in] interrupts   Binary interrupt enable flags. The flags can be combined by ORing them together
	 **                     	using the | operator. By default, the following interrupts are enabled: RX0IE, RX1IE, TX0IE, TX1IE, TX2IE.
	 */
	void Setup(byte interrupts = RX0IE | RX1IE | TX1IE | TX2IE | TX0IE );

	bool Sleep(); 				//!< Put the controller to sleep. See the MCP2515 datasheet.
	bool Wake();  				//!< Wake the controller up from sleep. See the MCP2515 datasheet.
	void ResetController();		//!< Reinitialize the controller by calling MCP2515::Init() again.
	
	/** 
	 ** \brief Reconfigure the interrupts that are enabled on the MCP2515 
	 ** \param[in] interrupts 	Binary interrupt enable flags (see CAN_IO::Setup())
	 ** \return 				True if successful
	 */
	bool ConfigureInterrupts(byte interrupts);

	/**
	 ** \brief Toggles AutoFetch on and off.
	 ** \param[in] set 		true = attach interrupt pin to the CAN_ISR routine, 
	 						false = detatch interrupt from the interrupt pin [default]

	 ** AutoFetch is off by default because the interrupt feature it uses causes problems when
	 ** running at the same time as SPI. We were observing corrupted CAN packets being sent when 
	 ** AutoFetch was enabled.
	 */
	void setAutoFetch(bool set);

	/**
	 ** \brief Fetch any available messages from the MCP2515
	 **
	 ** Reads messages from the MCP2515 over SPI and stores them in the internal RX_Queue buffer as Frame
	 ** objects until the user calls CAN_IO::Read().
	 */
	void Fetch();

	/**
	 ** \brief Fetch the error status of the MCP2515 over SPI and update internal variables.
	 **
	 ** Makes reads the contents of the TEC, REC, and EFLG registers on the MCP2515.
	 **
	 ** TEC and REC each represent an integer count of the number of transmission and receive errors
	 ** and get stored directly in the tec and rec local variables.
	 **
	 ** EFLG is used to set the appropriate error flags in the local errors variable.
	 */
	void FetchErrors();

	/**
	 ** \brief Fetch the contents of \ref CANSTAT register
	 **
	 ** A copy of the register is storred in canstat_register. This can be used to determine
	 **   - The current mode of the MCP2515
	 **   - 
	 **
	 ** \todo Finish documenting \ref CANSTAT register.
	 */
	void FetchStatus();

	/** \brief Sends messages to the CAN bus via the controller. 
	 ** \param[in] layout 		Constant reference to a Layout object.
	 ** \param[in] buffer 		Specify the transmit buffer to use on the MCP2515. Possible values are: TXB0, TXB1, TXB2, TXBANY (i.e. automatically chosen)
	 ** \return 				True if successful
	 ** 
	 ** Sample sending a DC_Heartbeat packet using a right-hand constant reference through any TX buffer:
	 ** \code{.cpp} CAN_IO::Send(DC_Heartbeat(0x1234,0x5678), TXANY); \endcode
	 **
	 ** Sample sending a DC_Heartbeat packet using a standard object reference through any TX buffer:
	 ** \code{.cpp} DC_Heartbeat p = DC_Heartbeat(0x1234,0x5678); CAN_IO::Send(p, TXANY); 
	    \endcode
	 */
	bool Send(const Layout& layout, uint8_t buffer);

	/** \brief Sends messages to the CAN bus via the controller. 
	 ** \param[in] frame 		Constant reference to a Frame object
	 ** \param[in] buffer  		Specify the transmit buffer to use on the MCP2515. Possible values are: TXB0, TXB1, TXB2, TXBANY (i.e. automatically chosen)
	 ** \return 				True if successful
	 ** 
	 ** Similar to the Layout-based send function but accepts Frame object instead.
	 */
	bool Send(const Frame& frame, uint8_t buffer);

	/** \brief Sends messages to the CAN bus via the controller, confirming correct SPI transmission.
	 ** \param[in] layout 		Constant reference to a Layout object.
	 ** \param[in] buffer 		Specify the transmit buffer to use on the MCP2515. Possible values are: TXB0, TXB1, TXB2, TXBANY (i.e. automatically chosen)
	 ** \return 				True if successful
	 ** 
	 ** Same usage as the corresponding Send() function. However, after loading a message into the MCP2515 TX buffers, 
	 ** this function reads the loaded message back to confirm that it is not corrupted.
	 */
	bool SendVerified(const Layout& layout, uint8_t buffer);

	/** \brief Sends messages to the CAN bus via the controller, confirming correct SPI transmission.
	 ** \param[in] frame 		Constant reference to a Frame object
	 ** \param[in] buffer  		Specify the transmit buffer to use on the MCP2515. Possible values are: TXB0, TXB1, TXB2, TXBANY (i.e. automatically chosen)
	 ** \return 				True if successful
	 ** 
	 ** Same usage as the corresponding Send() function. However, after loading a message into the MCP2515 TX buffers, 
	 ** this function reads the loaded message back to confirm that it is not corrupted.
	 */
	bool SendVerified(const Frame& frame, uint8_t buffer);
	
	/**
	 ** \brief Reads the next packet received from the CAN bus.
	 ** \return A Frame& object which can be used to access the next object in the internal RX_Queue.
	 */
	inline Frame& Read()
	{
		return RXbuffer.dequeue();
	}
      
	/**
	 ** \brief Check if any incoming CAN packets are available.
	 ** \return True if the internal RX_Queue which holds incoming packets is not empty.
	 */
	inline bool Available()
	{
		return !RXbuffer.is_empty();
	}

        
    MCP2515 controller; 	//!< An instance of the MCP2515 class allowing easy access of MCP2515 functions over SPI.
   	RX_Queue<16> RXbuffer;	//!< A local queue for holding incoming message frames as they are retreived by the Fetch function. Can hold up to 16 frames at a time.

    // Status variables
    volatile uint8_t    canstat_register;	//!< Holds a copy of the \ref CANSTAT register.
    										/**< Updated when FetchErrors() is called.
    										     \todo Rename this to be consistent with the errors variable. */

	volatile uint32_t	errors;				//!< Bitmap of the errors on the CAN bus. 
											/**< Bits correspond to the \ref CANERRORS_GROUP "Standard CAN errors". Updated when FetchErrors() is called. */

	volatile uint32_t	tec;				//!< Transmission Error Counter
											/**< Integer value corresponding to the # of failed transmission attempts. The MCP2515 goes into BUS PASSIVE mode when
											 **  this value exceeds 127, and goes into BUS OFF mode when it exceeds 255. */

	volatile uint32_t 	rec;				//!< Receive Error Counter
											/**< Integer value corresponding to the # of message errors (when receiving). The MCP2515 goes into BUS PASSIVE mode when
											 **  this value exceeds 127, and goes into BUS OFF mode when it exceeds 255. */

	volatile long 		int_counter; 		//!< Increments every time an interrupt from the MCP2515 comes in
											/*!< Does nothing when AutoFetch is disabled (default behavior). */

	volatile uint8_t  	last_interrupt;		//!< Stores the return value of GetInterrupt() every time Fetch() is called.
											/**< This allows the user to see what the interrupt status was even after Fetch() clears
											 **  the register */

	CANFilterOpt filters;					//!< Stores the filters currently in use by the MCP2515
	
private:
    byte     		INT_pin;				//!< Pin which is attached to the MCP2515 interrupt pin
	int 	 		bus_speed;				//!< Configured bus speed (e.g. 500 kbps, 1mbps, ...)
	byte	 		bus_freq;				//!< MCP2515 oscillator frequency (16MHz on all NUsolar boards)
	volatile byte 	tx_open;				//!< Tracks which TX buffers are open.

	byte     my_interrupts;					//!< Stores the currently-active interrupts enable flags.

	/** 
	 ** \brief Helper function for configuring the RX masks/filters.
	 **
	 ** If first is true, sets the mask/filter for the first buffer;
	 ** otherwise sets the second.
	 */
	void write_rx_filter(uint8_t address, uint16_t);
	
	inline void init_controller();

	/**
	 ** \brief Helper function to select a TX buffer automatically.
	 **
	 ** This method is called by Send() or SendVerified() whenever the user specifies TXANY as the transmission buffer argument.
	 */
	inline uint8_t select_open_buffer();
};

/** 
 ** \brief A pointer to the main can_io instance used for calling the CAN_IO instance when AutoFetch is enabled.
 **
 ** We need this because interrupt functions can't have arguments. We can't assign CAN_IO::Fetch() to the ISR because it has an implicit
 ** this* pointer which goes to the specific instance. If you want multiple can controllers,
 ** you wll need to set up your own interrupts for them.
 **
 ** Note that interrupts are not currently enabled by default, so this pointer is useless. See \ref CAN_ISR()
 */
extern CAN_IO* main_CAN;
#endif
