NUsolar MCP2515 CAN_IO Library (v1.0)
==================================
Author: 	Alexander Martin
Date:		1/16/2015
Version:	1.0

Goals
-----
This library is intended to simplify CAN bus communication for Arduino MCUs in NUsolar's vehicles which use the MCP2515. The library encapsulates the entire sending procedure into a single fuction call, and adds a buffer for holding incoming messages. It also defines a large number of packet layouts as classes, which allow data to easily be place into the CAN frame format.

Installation
-----
To use the library, simply install the files in <C:/NUsolar/sc7-can/v1.0/> and copy the two libinclude files into your arduino sketch folder. 

#include sc7-can-libinclude.h
at the top of every code file which will use the CAN_IO library.


Usage
-----
1. Create a CAN_IO object, specifying the pins that will be used as the CS and INT pins

  CAN_IO can( CSpin, INTpin, baudrate (kbps), freq. Osc. (Mhz));

2. Create a CANFilterOpt object and use the two functions
	setRB0(m0, f0, f1)
	setRB1(m1, f2, f3, f4, f5)
to configure the masks and filters for the two receive buffers on the MCP2515.

3. Call can.Setup(filterOpts, errptr) to initialize the MCP2515.
	- filterOpts is the object you created in step 2.
	- errptr is the address of a uint16_t where the library will store error flags.

NOTE: Because of the way interrupt service routines work, only one CAN object can be used at a time. This may be fixed in future releases.

4. To send a packet, create a layout object:
	DC_Drive drivePacket(velocity, current);
Call
	can.Send(drivePacket,TXB0);
where TXB0 specifies one of the three transmission buffers to send the message out over (alternate buffers to send messages in quicker succession).

NOTE: Currently, the library does not wait for a buffer to become open before attempting to load it. If the buffer you are trying to send from is still waiting to send, your packet will not be sent.

5. Messages are automatically received by the Arduino and loaded into an internal frame FIFO buffer. To get the messages on this buffer, use
	can.Available();
to check whether it is full, and then
	Frame& f = can.Read();
to get a reference to the first frame in the queue.

You can find the packet type of this frame using f.id.

6. Once the packet type has been identified, convert it into the appropriate layout class:
	DC_Drive receivedPacket(f);

7. Access the data using the layout class variables:
	receivedPacket.velocity;

Example Code
------------
#define CANINT 5
#define CANCS 4
#define CAN_NBT 1000  //Nominal Bit Time
#define CAN_FOSC 16

CAN_IO can(CANCS,CANINT,CAN_NBT,CAN_FOSC);
uint16_t errors;

void setup()
{
	CANFilterOpt filter;
	filter.setRB0(MASK_Sxxx,DC_DRIVE_ID,0);
	filter.setRB1(MASK_Sxxx,BMS_SOC_ID,0,0,0);

	can.Setup(filter, &errors);
}

void loop()
{
	// Send a message
	DC_Drive packet(100.0, 0.5);
	can.Send(packet,TXB0);
	delay(10); //Short delay to allow message to be sent

	//Receive a message
	if (can.Available())
	{
		Frame& f = can.Read();
		
		switch (f.id)
		{
		  case DC_DRIVE_ID:
		  {
			DC_Drive packet(f); //Get the drive packet
			float a = f.velocity;
			float b = f.current;
			break;
		  }
		}
	}
}


Contact
-------
Alexander Martin -- a-martin@u.northwestern.edu
Spencer Williams -- projectmanager@nusolar.org

Licence
-------
This library is Copyright 2014 by NUsolar. The code may be used in non-commercial projects without compensation, but we would appreciate it if you let us know that you're using it.