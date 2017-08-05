/**
 ** \file Layouts.cpp
 ** \brief Implementation of CAN packet layouts.
 ** \todo Explain what the generate_frame() method does.
 ** \todo Describe how to program the generate_frame() method for a new CAN packet.
 */

 /** \note Uninitialized sections of Frames must be set to UNUSED within the Layout::generate_frame() functions.
     They are not automatically initialized and can have undefined values if not pre-set. */
#define UNUSED 0

#include "includes/Layouts.h"

/**
 ** The generate_frame() function, which is overloaded for each specific Layout, creates a new copy of a 
 ** Frame object f. It then copies the named variables from within the Layout into the correct positions 
 ** within that Frame object so that it can be sent out over the CAN bus. In additon, each instance of 
 ** generate_frame() calls the special helper function set_header(f) to set the correct values for the Frame's
 ** header variables (i.e. the type of frame, id, data length, etc.);
 ** 
 ** Each time a new CAN packet is defined in the form of a new child class of Layout, it's generate_frame() overload
 ** must be defined here. The general structure of a generate_frame() overload is as follows:
 **
 ** \code
 ** Frame PacketClassName::generate_frame() const {
 ** 	Frame f; 			// Declare a new frame object
 ** 	;					// Assign the packet's named variables to
 ** 	;					//   specific locations within the frame's
 ** 	;					//   annonymous union.
 ** 	set_header(f);		// Setup the header info in the Frame
 ** 	return f;			// Return the frame back to the calling function.
 ** }
 ** \endcode
 **
 ** Here is an example of a generate_frame() function for the \ref BMS_Status_Ext "BMS Extended Status CAN packet".
 ** 
 ** \code
 ** Frame BMS_Status_Ext::generate_frame() const {
 **		Frame f;						
 **		f.low = flags;					
 **		f.data[4] = hardware_version;	
 **		f.data[5] = model;				
 **		f.s3 = UNUSED;					
 **		set_header(f);					
 **		return f;						
 ** }
 ** \endcode
 **
 ** \note The \ref UNUSED value should be asssigned to all parts of the frame which are not filled with data. This will
 ** 	  default their value to 0. If \ref UNUSED is not assigned, the value of any unassigned part of the data frame is 
 **		  undefined.
 */
Frame Layout::generate_frame() const {
	Frame f;
	set_header(f);
	return f;
}


// Helper function.
inline void Layout::set_header(Frame& f, byte size) const {
	f.id = id;
	f.dlc = size; // send size bytes
	f.ide = 0; // make it a standard frame
	f.rtr = 0; // make it a data frame
	f.srr = 0;
}


Frame BMS_Heartbeat::generate_frame() const {
	Frame f;
	f.low = device_id;
	f.high = serial_no;
	set_header(f);
	return f;
}

Frame BMS_SOC::generate_frame() const {
	Frame f;
	f.low_f = power_consumed;
	f.high_f = percent_SOC;
	set_header(f);
	return f;
}

Frame BMS_BalanceSOC::generate_frame() const {
	Frame f;
	f.low_f = power_supplied;
	f.high_f = SOC_mismatch;
	set_header(f);
	return f;
}

Frame BMS_PrechargeStatus::generate_frame() const {
	Frame f;
	f.data[0] = driver_status;
	f.data[1] = 0x000F & (precharge_status >> 16);
	f.data[2] = 0x000F & (precharge_status >> 12);
	f.data[3] = 0x000F & (precharge_status >> 8);
	f.data[4] = 0x000F & (precharge_status >> 4);
	f.data[5] = 0x000F & precharge_status;
	f.data[6] = timer_elapsed;
	f.data[7] = precharge_timer;
}

Frame BMS_VoltageCurrent::generate_frame() const {
	Frame f;
	f.low = voltage;
	f.high_s = current; //store in high_s (signed)
	set_header(f);
	return f;
}

Frame BMS_Status::generate_frame() const {
	Frame f;
	f.s0 = voltage_rising;
	f.s1 = voltage_falling;
	f.data[4] = flags;
	f.data[5] = no_cmus;
	f.s3 = firmware_build;
	set_header(f);
	return f;
}

Frame BMS_Status_Ext::generate_frame() const {
	Frame f;
	f.low = flags;
	f.data[4] = hardware_version;
	f.data[5] = model;
	f.s3 = UNUSED;
	set_header(f);
	return f;
}

Frame BMS_FanStatus::generate_frame() const {
	Frame f;
	f.s0 = fan0_speed;
	f.s1 = fan1_speed;
	f.s2 = fan_consumption;
	f.s3 = cmu_consumption;
	set_header(f);
	return f;
}

Frame MC_Heartbeat::generate_frame() const {
	Frame f;
	f.low = trituim_id;
	f.high = serial_no;
	set_header(f);
	return f;
}

