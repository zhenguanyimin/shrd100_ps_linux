#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include "HalMgr.h"

using namespace std;

typedef int (HalBase::*Hal_Func)(const Hal_Data& inData, Hal_Data& outData);

class DataRecorder;
class DrvRecorderFormatter;
class HalBase
{
public:
    HalBase(HalMgr* halMgr);
    virtual ~HalBase(){}
    int Dispatch(uint16_t drvCode, const Hal_Data& inData, Hal_Data& outData);
    virtual void Init(){}
    void ShowKeyInfo(uint8_t level);
    virtual void ShowInfo(uint8_t level){}
    static void Clear();
    static Hal_Func* GetCodeProcFuns() { return s_codeProcFuns; }
    static void SetCodeProcFuns(Hal_Func* funcs);
    static void EnableStoreCmd() { s_ctrlIsStoreCmd = true; }
    static void DisableStoreCmd() { s_ctrlIsStoreCmd = false; }
    static void SetDataRecorder(DataRecorder* recorder) { s_recorder = recorder; }
    static void ShowRegisterInfo();
    static string ConvertStr(uint16_t drvCode);
protected:
    void _StoreMsg(DrvRecorderFormatter& formatter, const Hal_Data& inData, Hal_Data& outData, uint32_t result);
    virtual int _Dispatch(uint16_t drvCode, const Hal_Data& inData, Hal_Data& outData);
    void _Register(uint16_t drvCode, Hal_Func cmdFun);
    static DataRecorder* s_recorder;
private:
    static Hal_Func s_codeProcFuns[EAP_DRVCODE_MAX];
    HalMgr* _halMgr;
    static bool s_isStoreCmd;
    static bool s_ctrlIsStoreCmd;
};
