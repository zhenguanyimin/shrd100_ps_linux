#include "CmdprocBase.h"
#include "eap_sys_cmds.h"

#define ALINK_CORE_HEADER_LEN   (sizeof(alink_msg_head_t) - 1)

class AlinkCmdproc : public CmdprocBase
{
public:
    AlinkCmdproc();

    static void RegisterAlinkCmd(uint16_t cmd, Alink_Func func);
    static void ShowRegisterInfo();
    static void SendAlinkCmd(alink_msg_t &msg, int16_t dataLen, uint8_t alinkCmd);
	static void FillHeader(alink_msg_head_t& header, uint16_t length, uint8_t msgId, uint8_t destId,uint8_t seq,uint8_t ans);
	static void FillCRC(alink_msg_t& header, uint16_t dataLen);
protected:
    void _RegisterCmd() override;
    void _OnAlinkCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    uint16_t _ProcAlinkCmd(uint8_t cmd, uint8_t *request, uint8_t *response);
    static std::uint16_t _ProcUavALinkCmd(std::uint8_t *request, std::uint8_t *response);
    static std::uint16_t _ProcSetRFEnableALinkCmd(std::uint8_t *request, std::uint8_t *response);
    static string _ConvertStr(uint16_t drvCode);
    static uint8_t _GetHeaderChecksum(const alink_msg_head_t& header);
    static bool _IsHeaderValid(const alink_msg_head_t& header);
    static Alink_Func s_alinkFuns[EAP_ALINK_CMD_MAX];
};
