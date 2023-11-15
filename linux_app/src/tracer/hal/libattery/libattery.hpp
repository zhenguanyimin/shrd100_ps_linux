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

#pragma once

#ifndef _LIBATTERY_HPP_
#define _LIBATTERY_HPP_

#include "../i2c/I2CDriver.hpp"

namespace hal
{
namespace driver
{

class LiBattery : public I2CDriver
{
public:
    LiBattery(std::shared_ptr<I2CBus> ic2Bus,
              const std::uint16_t slaveAddr,
              const std::uint16_t bitMode  = I2C_7BIT_MODE,
              const std::uint16_t maxRetry = I2C_MAX_RETRY_TIMES,
              const std::uint16_t timeOut  = I2C_TIMEOUT_JIFFIES_VALUE);

    virtual ~LiBattery();
    LiBattery() = delete;

    virtual bool getTemperature(std::uint16_t& temperature); // 0.1K
    virtual bool getVoltage(std::uint16_t& voltage); // mV
    virtual bool getCurrent(std::int16_t& current); // mA
    virtual bool getRelativeStateOfCharge(std::uint16_t& stateOfCharge); // %
    virtual bool getRemainingCapacity(std::uint16_t& rcapacity); // mAh

    /*
    bit15       bit14       bit13       bit12       bit11       bit10       bit9        bit8
    RSVD        RSVD        RSVD        RSVD        RSVD        RSVD        CTO         Charger
                                                                            (充电超时)      (充电器)
    bit7        bit6        bit5        bit4        bit3        bit2        bit1        bit0
    HEAT_EN     BQ_COM_F    CFET2       DFET        CFET1       DSG         CHG         DILE
    (加热)        (通信异常)      (充电MOS2)   (放电MOS)      (充电MOS1)    (放电状态)      (充电状态)      (空闲状态)

    1: 表示有效, 0: 无效, 例如bit1 CHG为1, 表示充电状态
    */
    virtual bool getStatus(std::uint16_t& status);

protected:
    virtual bool getRegValue(const std::uint8_t regAddr,
                             std::uint16_t& regValue);

    virtual int read(const std::uint8_t regAddr,
                     std::uint8_t* buf,
                     const std::uint16_t bufLen);

    virtual int write(const std::uint8_t regAddr,
                      const std::uint8_t val);
private:
    unsigned char BatteryCrc8(unsigned char* data, unsigned char len);
};

}  // ~namespace driver
}  // ~namespace hal

#endif
