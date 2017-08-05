/**
  \file MCP2515_defs.h 

  \brief Library definitions for Microchip MCP2515 CAN Controller
  
  Author: David Harding
  
  Created: 11/08/2010
  
  For further information see:
  
  http://ww1.microchip.com/downloads/en/DeviceDoc/21801e.pdf

  http://en.wikipedia.org/wiki/CAN_bus
*/

#ifndef MCP2515_defs_h
#define MCP2515_defs_h

#include <stdint.h>
#include "Arduino.h"

/** \class Frame
 ** \brief An object representing a generic CAN frame
 */
typedef struct
{
      unsigned long id;      //!< EID if ide set, SID otherwise
      byte srr = 0;          //!< Standard Frame Remote Transmit Request flag
      byte rtr = 0;          //!< Remote Transmission Request flag
      byte ide = 0;          //!< Extended ID flag
      byte dlc = 0;          //!< Number of data bytes
      /** 
      */

      /** \name Frame Data Access variables 
       ** An annonymous union which overlays multiple variable types to allow the data in a CAN frame to be accessed in
          many different ways (e.g. as 8 individual bytes, 4 shorts, 4 signed integers, 2 floats, etc.)
       */
      /**@{*/

      //! Annonymous union of frame data
      union {
        // 8 bytes
        uint64_t value;             //!< Frame data as a single unsigned 8 byte integer
        // 8 bytes (signed)
        int64_t  value_s;           //!< Frame data as a single signed 8 byte integer
        // 4 bytes (float)
        struct {
          float low_f;              //!< First  4 bytes of Frame data as a float
          float high_f;             //!< Second 4 bytes of Frame data as a float
        };
        // 4 bytes (int)
        struct {
          uint32_t low;             //!< First  4 bytes of Frame data as a 4 byte unsigned integer
          uint32_t high;            //!< Second 4 bytes of Frame data as a 4 byte unsigned integer
        };
        // 4 bytes (signed int)
        struct {
          signed long  low_s;       //!< First  4 bytes of Frame data as a 4 byte signed integer
          signed long  high_s;      //!< Second 4 bytes of Frame data as a 4 byte signed integer
        };
        // 2 bytes
        struct {
          uint16_t s0;              //!< First  2 bytes of Frame data as a short (unsigned integer)
          uint16_t s1;              //!< Second 2 bytes of Frame data as a short (unsigned integer)
          uint16_t s2;              //!< Third  2 bytes of Frame data as a short (unsigned integer)
          uint16_t s3;              //!< Fourth 2 bytes of Frame data as a short (unsigned integer)
        };
        // 2 bytes (signed)
        struct {
          signed int i0;              //!< First  2 bytes of Frame data as a signed integer
          signed int i1;              //!< Second 2 bytes of Frame data as a signed integer
          signed int i2;              //!< Third  2 bytes of Frame data as a signed integer
          signed int i3;              //!< Fourth 2 bytes of Frame data as a signed integer
        };
        // 1 byte
        uint8_t data[8];              //!< All 8 bytes of Frame data packaged as individual elements of an array.
      };
      /**@}*/

  /** \brief Converts the data in the Frame to a string format which can be printed out over Serial */
  String toString()
  {
    char fstring[64];
    if (ide)            // Check whether the Frame is an extended frame or not
      sprintf(fstring, "E%03X|%02X%02X%02X%02X%02X%02X%02X%02X", id, data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
    else
      sprintf(fstring, "S%03X|%02X%02X%02X%02X%02X%02X%02X%02X", id, data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
    return String(fstring);
  }

} Frame;

//! \name MCP2515 SPI Commands
/**@{*/
#define CAN_RESET       0xC0
#define CAN_READ        0x03
#define CAN_WRITE       0x02
#define CAN_RTS         0x80
#define CAN_STATUS      0xA0
#define CAN_BIT_MODIFY  0x05  
#define CAN_RX_STATUS   0xB0
#define CAN_READ_BUFFER 0x90
#define CAN_LOAD_BUFFER 0X40  
/**@}*/

//! \name CANSTAT Register Bit Masks
/**@{*/
#define MODE_CONFIG            0x80
#define MODE_LISTEN            0x60
#define MODE_LOOPBACK          0x40
#define MODE_SLEEP             0x20
#define MODE_NORMAL            0x00
/**@}*/

//! \name CANINTF (interrupt flag) Register Bit Masks
/**@{*/
#define RX0IF                  0x01   //!< RX buffer 0 interrupt flag
#define RX1IF                  0x02   //!< RX buffer 1 interrupt flag
#define TX0IF                  0x04   //!< TX buffer 0 interrupt flag
#define TX1IF                  0x08   //!< TX buffer 1 interrupt flag
#define TX2IF                  0x10   //!< TX buffer 2 interrupt flag
#define ERRIF                  0x20   //!< CAN bus error interrupt flag
#define WAKIF                  0x40   //!< On-Wake interrupt flag
#define MERRF                  0x80   //!< Transmission error interrupt flag
#define INTALL                 0xFF   //!< Shortcut for all interrupt flags
/**@}*/

//! \name CANINTE (interrupt enable) Register Bit Masks
/**@{*/
#define RX0IE                  0x01   //!< RX buffer 0 interrupt enable flag
#define RX1IE                  0x02   //!< RX buffer 1 interrupt enable flag
#define TX0IE                  0x04   //!< TX buffer 0 interrupt enable flag
#define TX1IE                  0x08   //!< TX buffer 1 interrupt enable flag
#define TX2IE                  0x10   //!< TX buffer 2 interrupt enable flag
#define ERRIE                  0x20   //!< CAN bus error interrupt enable flag
#define WAKIE                  0x40   //!< On-Wake interrupt enable flag
#define MERRE                  0x80   //!< Transmission error interrupt enable flag
/**@}*/

//! \name Configuration Registers
/**@{*/
#define CANSTAT         0x0E
#define CANCTRL         0x0F
#define BFPCTRL         0x0C
#define TEC             0x1C
#define REC             0x1D
#define CNF3            0x28
#define CNF2            0x29
#define CNF1            0x2A
#define CANINTE         0x2B
#define CANINTF         0x2C
#define EFLG            0x2D
#define TXRTSCTRL       0x0D
/**@}*/

//! \name TX Buffer 0 Registers
/**@{*/
#define TXB0CTRL        0x30
#define TXB0SIDH        0x31
#define TXB0SIDL        0x32
#define TXB0EID8        0x33
#define TXB0EID0        0x34
#define TXB0DLC         0x35
#define TXB0D0          0x36
#define TXB0D1          0x37
#define TXB0D2          0x38
#define TXB0D3          0x39
#define TXB0D4          0x3A
#define TXB0D5          0x3B
#define TXB0D6          0x3C
#define TXB0D7          0x3D
/**@}*/
                         
//! \name TX Buffer 1 Registers
/**@{*/
#define TXB1CTRL        0x40
#define TXB1SIDH        0x41
#define TXB1SIDL        0x42
#define TXB1EID8        0x43
#define TXB1EID0        0x44
#define TXB1DLC         0x45
#define TXB1D0          0x46
#define TXB1D1          0x47
#define TXB1D2          0x48
#define TXB1D3          0x49
#define TXB1D4          0x4A
#define TXB1D5          0x4B
#define TXB1D6          0x4C
#define TXB1D7          0x4D
/**@}*/

//! \name TX Buffer 2 Registers
/**@{*/
#define TXB2CTRL        0x50
#define TXB2SIDH        0x51
#define TXB2SIDL        0x52
#define TXB2EID8        0x53
#define TXB2EID0        0x54
#define TXB2DLC         0x55
#define TXB2D0          0x56
#define TXB2D1          0x57
#define TXB2D2          0x58
#define TXB2D3          0x59
#define TXB2D4          0x5A
#define TXB2D5          0x5B
#define TXB2D6          0x5C
#define TXB2D7          0x5D
/**@}*/
                         
//! \name RX Buffer 0 Registers
/**@{*/
#define RXB0CTRL        0x60
#define RXB0SIDH        0x61
#define RXB0SIDL        0x62
#define RXB0EID8        0x63
#define RXB0EID0        0x64
#define RXB0DLC         0x65
#define RXB0D0          0x66
#define RXB0D1          0x67
#define RXB0D2          0x68
#define RXB0D3          0x69
#define RXB0D4          0x6A
#define RXB0D5          0x6B
#define RXB0D6          0x6C
#define RXB0D7          0x6D
/**@}*/
                         
//! \name RX Buffer 1 Registers
/**@{*/
#define RXB1CTRL        0x70
#define RXB1SIDH        0x71
#define RXB1SIDL        0x72
#define RXB1EID8        0x73
#define RXB1EID0        0x74
#define RXB1DLC         0x75
#define RXB1D0          0x76
#define RXB1D1          0x77
#define RXB1D2          0x78
#define RXB1D3          0x79
#define RXB1D4          0x7A
#define RXB1D5          0x7B
#define RXB1D6          0x7C
#define RXB1D7          0x7D
/**@}*/

//! \name Buffer Bit Masks
/**@{*/
#define RXB0            0x00
#define RXB1            0x02
#define TXB0            0x01
#define TXB1            0x02
#define TXB2            0x04
#define TXBALL          TXB0 | TXB1 | TXB2
#define TXBANY          0x00                // Specially defined for this library, allows the controller object to choose which buffer to use.
/**@}*/

//! \name Buffer Bit Masks
//! \todo description
/**@{*/
#define RXB0Full        0x01;
#define RXB1Full        0x02;
#define RXB_All         RXB1Full | RXB2Full
/**@}*/

//! \name RX Filter Registers
//! Filter registers for the RX buffers (RXF0-1 -> RXB0, RXF2-5 -> RXB1)
/**@{*/
#define RXF0SIDH   0x00
#define RXF0SIDL   0x01
#define RXF1SIDH   0x04
#define RXF1SIDL   0x05
#define RXF2SIDH   0x08
#define RXF2SIDL   0x09
#define RXF3SIDH   0x10
#define RXF3SIDL   0x11
#define RXF4SIDH   0x14
#define RXF4SIDL   0x15
#define RXF5SIDH   0x18
#define RXF5SIDL   0x19
/**@}*/

//! \name RX Filter Mask Registers
//! Mask registers for the RX buffers (RXM0 -> RXB0, RXM1 -> RXB1)
/**@{*/
#define RXM0SIDH 0x20
#define RXM0SIDL 0x21
#define RXM1SIDH 0x24
#define RXM1SIDL 0x25
/**@}*/

#endif
