/*
 * Layouts.h
 * Definition for CAN layouts.
 */

#ifndef __Layouts_h
#define __Layouts_h

#include <stdint.h>
#include "MCP2515_defs.h"

/*
 * Packet_IDs.h
 * Constant definitions for CAN packet IDs.
 */

// BMS TX
#define BMS_BASEADDRESS 	0x600
#define BMS_HEARTBEAT_ID	BMS_BASEADDRESS
#define BMS_SOC_ID	        0x6F4
#define BMS_BAL_SOC_ID		0x6F5
#define BMS_PRECHARGE_ID	0x6F7
#define BMS_VOLT_CURR_ID	0x6B0
#define BMS_STATUS_ID		0x6FB
#define BMS_FAN_STATUS_ID	0x6FC
#define BMS_STATUS_EXT_ID	0x6FD

// BMS19 TX
#define BMS19_VCSOC_ID			0x6B0
#define BMS19_MinMaxTemp_ID		0x6B1
#define BMS_BCL_HT_LT_ID		0x6B2

// motor controller TX
/**
#define MC_BASEADDRESS		0x400
#define MC_HEARTBEAT_ID		MC_BASEADDRESS
#define MC_STATUS_ID 		0x401
#define MC_BUS_STATUS_ID	0x402
#define MC_VELOCITY_ID		0x403
#define MC_PHASE_ID			0x404
#define MC_FANSPEED_ID		0x40A
#define MC_ODOAMP_ID		0x40E**/

// mitsuba motor controller
#define MTBA_REQUEST_COMMAND_REAR_LEFT_ID  0x08F89540
#define MTBA_REQUEST_COMMAND_REAR_RIGHT_ID	 0x08F91540
#define MTBA_FRAME0_REAR_LEFT_ID  0x08850225
#define MTBA_FRAME0_REAR_RIGHT_ID  0x08850245
#define MTBA_FRAME1_REAR_LEFT_ID  0x08950225
#define MTBA_FRAME1_REAR_RIGHT_ID  0x08950245
#define MTBA_FRAME2_REAR_LEFT_ID  0x08A50225
#define MTBA_FRAME2_REAR_RIGHT_ID  0x08A50245

// driver controls TX
#define DC_BASEADDRESS		0x500
#define DC_HEARTBEAT_ID		DC_BASEADDRESS
#define DC_DRIVE_ID			0x501
#define DC_POWER_ID			0x502
#define DC_RESET_ID			0x503
#define DC_INFO_ID			0x505
#define DC_STATUS_ID		0x506
#define DC_TEMP_0_ID		0x5F0 // Max Temp, Min Temp, Module 1 - 6 Temp
#define DC_TEMP_1_ID		0x5F1 // Module 7 - 14 Temp
#define DC_TEMP_2_ID		0x5F2 // Module 15 - 22 Temp
#define DC_TEMP_3_ID		0x5F3 // Module 23 - 26 Temp

// steering wheel TX (700 - 7F0)
#define SW_BASEADDRESS		0x700
#define SW_HEARTBEAT_ID		SW_BASEADDRESS
#define SW_DATA_ID 			0x701

// telemetry TX
#define TEL_BASEADDRESS		0x300
#define TEL_HEARTBEAT_ID 	TEL_BASEADDRESS
#define TEL_STATUS_ID		0x301

/*
 * Mask ID that specifically work with our SIDs
 * (packet ID's for f0-f5 can be found in Layouts.h)
 */
#define MASK_NONE			0x000000
#define MASK_Sx00			0x000700
#define MASK_Sxx0			0x0007F0
#define MASK_Sxxx			0x0007FF
#define MASK_EID			0x07FFFF

/* Function for easy masking: shifts the frame to the LSB and applies mask 
*	f		CAN Frame
*	mask	Mask to specify length of packet
*	lsb 	The rightmost bit of the value within the frame
*/
#define mask(f, mask, lsb) ( (f >> lsb ) & mask)

/* Function for setting member value within frame, reverse of mask process
*	f		CAN Frame
*	mask	Mask to specify length of packet
*	lsb 	The rightmost bit of the value within the frame
*/
#define makePrtlFrame(f, mask, lsb) ( ((f & mask) << lsb))

// Function
/*
 * Abstract base packet.
 */
class Layout {
public:
	uint32_t id;
	
	/*
	 * Creates a Frame object to represent this layout.
	 */
	virtual Frame generate_frame() const;

	String toString() const {
		return generate_frame().toString();
	}

protected:
	/*
	 * Fill out the header info for a frame.
	 */
	 inline void set_header(Frame& f, byte size = 8) const;
};


/*
 * BMS heartbeaat packet.
 */
class BMS_Heartbeat : public Layout {
public:
	BMS_Heartbeat(uint32_t d_id, uint32_t s_no) : device_id(d_id), serial_no(s_no) { id = BMS_HEARTBEAT_ID; }
	BMS_Heartbeat(const Frame& frame) : device_id(frame.low), serial_no(frame.high) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t device_id;
	uint32_t serial_no;
};

/*
 * BMS state of charging packet.
 */
