#pragma once

#include "eap_pub.h"
#include <cstdint>

class RemoteIdBufScanStrategy
{
public:
    virtual ~RemoteIdBufScanStrategy(){}
    virtual int32_t Scan(uint8_t* data, uint32_t dataLen, uint32_t& start) = 0;
protected:
private:
};
