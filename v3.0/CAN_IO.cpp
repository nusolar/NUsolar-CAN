/*
 * CAN_IO.cpp
 * Implementation of CAN_IO class.
 */

#include "CAN_IO.h"
#include <SPI.h>

CAN_IO::CAN_IO(byte CS_pin, byte INT_p, int baud, byte freq) : INT_pin(INT_p), controller(CS_pin, INT_p), bus_speed(baud), bus_freq(freq),
tec(0), rec(0), errors(0)  {}

/*
 * Define global interrupt function
 */
void CAN_ISR()
{
  main_CAN->Fetch();
  main_CAN->int_counter++;
}
// Make sure to initialize the mainCAN pointer to 0 here.
CAN_IO* main_CAN = 0;

/*
 * Setup function for CAN_IO. Arguments are a FilterInfo struct and a pointer to a place to raise error flags.
 */
void CAN_IO::Setup(const CANFilterOpt& filters, byte interrupts) {
	// SPI setup
	SPI.setClockDivider(10);
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.begin();

	// Clear error counters
	errors = 0;
	tec = 0;
	rec = 0;

	// Set as main can
	main_CAN = this;

	pinMode(INT_pin,INPUT_PULLUP);	
	attachInterrupt(INT_pin,CAN_ISR,LOW);

	// init the controller
	int baudRate = controller.Init(bus_speed, bus_freq);
	if (baudRate <= 0) { // error
		errors |= CANERR_SETUP_BAUDFAIL;
		if (Serial) Serial.println("Baud ERROR");
	}

	// return controller to config mode
	if (!controller.Mode(MODE_CONFIG)) { // error
		errors |= CANERR_SETUP_MODEFAIL;
		if (Serial) Serial.println("Mode ERROR");
	}

	// disable interrupts we don't care about
	controller.Write(CANINTE, interrupts);
	this->my_interrupts = interrupts;

	// config RX masks/filters
	this->my_filters = filters;
	write_rx_filter(RXM0SIDH, filters.RXM0);
	write_rx_filter(RXM1SIDH, filters.RXM1);
	write_rx_filter(RXF0SIDH, filters.RXF0);
	write_rx_filter(RXF1SIDH, filters.RXF1);
	write_rx_filter(RXF2SIDH, filters.RXF2);
	write_rx_filter(RXF3SIDH, filters.RXF3);
	write_rx_filter(RXF4SIDH, filters.RXF4);
	write_rx_filter(RXF5SIDH, filters.RXF5);

	// return controller to normal mode
	if (!controller.Mode(MODE_NORMAL)) { // error
			errors |= CANERR_SETUP_MODEFAIL;
	}
}

void CAN_IO::ResetController() {
	this->Setup(this->my_filters,this->my_interrupts);
}

void CAN_IO::Fetch() {
	// read status of CANINTF register
	byte interrupt = controller.GetInterrupt();

	// Note: Not all interrupts may be enabled. We add all the important ones here in case
	// you want to use them. Enabling interrupts other than the RXnIF interrupts may cause
	// certain microcontrollers *cough* arduino *cough* to freeze if a bus error happens.
	// It is recommended that only the RXnIF interrupts be enabled and the rest of the can
	// be read by periodically calling FetchErrors().

	if (interrupt & RX1IF) { // receive buffer 1 full
		RXbuffer.enqueue(controller.ReadBuffer(RXB1));
	}

	if (interrupt & RX0IF) { // receive buffer 0 full
		RXbuffer.enqueue(controller.ReadBuffer(RXB0));
	}

	if (interrupt & MERRF) { // message error
		this->errors |= CANERR_MESSAGE_ERROR;
	}
	else
		this->errors &= (~CANERR_MESSAGE_ERROR);

	if (interrupt & WAKIF) { // wake-up interrupt
		// No Error implemented
	}

	if (interrupt & ERRIF) { // error interrupt
		this->FetchErrors();
	}

	if (interrupt & TX2IF) { // transmit buffer 2 empty
		// No Error implemented
	}

	if (interrupt & TX1IF) { // transmit buffer 1 empty
		// No Error implemented
	}

	if (interrupt & TX0IF) { // transmit buffer 0 empty
		// No Error implemented
	}

	// clear interrupt
	controller.ResetInterrupt(INTALL); // reset all interrupts
}

void CAN_IO::FetchErrors() {
	this->tec = controller.Read(TEC);
	this->rec = controller.Read(REC);

	byte eflg = controller.Read(EFLG);

	if (eflg & 0x01) // If EWARN flag is set
	{
		if (eflg & 0x20) // if busmode flag is set 
			this->errors |= CANERR_BUSOFF_MODE;
		else
			this->errors &= (~CANERR_BUSOFF_MODE);

		if (this->tec > 135 || this->rec > 135 
			|| errors & CANERR_BUSOFF_MODE) // If any TX/RX errors have occured, raise this flag.
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

		if (eflg & 0xC0) // if RXnOVR
			controller.BitModify(EFLG,0xC0,0x00); // Clear RXnOVR bits
	}
	else
		errors &= ~(CANERR_HIGH_ERROR_COUNT & CANERR_BUSOFF_MODE & CANERR_RX0FULL_OCCURED & CANERR_RX1FULL_OCCURED);
}

void CAN_IO::Send(const Layout& layout, uint8_t buffer) {
	controller.LoadBuffer(buffer, layout.generate_frame());
	controller.SendBuffer(buffer);
}

void CAN_IO::Send(const Frame& frame, uint8_t buffer) {
	controller.LoadBuffer(buffer, frame);
	controller.SendBuffer(buffer);
}

// Define two macros for the following function, to improve readability.
#define first_byte(value) uint8_t((value >> 3) & 0x00FF)
#define second_byte(value) uint8_t((value << 5) & 0x00E0)

void CAN_IO::write_rx_filter(uint8_t address, uint16_t data) {
	uint8_t bytes[2] = { first_byte(data), second_byte(data) };
	controller.Write(address, bytes, 2);
}

bool CAN_IO::ConfigureInterrupts(byte interrupts)
{
	if (controller.Mode(MODE_CONFIG))
	{
		controller.Write(CANINTE,interrupts);
		if (controller.Mode(MODE_NORMAL))
			return true;
		else return false;
	}
	else return false;

}

#undef first_byte
#undef second_byte