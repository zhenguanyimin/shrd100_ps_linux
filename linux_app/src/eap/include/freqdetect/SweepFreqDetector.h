#pragma once

#include <vector>
#include "FreqDetector.h"

using namespace std;

class SweepFreqDetector : public FreqDetector
{
    DECLARE_SINGLETON(SweepFreqDetector)
public:
    void SetParams(float* freqs, uint8_t num);
    float NextFreq() override;
    uint8_t GetCurIndex() override { return _curFreqIndex; }
    void Init() override;
    void ShowInfo(uint8_t level) override;
protected:
private:
    vector<float> _sweepFreqs;
    uint8_t _curFreqIndex;
};