class BMS_SOC : public Layout {
public:
	BMS_SOC(float pow_cons, float per_SOC) : power_consumed(pow_cons), percent_SOC(per_SOC) { id = BMS_SOC_ID; }
	BMS_SOC(const Frame& frame) : power_consumed(frame.low_f), percent_SOC(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float power_consumed;
	float percent_SOC;
};

/*
 * BMS state of charging during balancing packet.
 */
class BMS_BalanceSOC : public Layout {
public:
	BMS_BalanceSOC(float pow_supp, float SOC_mis) : 
		power_supplied(pow_supp), SOC_mismatch(SOC_mis) 
		{ id = BMS_BAL_SOC_ID; }
	BMS_BalanceSOC(const Frame& frame) : power_supplied(frame.low_f), SOC_mismatch(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float power_supplied;
	float SOC_mismatch;
};

/*
 * BMS precharge status packet.
 */
class BMS_PrechargeStatus : public Layout {
public:
	BMS_PrechargeStatus(uint8_t d_status, uint64_t pc_status, uint8_t t_elapsed, uint8_t pc_timer) :
		driver_status(d_status), precharge_status(pc_status), timer_elapsed(t_elapsed), precharge_timer(pc_timer)
		{ id = BMS_PRECHARGE_ID; }
	BMS_PrechargeStatus(const Frame& frame) {
	id = frame.id;
	driver_status = frame.data[0];
	timer_elapsed = frame.data[6];
	precharge_timer = frame.data[7];
	precharge_status = 0x0000 | (frame.data[1] << 16) | (frame.data[2] << 12) | (frame.data[3] << 8) | (frame.data[4] << 4) | frame.data[5];
}

	Frame generate_frame() const;

	uint8_t driver_status;
	uint64_t precharge_status;
	uint8_t timer_elapsed;
	uint8_t precharge_timer;
};

/*
 * BMS voltage and current packet.
 */
class BMS_VoltageCurrent : public Layout {
public:
	BMS_VoltageCurrent(uint32_t v, int32_t c) : voltage(v), current(c) { id = BMS_VOLT_CURR_ID; }
	BMS_VoltageCurrent(const Frame& frame) : voltage(frame.low), current(frame.high_s) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t voltage;
	int32_t current;
};

/*
 * BMS Voltage, Current, and SOC packet (2019 BMS)
 */
class BMS19_VCSOC : public Layout {
public:
	BMS19_VCSOC(uint8_t v, uint8_t i, uint8_t soc) : voltage(v), current(i), packSOC(soc) 
	{id = BMS19_VCSOC_ID;} 
	BMS19_VCSOC(const Frame& frame) : voltage(frame.data[2]), current(frame.data[0]), packSOC(frame.data[4])
	{ id = frame.id; }

	Frame generate_frame() const;

	uint8_t voltage;
	uint8_t current;
	uint8_t packSOC;
};

/*
* Battery Array Max and Min Temperature (2019 BMS)
*/
class BMS19_MinMaxTemp : public Layout {
	public:
		BMS19_MinMaxTemp(uint8_t minT, uint8_t maxT) : minTemp(minT), maxTemp(maxT)
		{id = BMS19_MinMaxTemp_ID;}
		BMS19_MinMaxTemp(const Frame& frame) : minTemp(frame.data[4]), maxTemp(frame.data[5])
		{id = frame.id; }

		Frame generate_frame()	const;

		uint8_t minTemp;
		uint8_t maxTemp;
};

/*
 * BMS status packet.
 */
class BMS_Status : public Layout {
public:
	BMS_Status(uint16_t v_rising, uint16_t v_falling, uint8_t flg, uint8_t cmus, uint16_t firmware) :
		voltage_rising(v_rising), voltage_falling(v_falling), flags(flg), no_cmus(cmus), firmware_build(firmware)
		{ id = BMS_STATUS_ID; }
	BMS_Status(const Frame& frame) : voltage_rising(frame.s0), voltage_falling(frame.s1), 
		flags(frame.data[4]), no_cmus(frame.data[5]), firmware_build(frame.s3)
		{ id = frame.id; }

	Frame generate_frame() const;

	uint16_t voltage_rising;
	uint16_t voltage_falling;
	uint8_t flags;
	uint8_t no_cmus;
	uint16_t firmware_build;

};

/*
 * BMS Extended Status Packet.
 */
class BMS_Status_Ext : public Layout {
public:
	BMS_Status_Ext(uint32_t _flags, uint8_t _hardware_version, uint8_t _model) :
		flags(_flags), hardware_version(_hardware_version), model(_model)
		{ id = BMS_STATUS_EXT_ID; }
	BMS_Status_Ext(const Frame& frame) : flags(frame.low), hardware_version(frame.data[4]), 
		model(frame.data[5])
		{ id = frame.id; }

	Frame generate_frame() const;

	uint32_t flags;
	uint8_t  hardware_version;
	uint8_t  model;

	static const uint32_t F_OVERVOLTAGE 		= 0x00000001; // Cell Overvoltage Flag
	static const uint32_t F_UNDERVOLTAGE 	    = 0x00000002; // Cell Undervoltage Flag
												//0x00000004; // Cell Overtemp Flag (not implemented)
	static const uint32_t F_UNTRUSTED			= 0x00000008; // Untrusted Measurement Flag (channel mismatch)
	static const uint32_t F_CMULOST			    = 0x00000010; // Lost CMU Comm Fag
	static const uint32_t F_DRVCTRLSLOST	    = 0x00000020; // Lost Driver Controls Comm Flag
	static const uint32_t F_SETUPMODE		    = 0x00000040; // BMU in Setup Mode Flag
	static const uint32_t F_CMUCANPOWERSTATUS   = 0x00000080; // Power Status of CMU CAN bus
	static const uint32_t F_ISOLATIONFAIL       = 0x00000100; // Isolation Failure Flag
												//0x00000200; // SOC Measurement Not Valid Flag (not used)
	static const uint32_t F_12VLOW			    = 0x00000400; // Low 12V Supply on CAN bus Flag
	static const uint32_t F_CONTACTOR		    = 0x00000800; // Contactor Error Flag
	static const uint32_t F_EXTRACELL		    = 0x00001000; // Extra Cell Detected by CMU Flag
};

/*
 * BMS fan status packet.
 */
class BMS_FanStatus : public Layout {
public:
	BMS_FanStatus(uint16_t f0_speed, uint16_t f1_speed, uint16_t fan_c, uint16_t cmu_c) :
		fan0_speed(f0_speed), fan1_speed(f1_speed), fan_consumption(fan_c), cmu_consumption(cmu_c)
		{ id = BMS_FAN_STATUS_ID; }
	BMS_FanStatus(const Frame& frame) : fan0_speed(frame.s0), fan1_speed(frame.s1), 
		fan_consumption(frame.s2), cmu_consumption(frame.s3)
		{ id = frame.id; }

