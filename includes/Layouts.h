/**
 ** \file Layouts.h
 ** Declarations for NUsolar specific CAN packet layouts.
 */

#ifndef Layouts_h
#define Layouts_h

#include <stdint.h>
#include "MCP2515_defs.h"

//! \name BMS Packet IDs
//! See http://www.tritium.com.au/source/TRI67.010v2_BMS_BMU_Communications_Protocol.pdf
/**@{*/
#define BMS_BASEADDRESS 	0x600
#define BMS_HEARTBEAT_ID	BMS_BASEADDRESS
#define BMS_SOC_ID	        0x6F4
#define BMS_BAL_SOC_ID		0x6F5
#define BMS_PRECHARGE_ID	0x6F7
#define BMS_VOLT_CURR_ID	0x6FA
#define BMS_STATUS_ID		0x6FB
#define BMS_FAN_STATUS_ID	0x6FC
#define BMS_STATUS_EXT_ID	0x6FD
/**@}*/

//! \name Motor Controller Packet IDs
//! See http://www.tritium.com.au/wp-content/uploads/2017/06/TRI50.008_Communications_Specification_v5.pdf
/**@{*/
#define MC_BASEADDRESS		0x400
#define MC_HEARTBEAT_ID		MC_BASEADDRESS
#define MC_STATUS_ID 		0x401
#define MC_BUS_STATUS_ID	0x402
#define MC_VELOCITY_ID		0x403
#define MC_PHASE_ID			0x404
#define MC_FANSPEED_ID		0x40A
#define MC_ODOAMP_ID		0x40E
/**@}*/

//! \name Driver Controlls Packet IDs
/**@{*/
#define DC_BASEADDRESS		0x500
#define DC_HEARTBEAT_ID		DC_BASEADDRESS
#define DC_DRIVE_ID			0x501
#define DC_POWER_ID			0x502
#define DC_RESET_ID			0x503
#define DC_INFO_ID			0x505
#define DC_STATUS_ID		0x506
/**@}*/

//! \name Steering Wheel Packet IDs
//! Note: May only go up to 0x7F0 because Tritium has reserved addressses after that for their other products
/**@{*/
#define SW_BASEADDRESS		0x700
#define SW_HEARTBEAT_ID		SW_BASEADDRESS
#define SW_DATA_ID 			0x701
/**@}*/

//! \name Telemetry Packet IDs
/**@{*/
#define TEL_BASEADDRESS		0x300
#define TEL_HEARTBEAT_ID 	TEL_BASEADDRESS
#define TEL_STATUS_ID		0x301
/**@}*/

//! \name Packet ID Masks
//! Some useful mask IDs to use with the MCP2515 filtering function.
/**@{*/
#define MASK_NONE			0x000000 	//!< Matches nothing. Honestly I don't see a use for this.
#define MASK_Sx00			0x000700	//!< Matches only the first hex value of packet IDs.
#define MASK_Sxx0			0x0007F0	//!< Matches the first two hex values of packet IDs.
#define MASK_Sxxx			0x0007FF	//!< Matches the first three hex values of packet IDs.
#define MASK_EID			0x07FFFF	//!< Matches all four values of an extended packet ID (untested).
/**@}*/

/**
 ** \brief Abstract Layout Packet Base Class
 **
 ** Each Layout object contains an ID representing the Frame ID, a virtual function  Children of the Layout class contain named variables specific to their application. 
 ** For example, the DC_Drive packet contains two variables. 
 ** \todo Finish describing the parent-child relationship between individual layouts and this class.
 */
class Layout {
public:
	uint16_t id;
	
	/**
	 ** \brief Creates a Frame object to represent this layout.
	 */
	virtual Frame generate_frame() const;

	/**
	 ** \brief Generate a string representing the packet data
	 */
	String toString() const {
		return generate_frame().toString();
	}

protected:
	/**
	 ** \brief Fill out the header info for a frame.
	 */
	 inline void set_header(Frame& f, byte size = 8) const;
};

/** \defgroup LAYOUTS NUsolar CAN packet layouts 
 ** \todo Add Detailed Description
 ** @{
 */

/**
 ** \brief BMS heartbeaat packet.
 */
class BMS_Heartbeat : public Layout {
public:
	BMS_Heartbeat(uint32_t d_id, uint32_t s_no) : device_id(d_id), serial_no(s_no) { id = BMS_HEARTBEAT_ID; }
	BMS_Heartbeat(const Frame& frame) : device_id(frame.low), serial_no(frame.high) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t device_id;
	uint32_t serial_no;
};

/**
 ** \brief BMS state of charging packet.
 */
