@page def_packets_guide Creating New Packet Layouts Guide

NUsolar MCP2515 CAN Library for Arduino -- Defining New CAN Packet Layouts
=====================

Author:   Alexander Martin

Last Revised:	  9/8/2017

Library Version:	3.0

Full Documentation: https://nusolar.github.io/NUsolar-CAN/docs/html/index.html

Introduction
-------
This guide shows how to extend the functionality of the existing library by implementing your own CAN packet \ref Layout "layouts". Changes and improvements to the various peripherial systems in your solar vehicle will invariably require adding, removing, or modifying the types of data that are sent over the CAN bus. By creating a custom packet layout, you ensure that your data is sent and received in exactly the same manner. With a custom CAN packet layout, you don't need to know the exact location of the data within your CAN frame each time you wish to use your custom packet; that information is programmed into the packet layout so the work is done for you.

Before starting, you should have the following information at hand:
  * The type and size of the data that you want to send in the custom packet
  * The location (within the 64 bit can frame) where each peice of your data will be located (making sure they do not overlap)
  * A unique packet ID for your custom packet. A valid packet ID is a hex number between 0x000 and 0x7FF. See \ref Layouts.h for more information.

You should also be familar with the following concepts in C++:
  * Inheritance
  * Constructor initialization of variables
  * Overloaded functions
  * Bit-wise operators 
  * Unions (not strictly necessary, but helps to understand what is going on)

Planning
-------
For this guide, lets pretend that we are creating a packet to report information about the state of the driver foot controls. (The DC_Info packet does this already, but we will be simiplifying things for pedegogical purposes.) We will need to transmit the following information and data types:
  * Accelerator pedal position -- Unsigned integer value from 0 to 100
  * Regen pedal position       -- Unsigned integer value from 0 to 100
  * Brake status			   -- Boolean value (on or off)
  * Direction of Motion (gear) -- Signed integer value (-1, 0, 1) corresponding to (reverse, neutral, and forward)

Within the 8 byte CAN frame, we will structure the data as shown in the table below.

Table 1
Packet Location | Data
----------------|-----
Byte 0, 1  		| Accelerometer Position
Byte 2, 3		| Regen Position
Byte 4			| Brake status
Byte 5			| UNUSED
Byte 6, 7		| Direction of Motion

Note: Because brake status and direction of motion really only require 1 and 2 bits of information respectively, I could have decided the package them together into the first 3 bits of Byte 4, leaving 3 bytes open for communicating more information.

Now that we have a plan for where our data will go, we are ready to begin writing a child Layout class.

Creating a child Layout class
-------
To create a new custom packet layout, you will define a new child class of the Layout class. The best place to do this is within the Layouts.h and Layouts.cpp files. Within this new class, you must define the following:
  * Member variables for every peice of data you wish to send within the CAN packet.
  * A constructor to intialize the packet from data that you specify in arguments
  * A constructor to initialize the packet from a Frame object which already contains data (i.e. one that you have received from the CAN bus)
  * A generate_frame() method which places the data values into the correct locations within a Frame object.

The easiest way to make a new child Layout class is to copy the code for an existing child Layout class and then modify it to meet your needs. In this guide, however, we will start from scratch. Remember that if you do copy code, you will need to make changes in both the Layout.h and Layout.cpp files.

The base structure for our new child class will be:
~~~~~~~~~~~~~~~~~~~~~{.cpp}
/*/** */*
 ** \/** */brief A one-line comment describing your packet's purpose.
 */
class DriverFootControls_pkt : public Layout {
public:
	Frame generate_frame() const;
};
~~~~~~~~~~~~~~~~~~~~~

Here, we have created a new class DriverFootContrls_pkt that inherits the public members of the Layout class. Furthermore, we have declared a constant method called generate_frame() which returns a frame object.

Next, we will define variables within the class to hold our data values.

Packet variables
-------
Since we already know what variables we want to include in the packet, as well as their types, this next part is easy. Create member variables of the new class to temporarily store these values. Like this:

~~~~~~~~~~~~~~~~~~~~{.cpp}
/*/** */*
 ** \/** */brief A one-line comment describing your packet's purpose.
 */
class DriverFootControls_pkt : public Layout {
public:
	Frame generate_frame() const;

	unsigned int 	acclerator_position;
	unsigned int 	regen_position;
	bool 			brake_status;
	int 			motion_dir;
};
~~~~~~~~~~~~~~~~~~~~

Note that the order in which you declare these variables doesn't affect the order that they go into the CAN packet. However, it helps to define them in the same order for readability. Also note that the types of the variables correspond to the way that they will be stored in the CAN packet. In general, you should stick to the varaible types listed in Table 2, as they are compatible with the type overrides that have been defined for the CAN packet within the Frame class.

Defining constructors
-------
Value constructor and Frame constructor

Defining generate_frame()
-------
Basic description with link to generate_frame() documentation
Using UNUSED.

Adding packet-related constants
-------
Purpose, use cases and examples.
Defining static members (link to static members tutorials)
Examples of how to access static members from external code.