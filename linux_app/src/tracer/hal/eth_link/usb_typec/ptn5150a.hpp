/*****************************************************************************
 * Copyright (c) 2023
 * Skyfend Technology Co., Ltd
 *
 * All rights reserved. Any unauthorized disclosure or publication of the
 * confidential and proprietary information to any other party will constitute
 * an infringement of copyright laws.
 *
 * $Id$
 * Author  : Fang kai, fangkai@autel.com
 * Created : 20 September 2023
 * Purpose : Declare I2C driver for PTN5150A USB Type-C chip in C++ format
 *
 * $Rev$
 *
 * Notes: 
 *
 *****************************************************************************
 */
#pragma once

#ifndef _PTN5150A_HPP_
#define _PTN5150A_HPP_

#include <string>
#include <map>
#include <mutex>

#include "I2CDriver.hpp"
#include "usb_typec.h"

using namespace std;


namespace tracer
{
namespace ptn5150a_usb_type_c
{

using namespace hal::driver;

class Ptp5150aDriverMgr;
class Ptn5150aUsbTypc_Driver : public I2CDriver
{
public:
    //static Ptn5150aUsbTypc_Driver& getInstance(const string& devName, uint16_t slaveAddr);

    bool setTypecMode(UsbTypecMode pMode);
    UsbTypecMode getTypecMode(void);
    uint8_t getVersionID(void);
    uint8_t getVendorID(void);

protected:

    static std::uint8_t getRegAddr_Vendor_Device_ID()
    {
        return sRegAddr_Vendor_Device_ID;
    }

    static std::uint8_t getRegAddr_ModeCtrl()
    {
        return sRegAddr_ModeCtrl;
    }

    uint8_t _getDeviceInfo();
    Ptn5150aUsbTypc_Driver(std::shared_ptr<I2CBus> i2cBus, const string& devName, uint16_t slaveAddr);

    virtual ~Ptn5150aUsbTypc_Driver();
    Ptn5150aUsbTypc_Driver() = delete;
private:
    friend class Ptp5150aDriverMgr;
    static const uint8_t sRegAddr_Vendor_Device_ID; // Register address for VendorID/DeviceID 
    static const uint8_t sRegAddr_ModeCtrl; // Register address Usb Mode control 

    // static once_flag _sFlagOnce;
    //static Ptn5150aUsbTypc_Driver* _sInstancePtr;
};


class Ptp5150aDriverMgr
{
public:
    static Ptn5150aUsbTypc_Driver& initInstance(const string& pDevName, uint16_t pSlaveAddr);
    static Ptn5150aUsbTypc_Driver* getInstance(const string& pDevName, uint16_t pSlaveAddr);
    static std::shared_ptr<hal::driver::I2CBus> _i2cBus;

private:
    class _DriverKey
    {
        public:
            _DriverKey(const string& pDevName, uint16_t pHwAddr) : _devI2cBusName(), _devHwAddr() {}
            ~_DriverKey() {};

            inline bool operator< (const _DriverKey& pOther) const
            {
                if(_devI2cBusName != pOther._devI2cBusName)
                {
                    return _devI2cBusName < pOther._devI2cBusName;
                }
                if(_devHwAddr != pOther._devHwAddr)
                {
                    return _devHwAddr < pOther._devHwAddr;
                }
                return false;
            }

            inline bool operator== (const _DriverKey& pOther) const
            {
                return ((_devI2cBusName == pOther._devI2cBusName)
                        && (_devHwAddr == pOther._devHwAddr));
            }

            string      _devI2cBusName; //Dev name
            uint16_t    _devHwAddr; //Hardware address

    };
    static mutex _sInstMapMutex;
    static std::map<_DriverKey, Ptn5150aUsbTypc_Driver*> _sDriverInstMap;
   
};

}  // ~namespace ptn5150a_usb_type_c
}  // ~namespace tracer

#endif // _PTN5150A_HPP_