	Frame generate_frame() const;

	uint16_t fan0_speed;
	uint16_t fan1_speed;
	uint16_t fan_consumption;
	uint16_t cmu_consumption;
};

//MITSUBA
class MTBA_ReqCommR: public Layout {
public:

	MTBA_ReqCommR(uint8_t f0_req) :
		frame0_request(f0_req)
		{ id = MTBA_REQUEST_COMMAND_REAR_LEFT_ID; }
	MTBA_ReqCommR(const Frame& frame) : frame0_request(frame.s0)
		{ id = frame.id; }
	Frame generate_frame() const;

	uint8_t frame0_request;
};
class MTBA_ReqCommRRight : public Layout {
public:
	MTBA_ReqCommRRight(uint8_t f0_req) :
		frame0_request(f0_req)
		{ id = MTBA_REQUEST_COMMAND_REAR_RIGHT_ID; }
	MTBA_ReqCommRRight(const Frame& frame) : frame0_request(frame.s0)
		{ id = frame.id; }
	Frame generate_frame() const;

	uint8_t frame0_request;
}; 
class MTBA_F0_RLeft : public Layout{
public:
	MTBA_F0_R(uint16_t bv, uint16_t bc, bool bcd, uint8_t mcpa, uint16_t ft, uint16_t mrs, uint16_t pd, uint8_t la) :
		battery_voltage(bv), 					// 10 bits
		battery_current(bc), 					// 9 bits
		battery_current_direction(bcd),			// 1 bit
		motor_current_peak_avg(mcpa), 			// 10 bits
		fet_temperature(ft), 					// 5 bits
		motor_rotating_speed(mrs), 				// 12 bits
		pwm_duty(pd), 							// 10 bits
		lead_angle(la)                          // 7 bits
		{ id = MTBA_FRAME0_REAR_LEFT_ID; }
	MTBA_F0_R(const Frame& frame) : 
		battery_voltage(mask(frame.value, MASK_MTBA_BATT_VOLT, MTBA_BATT_VOLT_LSB )), 
		battery_current(mask(frame.value, MASK_MTBA_BATT_CURR, MTBA_BATT_CURR_LSB)), 
		battery_current_direction(mask(frame.value, MASK_MTBA_BATT_CURR_DIREC, MTBA_BATT_CURR_DIREC_LSB)),
		motor_current_peak_avg(mask(frame.value, MASK_MTBA_MOTOR_CURR, MTBA_MOTOR_CURR_LSB)), 
		fet_temperature(mask(frame.value,MASK_MTBA_FET_TEMP, MTBA_FET_TEMP_LSB)), 
		motor_rotating_speed(mask(frame.value, MASK_MTBA_MOTOR_SPEED, MTBA_MOTOR_SPEED_LSB)), 
		pwm_duty(mask(frame.value, MASK_MTBA_PWM_DUTY, MTBA_PWM_DUTY_LSB)), 
		lead_angle(mask(frame.value, MASK_MTBA_LEAD_ANGLE, MTBA_LEAD_ANGLE_LSB))
		{ id = frame.id; }

	Frame generate_frame() const;

	uint16_t battery_voltage;
	uint16_t battery_current;
	bool battery_current_direction;
	uint8_t motor_current_peak_avg;
	uint16_t fet_temperature;
	uint16_t motor_rotating_speed;
	uint16_t pwm_duty;
	uint8_t lead_angle;

	/*
	* Apply these masks after shifting frame to LSB
	* MTBA Can packets don't follow bytes, so alot of masking is necessary 
	*/
	static const uint16_t MASK_MTBA_BATT_VOLT				= 0x3FF;
	static const uint16_t MASK_MTBA_BATT_CURR				= 0x1FF;
	static const uint16_t MASK_MTBA_BATT_CURR_DIREC			= 0x01;
	static const uint16_t MASK_MTBA_MOTOR_CURR				= 0x3FF;
	static const uint16_t MASK_MTBA_FET_TEMP				= 0x1F;
	static const uint16_t MASK_MTBA_MOTOR_SPEED			    = 0x3FF;
	static const uint16_t MASK_MTBA_PWM_DUTY				= 0x3FF;
	static const uint16_t MASK_MTBA_LEAD_ANGLE				= 0x7F;

	// The position right-most bit (LSB) after masking
	static const int MTBA_BATT_VOLT_LSB						= 0;
	static const int MTBA_BATT_CURR_LSB						= 10;
	static const int MTBA_BATT_CURR_DIREC_LSB				= 19;
	static const int MTBA_MOTOR_CURR_LSB					= 20;
	static const int MTBA_FET_TEMP_LSB						= 30;
	static const int MTBA_MOTOR_SPEED_LSB					= 35;
	static const int MTBA_PWM_DUTY_LSB						= 47;
	static const int MTBA_LEAD_ANGLE_LSB					= 57;
	
};
class MTBA_F0_RRight : public Layout{
public:
	MTBA_F0_R(uint16_t bv, uint16_t bc, bool bcd, uint8_t mcpa, uint16_t ft, uint16_t mrs, uint16_t pd, uint8_t la) :
		battery_voltage(bv), 					// 10 bits
		battery_current(bc), 					// 9 bits
		battery_current_direction(bcd),			// 1 bit
		motor_current_peak_avg(mcpa), 			// 10 bits
		fet_temperature(ft), 					// 5 bits
		motor_rotating_speed(mrs), 				// 12 bits
		pwm_duty(pd), 							// 10 bits
		lead_angle(la)                          // 7 bits
		{ id = MTBA_FRAME0_REAR_RIGHT_ID; }
	MTBA_F0_R(const Frame& frame) : 
		battery_voltage(mask(frame.value, MASK_MTBA_BATT_VOLT, MTBA_BATT_VOLT_LSB )), 
		battery_current(mask(frame.value, MASK_MTBA_BATT_CURR, MTBA_BATT_CURR_LSB)), 
		battery_current_direction(mask(frame.value, MASK_MTBA_BATT_CURR_DIREC, MTBA_BATT_CURR_DIREC_LSB)),
		motor_current_peak_avg(mask(frame.value, MASK_MTBA_MOTOR_CURR, MTBA_MOTOR_CURR_LSB)), 
		fet_temperature(mask(frame.value,MASK_MTBA_FET_TEMP, MTBA_FET_TEMP_LSB)), 
		motor_rotating_speed(mask(frame.value, MASK_MTBA_MOTOR_SPEED, MTBA_MOTOR_SPEED_LSB)), 
		pwm_duty(mask(frame.value, MASK_MTBA_PWM_DUTY, MTBA_PWM_DUTY_LSB)), 
		lead_angle(mask(frame.value, MASK_MTBA_LEAD_ANGLE, MTBA_LEAD_ANGLE_LSB))
		{ id = frame.id; }

