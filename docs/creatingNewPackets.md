@page def_packets_guide Creating New Packet Layouts Guide

NUsolar MCP2515 CAN Library for Arduino -- Defining New CAN Packet Layouts
=====================

Author:   Alexander Martin

Last Revised:	  9/8/2017

Library Version:	3.0

Full Documentation: https://nusolar.github.io/NUsolar-CAN/docs/html/index.html

### List of Tables
1. [Table 1 -- New Packet Layout Specification](#Table 1)
2. [Table 2 - CAN data representation within the Frame object](#Table 2)

Introduction
-------
This guide shows how to extend the functionality of the existing library by implementing your own CAN packet \ref Layout "layouts". Changes and improvements to the various peripherial systems in your solar vehicle will invariably require adding, removing, or modifying the types of data that are sent over the CAN bus. By creating a custom packet layout, you ensure that your data is sent and received in exactly the same manner. With a custom CAN packet layout, you don't need to know the exact location of the data within your CAN frame each time you wish to use your custom packet; that information is programmed into the packet layout so the work is done for you.

Before starting, you should have the following information at hand:
  * The type and size of the data that you want to send in the custom packet
  * The location (within the 64 bit can frame) where each peice of your data will be located (making sure they do not overlap)
  * A unique packet ID for your custom packet. A valid packet ID is a hex number between 0x000 and 0x7FF. See \ref Layouts.h for more information.

You should also be familar with the following concepts in C++:
  * Inheritance
  * Member initialization syntax for constructors
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

### <a name="Table 1"></a>Table 1 -- New Packet Layout Specification
Packet Location | Data
----------------|-----
Byte 0, 1  		| Accelerometer Position
Byte 2, 3		| Regen Position
Byte 4			| Brake status
Byte 5			| UNUSED
Byte 6, 7		| Direction of Motion

Note: Because brake status and direction of motion really only require 1 and 2 bits of information respectively, I could have decided the package them together into the first 3 bits of Byte 4, leaving 3 bytes open for communicating more information.

Now that we have a plan for where our data will go, we are ready to begin writing a child layout class.

Creating a child layout class
-------
To create a new custom packet layout, you will define a new child class of the Layout class. The best place to do this is within the Layouts.h and Layouts.cpp files. Within this new class, you must define the following:
  * Member variables for every peice of data you wish to send within the CAN packet.
  * A constructor to intialize the packet from data that you specify in arguments
  * A constructor to initialize the packet from a Frame object which already contains data (i.e. one that you have received from the CAN bus)
  * A generate_frame() method which places the data values into the correct locations within a Frame object.

The easiest way to make a new child Layout class is to copy the code for an existing child layout class and then modify it to meet your needs. In this guide, however, we will start from scratch. Remember that if you do copy code, you will need to make changes in both the Layout.h and Layout.cpp files.

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

	uint16_t 	acclerator_position; //<--
	uint16_t 	regen_position;		 //<--
	bool 		brake_status;		 //<--
	int16_t 	motion_dir;			 //<--
};
~~~~~~~~~~~~~~~~~~~~

Note that the order in which we declare these variables doesn't affect the order that they go into the CAN packet. However, it helps to define them in the same order for readability. Also note that the types of the variables correspond to the way that they will be stored in the CAN packet. In general, we should stick to the varaible types listed in Table 2, as they are compatible with the type overrides that have been defined for the CAN packet within the Frame class.

Now that you have member variables defined, we need to set up an easy way to put values into these variables, as well as a way to "load" the variables from an incoming CAN Frame. We will do this in the next section with constructors.

Defining constructors
-------
There are two types of constructor that you should define for a custom packet layout. 
  1. A "value-based" constructor that initializes your packet based on arguments that the user provides.
  2. A "Frame-based" constructor that loads the data into your packet layout object from the appropriate location in a CAN frame.

The type 1 constructor is typically used when sending a packet over CAN, while the type 2 constructor is used to help read incoming Frame objects that have been read from the CAN bus. See the sendandreceive.ino example to see this distinction in practice.

We will write all of our constructor code in the Layout.h file, inside the class you created in the previous section. First, create declarations for the constructors as shown below. In the code for each constructor method, we will set the ID of the packet layout. ```id``` is a variable that your custom packet layout inherits from the Layout class.  Here, I have already added DRIVERFOOTCTRLS_ID as a constant at the top of the Layouts.h file.

~~~~~~~~~~~~~~~~~~~~{.cpp}
/*/** */*
 ** \/** */brief A one-line comment describing your packet's purpose.
 */
class DriverFootControls_pkt : public Layout {
public:
	DriverFootControls_pkt(uint16_t accl, uint16_t regen, bool brake, int16_t motion_dir) //<-- Type 1 constructor
	{ id = DRIVERFOOTCTRLS_ID; }
	DriverFootControls_pkt(const Frame& frame) //<-- Type 2 constructor
	{ id = frame.id; }

	Frame generate_frame() const;

	uint16_t 	acclerator_position;
	uint16_t 	regen_position;		
	bool 		brake_status;		
	int16_t 	motion_dir;			
};
~~~~~~~~~~~~~~~~~~~~

\note It is important that your constructor arguments not be named identically to your member variables, or else you will get naming conflicts in the next step.

Finally, we use member initialzation syntax to initialize values in the member variables we created. 

~~~~~~~~~~~~~~~~~~~~{.cpp}
/*/** */*
 ** \/** */brief A one-line comment describing your packet's purpose.
 */
class DriverFootControls_pkt : public Layout {
public:
	// Type 1 constructor
	DriverFootControls_pkt(uint16_t accl, uint16_t regen, bool brake, int16_t dir) 
	: accelerator_position(accl), regen_position(regen), brake_status(brake), motion_dir(dir) //<-- Type 1 member initialization
	{ id = DRIVERFOOTCTRLS_ID; }

	// Type 2 constructor
	DriverFootControls_pkt(const Frame& frame)
	accelerator_position(frame.i0), regen_position(frame.i1), brake_status(frame.data[4]), motion_dir(frame.s3) //<-- Type 2 member initialization
	{ id = frame.id; }

	Frame generate_frame() const;

	uint16_t 	acclerator_position;
	uint16_t 	regen_position;		
	bool 		brake_status;		
	int16_t 	motion_dir;			
};
~~~~~~~~~~~~~~~~~~~~

Note that for the Type 1 constructor, initialzation is easy; initialze each member variable to its corresponding constructor argument, i.e.

*  accl  -> accelerator_position  
*  regen -> regen_position  
*  brake -> brake_status  
*  dir   -> motion_dir  

For the Type 2 constructor, initialzation gets more tricky. The pairing goes like this:

*  frame.i0  	 -> accelerator_position  
*  frame.i1 	 -> regen_position  
*  frame.data[4] -> brake_status  
*  frame.s3   	 -> motion_dir 

The variables ```i0, i1, data[4], and s3``` all represent different ways to access the data stored within the Frame object. Remember that the CAN data frame is 8 bytes that can be broken down several different ways: 1 64 bit integer, 2 floats (32 bit numbers), 4 ints (signed or unsigned), or as individual bytes, just to name a few. You might have already guessed that i0 represents an unsigned integer, while s3 represents a signed integer. Now is probably a good time to show you all the ways you can access the Frame data.

### <a name="Table 2"></a> Table 2 - CAN data representation within the Frame object
<table class="doxtable" style="text-align:center"><tr>
<th>Bits 0..7</th><th>Bits 8..15</th><th>Bits 16..23</th><th>Bits 24..31</th><th>Bits 32..39</th><th>Bits 40..47</th><th>Bits 48..55</th><th>Bits 56..63</th></tr>
<tr>
<td>uint8_t **data[0]** </td><td>uint8_t **data[1]** </td><td>uint8_t **data[2]** </td><td>uint8_t **data[3]** </td><td>uint8_t **data[4]** </td><td>uint8_t **data[5]** </td><td>uint8_t **data[6]** </td><td>uint8_t **data[7]**  </td></tr>
<tr>
<td colspan="2">int16_t **i0** </td><td colspan="2">int16_t **i1** </td><td colspan="2">int16_t **i2** </td><td colspan="2">int16_t **i3** </td></tr>
<tr>
<td colspan="2">uint16_t **s0** </td><td colspan="2">uint16_t **s1** </td><td colspan="2">uint16_t **s2** </td><td colspan="2">uint16_t **s3** </td></tr>
<tr>
<td colspan="4">int32_t **high_s** </td><td colspan="4">int32_t **low_s** </td></tr>
<tr>
<td colspan="4">uint32_t **high** </td><td colspan="4">uint32_t **low** </td></tr>
<tr>
<td colspan="8">int64_t **value_s** </td></tr>
<tr>
<td colspan="8">uint64_t **value** </td></tr></table>

This information will also be useful to us in the next section, where we write the code that will generate a Frame object using the data stored in your new custom packet.

Defining generate_frame()
-------
The Layout::generate_frame() method is called by CAN_IO::Send() whenever you pass a child class of Layout into Send() as an argument. generate_frame() returns a Frame object that has been filled with the appropriate data at the correct positions, so it is unique for every type of CAN packet layout. Our next step is to define the generate_frame() method of the DriverFootControls class so that CAN_IO::Send() can process it.

To write the generate_frame() method, switch over to the Layouts.cpp file. Again, it may be easier in practice to copy an existing method and modify it to suit your needs, but we will begin from scratch. The general structure for a generate_frame() method is:

~~~~{.cpp}
Frame DriverFootControls::generate_frame() const {
	Frame f; 			// Declare a new frame object
	;					// Assign the packet's named variables to
	;					//   specific locations within the frame's
	;					//   annonymous union (using the varaibles from Table 2)
	set_header(f);		// Setup the header info in the Frame -- this is just calling a helper function that is defined in Layouts.cpp
	return f;			// Return the frame back to the calling function, usually CAN_IO::Send()
}
~~~~

Now all we need to do is fill in those missing lines with code that will take data from the member variables of DriverFootControls and put it into the correct location within the Frame object "f". Given how we have defined our positions and types in [Table 1](#Table 1), our final code will look like this:

~~~~{.cpp}
Frame DriverFootControls::generate_frame() const {
	Frame f;

	f.i0 = accelerometer_position;
	f.i1 = regen_position;
	f.data[4] = brake_status;
	f.data[5] = UNUSED;
	f.s3 = motion_dir;

	set_header(f);
	return f;
}
~~~~

Notice how this code is essentially the mirror operation of the what happens in the member initialization of the Type 2 constructor. You should be very careful to ensure these two match in terms of where data is read from and written to within the Frame object.

\note We set frame.data[5] to the special constant UNUSED (defined as 0). This ensures that data[5], which we did not specify we were going to put data in, is not filled with random (undefined) values. It's not strictly necessary to set all unused parts of the Frame to UNUSED, but it can help prevent mistakes further down the road.

And that's it; we defined a custom packet layout for the CAN library. Double-check that your code compiles and then start using ```CanControl.Send(DriverFootControls(50,0,false,FORWARD))``` in your code just like any other packet type.

\todo Adding packet-related constants:
Purpose, use cases and examples.
Defining static members (link to static members tutorials)
Examples of how to access static members from external code.