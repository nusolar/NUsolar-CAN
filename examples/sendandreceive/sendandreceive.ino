#include <CAN_IO.h>
#include <SPI.h>

/*************************************         NOTE        ******************************************************/
/* This example code is desgined to be run on two Arduino processors connected to a common 1000 kbps 
/* CAN bus via MCP2515 can controllers. Designate one arduino to be the receiver, and one to be the transmitter.
/* On the receiver, comment out the SEND block in the loop(). On the transmitter, comment out the RECEIVE block.
/****************************************************************************************************************/

// First the CAN parameters are initialized
const byte     CAN_CS      = 10;      // The arduino pin to which the MCP2515 CS (chip select) input is attached
const byte     CAN_INT     = 2;       // The arduino pin to which the MPC2515 INT (interrupt) pin is attached
const uint16_t CAN_BAUD_RATE = 1000;  // MUST match the baud rate of the CAN bus. Setting this to 0 will enable Auto-BAUD (untested)
const byte     CAN_FREQ      = 16;    // MUST BE the frequency of the oscillator you use

unsigned long previous_send_time = 0; // A timing variable we define to keep 

// Then the can controller object is set up using the configuration variables.
CAN_IO CanControl(CAN_CS,CAN_INT,CAN_BAUD_RATE,CAN_FREQ);

//  
//  The Arduino Setup() routine begins the SPI protocols to output received data to the COM window, Sets the applicable CAN filters,
//  and then calls the CAN_IO::Setup() routine. In this case, the CAN controller is set up to listen only for BMS State of Charge and 
//  motor controller velocity data packets.
//
void setup() {
  // Start serial here so that we can communicate with the test computer. Not necessary for CAN to work.
  Serial.begin(9600);

  // Here is where we specify which packets we want to filter out, and then instruct the controler to
  // initialize using the Setup command
  CanControl.filters.setRB0(MASK_Sxxx,DC_DRIVE_ID, MC_VELOCITY_ID); 
  CanControl.filters.setRB1(MASK_Sxxx,0,0,0,0);
  CanControl.Setup();
}

void loop() {
  /******  SEND  ********/
  // Send data over the CAN bus every 500 ms
  if (millis() - previous_send_time > 500) // Check and see whether it is time to send another packet
  {
    // Send the DC_Drive packet, which controls the (velocity, current) of the Wavesculptor 22 motor controller, and let the arduino choose which TX (transmit) buffer on the MCP2515 to use
    // If all you care about is sending data, you can stop after running this command.
    CanControl.Send(DC_Drive(37.5,100),TXBANY);

    // To help with debugging the CAN bus, you can print out the error counters of the MCP2515. 
    // Before you read these values, you need to fetch them using the FetchErrors() command, which grabs
    // status and error counter registers.
    // If you see the TEC and/or REC counters continuously incrementing, it means that your MCP2515
    // is not able to sucessfully read and/or write messages to the CAN bus. You should troubleshoot the issue
    // (See the troubleshooting guide in the documentation)

    CanControl.FetchErrors(); //Call this first to get the error data from the MCP2515
    Serial.print("TEC/REC ");
    Serial.print(CanControl.tec); Serial.print(", ");
    Serial.println(CanControl.rec);

    // This line resets the timing variable we use to send CAN packets.
    previous_send_time = millis();
  }
  /****** END SEND *******/


  /******* RECEIVE *******/
  // If there are any new messages, they will be loaded from the MCP2515 into an internal buffer with
  // this function call
  CanControl.Fetch(); 

  // Now we check to see if we fetched any messages.
  if (CanControl.Available())
  {
    // Get the frame of of the buffer. Read() returns a reference to a frame object which we can then convert into a string
    Frame& f = CanControl.Read();

    // Determine which packet type the frame is using frame.id
    switch (f.id) {
      case DC_DRIVE_ID:
        DC_Drive packet(f); // Initialze a packet object using the data in the frame.

        // Print the data over serial to a test computer
        Serial.println("Velocity and Current Data Received:");
        Serial.println(packet.velocity);
        Serial.println(packet.current);
        Serial.println("--");
        break;
      default: 
        Serial.print("Unknown Packet Received with id ");
        Serial.println(f.id);
        break;
    }
  }
  /******** END RECEIVE *********/

  // Run the loop() function every 100 ms
  delay(100);
}