	Frame generate_frame() const;

	uint16_t battery_voltage;
	uint16_t battery_current;
	bool battery_current_direction;
	uint8_t motor_current_peak_avg;
	uint16_t fet_temperature;
	uint16_t motor_rotating_speed;
	uint16_t pwm_duty;
	uint8_t lead_angle;

	/*
	* Apply these masks after shifting frame to LSB
	* MTBA Can packets don't follow bytes, so alot of masking is necessary 
	*/
	static const uint16_t MASK_MTBA_BATT_VOLT				= 0x3FF;
	static const uint16_t MASK_MTBA_BATT_CURR				= 0x1FF;
	static const uint16_t MASK_MTBA_BATT_CURR_DIREC			= 0x01;
	static const uint16_t MASK_MTBA_MOTOR_CURR				= 0x3FF;
	static const uint16_t MASK_MTBA_FET_TEMP				= 0x1F;
	static const uint16_t MASK_MTBA_MOTOR_SPEED			    = 0x3FF;
	static const uint16_t MASK_MTBA_PWM_DUTY				= 0x3FF;
	static const uint16_t MASK_MTBA_LEAD_ANGLE				= 0x7F;

	// The position right-most bit (LSB) after masking
	static const int MTBA_BATT_VOLT_LSB						= 0;
	static const int MTBA_BATT_CURR_LSB						= 10;
	static const int MTBA_BATT_CURR_DIREC_LSB				= 19;
	static const int MTBA_MOTOR_CURR_LSB					= 20;
	static const int MTBA_FET_TEMP_LSB						= 30;
	static const int MTBA_MOTOR_SPEED_LSB					= 35;
	static const int MTBA_PWM_DUTY_LSB						= 47;
	static const int MTBA_LEAD_ANGLE_LSB					= 57;
	
};
class MTBA_F1_RRight : public Layout{
public:
	MTBA_F1_R(bool pm, bool mcm, uint16_t ap, uint16_t rvp, uint16_t dsp, uint16_t otv, uint16_t das, bool rs) :
		power_mode(pm), 
		motor_control_mode(mcm), 
		accelerator_position(ap), 
		regeneration_vr_position(rvp),
		 digit_sw_position(dsp),
		output_target_value(otv), 
		drive_action_status(das), 
		regeneration_status(rs)
		{ id = MTBA_FRAME1_REAR_RIGHT_ID; }
	MTBA_F1_R(const Frame& frame) : 
		power_mode(mask(frame.value, MASK_MTBA_POWER_MODE, MTBA_POWER_MODE_LSB)), 
		motor_control_mode(mask(frame.value, MASK_MTBA_MODE_CONTROL_MODE, MTBA_MODE_CONTROL_MODE_LSB)), 
		accelerator_position(mask(frame.value, MASK_MTBA_ACCELERATOR_POSITION, MTBA_ACCELERATOR_POSITION_LSB)), 
		regeneration_vr_position(mask(frame.value, MASK_MTBA_REGENERATION_VR_POSITION, MTBA_REGENERATION_VR_POSITION)), 
		digit_sw_position(mask(frame.value, MASK_MTBA_DIGITAL_SW_POSITION, MTBA_DIGITAL_SW_POSITION_LSB)), 
		output_target_value(mask(frame.value, MASK_MTBA_OUTPUT_TARGET_VALUE, MTBA_OUTPUT_TARGET_VALUE_LSB)), 
		drive_action_status(mask(frame.value, MASK_MTBA_DRIVE_ACTION_STATUS, MTBA_DRIVE_ACTION_STATUS_LSB)), 
		regeneration_status(mask(frame.value, MASK_MTBA_REGENERATION_STATUS, MTBA_REGENERATION_STATUS_LSB))
		{ id = frame.id; }

	Frame generate_frame() const;

	bool power_mode;					// 1 bit
	bool motor_control_mode;			// 1 bit
	uint16_t accelerator_position;		// 10 bits
	uint16_t regeneration_vr_position;	// 10 bits
	uint8_t digit_sw_position;			// 4 bits
	uint16_t output_target_value;		// 10 bits
	uint8_t drive_action_status;		// 2 bits
	bool regeneration_status;			// 1 bit

