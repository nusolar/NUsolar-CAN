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
	//set .ide frame vairable to True when sending/receiving an extended packet
	if (id > 0xffff){
		f.ide = 1;
	}
	f.rtr = 0; // make it a data frame
	f.srr = 0;
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

Frame MTBA_ReqCommRLeft::generate_frame() const {
	Frame f;
	f.data[0] = frame0_request;
	set_header(f);
	return f;
}
Frame MTBA_ReqCommRRight::generate_frame() const {
	Frame f;
	f.data[0] = frame0_request;
	set_header(f);
	return f;
}

Frame MTBA_F0_RRight::generate_frame() const {
	Frame f;
	
	// applying mask in reverse
	f.value =  makePrtlFrame(battery_voltage, MASK_MTBA_BATT_VOLT, MTBA_BATT_VOLT_LSB ); 
	f.value |= makePrtlFrame(battery_current, MASK_MTBA_BATT_CURR, MTBA_BATT_CURR_LSB); 
	f.value |= makePrtlFrame(battery_current_direction, MASK_MTBA_BATT_CURR_DIREC, MTBA_BATT_CURR_DIREC_LSB);
	f.value |= makePrtlFrame(motor_current_peak_avg, MASK_MTBA_MOTOR_CURR, MTBA_MOTOR_CURR_LSB); 
	f.value |= makePrtlFrame(fet_temperature,MASK_MTBA_FET_TEMP, MTBA_FET_TEMP_LSB); 
	f.value |= makePrtlFrame(motor_rotating_speed, MASK_MTBA_MOTOR_SPEED, MTBA_MOTOR_SPEED_LSB); 
	f.value |= makePrtlFrame(pwm_duty, MASK_MTBA_PWM_DUTY, MTBA_PWM_DUTY_LSB); 
	f.value |= makePrtlFrame(lead_angle, MASK_MTBA_LEAD_ANGLE, MTBA_LEAD_ANGLE_LSB);
	set_header(f);
	return f;
}
Frame MTBA_F0_RLeft::generate_frame() const {
	Frame f;
	
	// applying mask in reverse
	f.value =  makePrtlFrame(battery_voltage, MASK_MTBA_BATT_VOLT, MTBA_BATT_VOLT_LSB ); 
	f.value |= makePrtlFrame(battery_current, MASK_MTBA_BATT_CURR, MTBA_BATT_CURR_LSB); 
	f.value |= makePrtlFrame(battery_current_direction, MASK_MTBA_BATT_CURR_DIREC, MTBA_BATT_CURR_DIREC_LSB);
	f.value |= makePrtlFrame(motor_current_peak_avg, MASK_MTBA_MOTOR_CURR, MTBA_MOTOR_CURR_LSB); 
	f.value |= makePrtlFrame(fet_temperature,MASK_MTBA_FET_TEMP, MTBA_FET_TEMP_LSB); 
	f.value |= makePrtlFrame(motor_rotating_speed, MASK_MTBA_MOTOR_SPEED, MTBA_MOTOR_SPEED_LSB); 
	f.value |= makePrtlFrame(pwm_duty, MASK_MTBA_PWM_DUTY, MTBA_PWM_DUTY_LSB); 
	f.value |= makePrtlFrame(lead_angle, MASK_MTBA_LEAD_ANGLE, MTBA_LEAD_ANGLE_LSB);
	set_header(f);
	return f;
}
Frame MTBA_F1_RRight::generate_frame() const {
	Frame f;

	f.value =  makePrtlFrame(power_mode, MASK_MTBA_POWER_MODE, MTBA_POWER_MODE_LSB); 
	f.value |= makePrtlFrame(motor_control_mode, MASK_MTBA_MODE_CONTROL_MODE, MTBA_MODE_CONTROL_MODE_LSB); 
	f.value |= makePrtlFrame(accelerator_position, MASK_MTBA_ACCELERATOR_POSITION, MTBA_ACCELERATOR_POSITION_LSB); 
	f.value |= makePrtlFrame(regeneration_vr_position, MASK_MTBA_REGENERATION_VR_POSITION, MTBA_REGENERATION_VR_POSITION_LSB); 
	f.value |= makePrtlFrame(digit_sw_position, MASK_MTBA_DIGITAL_SW_POSITION, MTBA_DIGITAL_SW_POSITION_LSB); 
	f.value |= makePrtlFrame(output_target_value, MASK_MTBA_OUTPUT_TARGET_VALUE, MTBA_OUTPUT_TARGET_VALUE_LSB); 
	f.value |= makePrtlFrame(drive_action_status, MASK_MTBA_DRIVE_ACTION_STATUS, MTBA_DRIVE_ACTION_STATUS_LSB); 
	f.value |= makePrtlFrame(regeneration_status, MASK_MTBA_REGENERATION_STATUS, MTBA_REGENERATION_STATUS_LSB);

	set_header(f);
	return f;
}
Frame MTBA_F1_RLeft::generate_frame() const {
	Frame f;

	f.value =  makePrtlFrame(power_mode, MASK_MTBA_POWER_MODE, MTBA_POWER_MODE_LSB); 
	f.value |= makePrtlFrame(motor_control_mode, MASK_MTBA_MODE_CONTROL_MODE, MTBA_MODE_CONTROL_MODE_LSB); 
	f.value |= makePrtlFrame(accelerator_position, MASK_MTBA_ACCELERATOR_POSITION, MTBA_ACCELERATOR_POSITION_LSB); 
	f.value |= makePrtlFrame(regeneration_vr_position, MASK_MTBA_REGENERATION_VR_POSITION, MTBA_REGENERATION_VR_POSITION_LSB); 
	f.value |= makePrtlFrame(digit_sw_position, MASK_MTBA_DIGITAL_SW_POSITION, MTBA_DIGITAL_SW_POSITION_LSB); 
	f.value |= makePrtlFrame(output_target_value, MASK_MTBA_OUTPUT_TARGET_VALUE, MTBA_OUTPUT_TARGET_VALUE_LSB); 
	f.value |= makePrtlFrame(drive_action_status, MASK_MTBA_DRIVE_ACTION_STATUS, MTBA_DRIVE_ACTION_STATUS_LSB); 
	f.value |= makePrtlFrame(regeneration_status, MASK_MTBA_REGENERATION_STATUS, MTBA_REGENERATION_STATUS_LSB);
	set_header(f);
	return f;
}
Frame MTBA_F2_RLeft::generate_frame() const {
	Frame f;

	f.value  = makePrtlFrame(ADSensorErr, MASK_MTBA_AD_SENS_ERR, MTBA_AD_SENS_ERR_LSB);
	f.value |= makePrtlFrame(powerSysErr, MASK_MTBA_POWER_SYS_ERR, MTBA_POWER_SYS_ERR_LSB);
	f.value |= makePrtlFrame(motorSysErr, MASK_MTBA_MOTOR_SYS_ERR, MTBA_MOTOR_SYS_ERR_LSB);
	f.value |= makePrtlFrame(FETOverHeatErr, MASK_MTBA_FET_OVER_HEAT_ERR, MTBA_FET_OVER_HEAT_ERR_LSB);
	set_header(f);
	return f;
}
Frame MTBA_F2_RRight::generate_frame() const {
	Frame f;
	
	f.value  = makePrtlFrame(ADSensorErr, MASK_MTBA_AD_SENS_ERR, MTBA_AD_SENS_ERR_LSB);
	f.value |= makePrtlFrame(powerSysErr, MASK_MTBA_POWER_SYS_ERR, MTBA_POWER_SYS_ERR_LSB);
	f.value |= makePrtlFrame(motorSysErr, MASK_MTBA_MOTOR_SYS_ERR, MTBA_MOTOR_SYS_ERR_LSB);
	f.value |= makePrtlFrame(FETOverHeatErr, MASK_MTBA_FET_OVER_HEAT_ERR, MTBA_FET_OVER_HEAT_ERR_LSB);
	set_header(f);
	return f;
}

