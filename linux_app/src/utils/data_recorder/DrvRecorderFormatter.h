#pragma once

#include "RecorderFormatter.h"

class DrvRecorderFormatter : public RecorderFormatter
{
public:
    DrvRecorderFormatter(uint32_t cmdId);
    void WriteBuf(uint8_t* inBuf, uint32_t inLen, uint8_t* outBuf, uint32_t outLen, uint32_t result);
protected:
    void _Format(uint8_t* inBuf, uint32_t inLen, uint32_t& usedLen) override;
    string _GetTypeStr() override;
    string _GetCmdIdStr(uint16_t id) override;
private:
};