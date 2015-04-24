/*
 * Layouts.h
 * Definition for CAN layouts.
 */

#ifndef Layouts_h
#define Layouts_h

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
#define BMS_VOLT_CURR_ID	0x6FA
#define BMS_STATUS_ID		0x6FB
#define BMS_FAN_STATUS_ID	0x6FC
#define BMS_STATUS_EXT_ID	0x6FD

//BMShub TX
#define BMSHUB_VOLT_CURR_ID 0x6FE // Reserved by Tritium, but we'll use it now.

// motor controller TX
#define MC_BASEADDRESS		0x400
#define MC_HEARTBEAT_ID		MC_BASEADDRESS
#define MC_STATUS_ID 		0x401
#define MC_BUS_STATUS_ID	0x402
#define MC_VELOCITY_ID		0x403
#define MC_PHASE_ID			0x404
#define MC_FANSPEED_ID		0x410
#define MC_ODOAMP_ID			0x414

// driver controls TX
#define DC_BASEADDRESS	0x500
#define DC_HEARTBEAT_ID		DC_BASEADDRESS
#define DC_DRIVE_ID			0x501
#define DC_POWER_ID			0x502
#define DC_RESET_ID			0x503
//#define DC_SWITCHPOS_ID		0x505
#define DC_INFO_ID			0x505

// steering wheel TX
#define SW_BASEADDRESS		0x700
#define SW_HEARTBEAT_ID		SW_BASEADDRESS
#define SW_DATA_ID 			0x701



// steering wheel data masks

/*
 * Abstract base packet.
 */
class Layout {
public:
	uint16_t id;
	
	/*
	 * Creates a Frame object to represent this layout.
	 */
	virtual Frame generate_frame() const;
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

/*
 * BMS extended status packet.
 */
class BMS_StatusExt : public Layout {
	// Implement! Need specs.
};

/*
 * Motor controller heartbeat packet.
 */
class MC_Heartbeat : public Layout {
public:
	MC_Heartbeat(uint32_t t_id, uint32_t s_no) : trituim_id(t_id), serial_no(s_no) { id = MC_HEARTBEAT_ID; }
	MC_Heartbeat(const Frame& frame) : trituim_id(frame.low), serial_no(frame.high) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t trituim_id; // is actually a char[8]
	uint32_t serial_no;
};

/*
 * Motor controller status packet.
 */
class MC_Status : public Layout {
public:
	MC_Status(uint16_t act_m, uint16_t err_f, uint16_t lim_f) : 
		active_motor(act_m), err_flags(err_f), limit_flags(lim_f)
		{ id = MC_STATUS_ID; }
	MC_Status(const Frame& frame) : active_motor(frame.s1), err_flags(frame.s2), limit_flags(frame.s3)
		{ id = frame.id; }

	Frame generate_frame() const;

