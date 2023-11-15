#pragma once

#include <iostream>
#include <iomanip>
#include "eap_pub.h"

using namespace std;

class FreqDetector
{
public:
    FreqDetector();
    virtual ~FreqDetector(){}
    void EnableSweep() { _isSweep = true; }
    void DisableSweep() { _isSweep = false; }
    virtual float NextFreq() = 0;
    virtual void Init();
    virtual uint8_t GetCurIndex() { return UINT8_INVALID; }
    virtual void ShowInfo(uint8_t level);
protected:
    bool _isSweep;
private:
};
