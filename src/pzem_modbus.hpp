/*
PZEM EDL - PZEM Event Driven Library

This code implements communication and data exchange with PZEM004T V3.0 module using MODBUS proto
and provides an API for energy metrics monitoring and data processing.

This file is part of the 'PZEM event-driven library' project.

Copyright (C) Emil Muratov, 2021
GitHub: https://github.com/vortigont/pzem-edl
*/

#pragma once
#include <Arduino.h>

namespace pzmbus {

// Read-Only 16-bit registers
#define REG_VOLTAGE             0x0000  // 1LSB correspond to 0.1 V
#define REG_CURRENT_L           0x0001  // 1LSB correspond to 0.001 A
#define REG_CURRENT_H           0X0002
#define REG_POWER_L             0x0003  // 1LSB correspond to 0.1 W
#define REG_POWER_H             0x0004
#define REG_ENERGY_L            0x0005  // 1LSB correspond to 1 W*h
#define REG_ENERGY_H            0x0006
#define REG_FREQUENCY           0x0007  // 1LSB correspond to 0.1 Hz
#define REG_PF                  0x0008  // 1LSB correspond to 0.01
#define REG_ALARM               0x0009  // 0xFFFF is alarm / 0x0000 is not alarm
#define REG_METER_DATA_START    REG_VOLTAGE
#define REG_METER_DATA_LEN      0x0A
#define REG_METER_RESP_LEN      0x14

// RW 16-bit registers
#define WREG_ALARM_THR          0x0001  // Alarm threshold, 1LSB correspond to 1W
#define WREG_ADDR               0x0002  // MODBUS Slave address register   (The range is 0x0001~0x00F7)

// Commands
#define CMD_RHR                 0x03    // Read Holding Register    (Read RW regs)
#define CMD_RIR                 0X04    // Read Input Register      (Read RO regs)
#define CMD_WSR                 0x06    // Write Single Register
#define CMD_CAL                 0x41    // Calibration
#define CMD_RST_ENRG            0x42    // Reset energy
#define CMD_RERR                0x84    // Read  Command error
#define CMD_WERR                0x86    // Write Command error
#define CMD_CALERR              0xC1    // Calibration Command error
#define CMD_RSTERR              0xC2    // Reset Energy Command error

// Slave addressing
#define ADDR_BCAST              0x00    // broadcast address    (slaves are not supposed to answer here)
#define ADDR_MIN                0x01    // lowest slave address
#define ADDR_MAX                0xF7    // highest slave address
#define ADDR_ANY                0xF8    // default catch-all address

// ERR Codes
#define ERR_FUNC                0x01    // Illegal function
#define ERR_ADDR                0x02    // Illegal address
#define ERR_DATA                0x03    // Illegal data
#define ERR_SLAVE               0x04    // Slave error

// Factory calibration
#define CAL_ADDR                ADDR_ANY    // Calibration address
#define CAL_PWD                 0x3721      // Calibration password

// Power Alarm
#define ALARM_PRESENT           0xffff
#define ALARM_ABSENT            0x0000

#define GENERIC_MSG_SIZE        8
#define ENERGY_RST_MSG_SIZE     4
#define REPORT_ADDR_MSG_SIZE    5 

#define PZEM_REFRESH_PERIOD     1000    // PZEM updates it's internal register data every ~1 sec


// ESP32 is little endian here

// Enumeration of available energy metrics
enum class meter_t:uint8_t { vol, cur, pwr, enrg, frq, pf, alrm };

// Enumeration of available MODBUS commands
enum class pzemcmd_t:uint8_t {
    RHR = CMD_RHR,
    RIR = CMD_RIR,
    WSR = CMD_WSR,
    calibrate = CMD_CAL,
    RESET_ENERGY = CMD_RST_ENRG,
    read_err = CMD_RERR,
    write_err = CMD_WERR,
    calibrate_err = CMD_CALERR,
    reset_err = CMD_RSTERR
};

// Some of the possible Error states
enum class pzem_err_t:uint8_t {
    err_ok = 0,
    err_func = ERR_FUNC,
    err_addr = ERR_ADDR,
    err_data = ERR_DATA,
    err_slave = ERR_SLAVE,
    err_parse                   // error parsing reply
};

// Check MODBUS CRC16 over provided data vector
bool checkcrc16(const uint8_t *buf, uint16_t len);

/**
 * @brief Structure with crafted PZEM message command data
 * ment to be sent to the PZEM device
 */
struct TX_msg {
    const size_t len;       // msg size
    uint8_t* data;          // data pointer
    bool w4rx;              // 'wait for reply' - a reply for message expected, should block TX queue handler

