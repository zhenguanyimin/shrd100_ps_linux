#include "SweepFreqDetector.h"

DEFINE_SINGLETON_NO_CONSTRUCT(SweepFreqDetector)

SweepFreqDetector::SweepFreqDetector()
{
    _sweepFreqs.reserve(16);
    _curFreqIndex = UINT8_INVALID;
}

void SweepFreqDetector::SetParams(float* freqs, uint8_t num)
{
    _sweepFreqs.clear();

    for (uint8_t i = 0; i < num; ++i)
    {
        _sweepFreqs.push_back(freqs[i]);
    }
    Init();
}

void SweepFreqDetector::Init()
{
    _curFreqIndex = UINT8_INVALID;
    FreqDetector::Init();
}

float SweepFreqDetector::NextFreq()
{
    if (UINT8_INVALID == _curFreqIndex) _curFreqIndex = 0;

    if (_isSweep)
    {
        if (_curFreqIndex + 1 < _sweepFreqs.size()) ++_curFreqIndex;
        else _curFreqIndex = 0;
    }

    if (_curFreqIndex >= _sweepFreqs.size()) return UINT32_INVALID;
    return _sweepFreqs[_curFreqIndex];
}

void SweepFreqDetector::ShowInfo(uint8_t level)
{
    FreqDetector::ShowInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "_curFreqIndex: " << (uint16_t)_curFreqIndex;
    cout << EAP_BLANK_PREFIX(level) << "_sweepFreqs: [";
    for (auto& freq : _sweepFreqs)
    {
        cout << freq << ", ";
    }
    if (_sweepFreqs.size() > 0) cout << EAP_OUT_BACKSPACE;
    cout << "]";
}
