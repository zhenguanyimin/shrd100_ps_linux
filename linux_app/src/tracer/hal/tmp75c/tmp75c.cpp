/**
# 
# tmp75c.cpp - TMP75C-Q1 1.8-V Digital Temperature Sensor driver
# 
# All rights reserved@Skyfend.com
# 
# Copyright (c) 2023 JiaWei
# @author JiaWei
# @date Sep 15,2023
*/

#include "tmp75c.hpp"

#include "eap_pub.h"

namespace hal
{
namespace driver
{


// example, hal::driver::Tmp75c temperMonitor("/dev/i2c-1", 0x48);


Tmp75c::Tmp75c(std::shared_ptr<I2CBus> ic2Bus,
               const std::uint16_t slaveAddr,
               const std::uint16_t bitMode,
               const std::uint16_t maxRetry,
               const std::uint16_t timeOut)
    : I2CDriver(ic2Bus, slaveAddr, bitMode, maxRetry, timeOut)
{
}

Tmp75c::~Tmp75c()
{
}

int Tmp75c::read(const std::uint8_t regAddr, std::uint8_t* buf, const std::uint16_t bufLen)
{
    return i2c_read(regAddr, buf, bufLen);
}

int Tmp75c::write(const std::uint8_t regAddr, const std::uint8_t val)
{
    return i2c_write(regAddr, val);
}

/*
-----------------------------------------------------
Temperature Register (pointer = 0h) [reset = 0000h] |
-----------------------------------------------------
bit |  15     14    13    12    11    10     9     8|
-----------------------------------------------------
    | T11    T10    T9    T8    T7    T6    T5    T4|
-----------------------------------------------------
bit |   7      6     5     4     3     2     1     0|
-----------------------------------------------------
    |  T3     T2    T1    T0     Reserved           |
-----------------------------------------------------
Name         Description
T11 to T4    The 8 MSBs of the temperature result (resolution of 1°C)
T3  to T0    The 4 LSBs of the temperature result (resolution of 0.0625°C)
*/
std::uint16_t Tmp75c::getTemperature(void)
{
    std::uint8_t  data[2]   = {0, 0};
    std::uint16_t tempValue = 0;

    if (0 <= read(0, data, 2))
    {
        tempValue = data[0];
        if (static_cast<std::uint16_t>(data[1]) >= 128)
        {
            tempValue++;  // 0.5°C is considered to 1°C
        }
    }

    // tempValue = (static_cast<std::uint16_t>(data[0]) << 4) | (static_cast<std::uint16_t>(data[1]) >> 4);
    // float temperature = 0.0625 * tempValue;
    // std::cout << "temperature = " << temperature << std::endl;

    return tempValue;
}


}  // ~namespace driver
}  // ~namespace hal

