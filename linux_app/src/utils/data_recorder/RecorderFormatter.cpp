#include <sstream>
#include <iomanip>
#include "RecorderFormatter.h"

RecorderFormatter::RecorderFormatter(uint16_t cmdId) : _cmdId(cmdId)
{

}

void RecorderFormatter::Begin()
{
    _begin = chrono::system_clock::now();
}

void RecorderFormatter::End()
{
    _end = chrono::system_clock::now();
}

void RecorderFormatter::_Format(uint8_t* inBuf, uint32_t inLen, uint32_t& usedLen)
{
    usedLen = 0;
    usedLen += sprintf((char*)(_serialBuf + usedLen), "[%s]%s(%-4d) ", _GetTypeStr().c_str(), _GetCmdIdStr(_cmdId).c_str(), _cmdId);
    EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    chrono::duration<double> duration = _end - _begin;
    usedLen += sprintf((char*)(_serialBuf + usedLen),
        "[Begin]%s [End]%s [Duration]%.2lfms [Len]%d", 
        _SerializeTimePoint(_begin,"%Y-%m-%d %H:%M:%S").c_str(),
        _SerializeTimePoint(_end,"%Y-%m-%d %H:%M:%S").c_str(),
        duration.count() * 1000, inLen);
}

string RecorderFormatter::_SerializeTimePoint(const TimePoint& time, const string& format)
{
    time_t tt = chrono::system_clock::to_time_t(time);
    tm tm = *gmtime(&tt); //GMT (UTC)
    auto tSeconds = chrono::duration_cast<chrono::seconds>(time.time_since_epoch());
    auto tMilli   = chrono::duration_cast<chrono::milliseconds>(time.time_since_epoch());
    auto ms       = tMilli - tSeconds;
    stringstream ss;
    ss << put_time( &tm, format.c_str() ) << '.' << setw(3) << ms.count();
    return ss.str();
}

void RecorderFormatter::_SprintfCmdWriteBuf(uint8_t* inBuf, uint32_t length, uint8_t rowLen, uint32_t& usedLen)
{
    for (uint32_t i = 0; i < length; ++i)
    {
        if (0 == (i % rowLen))
        {
            usedLen += sprintf((char*)(_serialBuf + usedLen), "\n\t");
        }
        usedLen += sprintf((char*)(_serialBuf + usedLen), "%02x ", *(inBuf + i));
        EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    }
}

void RecorderFormatter::Write(uint8_t* buf, uint32_t length)
{
    uint32_t usedLen = 0;
    _serialLen = 0;
    _Format(buf, length, usedLen);
    _serialLen = usedLen;
}