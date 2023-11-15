#include "FreqDetector.h"

FreqDetector::FreqDetector()
{
    _isSweep = true;
}

void FreqDetector::Init()
{
}

void FreqDetector::ShowInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "Class: " << typeid(*this).name();
    cout << EAP_BLANK_PREFIX(level) << "_isSweep: " << _isSweep;
}
