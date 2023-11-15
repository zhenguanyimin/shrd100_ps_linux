#pragma once

#include "FreqDetector.h"

class DefinedFreqDetector : public FreqDetector
{
    DECLARE_SINGLETON(DefinedFreqDetector)
public:
    void SetParams(uint8_t definedFreqIndex, uint32_t definedFreq);
    void SetDefinedFreq(uint32_t freq) { _definedFreq = freq; }
    float NextFreq() override;
    uint8_t GetCurIndex() override { return _definedFreqIndex; }
    void ShowInfo(uint8_t level) override;
protected:
private:
    /* Unit: MHz */
    uint8_t _definedFreqIndex;
    uint32_t _definedFreq;
};
 