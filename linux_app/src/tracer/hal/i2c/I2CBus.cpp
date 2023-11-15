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
 * Created : 22 September 2023
 * Purpose : Provide I2C bus mutex
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */

#include "I2CBus.hpp"

#include <unistd.h>

namespace hal
{
namespace driver
{

I2CBus::I2CBus(const std::string& devName)
    : _devName(devName)
{
}

I2CBus::~I2CBus()
{
}

void I2CBus::lock()
{
    _mutex.lock();
}

void I2CBus::unlock()
{
    _mutex.unlock();
}

const std::string& I2CBus::getDevName(void) const
{
    return _devName;
}

}  // ~namespace driver
}  // ~namespace hal


