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
Serial.print("h");
}
// Make sure to initialize the mainCAN pointer to 0 here.
CAN_IO* main_CAN = 0;

/*
 * Setup function for CAN_IO. Arguments are a FilterInfo struct and a pointer to a place to raise error flags.
 */
void CAN_IO::Setup(const CANFilterOpt& filters, uint16_t* errorflags) {
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
		*errptr |= CANERR_SETUP_BAUDFAIL;
		Serial.println("Baud ERROR");
	}

	// return controller to config mode
	if (!controller.Mode(MODE_CONFIG)) { // error
		*errptr |= CANERR_SETUP_MODEFAIL;
		Serial.println("Mode ERROR");
	}

	// disable interrupts we don't care about
	controller.Write(CANINTE, 0xA3); // 10100011

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
		*errptr |= CANERR_SETUP_MODEFAIL;
	}
}

void CAN_IO::Fetch() {
	// read status of CANINTF register
	byte interrupt = controller.GetInterrupt();

	*errptr = 0x00;

	if (interrupt & MERRF) { // message error
		*errptr |= CANERR_MESSAGE_ERROR;
	}

	if (interrupt & WAKIF) { // wake-up interrupt
		// No Error implemented
	}

	if (interrupt & ERRIF) { // error interrupt
		byte errors = controller.Read(EFLG);
		/* Extract Errors from the extended error flag */
		if (errors & 0xC0 == true)
		{
			*errptr |= CANERR_MCPBUF_FULL;
		}

		if (errors & 0x40 == true)
		{
			*errptr |= CANERR_BUSOFF_MODE;
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

	if (interrupt & RX1IF) { // receive buffer 1 full
		RXbuffer.enqueue(controller.ReadBuffer(RXB1));
	}

	if (interrupt & RX0IF) { // receive buffer 0 full
		RXbuffer.enqueue(controller.ReadBuffer(RXB0));
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

#undef first_byte
#undef second_byte