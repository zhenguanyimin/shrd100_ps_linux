#pragma once


#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include "HalBase.h"
#include "eap_pub.h"

class AlinkCommonHal : public HalBase
{
public:
    AlinkCommonHal(HalMgr* halMgr);
    void Init() override;

protected:
	int _SendAlinkRspMsg(const Hal_Data& inData, Hal_Data& outData);
private:

};
