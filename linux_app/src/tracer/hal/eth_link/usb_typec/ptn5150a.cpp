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
 * Created : 23 September 2023
 * Purpose : Declare I2C driver for PTN5150A USB Type-C chip
 *
 * $Rev$
 *
 * Notes: 
 *
 *****************************************************************************
 */
 
#include "TracerUtils_c.h"
#include "ptn5150a.hpp"

namespace tracer
{
namespace ptn5150a_usb_type_c
{

using namespace hal::driver;
const uint8_t Ptn5150aUsbTypc_Driver::sRegAddr_Vendor_Device_ID = 0x1; // Register address for VendorID/DeviceID 
const uint8_t Ptn5150aUsbTypc_Driver::sRegAddr_ModeCtrl = 0x2; // Register address Usb Mode control 

std::shared_ptr<hal::driver::I2CBus> Ptp5150aDriverMgr::_i2cBus = std::shared_ptr<hal::driver::I2CBus>(new hal::driver::I2CBus("/dev/i2c-0"));

/*
//once_flag Ptn5150aUsbTypc_Driver::_sFlagOnce;
//Ptn5150aUsbTypc_Driver* Ptn5150aUsbTypc_Driver::_sInstancePtr = nullptr;

Ptn5150aUsbTypc_Driver& Ptn5150aUsbTypc_Driver::getInstance(const string& devName, uint16_t slaveAddr)
{
    call_once(_sFlagOnce, [&] () {_sInstancePtr = new Ptn5150aUsbTypc_Driver(devName, slaveAddr); });
    return *_sInstancePtr;
}
*/
Ptn5150aUsbTypc_Driver::Ptn5150aUsbTypc_Driver(std::shared_ptr<I2CBus> i2cBus, const string& devName, uint16_t slaveAddr)
    : I2CDriver(i2cBus, slaveAddr)
{
}

Ptn5150aUsbTypc_Driver::~Ptn5150aUsbTypc_Driver()
{
}

bool Ptn5150aUsbTypc_Driver::setTypecMode(UsbTypecMode pMode)
{
    auto lRetVal = true;
    switch(pMode)
    {
        case TYPEC_MODE_DEVICE:
        case TYPEC_MODE_HOST:
        case TYPEC_MODE_DUAL_ROLE:
        {
            auto lWriteResult = i2c_write(getRegAddr_ModeCtrl(), (uint8_t)pMode);
            TACER_UTILS_PRINTF_INFO("TypeC=Mode=%d, WriteResult=%d\n", (int)pMode, lWriteResult);
            if(lWriteResult < 0)
            {
                lRetVal = false;
                TACER_UTILS_PRINTF_ERR("i2c_write failed! TypeC=Mode=%d, regAddr=%x, WriteResult=%d\n",
                                        (int)pMode, getRegAddr_ModeCtrl(), lWriteResult);
            }
            break;
        }
        default:
        {
            //Error, invalid mode
            TACER_UTILS_PRINTF_ERR("Invalid TypeC Mode %d\n", (int)pMode);
            lRetVal = false;
            break;
        }
    }
    return lRetVal;
}

UsbTypecMode Ptn5150aUsbTypc_Driver::getTypecMode(void)
{
    auto lMode = TYPEC_MODE_UNKNOWN;
    uint8_t lRegValue = 0;
    auto lReadResult = i2c_read(getRegAddr_ModeCtrl(), &lRegValue, sizeof(lRegValue));
    TACER_UTILS_PRINTF_INFO("RegValue=Mode=%d, ReadResult=%d", lRegValue, lReadResult);
    if(lReadResult < 0)
    {
        TACER_UTILS_PRINTF_ERR("i2c_read failed! regAddr=%x, lReadResult=%d\n", getRegAddr_ModeCtrl(), lReadResult);
        return lMode;
    }
    else
    {
        lMode = (UsbTypecMode)lRegValue;
        if(lMode < TYPEC_MODE_DEVICE || lMode > TYPEC_MODE_DUAL_ROLE)
        {
            TACER_UTILS_PRINTF_ERR("Invalid  TypeC-Mode: %d, regAddr=%x, lReadResult=%d\n",
                                    lRegValue, getRegAddr_ModeCtrl(), lReadResult);
            return TYPEC_MODE_UNKNOWN;

        }
    }
    return lMode;
}
/*******************************
****Register offset:    01H 
****RW Mode:            Read Only

****Register name ****|**** Bits ****|**** Reset value ****|**** Description ****
    Version ID        |    [7:3]     |       00001         |   Device version ID
    Vendor ID         |    [2:0]     |       011           |   Vendor ID
************************************************/
uint8_t Ptn5150aUsbTypc_Driver::getVersionID(void)
{
    auto lVersionId = _getDeviceInfo();
    lVersionId = ((lVersionId >> 3) & 0x1F);
    return lVersionId;
}

uint8_t Ptn5150aUsbTypc_Driver::getVendorID(void)
{
    auto lVendorId = _getDeviceInfo();
    lVendorId = (lVendorId & 0x7);
    return lVendorId;
}

uint8_t Ptn5150aUsbTypc_Driver::_getDeviceInfo(void)
{
    uint8_t lRegValue = 0;
    auto lReadResult = i2c_read(getRegAddr_Vendor_Device_ID(), &lRegValue, sizeof(lRegValue));
    TACER_UTILS_PRINTF_INFO("DeviceInfo=%d, ReadResult=%d", lRegValue, lRegValue);
    if(lReadResult < 0)
    {
        TACER_UTILS_PRINTF_ERR("i2c_read failed! regAddr=%x, lReadResult=%d\n", getRegAddr_Vendor_Device_ID(), lReadResult);
    }
    return lRegValue;
}

mutex Ptp5150aDriverMgr::_sInstMapMutex;
std::map<Ptp5150aDriverMgr::_DriverKey, Ptn5150aUsbTypc_Driver*> Ptp5150aDriverMgr::_sDriverInstMap;

Ptn5150aUsbTypc_Driver& Ptp5150aDriverMgr::initInstance(const string& pDevName, uint16_t pSlaveAddr)
{
    std::lock_guard<std::mutex> lock(_sInstMapMutex);

    _DriverKey lKey(pDevName, pSlaveAddr);
    auto lDriverInstPtr = _sDriverInstMap[lKey];
    if(lDriverInstPtr == nullptr)
    {
        lDriverInstPtr = new Ptn5150aUsbTypc_Driver(Ptp5150aDriverMgr::_i2cBus, pDevName, pSlaveAddr);
        _sDriverInstMap[lKey] = lDriverInstPtr;
    }
    return *lDriverInstPtr;
}

Ptn5150aUsbTypc_Driver* Ptp5150aDriverMgr::getInstance(const string& pDevName, uint16_t pSlaveAddr)
{
    std::lock_guard<std::mutex> lock(_sInstMapMutex);
    auto lIterFind = _sDriverInstMap.find(_DriverKey(pDevName, pSlaveAddr));
    if(_sDriverInstMap.end() != lIterFind)
    {
        return lIterFind->second;
    }
    return nullptr;
}

}  // ~namespace ptn5150a_usb_type_c
}  // ~namespace tracer