	/*
	* Apply these masks after shifting frame to LSB
	* MTBA Can packets don't follow bytes, so alot of masking is necessary 
	*/
	static const uint16_t MASK_MTBA_POWER_MODE							= 0x01;
	static const uint16_t MASK_MTBA_MODE_CONTROL_MODE					= 0x01;
	static const uint16_t MASK_MTBA_ACCELERATOR_POSITION				= 0x3FF;
	static const uint16_t MASK_MTBA_REGENERATION_VR_POSITION			= 0x3FF;
	static const uint16_t MASK_MTBA_DIGITAL_SW_POSITION					= 0x0F;
	static const uint16_t MASK_MTBA_OUTPUT_TARGET_VALUE			    	= 0x3FF;
	static const uint16_t MASK_MTBA_DRIVE_ACTION_STATUS					= 0x03;
	static const uint16_t MASK_MTBA_REGENERATION_STATUS					= 0x01;

	// The position right-most bit (LSB) after masking
	static const int MTBA_POWER_MODE_LSB								= 0;
	static const int MTBA_MODE_CONTROL_MODE_LSB							= 1;
	static const int MTBA_ACCELERATOR_POSITION_LSB						= 2;
	static const int MTBA_REGENERATION_VR_POSITION_LSB					= 12;
	static const int MTBA_DIGITAL_SW_POSITION_LSB						= 22;
	static const int MTBA_OUTPUT_TARGET_VALUE_LSB						= 26;
	static const int MTBA_DRIVE_ACTION_STATUS_LSB						= 36;
	static const int MTBA_REGENERATION_STATUS_LSB						= 38;
};
class MTBA_F1_R : public Layout{
public:
	MTBA_F1_R(bool pm, bool mcm, uint16_t ap, uint16_t rvp, uint16_t dsp, uint16_t otv, uint16_t das, bool rs) :
		power_mode(pm), 
		motor_control_mode(mcm), 
		accelerator_position(ap), 
		regeneration_vr_position(rvp),
		 digit_sw_position(dsp),
		output_target_value(otv), 
		drive_action_status(das), 
		regeneration_status(rs)
		{ id = MTBA_FRAME1_REAR_LEFT_ID; }
	MTBA_F1_R(const Frame& frame) : 
		power_mode(mask(frame.value, MASK_MTBA_POWER_MODE, MTBA_POWER_MODE_LSB)), 
		motor_control_mode(mask(frame.value, MASK_MTBA_MODE_CONTROL_MODE, MTBA_MODE_CONTROL_MODE_LSB)), 
		accelerator_position(mask(frame.value, MASK_MTBA_ACCELERATOR_POSITION, MTBA_ACCELERATOR_POSITION_LSB)), 
		regeneration_vr_position(mask(frame.value, MASK_MTBA_REGENERATION_VR_POSITION, MTBA_REGENERATION_VR_POSITION)), 
		digit_sw_position(mask(frame.value, MASK_MTBA_DIGITAL_SW_POSITION, MTBA_DIGITAL_SW_POSITION_LSB)), 
		output_target_value(mask(frame.value, MASK_MTBA_OUTPUT_TARGET_VALUE, MTBA_OUTPUT_TARGET_VALUE_LSB)), 
		drive_action_status(mask(frame.value, MASK_MTBA_DRIVE_ACTION_STATUS, MTBA_DRIVE_ACTION_STATUS_LSB)), 
		regeneration_status(mask(frame.value, MASK_MTBA_REGENERATION_STATUS, MTBA_REGENERATION_STATUS_LSB))
		{ id = frame.id; }

	Frame generate_frame() const;

	bool power_mode;					// 1 bit
	bool motor_control_mode;			// 1 bit
	uint16_t accelerator_position;		// 10 bits
	uint16_t regeneration_vr_position;	// 10 bits
	uint8_t digit_sw_position;			// 4 bits
	uint16_t output_target_value;		// 10 bits
	uint8_t drive_action_status;		// 2 bits
	bool regeneration_status;			// 1 bit

	/*
	* Apply these masks after shifting frame to LSB
	* MTBA Can packets don't follow bytes, so alot of masking is necessary 
	*/
	static const uint16_t MASK_MTBA_POWER_MODE							= 0x01;
	static const uint16_t MASK_MTBA_MODE_CONTROL_MODE					= 0x01;
	static const uint16_t MASK_MTBA_ACCELERATOR_POSITION				= 0x3FF;
	static const uint16_t MASK_MTBA_REGENERATION_VR_POSITION			= 0x3FF;
	static const uint16_t MASK_MTBA_DIGITAL_SW_POSITION					= 0x0F;
	static const uint16_t MASK_MTBA_OUTPUT_TARGET_VALUE			    	= 0x3FF;
	static const uint16_t MASK_MTBA_DRIVE_ACTION_STATUS					= 0x03;
	static const uint16_t MASK_MTBA_REGENERATION_STATUS					= 0x01;

	// The position right-most bit (LSB) after masking
	static const int MTBA_POWER_MODE_LSB								= 0;
	static const int MTBA_MODE_CONTROL_MODE_LSB							= 1;
	static const int MTBA_ACCELERATOR_POSITION_LSB						= 2;
	static const int MTBA_REGENERATION_VR_POSITION_LSB					= 12;
	static const int MTBA_DIGITAL_SW_POSITION_LSB						= 22;
	static const int MTBA_OUTPUT_TARGET_VALUE_LSB						= 26;
	static const int MTBA_DRIVE_ACTION_STATUS_LSB						= 36;
	static const int MTBA_REGENERATION_STATUS_LSB						= 38;
};

class MTBA_F2_RLeft : public Layout{
	MTBA_F2_RLeft(uint16_t adErr, uint8_t psErr, uint8_t msErr, uint8_t fetOHErr):
		ADSensorErr(adErr),
		powerSysErr(psErr),
		motorSysErr(msErr),
		FETOverHeatErr(fetOHErr)
		{ id = MTBA_FRAME1_REAR_LEFT_ID; }
	MTBA_F2_RLeft(const Frame& frame) :
		ADSensorErr(mask(frame.value, MASK_MTBA_AD_SENS_ERR, MTBA_AD_SENS_LSB)),
		powerSysErr(mask(frame.value, MASK_MTBA_POWER_SYS_ERR, MTBA_POWER_SYS_ERR_LSB)),
		motorSysErr(mask(frame.value, MASK_MTBA_MOTOR_SYS_ERR, MTBA_MOTOR_SYS_ERR_LSB)),
		FETOverHeatErr(mask(frame.value, MASK_MTBA_FET_OVER_HEAT_ERR, MTBA_FET_OVER_HEAT_ERR_LSB)),

