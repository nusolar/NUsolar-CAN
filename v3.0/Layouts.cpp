/*
 * Layouts.cpp
 * Implementation of CAN packet layouts.
 */

 /* Note: Uninitialized packets in frames must be set to UNUSED. They are not automatically initialized */
#define UNUSED 0

#include "Layouts.h"

Frame Layout::generate_frame() const {
	Frame f;
	set_header(f);
	return f;
}


// Helper function. We should really use a Macro or __inline__ function here.
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

//BMS_PrechargeStatus::BMS_PrechargeStatus(const Frame& frame)

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
	f.high_f = phase_aphase_b;
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

/*Frame DC_SwitchPos::generate_frame() const {
	Frame f;
	f.s0 = (state == 0x0040) ? 0x0100 : 0x0000;// | 0x0100; //Switches don't seem to work, so we use the fuel door bit.
	f.s1 = UNUSED;
	f.s2 = UNUSED;
	f.s3 = UNUSED;
	set_header(f);
	return f;
}*/

Frame DC_Info::generate_frame() const {
	Frame f;

	//Byte 0 + 1
	f.s0 = 0; //Clear bytes 1 and 2 (set any unused bits to 0, since this is a flag field)
	f.s0 |= ignition_state; // These are in the right place already.
	f.s0 |= brake_engaged << 7;
	f.s0 |= fuel_door << 8;	// This is currently what turns on the BMS, until we figure out how to get the ignition bits to work.
	f.s0 |= was_reset << 9;
	//Byte 2
	f.data[2] = (uint8_t) (accel_ratio * 100); // convert to integer 0-100 so only one byte required
	//Byte 3
	f.data[3] = (uint8_t) (regen_ratio * 100);
	//Byte 4 + 5
	f.s2 = can_error_flags;
	//Byte 6
	f.data[6] = dc_error_flags;
	//Byte 7
	f.data[7] = gear;

	set_header(f);
	return f;
}

Frame SW_Data::generate_frame() const {
    Frame f;
    f.data[0] = flags;
    set_header(f,1); // Note: using a byte number other than 8 causes a packet to be rejected by the telemetry system.
    return f;
}

Frame BMShub_VoltageCurrent::generate_frame() const {
	Frame f;
	f.low_f = voltage;
	f.high_f = current; //store in high_f (float)
	set_header(f);
	return f;
}
