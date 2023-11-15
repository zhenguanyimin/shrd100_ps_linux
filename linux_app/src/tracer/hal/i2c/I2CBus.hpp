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

#pragma once

#ifndef _I2C_BUS_HPP_
#define _I2C_BUS_HPP_

#include <iostream>
#include <mutex>
#include <string>

namespace hal
{
namespace driver
{

class I2CBus
{
public:
    I2CBus(const std::string& devName);
    ~I2CBus();

    void lock();
    void unlock();

    const std::string& getDevName(void) const;

protected:

private:
    std::mutex  _mutex;
    std::string _devName;
};

}  // ~namespace driver
}  // ~namespace hal

#endif
