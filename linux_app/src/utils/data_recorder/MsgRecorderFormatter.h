#pragma once

#include "RecorderFormatter.h"

class MsgRecorderFormatter : public RecorderFormatter
{
public:
    MsgRecorderFormatter(uint32_t cmdId);
protected:
    void _Format(uint8_t* inBuf, uint32_t inLen, uint32_t& usedLen) override;
    void _FormatMsgHeadTitle(uint32_t& usedLen);
    string _GetTypeStr() override;
    string _GetCmdIdStr(uint16_t id) override;    
private:
};