#include "AutoFreqDetector.h"

DEFINE_SINGLETON_NO_CONSTRUCT(AutoFreqDetector)

AutoFreqDetector::AutoFreqDetector()
{
    _curFreq = 0;
    _startFreq = 0;
    _endFreq = 0;
    _freqInc = 0;
}

void AutoFreqDetector::SetParams(uint32_t startFreq, uint32_t endFreq, uint32_t freqInc)
{
    _startFreq = startFreq;
    _endFreq = endFreq;
    _freqInc = freqInc;
    Init();
}

void AutoFreqDetector::Init()
{
    _curFreq = _endFreq;
    FreqDetector::Init();
}

float AutoFreqDetector::NextFreq()
{
    if (_isSweep)
    {
        _curFreq += _freqInc;
        if (_curFreq > _endFreq) _curFreq = _startFreq;
    }
    return _curFreq;
}

void AutoFreqDetector::ShowInfo(uint8_t level)
{
    FreqDetector::ShowInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "_curFreq: " << _curFreq;
    cout << EAP_BLANK_PREFIX(level) << "_startFreq: " << _startFreq;
    cout << EAP_BLANK_PREFIX(level) << "_endFreq: " << _endFreq;
    cout << EAP_BLANK_PREFIX(level) << "_freqInc: " << _freqInc;
}