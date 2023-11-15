#pragma once

#include "SysBase.h"

#define FLAG_DEBUG_TRACER_S "/run/media/mmcblk0p1/flag_debug_tracer_s"

class TracerMainSys : public SysBase
{
public:
    TracerMainSys(uint8_t deviceId, int16_t processId);
    void Init() override;

    int PlGenDmaCalc(int drvCode, uint8_t *para_x, uint32_t leng_x, uint8_t *para_y, uint32_t leng_y) override;

protected:
    void _RegisterAlgCb() override;
    void _CreateDebugLogDir();
    void _SetFrequencyParam();
};
