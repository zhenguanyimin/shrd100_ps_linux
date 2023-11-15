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

#include "AAPSInfo.hpp"

#include <sstream>
#include <iomanip>
#include "../../eap/alink_pb/TracerUavMsg.pb.h"

//#include "eap_pub.h"

namespace hal
{
namespace driver
{

const std::uint32_t MAX_BUFF_LENGTH = 256;

AAPSInfo::AAPSInfo()
{
    _buffer = nullptr;
    _buffLength = 0;
}

AAPSInfo::AAPSInfo(const AAPSInfoDataType type,
                   const std::uint8_t* buf,
                   const std::uint32_t length)
{
    setData(type, buf, length);
}

AAPSInfo::~AAPSInfo()
{
}

AAPSInfo::AAPSInfo(const AAPSInfo& o)
{
    *this = o;
}

AAPSInfo& AAPSInfo::operator=(const AAPSInfo& o)
{
    _buffer = o._buffer;
    _buffLength = o._buffLength;

    return *this;
}

AAPSInfo::AAPSInfo(AAPSInfo&& o)
{
    *this = std::move(o);
}

AAPSInfo& AAPSInfo::operator=(const AAPSInfo&& o)
{
    _buffer = std::move(o._buffer);
    _buffLength = o._buffLength;
    return *this;
}

AAPSInfoDataType AAPSInfo::getDataType(void) const
{
    AAPSInfoDataType dataType = AAPSInfoDataType::ENUM_NULL_TYPE;

    if (nullptr != _buffer)
    {
        dataType = static_cast<AAPSInfoDataType>(_buffer.get()[0]);
    }

    return dataType;
}

bool AAPSInfo::setData(const AAPSInfoDataType type, 
                       const std::uint8_t* buf,
                       const std::uint32_t length)
{
    std::lock_guard<std::mutex> lock(_mutex);

    bool ret = false;

    if (length + 1 < MAX_BUFF_LENGTH)
    {
        _buffer = std::shared_ptr<std::uint8_t[]>(new(std::uint8_t[MAX_BUFF_LENGTH]));
        _buffLength = length + 1;
        _buffer.get()[0] = static_cast<std::uint8_t>(type);

        memcpy(_buffer.get() + 1, buf, length);

        /*
        std::ostringstream err;
        for (uint32_t pos = 0; pos < length + 1; pos++)
        {
            err << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(_buffer[pos]);
        }
        std::cout << "length = " << std::dec << length << ", " << err.str() << std::endl;
        */

        ret = true;
    }

    return ret;
}

bool AAPSInfo::getData(std::uint8_t* buf, std::uint32_t& length)
{
    std::lock_guard<std::mutex> lock(_mutex);

    bool ret = false;

    if ((nullptr != _buffer) && (_buffLength <= length))
    {
        memcpy(buf, _buffer.get(), _buffLength);

        ret = true;
    }

    return ret;
}

std::ostream& operator<<(std::ostream& stream, const AAPSInfo& object)
{
    if (nullptr != object._buffer)
    {
        stream << "_dataType " << static_cast<std::uint16_t>(object._buffer.get()[0]) << std::endl;
    }

    return stream;
}

bool AAPSInfo::getAAPSData(protobuf::traceruav::FlyStatusPB& data) const
{
    bool ret = false;

    if ((ENUM_AAPS_TYPE == getDataType()) && (_buffLength > 1))
    {
        ret = data.ParseFromArray(_buffer.get() + 1, _buffLength - 1);
    }

    return ret;
}

/*
bool AAPSInfo::getAngleData(protobuf::traceruav::AngleInfoPB& data) const
{
    bool ret = false;

    if (ENUM_AAPS_TYPE == _dataType)
    {
        data = _angleData;
        ret = true;
    }

    return ret;
}
*/

/*
// parameter size is "data" length
bool AAPSInfo::ParseFromArray(const void* data,  std::size_t size)
{
    bool ret = false;

    if (ENUM_ANGLE_TYPE == _dataType)
    {
        ret = _angleData.ParseFromArray(data, size);
    }
    else if (ENUM_AAPS_TYPE == _dataType)
    {
        ret = _aapsData.ParseFromArray(data, size);
    }

    return ret;
}

bool AAPSInfo::SerializeToArray(void* data, std::size_t size) const
{
    bool ret = false;

    if (ENUM_ANGLE_TYPE == _dataType)
    {
        if (_angleData.ByteSizeLong() <= size)
        {
            ret = _angleData.SerializeToArray(data, _angleData.ByteSizeLong());
        }
    }
    else if (ENUM_AAPS_TYPE == _dataType)
    {
        if (_aapsData.ByteSizeLong() <= size)
        {
            ret = _aapsData.SerializeToArray(data, _aapsData.ByteSizeLong());
        }
    }

    return ret;
}
*/
    
/*
std::variant<AngleInfoData, AAPSInfoData> AAPSInfo::getData() const
{
    return _data;
}

std::optional<AngleInfoData> AAPSInfo::getAngleData(void) const
{
    if (std::holds_alternative<AngleInfoData>(_data))
    {
        return std::get<AngleInfoData>(_data);
    }

    return std::nullopt;
}

std::optional<AAPSInfoData> AAPSInfo::getAAPSData(void) const
{
    if (std::holds_alternative<AAPSInfoData>(_data))
    {
        return std::get<AAPSInfoData>(_data);
    }

    return std::nullopt;
}
*/

}  // ~namespace driver
}  // ~namespace hal


