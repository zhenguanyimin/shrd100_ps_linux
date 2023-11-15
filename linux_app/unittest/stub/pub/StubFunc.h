#pragma once

#include "Module.h"
#include "ModuleStrategy.h"
#include "SysBase.h"

extern "C"
{
#include "droneID_system_param.h"

extern Rpt_DroneInfo g_stubOutputList;
extern int g_stubOutputListNum;

extern void Stub_SetOutputList(Rpt_DroneInfo* output);
}

extern SysBase* g_sys;

typedef struct Stub_DrvData
{
    uint16_t drvCode;
    uint8_t* buf;
    uint32_t bufLen;
} Stub_DrvData;

extern Stub_DrvData g_stubHalInDatas[100];
extern uint32_t g_stubHalInDatasNum;
extern Stub_DrvData g_stubHalOutDatas[100];
extern uint32_t g_stubHalOutDatasNum;
extern SysBase* g_sys;
extern Drv_Drone_Freq g_stubDroneFreq;

void ClearDatas();
void Stub_Restore();
void Stub_ReInit(SysBase* newSys);
bool Stub_CheckRptInfoHeadByAlink(uint8_t* buf, uint8_t msgId, uint16_t msgLen);
bool Stub_CheckRptTargetNum(uint8_t* buf, uint8_t num);
void Stub_SetDroneFreq(int freq, int freqOffset);
bool Stub_CheckDupBoradcastResponse(uint8_t* buf, uint32_t protocol, uint16_t devType);
uint8_t* Stub_GetAlinkBuf(uint8_t* buf);
uint32_t Stub_GetAlinkBufLen(uint32_t bufLen);