	uint16_t active_motor;
	uint16_t err_flags;
	uint16_t limit_flags;
};

/*
 * Motor controller bus status packet.
 */
class MC_BusStatus : public Layout {
public:
	MC_BusStatus(float bc, float bv) : bus_current(bc), bus_voltage(bv) { id = MC_BUS_STATUS_ID; }
	MC_BusStatus(const Frame& frame) : bus_current(frame.low_f), bus_voltage(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float bus_current;
	float bus_voltage;
};

/*
 * Motor controller velocity packet.
 */
class MC_Velocity : public Layout {
public:
	MC_Velocity(float car_v, float motor_v) : car_velocity(car_v), motor_velocity(motor_v) { id = MC_VELOCITY_ID; }
	MC_Velocity(const Frame& frame) : car_velocity(frame.low_f), motor_velocity(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float car_velocity;
	float motor_velocity;
};

/*
 * Motor controller motor phase current packet.
 */
class MC_PhaseCurrent : public Layout {
public:
	MC_PhaseCurrent(float a, float b) : phase_a(a), phase_b(b) { id = MC_PHASE_ID; }
	MC_PhaseCurrent(const Frame& frame) : phase_a(frame.low_f), phase_b(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float phase_a;
	float phase_b;
};

class MC_FanSpeed : public Layout {
public:
	MC_FanSpeed(float rpm, float v) : speed(rpm), drive(v) {id = MC_FANSPEED_ID; }
	MC_FanSpeed(const Frame& frame) : speed(frame.low_f), drive(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float speed;
	float drive;
};

class MC_OdoAmp : public Layout {
public:
	MC_OdoAmp(float Ah, float odom) : bus_amphours(Ah), odometer(odom) {id = MC_ODOAMP_ID; }
	MC_OdoAmp(const Frame& frame) : bus_amphours(frame.low_f), odometer(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float bus_amphours;
	float odometer;
};

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
	DC_Power(const Frame& frame) : bus_current(frame.low_f) { id = frame.id; }

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
 * Driver controls switch position packet.
 */
/*class DC_SwitchPos : public Layout {
public:
	DC_SwitchPos(uint16_t _state) : state(_state) { id = DC_SWITCHPOS_ID; }
	DC_SwitchPos(const Frame& frame) : state(uint16_t(frame.s0)) { id = frame.id; }

  uint16_t state;

        Frame generate_frame() const;
};*/

/*
 * Driver controls information packet (this is really really messy code, I'm sorry. I'll clean it up later)
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
					uint16_t ignition) { 

			accel_ratio = accel;
			regen_ratio = regen;
			brake_engaged = brake;
			can_error_flags = can_errors;
			dc_error_flags = dc_errors;
			was_reset = reset;
			gear = current_gear;
			ignition_state = ignition;
			fuel_door = fuel;

			id = DC_INFO_ID; }

	DC_Info(const Frame& frame)
		{ 
			accel_ratio = frame.data[2]/100.0f;
			regen_ratio = frame.data[3]/100.0f;
			brake_engaged = (bool)frame.s0 & 0x0080;
			can_error_flags = frame.s2;
			dc_error_flags = frame.data[6];
			was_reset = (bool)frame.s0 & 0x0200; 
			gear = frame.s0 & 0x000F; 
			ignition_state = frame.s0 & 0x0070;
			fuel_door = (bool)frame.s0 & 0x0100;

			id = frame.id; }

	float accel_ratio, regen_ratio; // these will be stored as integers 0-100 in frame 
	uint16_t can_error_flags;
	byte dc_error_flags, gear;
	bool brake_engaged, was_reset, fuel_door;
	uint16_t ignition_state;

	Frame generate_frame() const;
};

/* 
 * Steering wheel data packet, sent to the driver controls.
 */
class SW_Data : public Layout {
public:
	SW_Data(byte data) : flags(data) { 
		id = SW_DATA_ID; 
		populate_fields();
	}
	SW_Data(byte _gear, bool _headlights, bool _hazards, bool _cruisectrl, bool _horn, bool _lts, bool _rts) { 
		id = SW_DATA_ID;
		flags = 0;
		flags |= _gear;
		flags |= _headlights << 2;
		flags |= _hazards << 3;
		flags |= _cruisectrl << 4;
		flags |= _horn << 5;
		flags |= _lts << 6;
		flags |= _rts << 7;

		populate_fields();
	}
	SW_Data(const Frame& frame) : flags(frame.data[0]) { 
		id = frame.id; 
		populate_fields();
	}

	byte flags;

	//data members
	bool headlights, hazards, lts, rts, cruisectrl, horn;
	byte gear;

	Frame generate_frame() const;

private:
	void populate_fields() { 
		gear 		= (flags)      & 3u; //Get first two bits. 0 = off, 1 = fwd, 2 = rev, 4 = undefined
		headlights 	= (flags >> 2) & 1u;
		hazards 	= (flags >> 3) & 1u;
		cruisectrl  = (flags >> 4) & 1u;
		horn 		= (flags >> 5) & 1u;
		lts 	 	= (flags >> 6) & 1u;
		rts 	 	= (flags >> 7) & 1u;
	}
};

/*
 * BMShub voltage and current packet.
 */
class BMShub_VoltageCurrent : public Layout {
public:
	BMShub_VoltageCurrent(float v, float c) : voltage(v), current(c) { id = BMSHUB_VOLT_CURR_ID; }
	BMShub_VoltageCurrent(const Frame& frame) : voltage(frame.low_f), current(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float voltage;
	float current;
};
#endif
