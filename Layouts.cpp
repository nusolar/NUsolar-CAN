/*
 * Layouts.cpp
 * Implementation of CAN packet layouts.
 */

 /* Note: Uninitialized packets in frames must be set to UNUSED. They are not automatically initialized */
#define UNUSED 0

#include "includes/Layouts.h"

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
/**
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
**/
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

Frame MTBA_ReqCommRLeft::generate_frame() const {
	Frame f;
	f.s0=frame0_request;
	f.s1=frame1_request; 
	f.s2=frame2_request;
	set_header(f);
	return f;
}
Frame MTBA_ReqCommRRight::generate_frame() const {
	Frame f;
	f.s0=frame0_request;
	f.s1=frame1_request; 
	f.s2=frame2_request;
	set_header(f);
	return f;
}
Frame MTBA_F0_RRight::generate_frame() const {
	Frame f;
	f.s0=battery_voltage; 
	f.s1=battery_current;
	f.s2=battery_current_direction;
	f.s3=motor_current_peak_avg;
	f.s4=fet_temperature;
	f.s5=motor_rotating_speed;
	f.s6=pwm_duty;
	f.s7=lead_angle;
	set_header(f);
	return f;
}
Frame MTBA_F0_RLeft::generate_frame() const {
	Frame f;
	f.s0=battery_voltage; 
	f.s1=battery_current;
	f.s2=battery_current_direction;
	f.s3=motor_current_peak_avg;
	f.s4=fet_temperature;
	f.s5=motor_rotating_speed;
	f.s6=pwm_duty;
	f.s7=lead_angle;
	set_header(f);
	return f;
}
Frame MTBA_F1_RRight::generate_frame() const {
	Frame f;
	f.s0=power_mode; 
	f.s1=motor_control_mode;
	f.s2=accelerator_position;
	f.s3=regeneration_vr_position;
	f.s4=digit_sw_position;
	f.s5=output_target_value;
	f.s6=drive_action_status;
	f.s7=regeneration_status;
	set_header(f);
	return f;
}
Frame MTBA_F1_RLeft::generate_frame() const {
	Frame f;
	f.s0=power_mode; 
	f.s1=motor_control_mode;
	f.s2=accelerator_position;
	f.s3=regeneration_vr_position;
	f.s4=digit_sw_position;
	f.s5=output_target_value;
	f.s6=drive_action_status;
	f.s7=regeneration_status;
	set_header(f);
	return f;
}
Frame MTBA_F2_RLeft::generate_frame() const {
	Frame f;
	f.s0=analog_sensor_error;
	f.s1=motor_current_sensor_u_error;
	f.s2=motor_current_sensor_w_error;
	f.s3=fet_thermistor_error;
	f.s4=rfu1;
	f.s5=battery_voltage_sensor_error;
	f.s6=battery_current_sensor_error;
	f.s7=battery_current_sensor_adjust_error;
	f.s8=motor_current_sensor_adjust_error;
	f.s9=accelerator_position_error;
	f.s10=rfu2;
	f.s11=controller_voltage_sensor_error;
	f.s12=rfu3;
	f.s13=power_system_error;
	f.s14=over_current_error;
	f.s15=rfu4;
	f.s16=over_voltage_error;
	f.s17=rfu5;
	f.s18=over_current_limit;
	f.s19=rfu6;
	f.s20=motor_system_error;
	f.s21=motor_lock;
	f.s22=hall_sensor_short;
	f.s23=hall_sensor_open;
	f.s24=rfu7;
	f.s25=over_heat_level;
	set_header(f);
	return f;
}
Frame MTBA_F2_RRight::generate_frame() const {
	Frame f;
	f.s0=analog_sensor_error;
	f.s1=motor_current_sensor_u_error;
	f.s2=motor_current_sensor_w_error;
	f.s3=fet_thermistor_error;
	f.s4=rfu1;
	f.s5=battery_voltage_sensor_error;
	f.s6=battery_current_sensor_error;
	f.s7=battery_current_sensor_adjust_error;
	f.s8=motor_current_sensor_adjust_error;
	f.s9=accelerator_position_error;
	f.s10=rfu2;
	f.s11=controller_voltage_sensor_error;
	f.s12=rfu3;
	f.s13=power_system_error;
	f.s14=over_current_error;
	f.s15=rfu4;
	f.s16=over_voltage_error;
	f.s17=rfu5;
	f.s18=over_current_limit;
	f.s19=rfu6;
	f.s20=motor_system_error;
	f.s21=motor_lock;
	f.s22=hall_sensor_short;
	f.s23=hall_sensor_open;
	f.s24=rfu7;
	f.s25=over_heat_level;
	set_header(f);
	return f;
}