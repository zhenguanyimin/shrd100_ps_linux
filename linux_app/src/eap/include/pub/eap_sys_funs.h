#pragma once

//#include <sys/_stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void eap_main();
void SendAsynMsg(uint16_t cmdCode, uint8_t *buf, uint32_t dataLen);
void SendAsynMsg2Thread(uint16_t to, uint8_t *buf, uint32_t dataLen);
void SendSynMsg(uint16_t from, uint16_t cmdCode, uint8_t* reqBuf, uint32_t reqDataLen, uint8_t* respData, uint32_t *respLen);
void SendAsynData(uint16_t cmdCode, uint8_t *data, uint32_t dataLen);
void SendSynData(uint16_t from, uint16_t cmdCode, uint8_t* reqData, uint32_t reqDataLen, uint8_t* respData, uint32_t *respLen);
void SendAsynCmdCode(uint16_t cmdCode);
bool IsBig_Endian();

typedef uint16_t (*Alink_Func)(uint8_t *request, uint8_t *response);
void register_alink_cmd(uint16_t cmd, Alink_Func func);
void alink_head_fill_crc(void * alink_msg, uint16_t dataLen);
void alink_msg_fill_header(void *msg_header, uint16_t length, uint8_t msgId, uint8_t destId,uint8_t seq,uint8_t ans);


void eap_help();
void eap_do(uint16_t index, int count, ...);
void ShowAllThreadsMsgStats();
void ShowOneThreadMsgStats(uint16_t threadId);
void OpenThreadRunDetect();
void ThreadPause(uint16_t threadId);
void ThreadIdling(uint16_t threadId);
void ThreadResume(uint16_t threadId);
void ShowAllThreadsInfo();
void EnableStoreCmd(uint8_t type);
void DisableStoreCmd(uint8_t type);
void ShowOneModuleInfo(uint8_t moduleId);
void ShowOneModuleInfoUnSafe(uint8_t moduleId);
void ShowAllCmdStat();
void ShowOneCmdStat(uint16_t cmdCode);
void ClearAllCmdStat();
void ClearOneCmdStat(uint16_t cmdCode);
void ShowCmdprocRegisterInfo();
void ShowSysInfo();
void ShowProductFeatures();
void ShowVersionInfo();
void ShowHalMgrInfo();
void ShowOneHalInfo(uint8_t index);
void ShowHalRegisterInfo();
void ShowAlinkCmdRegisterInfo();
void ShowCliRegisteredCmds();
void AddIgnoreCmd(uint16_t cmdCode);
void DelIgnoreCmd(uint16_t cmdCode);
void ReCreateThread(uint16_t threadId);
void ClearRemainDupTimes();
void SetWarningLevel(uint8_t gearPosition,uint8_t targetState,uint8_t workMode,uint8_t level);
void ShowOneThreadInfo(uint16_t threadId);
void SetTargetAppear(uint8_t isTargetAppear);
void SendALinkMsg(uint8_t channelType, uint8_t msgType, uint32_t p1, uint32_t p2, uint32_t p3);
void AirBorneCtrl(uint16_t workMode, uint16_t dxNum, uint16_t dxFreq);
void SetLedStatus(uint8_t ledNo, uint8_t status);
void AdcSetGain(uint32_t value);
void SetReceivedALinkMsg(const int * para, const uint32_t length);
void SetRemoteIdWifiChannel(const int * para, const uint32_t length);

#ifdef __cplusplus
}
#endif
