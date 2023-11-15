#pragma once

#include "RemoteIdBufScanStrategy.h"

class RemoteIdBufScanDefaultStrategy : public RemoteIdBufScanStrategy
{
public:
    int32_t Scan(uint8_t* data, uint32_t dataLen, uint32_t& start) override;
protected:
private:
};