	uint16_t ADSensorErr;
	uint8_t powerSysErr; 
	uint8_t motorSysErr;
	uint8_t FETOverHeatErr;

	/*
	* Apply these masks after shifting frame to LSB
	* MTBA Can packets don't follow bytes, so alot of masking is necessary 
	*/
	static const uint16_t MASK_MTB_AD_SENS_ERR						= 0xFFFF;
	static const uint16_t MASK_MTBA_POWER_SYS_ERR					= 0xFF;
	static const uint16_t MASK_MTBA_MOTOR_SYS_ERR					= 0xFF;
	static const uint16_t MASK_MTBA_FET_OVER_HEAT_ERR				= 0x03;

	// The position right-most bit (LSB) after masking
	static const int MTB_AD_SENS_ERR_LSB							= 0;
	static const int MTBA_POWER_SYS_ERR_LSB							= 16;
	static const int MTBA_MOTOR_SYS_ERR_LSB							= 24;
	static const int MTBA_FET_OVER_HEAT_ERR_LSB						= 32;
};

class MTBA_F2_RRight : public Layout{
	MTBA_F2_RLeft(uint16_t adErr, uint8_t psErr, uint8_t msErr, uint8_t fetOHErr):
		ADSensorErr(adErr),
		powerSysErr(psErr),
		motorSysErr(msErr),
		FETOverHeatErr(fetOHErr)
		{ id = MTBA_FRAME1_REAR_RIGHT_ID; }
	MTBA_F2_RLeft(const Frame& frame) :
		ADSensorErr(mask(frame.value, MASK_MTBA_AD_SENS_ERR, MTBA_AD_SENS_LSB)),
		powerSysErr(mask(frame.value, MASK_MTBA_POWER_SYS_ERR, MTBA_POWER_SYS_ERR_LSB)),
		motorSysErr(mask(frame.value, MASK_MTBA_MOTOR_SYS_ERR, MTBA_MOTOR_SYS_ERR_LSB)),
		FETOverHeatErr(mask(frame.value, MASK_MTBA_FET_OVER_HEAT_ERR, MTBA_FET_OVER_HEAT_ERR_LSB)),

	uint16_t ADSensorErr;
	uint8_t powerSysErr; 
	uint8_t motorSysErr;
	uint8_t FETOverHeatErr;

	/*
	* Apply these masks after shifting frame to LSB
	* MTBA Can packets don't follow bytes, so alot of masking is necessary 
	*/
	static const uint16_t MASK_MTB_AD_SENS_ERR						= 0xFFFF;
	static const uint16_t MASK_MTBA_POWER_SYS_ERR					= 0xFF;
	static const uint16_t MASK_MTBA_MOTOR_SYS_ERR					= 0xFF;
	static const uint16_t MASK_MTBA_FET_OVER_HEAT_ERR				= 0x03;

	// The position right-most bit (LSB) after masking
	static const int MTB_AD_SENS_ERR_LSB							= 0;
	static const int MTBA_POWER_SYS_ERR_LSB							= 16;
	static const int MTBA_MOTOR_SYS_ERR_LSB							= 24;
	static const int MTBA_FET_OVER_HEAT_ERR_LSB						= 32;
};

/*
 * Motor controller heartbeat packet.
 */
/**
class MC_Heartbeat : public Layout {
public:
	MC_Heartbeat(uint32_t t_id, uint32_t s_no) : trituim_id(t_id), serial_no(s_no) { id = MC_HEARTBEAT_ID; }
	MC_Heartbeat(const Frame& frame) : trituim_id(frame.low), serial_no(frame.high) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t trituim_id; // is actually a char[8]
	uint32_t serial_no;
};**/

/*
 * Motor controller status packet.
 */
/**
class MC_Status : public Layout {
public:
	MC_Status(uint16_t act_m, uint16_t err_f, uint16_t lim_f) : 
		active_motor(act_m), err_flags(err_f), limit_flags(lim_f)
		{ id = MC_STATUS_ID; }
	MC_Status(const Frame& frame) : active_motor(frame.s3), err_flags(frame.s2), limit_flags(frame.s1)
		{ id = frame.id; }

	Frame generate_frame() const;

	uint16_t active_motor;
	uint16_t err_flags;
	uint16_t limit_flags;
};
**/
/*
 * Motor controller bus status packet.
 */
/**
class MC_BusStatus : public Layout {
public:
	MC_BusStatus(float bc, float bv) : bus_current(bc), bus_voltage(bv) { id = MC_BUS_STATUS_ID; }
	MC_BusStatus(const Frame& frame) : bus_current(frame.high_f), bus_voltage(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float bus_current;
	float bus_voltage;
};**/

/*
 * Motor controller velocity packet.
 */
/**
class MC_Velocity : public Layout {
public:
	MC_Velocity(float car_v, float motor_v) : car_velocity(car_v), motor_velocity(motor_v) { id = MC_VELOCITY_ID; }
	MC_Velocity(const Frame& frame) : car_velocity(frame.high_f), motor_velocity(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float car_velocity;
	float motor_velocity;
};**/

/*
 * Motor controller motor phase current packet.
 */
/**
class MC_PhaseCurrent : public Layout {
public:
	MC_PhaseCurrent(float a, float b) : phase_a(a), phase_b(b) { id = MC_PHASE_ID; }
	MC_PhaseCurrent(const Frame& frame) : phase_a(frame.high_f), phase_b(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float phase_a;
	float phase_b;
};

class MC_FanSpeed : public Layout {
public:
	MC_FanSpeed(float rpm, float v) : speed(rpm), drive(v) {id = MC_FANSPEED_ID; }
	MC_FanSpeed(const Frame& frame) : speed(frame.high_f), drive(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float speed;
	float drive;
};

class MC_OdoAmp : public Layout {
public:
	MC_OdoAmp(float Ah, float odom) : bus_amphours(Ah), odometer(odom) {id = MC_ODOAMP_ID; }
	MC_OdoAmp(const Frame& frame) : bus_amphours(frame.high_f), odometer(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float bus_amphours;
	float odometer;
};**/

/*
 * Driver controls heartbeat packet.
 */
class DC_Heartbeat : public Layout {
public:
	DC_Heartbeat(uint32_t d_id, uint32_t s_no) : dc_id (d_id), serial_no (s_no) { id = DC_HEARTBEAT_ID; }
	DC_Heartbeat(Frame& frame) : dc_id(frame.low), serial_no(frame.high) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t dc_id;
	uint32_t serial_no;
};

/*
 * Driver controls drive command packet.
 */
class DC_Drive : public Layout {
public:
	DC_Drive(float v, float c) : velocity(v), current(c) { id = DC_DRIVE_ID; }
	DC_Drive(const Frame& frame) : velocity(frame.low_f), current(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float velocity;
	float current;
};

/*
 * Driver controls power command packet.
 */
class DC_Power : public Layout {
public:
	DC_Power(float bc) : bus_current(bc) { id = DC_POWER_ID; }
	DC_Power(const Frame& frame) : bus_current(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float bus_current;
};

/*
 * Driver controls reset packet.
 */
class DC_Reset : public Layout {
public:
	DC_Reset() { id = DC_RESET_ID; }
	DC_Reset(const Frame& frame) { id = frame.id; }

