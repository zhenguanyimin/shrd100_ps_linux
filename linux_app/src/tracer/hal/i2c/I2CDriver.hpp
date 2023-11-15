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
 * Created : 23 September 2023
 * Purpose : Provide I2C driver
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */

#pragma once

#ifndef _I2C_DRIVER_HPP_
#define _I2C_DRIVER_HPP_

#include "I2CBus.hpp"

#include <iostream>
#include <mutex>
#include <string>
#include <memory>

namespace hal
{
namespace driver
{

#define I2C_7BIT_MODE              0 
#define I2C_10BIT_MODE             1
#define I2C_MAX_RETRY_TIMES        1
#define I2C_TIMEOUT_JIFFIES_VALUE  1000  // TBD..., need to find a suitable value

class I2CBusLockGuard
{
public:
    I2CBusLockGuard(std::shared_ptr<I2CBus> ic2Bus);
    virtual ~I2CBusLockGuard();

    I2CBusLockGuard& operator = (const I2CBusLockGuard& o) = delete;
    I2CBusLockGuard() = delete;

protected:
private:
    std::shared_ptr<I2CBus> _ic2Bus;
};

class I2CDriver
{
public:

    // bitMode, maxRetry and timeOut are not used so far
    I2CDriver(std::shared_ptr<I2CBus> i2cBus,
              const std::uint16_t slaveAddr,
              const std::uint16_t bitMode  = I2C_7BIT_MODE,
              const std::uint16_t maxRetry = I2C_MAX_RETRY_TIMES,
              const std::uint16_t timeOut  = I2C_TIMEOUT_JIFFIES_VALUE);

    virtual ~I2CDriver();

    I2CDriver& operator = (const I2CDriver& o) = delete;
    I2CDriver() = delete;

    virtual int i2c_read(const std::uint8_t regAddr, 
                         std::uint8_t* buf,
                         const std::uint16_t bufLen);

    virtual int i2c_write(const std::uint8_t regAddr,
                          const std::uint8_t val);

    virtual int i2c_write(const std::uint8_t regAddr,
                          const std::uint8_t* buf,
                          const std::uint16_t bufLen);


    const std::string& getDevName() const;

    const std::uint16_t& getSlaveAddr() const;

protected:
    int init();

private:
    std::shared_ptr<I2CBus> _ic2Bus;
    std::uint16_t _slaveAddr;
    std::uint16_t _bitMode;    // 0: 7bits, other: 10bits
    std::uint16_t _maxRetry;   // retry when ACT is not received
    std::uint16_t _timeout;    // jiffies

    int _fd;
};

}  // ~namespace driver
}  // ~namespace hal

#endif