    TX_msg(const size_t size = GENERIC_MSG_SIZE, bool rxreq = true) : len(size), w4rx(rxreq) {
        data = (uint8_t*)calloc(len, sizeof(uint8_t));
        //memcpy(data, srcdata, len);
    }
    ~TX_msg(){ delete[] data; }
};

/**
 * @brief struct with PZEM reply message
 * 
 */
struct RX_msg {
    uint8_t *rawdata;                               // raw serial data pointer
    const size_t len;                               // msg size
    const bool valid;                               // valid MODBUS message (CRC16 OK)
    const uint8_t addr = rawdata[0];                // slave address
    const pzemcmd_t cmd = (pzemcmd_t)rawdata[1];    // command code

    RX_msg(uint8_t *data, const size_t size) : rawdata(data), len(size), valid(checkcrc16(data, size)) {}
    ~RX_msg(){ delete[] rawdata; rawdata = nullptr; }
};


/**
 * @brief struct with energy metrics data
 * contains raw-mapped byte values
 * 
 * this struct is nicely 32-bit aligned :)
 */
struct metrics {
    uint16_t voltage=0;
    uint32_t current=0;
    uint32_t power=0;
    uint32_t energy=0;
    uint16_t freq=0;
    uint16_t pf=0;
    uint16_t alarm=0;

    float asFloat(meter_t m) const {
        switch (m)
        {
        case meter_t::vol :
            return voltage / 10.0;
            break;
        case meter_t::cur :
            return current / 1000.0;
            break;
        case meter_t::pwr :
            return power / 10.0;
            break;
        case meter_t::enrg :
            return static_cast< float >(energy);
            break;
        case meter_t::frq :
            return freq / 10.0;
            break;
        case meter_t::pf :
            return pf / 100.0;
            break;
        case meter_t::alrm :
            return alarm ? 1.0 : 0.0;
            break;
        default:
            return NAN;
        }
    }

    bool parse_rx_msg(const RX_msg *m){
        if (m->cmd != pzemcmd_t::RIR || m->rawdata[2] != REG_METER_RESP_LEN)
            return false;

        uint8_t const *value = &m->rawdata[3];

        voltage = __builtin_bswap16(*(uint16_t*)&value[REG_VOLTAGE*2]);
        current = __builtin_bswap16(*(uint16_t*)&value[REG_CURRENT_L*2]) | __builtin_bswap16(*(uint16_t*)&value[REG_CURRENT_H*2]  << 16);
        power   = __builtin_bswap16(*(uint16_t*)&value[REG_POWER_L*2])   | __builtin_bswap16(*(uint16_t*)&value[REG_POWER_H*2]    << 16);
        energy  = __builtin_bswap16(*(uint16_t*)&value[REG_ENERGY_L*2])  | __builtin_bswap16(*(uint16_t*)&value[REG_ENERGY_H*2]   << 16);
        freq    = __builtin_bswap16(*(uint16_t*)&value[REG_FREQUENCY*2]);
        pf      = __builtin_bswap16(*(uint16_t*)&value[REG_PF*2]);
        alarm   = __builtin_bswap16(*(uint16_t*)&value[REG_ALARM*2]);
        return true;
    }
};

/**
 * @brief a structure that reflects PZEM state/data values
 * 
 */
struct pzem_state {
    uint8_t addr = ADDR_ANY;
    metrics data;
    uint16_t alrm_thrsh=0;
    bool alarm=false;
    pzem_err_t err;
    int64_t poll_us=0;     // last poll request sent time, microseconds since boot
    int64_t update_us=0;   // last succes update time, us since boot

    /**
     * @brief return age time since last update in ms
     * 
     * @return int64_t age time in ms
     */
    int64_t dataAge() const { return (esp_timer_get_time() - update_us)/1000; }

    /**
     * @brief update poll_us to current value
     * should be called on each request set to PZEM
     * 
     */
    void reset_poll_us(){ poll_us=esp_timer_get_time(); }

    /**
     * @brief data considered stale if last update time is more than PZEM_REFRESH_PERIOD
     * 
     * @return true if stale
     * @return false if data is fresh and valid
     */
    bool dataStale() const {return (esp_timer_get_time() - update_us > PZEM_REFRESH_PERIOD );}