	Frame generate_frame() const;
};

/*
 * Driver controls information packet.
 */
class DC_Info : public Layout {
public:
	DC_Info(float accel, 
			float regen,
			bool brake,
			uint16_t can_errors,
			byte dc_errors,
			bool reset,
			bool fuel,
			byte current_gear,
			uint16_t ignition,
			bool tripcondition) { 

		accel_ratio = accel;
		regen_ratio = regen;
		brake_engaged = brake;
		can_error_flags = can_errors;
		dc_error_flags = dc_errors;
		was_reset = reset;
		gear = current_gear;
		ignition_state = ignition;
		fuel_door = fuel;
		tripped = tripcondition;

		id = DC_INFO_ID; 
	}

	DC_Info(const Frame& frame) { 
		// bytes 0, 1 (ignition switch, fuel door)
		ignition_state = frame.s0 & 0x0070;
		fuel_door = (bool)(frame.s0 & 0x0100);
		
		// byte 2
		accel_ratio = frame.data[2]/100.0f;

		// byte 3
		regen_ratio = frame.data[3]/100.0f;

		// byte 4 + 5
		can_error_flags = frame.s2;

		// byte 6
		dc_error_flags = frame.data[6];

		// byte 7 (status flags)
		gear = frame.data[7] & 0x0F;
		brake_engaged = (bool)(frame.data[7] & 0x10);
		was_reset = (bool)(frame.data[7] & 0x20);
		tripped = (bool)(frame.data[7] & 0x40);

		id = frame.id; 
	}

	float accel_ratio, regen_ratio; // these will be stored as integers 0-100 in frame 
	uint16_t can_error_flags;
	byte dc_error_flags, gear;
	bool brake_engaged, was_reset, fuel_door;
	uint16_t ignition_state;
	bool tripped;
	
	Frame generate_frame() const;
};

/*
 * Driver controls status packet.
 */
class DC_Status : public Layout {
public:
	DC_Status(uint32_t flags) { 

		this->flags = flags;

		id = DC_STATUS_ID; 
	}

	DC_Status(const Frame& frame) { 
		// bytes 0, 1
		flags  = frame.low;

		id = frame.id; 
	}

	uint32_t flags;

	static const uint8_t F_CHARGING_OVER_TEMP 		= 0x01;
	static const uint8_t F_DISCHARGING_OVER_TEMP    = 0x02;
	static const uint8_t F_CHARGING_OVER_CURRENT    = 0x04;
	static const uint8_t F_DISCHARGING_OVER_CURRENT = 0x08;
	static const uint8_t F_NO_TRIP					= 0x00;

	// 0x80 also defined by driver controls as the flag for a BMS-controlled trip
	
	Frame generate_frame() const;
};

/* Temperature Sensor Data Packets 
   Packet 1: Min, Max, and Modules 1 - 6
   Packet 2: Modules 7 - 14
   Packet 3: Modules 15 - 22
   Packet 4: Modules 23 - 30 */

class DC_Temp_0 : public Layout {
public:
	DC_Temp_0(uint8_t maxT, uint8_t minT, uint8_t T1, uint8_t T2, uint8_t T3, uint8_t T4, uint8_t T5, uint8_t T6) { 

		max_temp = maxT;
		min_temp = minT;
		temp[1] = T1;
		temp[2] = T2;
		temp[3] = T3;
		temp[4] = T4;
		temp[5] = T5;
		temp[6] = T6;

		id = DC_TEMP_0_ID; 
	}

	DC_Temp_0(uint8_t maxT, uint8_t minT, uint8_t *_temps) { 

		max_temp = maxT;
		min_temp = minT;
		memcpy(temp+1,_temps, 6);

		id = DC_TEMP_0_ID; 
	}

	DC_Temp_0(const Frame& frame) { 
		max_temp = frame.data[0];
		min_temp = frame.data[1];
		temp[1]  = frame.data[2];
		temp[2]  = frame.data[3];
		temp[3]  = frame.data[4];
		temp[4]  = frame.data[5];
		temp[5]  = frame.data[6];
		temp[6]  = frame.data[7];
	}

