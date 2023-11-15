/*****************************************************************************
 * Copyright (c) 2023
 * Skyfend Technology Co., Ltd
 *
 * All rights reserved. Any unauthorized disclosure or publication of the
 * confidential and proprietary information to any other party will constitute
 * an infringement of copyright laws.
 *
 * $Id$
 * Author  : JiaWei, jiawei@autel.com
 * Created : 8 October 2023
 * Purpose : Provide rechargeable lithium-ion battery method
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */

#include "libattery.hpp"

#include "eap_pub.h"

namespace hal
{
namespace driver
{

const std::uint8_t REG_ADDR_TEMPTERATURE          = 0;
const std::uint8_t REG_ADDR_VOLTAGE               = 1;
const std::uint8_t REG_ADDR_CURRENT               = 2;
const std::uint8_t REG_ADDR_RELATIVESTATEOFCHARGE = 3;
const std::uint8_t REG_ADDR_REMAININGCAPACITY     = 4;
const std::uint8_t REG_ADDR_FULLCHARGECAPACITY    = 5;
const std::uint8_t REG_ADDR_AVERAGETIMETOEMPTY    = 6;
const std::uint8_t REG_ADDR_AVERAGETIMETOFULL     = 7;
const std::uint8_t REG_ADDR_CYCLECOUNT            = 8;
const std::uint8_t REG_ADDR_DESIGNCAPACITY        = 9;
const std::uint8_t REG_ADDR_MANUFACTURERDATE      = 0x0A;
const std::uint8_t REG_ADDR_SERIALNUMBER          = 0x0B;
const std::uint8_t REG_ADDR_MANUFACTURERNAME      = 0x0C;

const std::uint8_t REG_ADDR_CELLVOLTAGE1 = 0x0D;
const std::uint8_t REG_ADDR_CELLVOLTAGE2 = 0x0E;
const std::uint8_t REG_ADDR_CELLVOLTAGE3 = 0x0F;
const std::uint8_t REG_ADDR_CELLVOLTAGE4 = 0x10;
const std::uint8_t REG_ADDR_CELLVOLTAGE5 = 0x11;
const std::uint8_t REG_ADDR_CELLVOLTAGE6 = 0x12;

const std::uint8_t REG_ADDR_STATUS       = 0x13;
const std::uint8_t REG_ADDR_PROTECT      = 0x14;
const std::uint8_t REG_ADDR_RSVD1        = 0x15;
const std::uint8_t REG_ADDR_RSVD2        = 0x16;
const std::uint8_t REG_ADDR_RSVD3        = 0x17;
const std::uint8_t REG_ADDR_RSVD4        = 0x18;

// example, hal::driver::LiBattery battery("/dev/i2c-3", 0x55);
LiBattery::LiBattery(std::shared_ptr<I2CBus> ic2Bus,
                     const std::uint16_t slaveAddr,
                     const std::uint16_t bitMode,
                     const std::uint16_t maxRetry,
                     const std::uint16_t timeOut)
    : I2CDriver(ic2Bus, slaveAddr, bitMode, maxRetry, timeOut)
{
}

LiBattery::~LiBattery()
{
}

int LiBattery::read(const std::uint8_t regAddr, std::uint8_t* buf, const std::uint16_t bufLen)
{
    return i2c_read(regAddr, buf, bufLen);
}

int LiBattery::write(const std::uint8_t regAddr, const std::uint8_t val)
{
    return i2c_write(regAddr, val);
}

unsigned char LiBattery::BatteryCrc8(unsigned char* data, unsigned char len)
{
    unsigned char crc = 0;
    unsigned char i = 0;

    while(len--)
    {
        crc ^= *data++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x80)
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

bool LiBattery::getRegValue(const std::uint8_t regAddr, std::uint16_t& regValue)
{
	std::uint8_t crcData[5]  = {0, 0, 0, 0, 0};
    std::uint8_t readData[3] = {0, 0, 0};

    bool ret = false;
    regValue = 0;

    if (0 <= read(regAddr, readData, 3))
    {
    	crcData[0] = 0xaa;
    	crcData[1] = regAddr;
    	crcData[2] = 0xab;
        crcData[3] = readData[0];
        crcData[4] = readData[1];

        std::uint8_t readCrc = readData[2];

        const std::uint8_t calCrc = BatteryCrc8(crcData, 5);

        if (calCrc == readCrc)
        {
            regValue = (readData[1] << 8) | readData[0];
            ret = true;
        }

        /*
        std::cout << "  read data: " << std::hex << (std::uint16_t)readData[0] 
                  << " " << (std::uint16_t)readData[1]
                  << " " << (std::uint16_t)readData[2]
                  << " ,calCrc " << (std::uint16_t)calCrc
                  << " ,readCrc " << (std::uint16_t)readCrc
                  << " ,regValue " << regValue
                  << std::endl;
        */
    }

    return ret;
}

bool LiBattery::getTemperature(std::uint16_t& temperature)
{
    return getRegValue(REG_ADDR_TEMPTERATURE, temperature);
}

bool LiBattery::getVoltage(std::uint16_t& voltage)
{
    return getRegValue(REG_ADDR_VOLTAGE, voltage);
}

bool LiBattery::getCurrent(std::int16_t& current)
{
    std::uint16_t value = 0;
    bool result = getRegValue(REG_ADDR_CURRENT, value);
    current = value;

    return result;
}

bool LiBattery::getRelativeStateOfCharge(std::uint16_t& stateOfCharge)
{
    return getRegValue(REG_ADDR_RELATIVESTATEOFCHARGE, stateOfCharge);
}

bool LiBattery::getRemainingCapacity(std::uint16_t& rcapacity)
{
    return getRegValue(REG_ADDR_REMAININGCAPACITY, rcapacity);
}

bool LiBattery::getStatus(std::uint16_t& status)
{
    return getRegValue(REG_ADDR_STATUS, status);
}

}  // ~namespace driver
}  // ~namespace hal

