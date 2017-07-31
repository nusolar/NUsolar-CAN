/** \file Main Example for general use of the NUsolar CAN library.
 ** 
 ** This example demonstrates the most typical use cases of the CAN library on a solar powered vehicle.
 */

#include <CAN_IO.h>
#include <SPI.h>


//! First the CAN parameters are initialized
const byte     CAN_CS      = 10;
const byte     CAN_INT     = 2;
const uint16_t CAN_BAUD_RATE = 1000;  // MUST match the baud rate of the CAN bus. Setting this to 0 will enable Auto-BAUD (untested)
const byte     CAN_FREQ      = 16;    // MUST BE the frequency of the oscillator you use

unsigned long previous_send_time = 0;

//! Then the can controller object is set up using the configuration variables.
CAN_IO CanControl(CAN_CS,CAN_INT,CAN_BAUD_RATE,CAN_FREQ);

/**  
 **  The Arduino Setup() routine begins the SPI protocols to output received data to the COM window, Sets the applicable CAN filters,
 **  and then calls the CAN_IO::Setup() routine. In this case, the CAN controller is set up to listen only for BMS State of Charge and 
 **  motor controller velocity data packets.
 */
void setup() {
  Serial.begin(9600);
  
  CanControl.filters.setRB0(MASK_Sxxx,BMS_SOC_ID, MC_VELOCITY_ID); 
  CanControl.filters.setRB1(MASK_Sxxx,0,0,0,0);
  CanControl.Setup();
}

void loop() {
  CanControl.Fetch(); //If there are any new messages, they will be received.

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
    Serial.println(CanControl.ReadRegister(CNF2));

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