using namespace tracer::ptn5150a_usb_type_c;

bool usb_typec_InitDev(const char* pDevName, uint16_t pSlaveAddr)
{
    Ptp5150aDriverMgr::initInstance(pDevName, pSlaveAddr);
    return true;
}
bool usb_typec_SetMode(const char* pDevName, uint16_t pSlaveAddr, UsbTypecMode pMode)
{
    auto lDriverPtr = Ptp5150aDriverMgr::getInstance(pDevName, pSlaveAddr);
    if(lDriverPtr != nullptr)
    {
        return lDriverPtr->setTypecMode((UsbTypecMode)pMode);
    }
    else
    {
        TACER_UTILS_PRINTF_ERR("USB TypeC driver is not found! pDevName=%s, pSlaveAddr=%x, pMode=%d\n", pDevName, pSlaveAddr, pMode);
    }
    return false;
}
uint8_t usb_typec_GetMode(const char* pDevName, uint16_t pSlaveAddr)
{
    auto lMode = TYPEC_MODE_UNKNOWN;
    auto lDriverPtr = Ptp5150aDriverMgr::getInstance(pDevName, pSlaveAddr);
    if(lDriverPtr != nullptr)
    {
        lMode = lDriverPtr->getTypecMode();
    }
    else
    {
        TACER_UTILS_PRINTF_ERR("USB TypeC driver is not found! pDevName=%s, pSlaveAddr=%x\n", pDevName, pSlaveAddr);
    }
    return lMode;
}
