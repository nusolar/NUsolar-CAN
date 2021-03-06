/*
 * CAN_IO.cpp
 * Implementation of CAN_IO class.
 */

#include "CAN_IO.h"
#include <SPI.h>

CAN_IO::CAN_IO(byte CS_pin, byte INT_p, int baud, byte freq) : INT_pin(INT_p), controller(CS_pin, INT_p), bus_speed(baud), bus_freq(freq),
															   tec(0), rec(0), errors(0) {}

/*
 * Define global interrupt function
 */
void CAN_ISR()
{
	main_CAN->Fetch();
	main_CAN->int_counter++;
}
// Make sure to initialize the mainCAN pointer to 0 here.
CAN_IO *main_CAN = 0;

/*
 * Setup function for CAN_IO. Arguments are a FilterInfo struct and a pointer to a place to raise error flags.
 */
void CAN_IO::Setup(byte interrupts)
{ // default interrupts are RX0IE | RX1IE | TX1IE | TX2IE | TX0IE.
	// SPI setup
	SPI.setClockDivider(10);
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.begin();

	// reset tx tracker
	tx_open = 0x07;

	// Set as main can
	main_CAN = this;

	pinMode(INT_pin, INPUT_PULLUP);

	// Copy filters and interrupts to internal variables
	this->my_interrupts = interrupts;

	// init the controller
	init_controller(); //private helper function
}

inline void CAN_IO::init_controller() //private helper function
{
	// Clear error counters
	this->errors = 0;
	this->tec = 0;
	this->rec = 0;

	int baudRate = controller.Init(this->bus_speed, this->bus_freq, 1); //SJW of 1
	if (baudRate <= 0)
	{ // error
		this->errors |= CANERR_SETUP_BAUDFAIL;
		if (Serial)
			Serial.println(F("Baud ERROR"));
	}

	// return controller to config mode
	if (!controller.Mode(MODE_CONFIG))
	{ // error
		this->errors |= CANERR_SETUP_MODEFAIL;
		if (Serial)
			Serial.println(F("Mode ERROR"));
	}

	// disable interrupts we don't care about
	controller.Write(CANINTE, this->my_interrupts);

	// config RX masks/filters
	write_rx_filter(RXM0SIDH, this->filters.RXM0, this->filters.eidM0);
	write_rx_filter(RXF1SIDH, this->filters.RXF1, this->filters.eidM0);
	write_rx_filter(RXF2SIDH, this->filters.RXF2, this->filters.eidM0);

	write_rx_filter(RXM1SIDH, this->filters.RXM1, this->filters.eidM1);
	write_rx_filter(RXF3SIDH, this->filters.RXF3, this->filters.eidM1);
	write_rx_filter(RXF4SIDH, this->filters.RXF4, this->filters.eidM1);
	write_rx_filter(RXF5SIDH, this->filters.RXF5, this->filters.eidM1);
	write_rx_filter(RXF0SIDH, this->filters.RXF0, this->filters.eidM1);

	// return controller to normal mode
	if (!controller.Mode(MODE_NORMAL))
	{ // error
		this->errors |= CANERR_SETUP_MODEFAIL;
	}
}

bool CAN_IO::Sleep()
{
	return controller.Mode(MODE_SLEEP);
}

bool CAN_IO::Wake()
{

	controller.BitModify(CANINTF, WAKIF, WAKIF); // Set the WAKEIF bit to request that the controller wake up.
	noInterrupts();
	delayMicroseconds(5000); //Wait for it to run the start-up timer
	interrupts();
	return controller.Mode(MODE_NORMAL); // The device wakes up in listen-only mode. Put it back in normal (we may have to clear the TX registers)
}

void CAN_IO::ResetController()
{
	this->init_controller(); // Re-initialize the controller.
}

