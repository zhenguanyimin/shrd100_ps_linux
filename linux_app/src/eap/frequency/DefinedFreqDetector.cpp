#include "DefinedFreqDetector.h"

DEFINE_SINGLETON_NO_CONSTRUCT(DefinedFreqDetector)

DefinedFreqDetector::DefinedFreqDetector()
{
    _definedFreqIndex = 0;
    _definedFreq = 0;
}

void DefinedFreqDetector::SetParams(uint8_t definedFreqIndex, uint32_t definedFreq)
{
    _definedFreqIndex = definedFreqIndex;
    _definedFreq = definedFreq;
    Init();
}

float DefinedFreqDetector::NextFreq()
{
    return _definedFreq;
}

void DefinedFreqDetector::ShowInfo(uint8_t level)
{
    FreqDetector::ShowInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "_definedFreqIndex: " << _definedFreqIndex;
    cout << EAP_BLANK_PREFIX(level) << "_definedFreq: " << _definedFreq;
}