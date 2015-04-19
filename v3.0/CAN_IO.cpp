/*
 * CAN_IO.cpp
 * Implementation of CAN_IO class.
 */

#include "CAN_IO.h"
#include <SPI.h>

CAN_IO::CAN_IO(byte CS_pin, byte INT_p, int baud, byte freq) :
errptr(0), INT_pin(INT_p), controller(CS_pin, INT_p), bus_speed(baud), bus_freq(freq)  {}

/*
 * Define global interrupt function
 */
void CAN_ISR()
{
  main_CAN->Fetch();
}
// Make sure to initialize the mainCAN pointer to 0 here.
CAN_IO* main_CAN = 0;

/*
 * Setup function for CAN_IO. Arguments are a FilterInfo struct and a pointer to a place to raise error flags.
 */
void CAN_IO::Setup(const CANFilterOpt& filters, uint16_t* errorflags, byte interrupts) {
	// SPI setup
	//SPI.setClockDivider(10);
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.begin();

	// Set as main can
	main_CAN = this;

	pinMode(INT_pin,INPUT_PULLUP);	
	attachInterrupt(INT_pin,CAN_ISR,LOW);
	
	// Attach error flag pointer
	errptr = errorflags;

	// init the controller
	int baudRate = controller.Init(bus_speed, bus_freq);
	if (baudRate <= 0) { // error
		*errptr |= CANERR_SETUP_BAUDFAIL; //deprecated. Remove LATER
		errors |= CANERR_SETUP_BAUDFAIL;
		Serial.println("Baud ERROR");
	}

	// return controller to config mode
	if (!controller.Mode(MODE_CONFIG)) { // error
		*errptr |= CANERR_SETUP_MODEFAIL; //deprecated. Remove LATER
		errors |= CANERR_SETUP_MODEFAIL;
		Serial.println("Mode ERROR");
	}

	// disable interrupts we don't care about
	controller.Write(CANINTE, interrupts);

	// config RX masks/filters
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
		*errptr |= CANERR_SETUP_MODEFAIL; //deprecated. Remove LATER
		errors |= CANERR_SETUP_MODEFAIL;
	}
}

void CAN_IO::Fetch() {
	// read status of CANINTF register
	byte interrupt = controller.GetInterrupt();

	*errptr = 0x00;

	if (interrupt & RX1IF) { // receive buffer 1 full
		RXbuffer.enqueue(controller.ReadBuffer(RXB1));
	}

	if (interrupt & RX0IF) { // receive buffer 0 full
		RXbuffer.enqueue(controller.ReadBuffer(RXB0));
	}

	if (interrupt & MERRF) { // message error
		*errptr |= CANERR_MESSAGE_ERROR; //deprecated. Remove LATER
		this->errors |= CANERR_MESSAGE_ERROR;
	}
	else
		this->errors &= (~CANERR_MESSAGE_ERROR);

	if (interrupt & WAKIF) { // wake-up interrupt
		// No Error implemented
	}

	if (interrupt & ERRIF) { // error interrupt
		byte eflg = controller.Read(EFLG);

		if (eflg & 0x01) // If EWARN flag is set
		{

			if (eflg & 0x02) // If RXWAR flag is set
				this->rec = controller.Read(REC);

			if (eflg & 0x04) // if TXWAR flag is set
				this->tec = controller.Read(TEC);

			if (eflg & 0x20) { // if busmode flag is set 
				this->errors |= CANERR_BUSOFF_MODE;
				controller.Reset();
				delayMicroseconds(500);
				return; // Get out of here, since we don't want to write anything else while it is resetting.
			}
			else
				this->errors &= (~CANERR_BUSOFF_MODE);

			// Receive errors
			if (eflg & 0x40) // if RX0OVR
				this->errors |= CANERR_RX0FULL_OCCURED;

			if (eflg & 0x80) // if RX1OVR
				this->errors |= CANERR_RX1FULL_OCCURED;

			if (eflg & 0xC0) // if RXnOVR
				controller.BitModify(EFLG,0xC0,0x00); // Clear RXnOVR bits
		}

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