#include "DrvRecorderFormatter.h"
#include "HalBase.h"

DrvRecorderFormatter::DrvRecorderFormatter(uint32_t cmdId) : RecorderFormatter(cmdId)
{
}

void DrvRecorderFormatter::_Format(uint8_t* inBuf, uint32_t inLen, uint32_t& usedLen)
{
    RecorderFormatter::_Format(inBuf, inLen, usedLen);
    EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    _SprintfCmdWriteBuf(inBuf, inLen, 16, usedLen);
    EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    usedLen += sprintf((char*)(_serialBuf + usedLen), "\n");
}

string DrvRecorderFormatter::_GetTypeStr()
{
    return "Drv";
}

string DrvRecorderFormatter::_GetCmdIdStr(uint16_t id)
{
    return HalBase::ConvertStr(id);
}

void DrvRecorderFormatter::WriteBuf(uint8_t* inBuf, uint32_t inLen, uint8_t* outBuf, uint32_t outLen, uint32_t result)
{
    Write(inBuf, inLen);
    uint32_t usedLen = _serialLen;
    usedLen += sprintf((char*)(_serialBuf + usedLen), "\t[Ret]%d [OutLen]%d", result, outLen);
    EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    _SprintfCmdWriteBuf(outBuf, outLen, 16, usedLen);
    EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    usedLen += sprintf((char*)(_serialBuf + usedLen), "\n");
    _serialLen = usedLen;
}