void CAN_IO::Fetch()
{
	// read status of CANINTF register
	if (!controller.Interrupt())
		return; // Do nothing if there is not an interrupt

	byte interrupt = controller.GetInterrupt(); // Otherwise get the interrupt from the controller and process it.
	byte to_clear = 0;
	//Serial.print("FETCH ");
	//Serial.println(interrupt,BIN);

	if (interrupt == 0)
	{
		this->errors |= CANERR_EMPTY_INTERRUPT;
	}
	else
	{
		this->errors &= ~CANERR_EMPTY_INTERRUPT;
		this->last_interrupt = interrupt;

		// Note: Not all interrupts may be enabled. We add all the important ones here in case
		// you want to use them. Enabling interrupts other than the RXnIF interrupts may cause
		// certain microcontrollers *cough* arduino *cough* to freeze if a bus error happens.
		// It is recommended that only the RXnIF interrupts be enabled and the rest of the can
		// be read by periodically calling FetchErrors().

		// Get Messages
		if (interrupt & (RX0IF | RX1IF))
		{
			if (interrupt & RX1IF)
			{ // receive buffer 1 full
				RXbuffer.enqueue(controller.ReadBuffer(RXB1));
				to_clear |= RX1IF;
			}

			if (interrupt & RX0IF)
			{ // receive buffer 0 full
				RXbuffer.enqueue(controller.ReadBuffer(RXB0));
				to_clear |= RX0IF;
			}

			if (RXbuffer.is_full())
				errors |= CANERR_RXBUFFER_FULL;
			else
				errors &= ~CANERR_RXBUFFER_FULL;
		}

		// Handle any other interrupts that might be flagged.
		if (interrupt & MERRF)
		{ // message error
			this->errors |= CANERR_MESSAGE_ERROR;
			to_clear |= MERRF;
		}
		else
			this->errors &= (~CANERR_MESSAGE_ERROR);

		if (interrupt & WAKIF)
		{ // wake-up interrupt
			// No Error implemented
			to_clear |= WAKIF;
		}

		if (interrupt & ERRIF)
		{ // error interrupt
			this->FetchErrors();
			to_clear |= ERRIF;
		}

		if (interrupt & TX2IF)
		{ // transmit buffer 2 empty
			tx_open |= TXB2;
			to_clear |= TX2IF;
		}

		if (interrupt & TX1IF)
		{ // transmit buffer 1 empty
			tx_open |= TXB1;
			to_clear |= TX1IF;
		}

		if (interrupt & TX0IF)
		{ // transmit buffer 0 empty
			tx_open |= TXB0;
			to_clear |= TX0IF;
		}
	}

	// clear interrupt
	controller.ResetInterrupt(to_clear); // reset all interrupts
}

void CAN_IO::FetchErrors()
{
	this->tec = controller.Read(TEC);
	this->rec = controller.Read(REC);

	byte eflg = controller.Read(EFLG);

	if (eflg & 0x01) // If EWARN flag is set
	{
		if (eflg & 0x20) // if busmode flag is set
			this->errors |= CANERR_BUSOFF_MODE;
		else
			this->errors &= (~CANERR_BUSOFF_MODE);

		if (this->tec > 135 || this->rec > 135 || errors & CANERR_BUSOFF_MODE) // If any TX/RX errors have occured, raise this flag.
			this->errors |= CANERR_HIGH_ERROR_COUNT;
		else
			this->errors &= ~CANERR_HIGH_ERROR_COUNT;

		// Receive errors
		if (eflg & 0x40) // if RX0OVR
			this->errors |= CANERR_RX0FULL_OCCURED;
		else
			this->errors &= ~CANERR_RX0FULL_OCCURED;

		if (eflg & 0x80) // if RX1OVR
			this->errors |= CANERR_RX1FULL_OCCURED;
		else
			this->errors &= ~CANERR_RX1FULL_OCCURED;

		if (eflg & 0xC0)							// if RXnOVR
			controller.BitModify(EFLG, 0xC0, 0x00); // Clear RXnOVR bits
	}
	else
		errors &= ~(CANERR_HIGH_ERROR_COUNT & CANERR_BUSOFF_MODE & CANERR_RX0FULL_OCCURED & CANERR_RX1FULL_OCCURED);
}

void CAN_IO::FetchStatus()
{
	this->canstat_register = controller.Read(CANSTAT);
}


bool CAN_IO::SendVerified(const Layout &layout, uint8_t buffer)
{
	// The TXBANY buffer can be specified to allow the program to choose which buffer to send from.
	// The TXnIE interrupt flags should be enabled for this to work properly.
	if (buffer == TXBANY)
	{
		buffer = select_open_buffer();
	}
	if (buffer == 0x00)
	{
		return false;
	} // Fail

	if (!controller.LoadBuffer(buffer, layout.generate_frame(), true))
	{
		Serial.println(F("LOAD FAILED"));
		return false;
	}
	else
		controller.SendBuffer(buffer);

	//set a flag in the tx_open bitfield that this buffer is closed.
	//It will clear on the first interrupt received after the buffer finishes sending
	//For best performance, enable all TXnIE flags.
	tx_open &= ~buffer;
	return true;
} 

bool CAN_IO::SendVerified(const Frame &frame, uint8_t buffer)
{
	// The TXBANY buffer can be specified to allow the program to choose which buffer to send from.
	// The TXnIE interrupt flags should be enabled for this to work properly.
	if (buffer == TXBANY)
	{
		buffer = select_open_buffer();
	}
	if (buffer == 0x00)
	{
		return false;
	} // Fail

	if (!controller.LoadBuffer(buffer, frame, true))
	{
		Serial.println(F("LOAD FAILED"));
		return false;
	}
	else
		controller.SendBuffer(buffer);

	//set a flag in the tx_open bitfield that this buffer is closed.
	//It will clear on the first interrupt received after the buffer finishes sending
	//For best performance, enable all TXnIE flags.
	tx_open &= ~buffer;
	return true;
}


