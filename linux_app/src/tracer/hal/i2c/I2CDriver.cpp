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

#include "I2CDriver.hpp"

#include <fcntl.h>

#ifndef __UNITTEST__
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//#include <linux/jiffies.h>

#include "eap_pub.h"

namespace hal
{
namespace driver
{

I2CBusLockGuard::I2CBusLockGuard(std::shared_ptr<I2CBus> ic2Bus)
{
    _ic2Bus = nullptr;

    if (nullptr != ic2Bus)
    {
        _ic2Bus = ic2Bus;
        _ic2Bus->lock();
    }
}

I2CBusLockGuard::~I2CBusLockGuard()
{
    if (nullptr != _ic2Bus)
    {
        _ic2Bus->unlock();
    }
}

I2CDriver::I2CDriver(std::shared_ptr<I2CBus> i2cBus,
                     const std::uint16_t slaveAddr,
                     const std::uint16_t bitMode,
                     const std::uint16_t maxRetry,
                     const std::uint16_t timeOut)
    : _ic2Bus(i2cBus),
      _slaveAddr(slaveAddr),
      _bitMode(bitMode),
      _maxRetry(maxRetry),
      _timeout(timeOut)
{
    _fd = -1;
    init();
}

I2CDriver::~I2CDriver()
{
    if (_fd >= 0)
    {
        close(_fd);
    }

    _fd = -1;
}

int I2CDriver::init()
{
    int ret = -1;

    if (nullptr != _ic2Bus)
    {
        I2CBusLockGuard i2clock(_ic2Bus);

        /*
        std::cout << "I2CDriver::init(), name = " << _ic2Bus->getDevName()
                  << ", slaveAddr = 0x" << std::hex << _slaveAddr
                  << ", bitMode = " << std::dec << _bitMode
                  << ", timeout = " << std::dec << _timeout
                  << ", maxRetry = " << std::dec << _maxRetry
                  << std::endl;
        */

        const auto fd = open(_ic2Bus->getDevName().c_str(), O_RDWR);
        
        if (fd < 0)
        {
            EAP_LOG_ERROR("faliled to open dev %s, error: %s\n", _ic2Bus->getDevName(), strerror(errno));
        }
        else
        {
#ifndef __UNITTEST__
            //int a = ioctl(fd, I2C_RETRIES,     _maxRetry);
            //int a = ioctl(fd, I2C_TIMEOUT,     _timeout);

            ret = ioctl(fd, I2C_TIMEOUT, _timeout);
            if (0 != ret)
            {
                EAP_LOG_ERROR("ioctl error, addr=0x%x, ret=%d, error: %s\n", _slaveAddr, ret, strerror(errno));
            }

            ret = ioctl(fd, I2C_TENBIT, _bitMode);  // 0 : 7bits, other : 10bits
            if (0 != ret)
            {
                EAP_LOG_ERROR("ioctl error, addr=0x%x, ret=%d, error: %s\n", _slaveAddr, ret, strerror(errno));
            }

            ret = ioctl(fd, I2C_SLAVE, _slaveAddr);
            if (0 != ret)
            {
                EAP_LOG_ERROR("ioctl error, addr=0x%x, ret=%d, error: %s\n", _slaveAddr, ret, strerror(errno));
            }

            ret = ioctl(fd, I2C_SLAVE_FORCE, _slaveAddr);
            if (0 != ret)
            {
                EAP_LOG_ERROR("ioctl error, addr=0x%x, ret=%d, error: %s\n", _slaveAddr, ret, strerror(errno));
            }
#endif
            _fd = fd;
        }
    }

    return ret;
}

int I2CDriver::i2c_read(const std::uint8_t regAddr, std::uint8_t* buf, const std::uint16_t bufLen)
{
    int ret = -1;

    if (nullptr != _ic2Bus)
    {
        I2CBusLockGuard i2clock(_ic2Bus);

#ifndef __UNITTEST__
        struct i2c_rdwr_ioctl_data data;
        struct i2c_msg messages[2];
        std::uint8_t reg = regAddr;

        // write to set access address
        messages[0].addr = _slaveAddr;
        messages[0].flags = 0;
        messages[0].len = sizeof(reg);
        messages[0].buf = &reg;

        // read the value
        messages[1].addr = _slaveAddr;
        messages[1].flags = I2C_M_RD;
        messages[1].len = bufLen;
        messages[1].buf = buf;

        data.msgs = messages;
        data.nmsgs = 2;

        ret = ioctl(_fd, I2C_RDWR, &data);
        if (ret < 0)
        {
            // EAP_LOG_ERROR("i2c read error, addr=0x%x, ret=%d, error: %s\n", regAddr, ret, strerror(errno));
        }
#endif
    }

    return ret;
}

int I2CDriver::i2c_write(const std::uint8_t regAddr, const std::uint8_t val)
{
    int ret = -1;

#ifndef __UNITTEST__
    if (nullptr != _ic2Bus)
    {
        I2CBusLockGuard i2clock(_ic2Bus);

        struct i2c_rdwr_ioctl_data data;
        struct i2c_msg messages;
        std::uint8_t buf[2];

        buf[0] = regAddr;
        buf[1] = val;

        messages.addr = _slaveAddr;
        messages.flags = 0;
        messages.len = 2;
        messages.buf = buf;

        data.msgs = &messages;
        data.nmsgs = 1;

        ret = ioctl(_fd, I2C_RDWR, &data);
        if (ret < 0)
        {
            EAP_LOG_ERROR("i2c write error, addr=0x%x, ret=%d, error: %s\n", regAddr, ret, strerror(errno));
        }
    }
#endif
    return ret;
}

int I2CDriver::i2c_write(const std::uint8_t regAddr, const std::uint8_t* buf, const std::uint16_t bufLen)
{
    int ret = -1;

#ifndef __UNITTEST__
    if ((nullptr != _ic2Bus) && (256 > bufLen))  // write length is smaller than 256
    {
        I2CBusLockGuard i2clock(_ic2Bus);

        struct i2c_rdwr_ioctl_data data;
        struct i2c_msg messages;
        std::uint8_t valBuf[256] = {0};

        valBuf[0] = regAddr;
        memcpy(valBuf + 1, buf, bufLen);

        messages.addr = _slaveAddr;
        messages.flags = 0;
        messages.len = bufLen + 1;
        messages.buf = valBuf;

        data.msgs = &messages;
        data.nmsgs = 1;

        ret = ioctl(_fd, I2C_RDWR, &data);
        if (ret < 0)
        {
            EAP_LOG_ERROR("i2c write error, addr=0x%x, ret=%d, error: %s\n", regAddr, ret, strerror(errno));
        }
    }
#endif
    return ret;
}

const std::string& I2CDriver::getDevName() const
{
    return _ic2Bus->getDevName();
}

const std::uint16_t& I2CDriver::getSlaveAddr() const
{
    return _slaveAddr;
}


}  // ~namespace driver
}  // ~namespace hal


