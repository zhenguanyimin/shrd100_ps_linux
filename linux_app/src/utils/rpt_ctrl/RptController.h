#pragma once

#include "eap_pub.h"

class RptController
{
public:
    RptController();
    void Refresh(uint16_t oldNums, uint16_t newNums);
    bool Report(uint16_t curNums);
    void Clear() { _intervalTimes = 0; }
    uint8_t GetInterval() { return _intervalTimes; }
protected:
private:
    uint8_t _intervalTimes;
};