bool CAN_IO::Send(const Layout &layout, uint8_t buffer)
{
	// The TXBANY buffer can be specified to allow the program to choose which buffer to send from.
	// The TXnIE interrupt flags should be enabled for this to work properly.
	if (buffer == TXBANY)
	{
		buffer = select_open_buffer();
	}
	if (buffer == 0x00)
	{
		return false;
	} // Fail

	controller.LoadBuffer(buffer, layout.generate_frame());
	controller.SendBuffer(buffer);

	//set a flag in the tx_open bitfield that this buffer is closed.
	//It will clear on the first interrupt received after the buffer finishes sending
	//For best performance, enable all TXnIE flags.
	tx_open &= ~buffer;
	return true;
} 

bool CAN_IO::Send(const Frame &frame, uint8_t buffer)
{
	// The TXBANY buffer can be specified to allow the program to choose which buffer to send from.
	// The TXnIE interrupt flags should be enabled for this to work properly.
	if (buffer == TXBANY)
	{
		buffer = select_open_buffer();
	}
	if (buffer == 0x00)
	{
		return false;
	} // Fail

	controller.LoadBuffer(buffer, frame);
	controller.SendBuffer(buffer);

	//set a flag in the tx_open bitfield that this buffer is closed.
	//It will clear on the first interrupt received after the buffer finishes sending
	//For best performance, enable all TXnIE flags.
	tx_open &= ~buffer;
	return true;
}
// RX filters for Standard IDs (SID)
// Define two macros for the following function, to improve readability.
void CAN_IO::write_rx_filter(uint8_t address, uint16_t data)
{
	// FOR LEGACY, GENERATES SID
	write_rx_filter(address, data, false);
}

#define B1_S(value) uint8_t((value >> 3) & 0x00FF) //11111111
#define B2_S(value) uint8_t((value << 5) & 0x00E0) //11100000

#define B1_E(value) uint8_t((value >> (29 - 8)) & 0x00FF)
// For extended case (EID), need to set EID flag
#define B2_E(value) uint8_t((value >> (29 - 16) & B11100000) | B00001000 | (value >> (29-13)) & B111)
#define B3_E(value) uint8_t((value >> (29 - 21)) & 0x00FF)
#define B4_E(value) uint8_t(value & 0x00FF)
void CAN_IO::write_rx_filter(uint8_t address, uint32_t data, bool eid)
{
	uint8_t bytes[4] = {}; // initialize to all 0s
	if (eid) // Extended ID
	{
		bytes[0] = B1_E(data);
		bytes[1] = B2_E(data);
		bytes[2] = B3_E(data);
		bytes[3] = B4_E(data);
	}
	else
	{
		bytes[0] = B1_S(data);
		bytes[1] = B2_S(data);
	}
	controller.Write(address, bytes, 4);
}

// For writing mask of EIDs
void CAN_IO::write_rx_mask(uint8_t address, uint32_t data, bool eid)
{
	uint8_t bytes[4] = {}; // initialize to all 0s
	if (eid)
	{
		bytes[0] = B1_E(data);
		bytes[1] = B2_E(data);
		bytes[2] = B3_E(data);
		bytes[3] = B4_E(data);

		// Maks do not have EID flag
		bytes[1] = uint8_t(bytes[1] & ~B00001000);
	}
	else // Same procedure as RX Filter
	{
		bytes[0] = B1_S(data);
		bytes[1] = B2_S(data);
	}
	controller.Write(address, bytes, 4);
}

inline uint8_t CAN_IO::select_open_buffer()
{
	if (this->tx_open & TXB0)
		return TXB0;
	else if (this->tx_open & TXB1)
		return TXB1;
	else if (this->tx_open & TXB2)
		return TXB2;
	else
		return 0x00; //Failure
}

bool CAN_IO::ConfigureInterrupts(byte interrupts)
{

	if (controller.Mode(MODE_CONFIG))
	{
		controller.Write(CANINTE, interrupts);
		if (controller.Mode(MODE_NORMAL))
		{
			my_interrupts = interrupts;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

void CAN_IO::setAutoFetch(bool set)
{
	if (set)
		attachInterrupt(INT_pin, CAN_ISR, LOW);
	else
		detachInterrupt(INT_pin);
}

#undef first_byte
#undef second_byte
