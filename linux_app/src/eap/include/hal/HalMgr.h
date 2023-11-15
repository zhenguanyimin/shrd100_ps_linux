#pragma once

#include <list>

#include "eap_pub.h"

using namespace std;

typedef struct Hal_Data
{
	uint32_t length;
	uint8_t* buf;
} Hal_Data;

class HalBase;

class HalMgr
{
public:
    HalMgr();
    virtual ~HalMgr();
    int OnSet(uint16_t drvCode, const Hal_Data& inData);
    int OnGet(uint16_t drvCode, const Hal_Data& inData, Hal_Data& outData);
    void Init();
    void Register(uint16_t drvCode, HalBase* hal);
    void AddHal(HalBase* hal);
    void Clear();
    void ShowInfo();
    void ShowHalInfo(uint8_t index);
protected:
    HalBase* _GetImplHal(uint16_t drvCode);
    virtual void _InstallHals() = 0;
private:
    list<HalBase*> _hals;
    HalBase* _drvCode2Hal[EAP_DRVCODE_MAX];
};