// 2019 BMS
Frame BMS19_VCSOC::generate_frame() const {
	Frame f;

	f.data[0] = current;
	f.data[2] = voltage;
	f.data[4] = packSOC;
	set_header(f);
	return f;
}

Frame BMS19_MinMaxTemp::generate_frame() const {
	Frame f;

	f.data[4] = minTemp;
	f.data[5] = maxTemp;
	set_header(f);
	return f;
}

Frame BMS19_Batt_Stat::generate_frame() const {
	Frame f;

	f.value =  makePrtlFrame(cellID, MASK_CELL_ID, CELL_ID_LSB);
	f.value |= makePrtlFrame(instVolt, MASK_INST_VOLT, INST_VOLT_LSB);
	f.value |= makePrtlFrame(intR, MASK_INT_RESIS, INT_RESIS_LSB);
	f.value |= makePrtlFrame(shunt, MASK_SHUNT, SHUNT_LSB);
	f.value |= makePrtlFrame(ocVolt, MASK_OC_VOLT, OC_VOLT_LSB);

	set_header(f);
	return f;
}

Frame BMS19_Overheat_Precharge::generate_frame() const {
	Frame f;

	f.value = overTempLimit << OVERTEMPLIMIT_LSB;
	f.value = precharged << PRECHARGED_LSB;

	set_header(f);
	return f;
}

Frame BMS19_Strobe_Trip::generate_frame() const {
	Frame f;

	f.value = strobeTrip << STROBE_TRIP_LSB;

	set_header(f);
	return f;
}
