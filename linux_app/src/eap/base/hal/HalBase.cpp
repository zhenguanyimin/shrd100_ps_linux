#include <stdio.h>
#include "HalBase.h"
#include "DrvRecorderFormatter.h"
#include "DataRecorder.h"

Hal_Func HalBase::s_codeProcFuns[EAP_DRVCODE_MAX] = {0};
bool HalBase::s_isStoreCmd = true;
bool HalBase::s_ctrlIsStoreCmd = true;
DataRecorder* HalBase::s_recorder = nullptr;

HalBase::HalBase(HalMgr* halMgr) : _halMgr(halMgr)
{

}

void HalBase::_Register(uint16_t drvCode, Hal_Func drvFun)
{
    if (drvCode >= EAP_DRVCODE_MAX)
    {
        EAP_LOG_ERROR("register drvCode(%d) error!", drvCode);
        return;
    }
    s_codeProcFuns[drvCode] = drvFun;
    if (nullptr != _halMgr)
    {
        _halMgr->Register(drvCode, this);
    }
}

void HalBase::_StoreMsg(DrvRecorderFormatter& formatter, const Hal_Data& inData, Hal_Data& outData, uint32_t result)
{
    if (nullptr == s_recorder) return;
    formatter.WriteBuf(inData.buf, inData.length, outData.buf, outData.length, result);
    s_recorder->Save(formatter);
}

int HalBase::Dispatch(uint16_t drvCode, const Hal_Data& inData, Hal_Data& outData)
{
    DrvRecorderFormatter* formatter = nullptr;
    s_isStoreCmd = s_ctrlIsStoreCmd;
    if (s_isStoreCmd)
    {
        formatter = new DrvRecorderFormatter(drvCode);
        if (nullptr != formatter) formatter->Begin();
    }
    int ret = _Dispatch(drvCode, inData, outData);
    if (s_isStoreCmd)
    {
        if (nullptr != formatter)
        {
            formatter->End();
            _StoreMsg(*formatter, inData, outData, (uint32_t)ret);
        }
    }
    return ret;
}

int HalBase::_Dispatch(uint16_t drvCode, const Hal_Data& inData, Hal_Data& outData)
{
    if (drvCode >= EAP_DRVCODE_MAX)
    {
        EAP_LOG_ERROR("_Dispatch drvCode(%d) error!", drvCode);
        return EAP_FAIL;
    }
    if (nullptr == s_codeProcFuns[drvCode])
    {
        EAP_LOG_ERROR("s_codeProcFuns(drvCode:%d) is null!", drvCode);
        return EAP_FAIL;
    }
    return (this->*(s_codeProcFuns[drvCode]))(inData, outData);
}

void HalBase::Clear()
{
    memset(s_codeProcFuns, 0, sizeof(s_codeProcFuns));
}

void HalBase::SetCodeProcFuns(Hal_Func* funcs)
{
    memcpy(s_codeProcFuns, funcs, sizeof(s_codeProcFuns));
}

void HalBase::ShowKeyInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "Class: " << typeid(*this).name();
    EAP_OUT_THIS_PTR(level);
}

void HalBase::ShowRegisterInfo()
{
    cout << hex;
    cout << "\n" << "s_isStoreCmd: " << s_isStoreCmd;
    cout << "\n" << "s_ctrlIsStoreCmd: " << s_ctrlIsStoreCmd;
    cout << "\n" << "s_ctrlIsStoreCmd: " << s_ctrlIsStoreCmd;
    cout << "\n" << "CodeMap: ";
    cout << "\n" << "  No.  | Code                                | Func";
    for (int i = 0; i < EAP_DRVCODE_MAX; ++i)
    {
        cout << "\n  " << right << setw(4) << i;
        cout << " | " << left << setw(35) << ConvertStr(i);
        cout << " | " << right << setw(14) << reinterpret_cast<void*>(s_codeProcFuns[i]);
    }
}

string HalBase::ConvertStr(uint16_t drvCode)
{
    EAP_GET_DRVCODE_STR(drvCode);
    return "unknown";
}