	uint8_t max_temp, min_temp;
	uint8_t temp[7];
	
	Frame generate_frame() const;
};

class DC_Temp_1 : public Layout {
public:
	DC_Temp_1(uint8_t T7, uint8_t T8, uint8_t T9, uint8_t T10, uint8_t T11, uint8_t T12, uint8_t T13, uint8_t T14) { 

		temp[1] = T7;
		temp[2] = T8;
		temp[3] = T9;
		temp[4] = T10;
		temp[5] = T11;
		temp[6] = T12;
		temp[7] = T13;
		temp[8] = T13;

		id = DC_TEMP_1_ID; 
	}

	DC_Temp_1(uint8_t *_temps) { 

		memcpy(temp+1,_temps, 8);

		id = DC_TEMP_1_ID; 
	}

	DC_Temp_1(const Frame& frame) { 
		temp[1] = frame.data[0];
		temp[2] = frame.data[1];
		temp[3] = frame.data[2];
		temp[4] = frame.data[3];
		temp[5] = frame.data[4];
		temp[6] = frame.data[5];
		temp[7] = frame.data[6];
		temp[8] = frame.data[7];
	}

	uint8_t temp[9];
	
	Frame generate_frame() const;
};

class DC_Temp_2 : public Layout {
public:
	DC_Temp_2(uint8_t T15, uint8_t T16, uint8_t T17, uint8_t T18, uint8_t T19, uint8_t T20, uint8_t T21, uint8_t T22) { 

		temp[1] = T15;
		temp[2] = T16;
		temp[3] = T17;
		temp[4] = T18;
		temp[5] = T19;
		temp[6] = T20;
		temp[7] = T21;
		temp[8] = T22;

		id = DC_TEMP_2_ID; 
	}

	DC_Temp_2(uint8_t *_temps) { 

		memcpy(temp+1,_temps, 8);

		id = DC_TEMP_2_ID; 
	}

	DC_Temp_2(const Frame& frame) { 
		temp[1] = frame.data[0];
		temp[2] = frame.data[1];
		temp[3] = frame.data[2];
		temp[4] = frame.data[3];
		temp[5] = frame.data[4];
		temp[6] = frame.data[5];
		temp[7] = frame.data[6];
		temp[8] = frame.data[7];
	}

	uint8_t temp[9];
	
	Frame generate_frame() const;
};

class DC_Temp_3 : public Layout {
public:
	DC_Temp_3(uint8_t T23, uint8_t T24, uint8_t T25, uint8_t T26, uint8_t T27, uint8_t T28, uint8_t T29, uint8_t T30) { 

		temp[1] = T23;
		temp[2] = T24;
		temp[3] = T25;
		temp[4] = T26;
		temp[5] = T27;
		temp[6] = T28;
		temp[7] = T29;
		temp[8] = T30;

		id = DC_TEMP_3_ID; 
	}

	DC_Temp_3(uint8_t *_temps) { 

		memcpy(temp+1,_temps, 8);

		id = DC_TEMP_3_ID; 
	}

	DC_Temp_3(const Frame& frame) { 
		temp[1] = frame.data[0];
		temp[2] = frame.data[1];
		temp[3] = frame.data[2];
		temp[4] = frame.data[3];
		temp[5] = frame.data[4];
		temp[6] = frame.data[5];
		temp[7] = frame.data[6];
		temp[8] = frame.data[7];
	}

	uint8_t temp[9];
	
	Frame generate_frame() const;
};

/* 
 * Steering wheel data packet, sent to the driver controls.
 */
class SW_Data : public Layout {
public:
	byte byte0;
	byte byte1;
	bool headlights, hazards, lts, rts, horn, cruiseon, cruiseoff;
	byte gear;

	SW_Data(byte data0, byte data1) : byte0(data0), byte1(data1) { 
		id = SW_DATA_ID; 
		populate_fields();
	}

	SW_Data(byte _gear, bool _headlights, bool _hazards, bool _horn, bool _lts, bool _rts,
		bool _cruiseon, bool _cruiseoff) { 
		id = SW_DATA_ID;

		byte0 = 0;
		byte0 |= _gear;
		byte0 |= _headlights << 2;
		byte0 |= _hazards << 3;
		byte0 |= _horn << 5;
		byte0 |= _lts << 6;
		byte0 |= _rts << 7;

		byte1 = 0;
		byte1 |= _cruiseon;
		byte1 |= _cruiseoff << 1;

		populate_fields();
	}
	
	SW_Data(const Frame& frame) : byte0(frame.data[0]), byte1(frame.data[1]) { 
		id = frame.id; 
		populate_fields();
	}

	Frame generate_frame() const;

private:
	void populate_fields() { 
		gear 		= (byte0)      & 3u; // 0 = off, 1 = fwd, 2 = rev, 4 = undefined
		headlights 	= (byte0 >> 2) & 1u;
		hazards 	= (byte0 >> 3) & 1u;
		horn 		= (byte0 >> 5) & 1u;
		lts 	 	= (byte0 >> 6) & 1u;
		rts 	 	= (byte0 >> 7) & 1u;

		cruiseon 	= (byte1)	   & 1u;
		cruiseoff 	= (byte1 >> 1) & 1u;
	}
};

/*
 * Telemetry Heartbeat packet
 */
class TEL_Status : public Layout {
public:
	TEL_Status(bool sql_conn, bool com_conn) : sql_connected(sql_conn), com_connected(com_conn) { id = TEL_STATUS_ID; }

	TEL_Status(const Frame& frame) {
		id = frame.id; 

		sql_connected = (bool) (frame.data[0] & 0x01);
		com_connected = (bool) (frame.data[0] & 0x02);
	}

	Frame generate_frame() const;

	bool sql_connected;
	bool com_connected;
};

#endif