    /**
     * @brief try to parse PZEM reply packet and update structure state
     * 
     * @param m RX message struct
     * @param skiponbad try to parse even packet has wrong MODBUS add or has bad CRC
     * @return true on success
     * @return false on error
     */
    bool parse_rx_mgs(const RX_msg *m, bool skiponbad=true){
        if (!m->valid && skiponbad)          // check if message is valid before parsing it further
            return false;

        if (m->addr != addr && skiponbad)    // this is not "my" packet
            return false;

        switch (m->cmd){
            case pzemcmd_t::RIR : {
                if(data.parse_rx_msg(m))  // try to parse it as a full metrics packet
                    break;
                else {
                    err = pzem_err_t::err_parse;
                    return false;
                }
            }
            case pzemcmd_t::RHR : {
                if(m->rawdata[2] == WREG_ALARM_THR){        // alarm threshold
                    alrm_thrsh = __builtin_bswap16(*(uint16_t*)&m->rawdata[3]);           // from the 4th byte data follows
                } else if (m->rawdata[2] == WREG_ADDR){     // it's a modbus addr
                    addr = (uint8_t)__builtin_bswap16(*(uint16_t*)&m->rawdata[3]);
                    break;
                }
                // unknown regs
                break;
            }
            case pzemcmd_t::WSR : {
                // 4th byte is reg ADDR_L
                if (m->rawdata[3] == WREG_ADDR){
                    addr = m->rawdata[5];            // addr is only one byte
                    break;
                } else if(m->rawdata[3] == WREG_ALARM_THR){
                    alrm_thrsh = __builtin_bswap16(*(uint16_t*)&m->rawdata[4]);
                }
                break;
            }
            case pzemcmd_t::read_err :
            case pzemcmd_t::write_err :
            case pzemcmd_t::reset_err :
            case pzemcmd_t::calibrate_err :
                // стоит ли здесь инвалидировать метрики???
                err = (pzem_err_t)m->rawdata[2];
                return true;
            default:
                break;
        }

        err = pzem_err_t::err_ok;
        update_us = esp_timer_get_time();
        return true;
    }

};


/**
 * @brief Create a msg object with PZEM command wrapped into proper MODBUS message
 * this is a genereic command template
 * 
 * @param cmd - PZEM command
 * @param reg_addr - register address
 * @param value - command value
 * @param slave_addr - slave device modbus address
 * @param w4r - 'wait-4-reply' expexted flag
 * @return TX_msg* 
 */
TX_msg* create_msg(pzemcmd_t cmd, uint16_t reg_addr, uint16_t value, uint8_t slave_addr = ADDR_ANY, bool w4r = true);

/**
 * @brief message request for all energy metrics
 * 
 * @param addr - slave device modbus address
 * @return TX_msg* 
 */
TX_msg* cmd_get_metrics(uint8_t addr = ADDR_ANY);

/**
 * @brief  message request to change slave device modbus address
 * 
 * @param addr - new modbus address
 * @param current_addr - current modbus address
 * @return TX_msg* 
 */
TX_msg* cmd_set_modbus_addr(uint8_t addr, const uint8_t current_addr = ADDR_ANY);

/**
 * @brief message request to report current slave device modbus address
 * 
 * @param addr 
 * @return TX_msg* 
 */
TX_msg* cmd_get_modbus_addr(const uint8_t addr = ADDR_ANY);

/**
 * @brief message request to set new Power Alarm threshold value
 * 
 * @param w - watts threshold value
 * @param addr 
 * @return TX_msg* 
 */
TX_msg* cmd_set_alarm_thr(uint16_t w, const uint8_t addr = ADDR_ANY);

/**
 * @brief message request to get current Power Alarm threshold value
 * 
 * @param addr 
 * @return TX_msg* 
 */
TX_msg* cmd_get_alarm_thr(const uint8_t addr = ADDR_ANY);


/**
 * @brief create MSG - reset PZEM's Energy counter to zero
 * 
 * @param addr - device address
 * @return TX_msg* pointer to the message struct
 */
TX_msg* cmd_energy_reset(const uint8_t addr = ADDR_ANY);


/**
 * @brief calculate crc16 over *data array using CRC16_MODBUS precomputed table
 * 
 * @param data - byte array, must be 2 bytes at least
 * @param size  - array size
 * @return uint16_t CRC16
 */
uint16_t crc16(const uint8_t *data, uint16_t size);

/**
 * @brief set CRC16 to the byte array at position (len-2)
 * CRC16 calculated up to *data[len-2] bytes
 * @param data - source array fer
 * @param len - array size
 */
void setcrc16(uint8_t *data, uint16_t len);

/**
 * @brief dump content of received packet to the stdout
 * 
 * @param m 
 */
void rx_msg_debug(const RX_msg *m);

/**
 * @brief dump content of transmitted packet to the stdout
 * 
 * @param m 
 */
void tx_msg_debug(const TX_msg *m);

/**
 * @brief pretty print the content of RX packet data
 * 
 * @param m PZEM RX packet structure
 */
void rx_msg_prettyp(const RX_msg *m);
}