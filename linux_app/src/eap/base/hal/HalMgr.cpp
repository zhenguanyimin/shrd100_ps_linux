#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "HalMgr.h"
#include "HalBase.h"
#include "SysBase.h"

HalMgr::HalMgr()
{
    memset(_drvCode2Hal, 0, sizeof(_drvCode2Hal));
}

HalMgr::~HalMgr()
{
    Clear();
}

void HalMgr::Init()
{
    _InstallHals();
    for (auto hal : _hals)
    {
        hal->Init();
    }
}

void HalMgr::AddHal(HalBase* hal)
{
    _hals.emplace_back(hal);
}

HalBase* HalMgr::_GetImplHal(uint16_t drvCode)
{
    if (drvCode >= EAP_DRVCODE_MAX)
    {
        EAP_LOG_ERROR("drvCode(%d) is bigger than EAP_DRVCODE_MAX(%d)", drvCode, EAP_DRVCODE_MAX);
        return nullptr;
    }
    return _drvCode2Hal[drvCode];
}

int HalMgr::OnSet(uint16_t drvCode, const Hal_Data& inData)
{
    HalBase* hal = _GetImplHal(drvCode);
    if (nullptr == hal)
    {
        EAP_LOG_ERROR("_SetImplHal(drvCode:%d) error!", drvCode);
        return EAP_FAIL;
    }
    Hal_Data outData = {0};
    return hal->Dispatch(drvCode, inData, outData);
}

int HalMgr::OnGet(uint16_t drvCode, const Hal_Data& inData, Hal_Data& outData)
{
    HalBase* hal = _GetImplHal(drvCode);
    if (nullptr == hal)
    {
        EAP_LOG_ERROR("_GetImplHal(drvCode:%d) error!", drvCode);
        return EAP_FAIL;
    }
    return hal->Dispatch(drvCode, inData, outData);
}

void HalMgr::Register(uint16_t drvCode, HalBase* hal)
{
    if (drvCode >= EAP_DRVCODE_MAX)
    {
        EAP_LOG_ERROR("register drvCode(%d) error!", drvCode);
        return;
    }
    if (nullptr != _drvCode2Hal[drvCode])
    {
        EAP_LOG_ERROR("s_codeProcFuns(drvCode:%d) is not null!", drvCode);
        return;
    }
    _drvCode2Hal[drvCode] = hal;
}

void HalMgr::Clear()
{
    for (auto hal : _hals)
    {
        EAP_DELETE(hal);
    }
    _hals.clear();
}

void HalMgr::ShowInfo()
{
    cout << hex;
    cout << "\n" << "HalNum: " << _hals.size();
    cout << "\n" << "_hals: ";
    int i = 0;
    for (auto hal : _hals)
    {
        cout << EAP_BLANK_PREFIX(1) << "Hal[" << i << "]: ";
        hal->ShowKeyInfo(2);
        ++i;
    }
}

void HalMgr::ShowHalInfo(uint8_t index)
{
    if (index >= _hals.size()) 
    {
        cout << "\nParam is invalid!";
        return;
    }
    int i = 0;
    cout << hex;
    for (auto hal : _hals)
    {
        ++i;
        if (i == index)
        {
            cout << "Hal[" << i << "]: ";
            hal->ShowInfo(1);
            break;
        }
    }
}

extern "C"
{
    void ShowHalMgrInfo()
    {
        HalMgr *halMgr = EapGetSys().GetHalMgr();
        if (nullptr != halMgr) halMgr->ShowInfo();
    }

    void ShowOneHalInfo(uint8_t index)
    {
        HalMgr *halMgr = EapGetSys().GetHalMgr();
        if (nullptr != halMgr) halMgr->ShowHalInfo(index);
    }

    void ShowHalRegisterInfo()
    {
        HalBase::ShowRegisterInfo();
    }
}