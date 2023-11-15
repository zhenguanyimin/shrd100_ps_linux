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
 * Created : 25 September 2023
 * Purpose : Provide ADS1115 driver
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */


#pragma once

#ifndef _ADS1115_HPP_
#define _ADS1115_HPP_

#include "../i2c/I2CDriver.hpp"

namespace hal
{
namespace driver
{

enum Ads1115Channel : std::uint8_t
{
	ADS1115_CHANNEL_0 = 0,
	ADS1115_CHANNEL_1 = 1,
	ADS1115_CHANNEL_2 = 2 // 校准引角,R=0
};

class Ads1115 : public I2CDriver
{
public:
    Ads1115(std::shared_ptr<I2CBus> ic2Bus,
            const std::uint16_t slaveAddr,
            const std::uint16_t bitMode  = I2C_7BIT_MODE,
            const std::uint16_t maxRetry = I2C_MAX_RETRY_TIMES,
            const std::uint16_t timeOut  = I2C_TIMEOUT_JIFFIES_VALUE);

    virtual ~Ads1115();
    Ads1115() = delete;

    // 1. Write to Config register:
    //  – First byte: 0b10010000 (first 7-bit I2C address followed by a low R/W bit)
    //  – Second byte: 0b00000001 (points to Config register)
    //  – Third byte: 0b10000100 (MSB of the Config register to be written)
    //  – Fourth byte: 0b10000011 (LSB of the Config register to be written)
    // 2. Write to Address Pointer register:
    //  – First byte: 0b10010000 (first 7-bit I2C address followed by a low R/W bit)
    //  – Second byte: 0b00000000 (points to Conversion register)
    // 3. Read Conversion register:
    //  – First byte: 0b10010001 (first 7-bit I2C address followed by a high R/W bit)
    //  – Second byte: the ADS111x response with the MSB of the Conversion register
    //  – Third byte: the ADS111x response with the LSB of the Conversion register
    virtual std::int16_t getTemperature(const Ads1115Channel& channel);

protected:

    virtual int read(const std::uint8_t regAddr,
                     std::uint8_t* buf,
                     const std::uint16_t bufLen);

    virtual int write(const std::uint8_t regAddr,
                      const std::uint8_t* buf,
                      const std::uint16_t bufLen);

    virtual std::uint16_t readConversion(const Ads1115Channel& channel);

    virtual float convertToResistance(const std::uint16_t value);

private:
};

}  // ~namespace driver
}  // ~namespace hal

#endif
