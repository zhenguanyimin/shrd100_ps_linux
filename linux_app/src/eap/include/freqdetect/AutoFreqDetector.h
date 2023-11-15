#pragma once

#include "FreqDetector.h"

class AutoFreqDetector : public FreqDetector
{
    DECLARE_SINGLETON(AutoFreqDetector)
public:
    void SetParams(uint32_t startFreq, uint32_t endFreq, uint32_t freqInc);
    float NextFreq() override;
    void Init() override;
    void ShowInfo(uint8_t level) override;
protected:
private:
	/* Unit: MHz */
	uint32_t _curFreq;
	/* Unit: MHz */
	uint32_t _startFreq;
	/* Unit: MHz */
	uint32_t _endFreq;
	/* Unit: MHz */
	uint32_t _freqInc;
};
 