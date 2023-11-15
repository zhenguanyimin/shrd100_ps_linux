#pragma once

#include <chrono>
#include <string>
#include "eap_pub.h"

using namespace std;
using TimePoint = chrono::system_clock::time_point;

#define EAP_CHECK_CONDITION_RET(cond) \
    if (cond) \
    {\
        return;\
    }
#define RECORD_ITEM_SERIAL_BUF_LEN      4096


class RecorderFormatter
{
public:
    RecorderFormatter(uint16_t cmdId);
    virtual ~RecorderFormatter(){}
    void Begin();
    void End();
    uint8_t* GetSerialBuf() { return _serialBuf; }
    uint32_t GetSerialLength() { return _serialLen; }
    void Write(uint8_t* buf, uint32_t length);
protected:
    virtual void _Format(uint8_t* inBuf, uint32_t inLen, uint32_t& usedLen);
    virtual string _GetTypeStr() = 0;
    virtual string _GetCmdIdStr(uint16_t id) = 0;
    string _SerializeTimePoint( const TimePoint& time, const string& format);
    void _SprintfCmdWriteBuf(uint8_t* inBuf, uint32_t length, uint8_t rowLen, uint32_t& usedLen);
    uint16_t _cmdId;
    uint8_t _serialBuf[RECORD_ITEM_SERIAL_BUF_LEN];
    uint32_t _serialLen;
private:
    TimePoint _begin;
    TimePoint _end;
};