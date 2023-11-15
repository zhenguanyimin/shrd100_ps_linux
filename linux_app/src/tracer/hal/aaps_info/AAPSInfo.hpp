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
 * Created : 16 September 2023
 * Purpose : Provide AAPSInfo data access method to algorighm
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */

#pragma once

#ifndef _AAPSINFO_HPP_
#define _AAPSINFO_HPP_

#include <unistd.h>
#include <memory>
#include <iostream>
#include <string>
#include <variant>
#include <optional>
#include <mutex>

#include "../../eap/alink_pb/TracerUavMsg.pb.h"

namespace hal
{
namespace driver
{

enum AAPSInfoDataType : std::uint8_t
{
    ENUM_NULL_TYPE  = 0,  // no date is in AAPSInfo
    ENUM_ANGLE_TYPE = 1,  // AngleInfoDatais in AAPSInfo
    ENUM_AAPS_TYPE  = 2,  // AAPSInfoData is in AAPSInfo
};

// Angle, Attitude, Position, Speed
class AAPSInfo  
{
public:
    AAPSInfo();

    // ==========================================================
    // @function 
    // @param input: 
    //         type: set data type
    //         buf: raw buffer start address,
    //         length: raw buffer length
    // @param output: 
    //
    // @return: 
    // @description: use buff data to assign internal member variables
    // ==========================================================
    AAPSInfo(const AAPSInfoDataType type,
             const std::uint8_t* buf,
             const std::uint32_t length);

    // ==========================================================
    // @function setData
    // @param input: 
    //         type: set data type
    //         buf: raw buffer start address,
    //         length: raw buffer length
    // @param output: 
    //
    // @return: false when length is bigger than MAX_BUFF_LENGTH, otherwise true
    // @description: use buff data to assign internal member variables
    // ==========================================================
    virtual bool setData(const AAPSInfoDataType type,
                         const std::uint8_t* buf,
                         const std::uint32_t length);

    // ==========================================================
    // @function setData
    // @param input: 
    // @param output: 
    //         buf: raw buffer start address,
    //         length: raw buffer length
    // @return: false when length is less than cached data length
    //          or no cached data
    // @description: use buff data to assign internal member variables
    // ==========================================================
    virtual bool getData(std::uint8_t* buf,
                         std::uint32_t& length);

    virtual ~AAPSInfo();

    AAPSInfo(const AAPSInfo& o);
    AAPSInfo& operator = (const AAPSInfo& o);

    AAPSInfo(AAPSInfo&& o);
    AAPSInfo& operator = (const AAPSInfo&& o);

    // ==========================================================
    // @function getDataType
    // @param input: 
    // @param output: 
    //
    // @return: 
    //        ENUM_ANGLE_TYPE: orignal angle data type
    //        ENUM_AAPS_TYPE: UAV fly status data type
    // @description: 
    // ==========================================================
    AAPSInfoDataType getDataType(void) const;

    friend std::ostream& operator<<(std::ostream& stream, const AAPSInfo& object);

    // ==========================================================
    // @function ParseFromArray
    // @param input: 
    //         buf: raw buffer start address,
    //         length: raw buffer length
    // @param output: 
    //
    // @return: false when buf is nullptr, otherwise true
    // @description: use buff data to assign internal member variables
    //               invoke google::protobuf::MessageLite::ParseFromArray
    // ==========================================================
    // bool ParseFromArray(const void* data,  std::size_t size);

    // ==========================================================
    // @function SerializeToArray
    // @param input: 
    //         buf: raw buffer start address,
    //         bufLength: the maximum buffer length
    // @param output: 
    //         length: raw buffer length
    // @return: false when buf is nullptr or bufLength is less than length, otherwise true
    // @description: serialize internal member variables to buffer
    //               invoke google::protobuf::MessageLite::SerializeToArray
    // ==========================================================
    // bool SerializeToArray(void* data, std::size_t size) const;

    // ==========================================================
    // @function getAngleData
    // @param input: 
    // @param output: 
    //         data : AngleInfoData from local hardware
    // @return: false when data type is not ENUM_ANGLE_TYPE, otherwise true
    // @description: 
    // ==========================================================
    // bool getAngleData(protobuf::traceruav::AngleInfoPB& data) const;

    // ==========================================================
    // @function getAngleData
    // @param input: 
    // @param output: 
    //         data : AAPSInfoData from UAV
    // @return: false when data type is not ENUM_AAPS_TYPE, otherwise true
    // @description: 
    // ==========================================================
    bool getAAPSData(protobuf::traceruav::FlyStatusPB& data) const;

    // ==========================================================
    // @function fromRawBuffer
    // @param input: 
    //         buf: raw buffer start address,
    //         length: raw buffer length
    // @param output: 
    //
    // @return: false when buf is nullptr, otherwise true
    // @description: use buff data to assign internal member variables
    //               suggest to use google::protobuf::MessageLite::ParseFromArray
    // ==========================================================
    ///virtual bool fromRawBuffer(std::uint8_t* buf, const std::size_t length);
    // virtual bool fromRawBuffer(std::unique_ptr<std::uint8_t[]> buff, const std::size_t length);

    // ==========================================================
    // @function toRawBuffer
    // @param input: 
    //         buf: raw buffer start address,
    //         bufLength: the maximum buffer length
    // @param output: 
    //         length: raw buffer length
    // @return: false when buf is nullptr or bufLength is less than length, otherwise true
    // @description: serialize internal member variables to buffer
    //               suggest to use google::protobuf::MessageLite::SerializeToArray
    // ==========================================================
    // virtual bool toRawBuffer(std::uint8_t* buf, std::size_t& length, const std::size_t bufLength);
    // virtual bool toRawBuffer(std::unique_ptr<std::uint8_t[]> buff, std::size_t& length, const std::size_t bufLength);

    // ==========================================================
    // @function getData
    // @param input: 
    // @param output: 
    //
    // @return: 
    //        Drv_AngleInfo has orignal angle data
    //        AAPSInfoData has UAV fly status data
    // @description: 
    // ==========================================================
    // variant and optional is not supported by gcc 7.5.0
    // std::variant<AngleInfoData, AAPSInfoData> getData(void) const;
    // std::optional<AngleInfoData> getAngleData(void) const;
    // std::optional<AAPSInfoData> getAAPSData(void) const;

protected:

private:
    // AAPSInfoDataType _dataType;
    // protobuf::traceruav::AngleInfoPB _angleData;
    // protobuf::traceruav::FlyStatusPB _aapsData;

    std::shared_ptr<std::uint8_t[]> _buffer;
    std::uint32_t _buffLength;

    std::mutex _mutex;
};

}  // ~namespace driver
}  // ~namespace hal

#endif
