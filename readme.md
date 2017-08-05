NUsolar MCP2515 CAN Library for Arduino
=====================

Author:   Alexander Martin

Date:		  6/13/2015

Version:	3.0

Full Documentation: https://nusolar.github.io/NUsolar-CAN/docs/html/index.html

Goals
-----
This library is intended to simplify CAN bus communication for Arduino MCUs in NUsolar's vehicles which use the MCP2515. The library encapsulates the entire sending procedure into a single fuction call, and adds a buffer for holding incoming messages. It also defines a large number of packet layouts as classes, which allow data to easily be place into the CAN frame format.

Installation
-----
To use the library, simply download the zip file of this repository from github and use the library manager inside Aruduino to load the library into the Arduino IDE.

Cloning
-----
If you want to be able to use the library as well as push changes to it, you need to CLONE the git repository into your Arduino Libraries folder. This is typically located in C:/Users/YourUsername/Documents/Arduino/libraries. You should clone the SC7-can repo into this folder, so that the structure looks like: .../Documents/Arduino/libraries/sc7-can/

Once this is done, restart arduino and the NUsolar CAN library should show up under the "Sketch->Include Libraries" menu.

For more information on installing libraries, visit http://www.arduino.cc/en/Guide/Libraries.


Usage
-----
1. Create a CAN_IO object, specifying the pins that will be used as the CS and INT pins
```
    CAN_IO can( CSpin, INTpin, baudrate (kbps), freq. Osc. (Mhz));
```
 
 
2. Setupt filters by calling the setRB<n> methods of the built-in filters object. These methods return their calling object, so they can be daisy-chained, as shown below.
``` 
    can.filters.setRB0(m0, f0, f1).setRB1(m1, f2, f3, f4, f5);
```
to configure the masks and filters for the two receive buffers on the MCP2515.


3. Call ```CAN_IO::Setup(<interrupts>)``` to initialize the MCP2515.
	```<interrupts>``` is a byte containing the interrupt enable flags that you want to set (i.e. MERRIE, ERRIE, RX0IE, etc.) You can OR these flags together to combine them. For example, the following code enables the Message Receive Error, Generic Error, Receive Buffer 0, and Receive Buffer 1 interrupts.
```
    can.Setup(MERRIE|ERRIE|RX0IE|RX1IE);
```
\note We recommend that you DO NOT attach the interrupt pin INTpin to an actual Arduino interrupt. Instead, check for messages calling CAN_IO::Fetch, which will check the INTpin to see if messages are waiting to be transferred from the MCP2515 to the Arduino.
 

4. To send a packet, create a layout object:
```
    DC_Drive drivePacket(<velocity>, <current>);
```
Then call
```
    can.Send(drivePacket,TXB0);
```
where TXB0 specifies one of the three transmission buffers to send the message out over (alternate buffers to send messages in quicker succession).
You can also use an implicit packet object:
```	
    can.Send(DC_Drive(velocity, current), TXB0);
```
If you want the system to automatically select a TX buffer for you, pass the buffer TXBANY.
```
  	can.Send(DC_Drive(velocity, current), TXBANY);
```
/note Currently, the library does not wait for a buffer to become open before attempting to load it. If you try to send from a buffer that is currently being used, packet data may be corrupted. Use the TXBANY option to avoid this. Alternatively, you can call ```CAN_IO::Send_Verified(<packet>, <buffer>)``` to make sure the correct data was loaded onto the MCP2515.
 

5. Call ```CAN_IO::Fetch()``` at least once per main control loop. This checks for any messages on the MCP2515 and loads them. It is recommended that this function be used rather than attaching interrupts, as interrupts have been known to cause conflicts with serial communication that results in corrupted CAN data.
 

6. Messages retrieved by ```CAN_IO::Fetch()``` are loaded into an internal frame FIFO buffer. To get the messages on this buffer, use
```
	if (can.Available()) {
		Frame& f = can.Read();
		/*...*/
	}
```
The variable ``` f ``` is then a reference to the first frame in the buffer.
You can find the packet type of this frame using f.id, which will be a hexidecimal number between 0x000 and 0x7FF.


7. Once the packet type has been identified, convert it into the appropriate layout class:
```	
    DC_Drive receivedPacket(f);
```

8. Access the data using the layout class variables:
```
    receivedPacket.velocity;
```
 

9. The CAN_IO object keeps track of errors that occur in an internal state variable "errors", as well as the TEC and REC counters of the MCP2515. To update these, call CAN_IO::FetchErrors().
 

10. The MCP2515 may occasionally enter sleep mode for random reasons. Code to detect this will be written into the CAN_IO class in a future release, but for now the check and reset procedure if this occurs must be done by you.


Example Code
------------
```
#include <SPI.h>
#include <CAN_IO.h>

//CAN parameters
const byte     CAN_CS 	 		 = 10;
const byte     CAN_INT	 		 = 2;
const uint16_t CAN_BAUD_RATE = 1000;	// MUST match the baud rate of the CAN bus. Setting this to 0 will enable Auto-BAUD (untested)
const byte     CAN_FREQ      = 16;	// MUST BE the frequency of the oscillator you use

unsigned long previous_send_time = 0;

// Set up the can controller object.
CAN_IO CanControl(CAN_CS,CAN_INT,CAN_BAUD_RATE,CAN_FREQ);

void setup() {
  Serial.begin(9600);
  
  CanControl.filters.setRB0(MASK_Sxxx,BMS_SOC_ID, MC_VELOCITY_ID); 
  CanControl.filters.setRB1(MASK_Sxxx,0,0,0,0);
  CanControl.Setup();
}

void loop() {
  CanControl.Fetch(); //If there are any new messages or TX buffers that have been cleared, they will be received.

  // Sending CAN
  if (millis() - previous_send_time > 500) // Check and see whether the timer has expired
  {
    // This command sends data over any available TX port.
    CanControl.Send(SW_Data(0b01101100),TXBANY);
    
    // You can print out the error counters. You can also read registers on the board by using the controller.Read() command.
    CanControl.FetchErrors(); //Call this first to get the error data from the MCP2515
    Serial.print("TEC/REC ");
    Serial.print(CanControl.tec); Serial.print(", ");
    Serial.println(CanControl.rec);
    Serial.print("CNF2: " );
    Serial.println(CanControl.controller.Read(CNF2));

    previous_send_time = millis();
  }

  if (CanControl.Available()) // Check if there are messages that have been received
  {
  	// Get the frame of of the buffer
  	Frame& f = CanControl.Read();

  	// Print the frame
  	Serial.print(f.toString());
  }
  delay(100);
}
```

Contact
-------
Alexander Martin -- a-martin@u.northwestern.edu

Spencer Williams -- project-manager@nusolar.org

Licence
-------
This library is Copyright 2015 by NUsolar. The code may be used in non-commercial projects without compensation, but we would appreciate it if you let us know that you're using it.

\todo Documentation To-Dos
-------
- Write description of how to add a new CAN frame
- Example snippets for each major function.
- CANFilterOpt class description
- Detailed descriptions for each CAN_IO callable function.