Frame MC_Status::generate_frame() const {
	Frame f;
	f.s0 = UNUSED;
	f.s1 = limit_flags;
	f.s2 = err_flags;
	f.s3 = active_motor;
	set_header(f);
	return f;
}

Frame MC_BusStatus::generate_frame() const {
	Frame f;
	f.low_f = bus_voltage;
	f.high_f = bus_current;
	set_header(f);
	return f;
}

Frame MC_Velocity::generate_frame() const {
	Frame f;
	f.low_f = motor_velocity;
	f.high_f = car_velocity;
	set_header(f);
	return f;
}

Frame MC_PhaseCurrent::generate_frame() const {
	Frame f;
	f.low_f = phase_b;
	f.high_f = phase_a;
	set_header(f);
	return f;
}

Frame MC_FanSpeed::generate_frame() const {
	Frame f;
	f.low_f = drive;
	f.high_f = speed;
	set_header(f);
	return f;
}

Frame DC_Heartbeat::generate_frame() const {
	Frame f;
	f.low = dc_id;
	f.high = serial_no;
	set_header(f);
	return f;
}

Frame DC_Drive::generate_frame() const {
	Frame f;
	f.low_f = velocity;
	f.high_f = current;
	set_header(f);
	return f;
}

Frame DC_Power::generate_frame() const {
	Frame f;
	f.low_f = 0;
	f.high_f = bus_current;
	set_header(f);
	return f;
}

Frame DC_Reset::generate_frame() const {
	Frame f;
	f.low = 0;
	f.high = 0;
	set_header(f);
	return f;
}

Frame DC_Info::generate_frame() const {
	Frame f;

	// bytes 0, 1 (ingition swith, fuel door)
	f.s0 = 0; // clear bytes 1 and 2 (set any unused bits to 0, since this is a flag field)
	f.s0 |= ignition_state; // these are in the right place already.
	f.s0 |= fuel_door << 8;	// this is currently what turns on the BMS, until we figure out how to get the ignition bits to work.

	// byte 2
	f.data[2] = (uint8_t) (accel_ratio * 100); // convert to integer 0-100 so only one byte required

	// byte 3
	f.data[3] = (uint8_t) (regen_ratio * 100);

	// bytes 4, 5
	f.s2 = can_error_flags;

	// byte 6
	f.data[6] = dc_error_flags;

	// byte 7 (status flags)
	f.data[7] = 0;
	f.data[7] |= gear;
	f.data[7] |= brake_engaged << 4;
	f.data[7] |= was_reset << 5;
	f.data[7] |= tripped << 6;

	set_header(f);
	return f;
}

Frame DC_Status::generate_frame() const {
	Frame f;

	// bytes 0 - 3
	f.low = flags;
	f.high = 0;

	set_header(f);
	return f;
}

Frame DC_Temp_0::generate_frame() const {
    Frame f;
    f.data[0] = max_temp;
    f.data[1] = min_temp;
    f.data[2] = temp[1];
    f.data[3] = temp[2];
    f.data[4] = temp[3];
    f.data[5] = temp[4];
    f.data[6] = temp[5];
    f.data[7] = temp[6];
    set_header(f);
    return f;
}

Frame DC_Temp_1::generate_frame() const {
    Frame f;
	f.data[0] = temp[1];
	f.data[1] = temp[2];
	f.data[2] = temp[3];
	f.data[3] = temp[4];
	f.data[4] = temp[5];
	f.data[5] = temp[6];
	f.data[6] = temp[7];
	f.data[7] = temp[8];
    set_header(f);
    return f;
}

Frame DC_Temp_2::generate_frame() const {
    Frame f;
	f.data[0] = temp[1];
	f.data[1] = temp[2];
	f.data[2] = temp[3];
	f.data[3] = temp[4];
	f.data[4] = temp[5];
	f.data[5] = temp[6];
	f.data[6] = temp[7];
	f.data[7] = temp[8];
    set_header(f);
    return f;
}

Frame DC_Temp_3::generate_frame() const {
    Frame f;
	f.data[0] = temp[1];
	f.data[1] = temp[2];
	f.data[2] = temp[3];
	f.data[3] = temp[4];
	f.data[4] = temp[5];
	f.data[5] = temp[6];
	f.data[6] = temp[7];
	f.data[7] = temp[8];
	f.s1 = 0;
    set_header(f);
    return f;
}

Frame SW_Data::generate_frame() const {
    Frame f;
    f.data[0] = byte0;
    f.data[1] = byte1;
    set_header(f);
    return f;
}

Frame TEL_Status::generate_frame() const {
    Frame f;
    f.data[0] = 0;
    f.data[0] |= sql_connected;
    f.data[0] |= com_connected << 1;
    set_header(f);
    return f;
}
