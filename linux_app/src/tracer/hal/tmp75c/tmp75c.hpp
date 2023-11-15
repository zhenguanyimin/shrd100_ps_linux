/**
# 
# tmp75c.hpp - TMP75C-Q1 1.8-V Digital Temperature Sensor driver
# 
# All rights reserved@Skyfend.com
# 
# Copyright (c) 2023 JiaWei
# @author JiaWei
# @date Sep 15,2023
*/

#pragma once

#ifndef _TMP75C_HPP_
#define _TMP75C_HPP_

#include "../i2c/I2CDriver.hpp"

namespace hal
{
namespace driver
{

class Tmp75c : public I2CDriver
{
public:
    Tmp75c(std::shared_ptr<I2CBus> ic2Bus,
           const std::uint16_t slaveAddr,
           const std::uint16_t bitMode  = I2C_7BIT_MODE,
           const std::uint16_t maxRetry = I2C_MAX_RETRY_TIMES,
           const std::uint16_t timeOut  = I2C_TIMEOUT_JIFFIES_VALUE);

    virtual ~Tmp75c();
    Tmp75c() = delete;

    virtual std::uint16_t getTemperature(void);

protected:
    virtual int read(const std::uint8_t regAddr,
                     std::uint8_t* buf,
                     const std::uint16_t bufLen);

    virtual int write(const std::uint8_t regAddr,
                      const std::uint8_t val);
private:
};

}  // ~namespace driver
}  // ~namespace hal

#endif