class BMS_SOC : public Layout {
public:
	BMS_SOC(float pow_cons, float per_SOC) : power_consumed(pow_cons), percent_SOC(per_SOC) { id = BMS_SOC_ID; }
	BMS_SOC(const Frame& frame) : power_consumed(frame.low_f), percent_SOC(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float power_consumed;
	float percent_SOC;
};

/**
 ** \brief BMS state of charging during balancing packet.
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

/**
 ** \brief BMS precharge status packet.
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

/**
 ** \brief BMS voltage and current packet.
 */
class BMS_VoltageCurrent : public Layout {
public:
	BMS_VoltageCurrent(uint32_t v, int32_t c) : voltage(v), current(c) { id = BMS_VOLT_CURR_ID; }
	BMS_VoltageCurrent(const Frame& frame) : voltage(frame.low), current(frame.high_s) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t voltage;
	int32_t current;
};

/**
 ** \brief BMS status packet.
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

/**
 ** \brief BMS Extended Status Packet.
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

/**
 ** \brief BMS fan status packet.
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

/**
 ** \brief Motor controller heartbeat packet.
 */
class MC_Heartbeat : public Layout {
public:
	MC_Heartbeat(uint32_t t_id, uint32_t s_no) : trituim_id(t_id), serial_no(s_no) { id = MC_HEARTBEAT_ID; }
	MC_Heartbeat(const Frame& frame) : trituim_id(frame.low), serial_no(frame.high) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t trituim_id; // is actually a char[8]
	uint32_t serial_no;
};

/**
 ** \brief Motor controller status packet.
 */
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

/**
 ** \brief Motor controller bus status packet.
 */
class MC_BusStatus : public Layout {
public:
	MC_BusStatus(float bc, float bv) : bus_current(bc), bus_voltage(bv) { id = MC_BUS_STATUS_ID; }
	MC_BusStatus(const Frame& frame) : bus_current(frame.high_f), bus_voltage(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float bus_current;
	float bus_voltage;
};

/**
 ** \brief Motor controller velocity packet.
 */
class MC_Velocity : public Layout {
public:
	MC_Velocity(float car_v, float motor_v) : car_velocity(car_v), motor_velocity(motor_v) { id = MC_VELOCITY_ID; }
	MC_Velocity(const Frame& frame) : car_velocity(frame.high_f), motor_velocity(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float car_velocity;
	float motor_velocity;
};

/**
 ** \brief Motor controller motor phase current packet.
 */
class MC_PhaseCurrent : public Layout {
public:
	MC_PhaseCurrent(float a, float b) : phase_a(a), phase_b(b) { id = MC_PHASE_ID; }
	MC_PhaseCurrent(const Frame& frame) : phase_a(frame.high_f), phase_b(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float phase_a;
	float phase_b;
};

/**
 ** \brief Motor controller fan speed packet.
 */
class MC_FanSpeed : public Layout {
public:
	MC_FanSpeed(float rpm, float v) : speed(rpm), drive(v) {id = MC_FANSPEED_ID; }
	MC_FanSpeed(const Frame& frame) : speed(frame.high_f), drive(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float speed;
	float drive;
};

/**
 ** \brief Motor controller odometer and amp-hours packet.
 */
class MC_OdoAmp : public Layout {
public:
	MC_OdoAmp(float Ah, float odom) : bus_amphours(Ah), odometer(odom) {id = MC_ODOAMP_ID; }
	MC_OdoAmp(const Frame& frame) : bus_amphours(frame.high_f), odometer(frame.low_f) { id = frame.id; }

	Frame generate_frame() const;

	float bus_amphours;
	float odometer;
};

/**
 ** \brief Driver controls heartbeat packet.
 */
class DC_Heartbeat : public Layout {
public:
	DC_Heartbeat(uint32_t d_id, uint32_t s_no) : dc_id (d_id), serial_no (s_no) { id = DC_HEARTBEAT_ID; }
	DC_Heartbeat(Frame& frame) : dc_id(frame.low), serial_no(frame.high) { id = frame.id; }

	Frame generate_frame() const;

	uint32_t dc_id;
	uint32_t serial_no;
};

/**
 ** \brief Driver controls drive command packet.
 */
class DC_Drive : public Layout {
public:
	DC_Drive(float v, float c) : velocity(v), current(c) { id = DC_DRIVE_ID; }
	DC_Drive(const Frame& frame) : velocity(frame.low_f), current(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float velocity;
	float current;
};

/**
 ** \brief Driver controls power command packet.
 */
class DC_Power : public Layout {
public:
	DC_Power(float bc) : bus_current(bc) { id = DC_POWER_ID; }
	DC_Power(const Frame& frame) : bus_current(frame.high_f) { id = frame.id; }

	Frame generate_frame() const;

	float bus_current;
};

/**
 ** \brief Driver controls reset packet.
 */
class DC_Reset : public Layout {
public:
	DC_Reset() { id = DC_RESET_ID; }
	DC_Reset(const Frame& frame) { id = frame.id; }

	Frame generate_frame() const;
};

/**
 ** \brief Driver controls information packet.
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

		id = frame.id; 
	}

	float accel_ratio, regen_ratio; // these will be stored as integers 0-100 in frame 
	uint16_t can_error_flags;
	byte dc_error_flags, gear;
	bool brake_engaged, was_reset, fuel_door;
	uint16_t ignition_state;
	
	Frame generate_frame() const;
};

/**
 ** \brief Driver controls status packet.
 */
class DC_Status : public Layout {
public:
	DC_Status(uint16_t can_error, uint8_t error1, uint8_t error2,
		uint8_t status1, uint8_t status2) { 

		this->can_error = can_error;
		this->error1 = error1;
		this->error2 = error2;
		this->status1 = status1;
		this->status2 = status2;

		id = DC_STATUS_ID; 
	}

	DC_Status(const Frame& frame) { 
		// bytes 0, 1
		can_error  = frame.s0;
		
		// byte 2
		error1 = frame.data[2];

		// byte 3
		error2 = frame.data[3];

		// byte 4
		status1 = frame.data[4];

		// byte 5
		status2 = frame.data[5];

		id = frame.id; 
	}

	uint8_t error1, error2, status1, status2;
	uint16_t can_error;
	
	Frame generate_frame() const;
};


/** 
 ** \brief Steering wheel data packet, sent to the driver controls.
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

/**
 ** \brief Telemetry Heartbeat packet
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

/**@} End LAYOUTS group */

#endif
