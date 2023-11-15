#include "MsgRecorderFormatter.h"
#include "CmdprocBase.h"

MsgRecorderFormatter::MsgRecorderFormatter(uint32_t cmdId) : RecorderFormatter(cmdId)
{
}

void MsgRecorderFormatter::_Format(uint8_t* inBuf, uint32_t inLen, uint32_t& usedLen)
{
    RecorderFormatter::_Format(inBuf, inLen, usedLen);
    EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    _FormatMsgHeadTitle(usedLen);
    EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    _SprintfCmdWriteBuf(inBuf, inLen, sizeof(Thread_Msg_Head), usedLen);
    EAP_CHECK_CONDITION_RET(usedLen >= RECORD_ITEM_SERIAL_BUF_LEN);
    usedLen += sprintf((char*)(_serialBuf + usedLen), "\n");
}

string MsgRecorderFormatter::_GetTypeStr()
{
    return "Msg";
}

string MsgRecorderFormatter::_GetCmdIdStr(uint16_t id)
{
    return CmdprocBase::ConvertStr(id);
}

void MsgRecorderFormatter::_FormatMsgHeadTitle(uint32_t& usedLen)
{
    usedLen += sprintf((char*)(_serialBuf + usedLen), "\n\tFrom        To          rq ty code  seq         len");
}
