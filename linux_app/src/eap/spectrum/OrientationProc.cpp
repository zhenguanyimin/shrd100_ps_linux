#include <iostream>
#include <iomanip>
#include "OrientationProc.h"
#include "HalMgr.h"
#include "SysBase.h"
#include "SpectrumModule.h"

using namespace std;

extern "C"
{
#include "dirCalc.h"
#include "ifDirCalc.h"
}

OrientationProc::OrientationProc(SpectrumModule& module) : _module(module)
{
    _state = ORI_STATE_DETECT;
    _step = ORI_STEP_IF_ENTER_DIR;
    _adminEntryOri = false;
    _detectMode = DETECT_MODE_NONE;
    _groupCnt = 0;
    memset(_levelDetects, 0, sizeof(_levelDetects));
    memset(_pitchAngles, 0, sizeof(_pitchAngles));
    _levelAmpMax = 0;
    _pitchAmpMax = 0;
    _levelAngleCen = 0x7FFFFFFF;
    _pitchAngleCen = 0x7FFFFFFF;

    _droneLocationMode = 0;
    _droneAngle = 0;
    _dronePitch = 0;
}

void OrientationProc::Proc(float curDetectFreq)
{
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    Drv_AngleInfo angleInfo = {0};
    Hal_Data inData = {0, nullptr};
    Hal_Data outData = {sizeof(Drv_AngleInfo), (uint8_t*)&angleInfo};
    if (nullptr != halMgr && EAP_SUCCESS == halMgr->OnGet(EAP_DRVCODE_GET_ANGLE_INFO, inData, outData))
    {
        int32_t pitch = (int32_t)angleInfo.pitch * 100;
        int32_t angle = (int32_t)angleInfo.yaw * 100;
        const SpectrumDrone_Info* oriDrone = _module.GetOrientateDrone();
        float actualFreq = (nullptr == oriDrone) ? 0 : oriDrone->actualFreq;
        int32_t amp = (nullptr == oriDrone) ? 0 : (int32_t)oriDrone->amp;
        uint8_t oldDetectMode = _detectMode;
        switch (_step)
        {
        case ORI_STEP_IF_ENTER_DIR:
            _ProcEnterOrientateStep(curDetectFreq, actualFreq, amp, oldDetectMode);
            break;
        case ORI_STEP_IF_AZIMUTH_ENOUGH:
            _ProcAzimuthEnoughStep();
            break;
        case ORI_STEP_IF_AZIMUTH_AIMED:
            _ProcAzimuthAimedStep(angle);
            break;
        case ORI_STEP_IF_PITCH_ENOUGH:
            _ProcPitchEnoughStep(angle);
            break;
        case ORI_STEP_IF_PITCH_AIMED:
            _ProcPitchAimedStep(pitch, angle);
            break;
        default:
            break;
        }
        _SaveAngleInfo(oldDetectMode, angle, pitch, amp);

        SendAsynData(EAP_CMD_REFRESH_WORK_STATUS, (uint8_t*)&_state, 1);
    }
}

void OrientationProc::Clear()
{
	_step = ORI_STEP_IF_ENTER_DIR;
	_detectMode = DETECT_MODE_NONE;
    _state = ORI_STATE_DETECT;

    _levelAngleCen = 0x7FFFFFFF;
    _pitchAngleCen = 0x7FFFFFFF;

	_levelAmpMax = 0;
	_pitchAmpMax = 0;

    uint8_t freqMode = DEFINED_SWEEP_FREQ_MODE;
    SendAsynData(EAP_CMD_SET_FREQUENCY_MODE, &freqMode, 1);

    _adminEntryOri = false;
}

bool OrientationProc::IsOpenOrientation()
{
    return (ORI_STATE_DETECT == _state || ORI_STATE_ELIGIBLE_ENTRY_DIR == _state || ORI_STATE_NO_ELIGIBLE_ENTRY_DIR == _state);
}

void OrientationProc::_SaveAngleInfo(uint8_t oldDetectMode, int32_t angle, int32_t pitch, int32_t amp)
{
	if (DETECT_MODE_LEVELDETECTE == oldDetectMode)
	{
		if (_groupCnt < LEVEL_CNT_MAX)
		{
			_levelDetects[DATA_ANGLE][_groupCnt] = angle;
			_levelDetects[DATA_DIR_AMP][_groupCnt] = amp;
			_levelDetects[DATA_OMNI_AMP][_groupCnt] = amp;
			_groupCnt++;
		}
		else
		{
			Clear();
		}
	}
	else if (DETECT_MODE_PITCHANGLE == oldDetectMode)
	{
		if (_groupCnt < PITCH_CNT_MAX)
		{
			_pitchAngles[DATA_ANGLE][_groupCnt] = pitch;
			_pitchAngles[DATA_DIR_AMP][_groupCnt] = amp;
			_pitchAngles[DATA_OMNI_AMP][_groupCnt] = amp;
			_groupCnt++;
		}
		else
		{
			Clear();
		}
	}
	else if (DETECT_MODE_IF_ENTER_DIR == oldDetectMode)
	{
		if (_groupCnt < PITCH_CNT_MAX)
		{
			_pitchAngles[DATA_ANGLE][_groupCnt] = pitch;
			_groupCnt++;
		}
		else
		{
			_groupCnt = 0;
		}
	}
	else
	{
		_groupCnt = 0;
	}

	if (_detectMode != oldDetectMode && DETECT_MODE_NONE == _detectMode)
	{
		if (DETECT_MODE_LEVELDETECTE == oldDetectMode)
		{
			dirCalc(_levelDetects[DATA_DIR_AMP], _levelDetects[DATA_ANGLE], _groupCnt, &_levelAmpMax, &_levelAngleCen);
			EAP_LOG_DEBUG("LevelDetecte:  ampMax %d, angleCen:%d\r\n", _levelAmpMax, _levelAngleCen);
		}
		else if (DETECT_MODE_PITCHANGLE == oldDetectMode)
		{
			pitchCalcMax(_pitchAngles[DATA_DIR_AMP], _pitchAngles[DATA_ANGLE], _groupCnt, &_pitchAmpMax, &_pitchAngleCen);
			EAP_LOG_DEBUG("PitchAngle:  ampMax %d, angleCen:%d\r\n", _pitchAmpMax, _pitchAngleCen);
		}
		_groupCnt = 0;
	}
}

void OrientationProc::_ProcPitchAimedStep(int32_t pitch, int32_t angle)
{
    if ((ifPitchAimed(pitch, _pitchAngleCen) == 1) && (ifAzimuthAimed(angle, _levelAngleCen) == 1))
    {
        _step = ORI_STEP_ALREADY_AIMED;
        _state = ORI_STATE_AIM;
        _groupCnt = 0;
    }
}

void OrientationProc::_ProcPitchEnoughStep(int32_t angle)
{
    if ((ifAzimuthAimed(angle, _levelAngleCen) == 1) && (ifPitchEnough(_pitchAngles[DATA_ANGLE], _groupCnt) == 1))
    {
        pitchCalcMax(_pitchAngles[DATA_DIR_AMP], _pitchAngles[DATA_ANGLE], _groupCnt, &_pitchAmpMax, &_pitchAngleCen);
        EAP_LOG_DEBUG("_pitchAngles:  ampMax %d, angleCen:%d\r\n", _pitchAmpMax, _pitchAngleCen);
        if (_droneLocationMode == 1)
        {
            _pitchAngleCen = _dronePitch;
            EAP_LOG_DEBUG("_pitchAngles:  ampMax %d, angleCen:%d\r\n", _pitchAmpMax, _pitchAngleCen);
        }
        _step = ORI_STEP_IF_PITCH_AIMED;
        _groupCnt = 0;
        _state = ORI_STATE_PITCH_AIMING;
    }
}

void OrientationProc::_ProcAzimuthAimedStep(int32_t angle)
{
    if (ifAzimuthAimed(angle, _levelAngleCen) == 1)//判断是否已经水平瞄准成功
    {
        _step = ORI_STEP_IF_PITCH_ENOUGH;
        _groupCnt = 0;
        _detectMode = DETECT_MODE_PITCHANGLE;
        _state = ORI_STATE_SCANNING_PITCH;
    }
}

void OrientationProc::_ProcAzimuthEnoughStep()
{
    int totalAzimuth = 0;
    if (ifAzimuthEnough(_levelDetects[DATA_ANGLE], _groupCnt, &totalAzimuth) == 1)
    {
        dirCalc(_levelDetects[DATA_DIR_AMP], _levelDetects[DATA_ANGLE], _groupCnt, &_levelAmpMax, &_levelAngleCen);
        if (_levelAngleCen < 0)
        {
        	_levelAngleCen += (int)(36000);
        }
        EAP_LOG_DEBUG("_levelDetects:  ampMax %d, angleCen:%d\r\n", _levelAmpMax, _levelAngleCen);
        if (1 == _droneLocationMode)
        {
        	_levelAngleCen = _droneAngle;
        	EAP_LOG_DEBUG("_levelDetects:  ampMax %d, angleCen:%d\r\n", _levelAmpMax, _levelAngleCen);
        }

        _pitchAngleCen = 0x7FFFFFFF;

 		_step = ORI_STEP_IF_AZIMUTH_AIMED;
        _groupCnt = 0;
        _state = ORI_STATE_HORI_AIMING;
    }
}

void OrientationProc::_ProcEnterOrientateStep(float curDetectFreq, float actualFreq, int32_t amp, uint8_t& oldDetectMode)
{
    if (ifDirCircFreq(actualFreq))
    {
        _state = ORI_STATE_ELIGIBLE_ENTRY_DIR;
        if (_adminEntryOri)
        {
            uint8_t IfChangeAntenna = ifDirCircFreq(curDetectFreq);

            _levelAngleCen = 0x7FFFFFFF;
            _pitchAngleCen = 0x7FFFFFFF;

 			_step = ORI_STEP_IF_AZIMUTH_ENOUGH;

            uint8_t freqMode = DEFINED_FREQ_MODE;
            SendAsynData(EAP_CMD_SET_FREQUENCY_MODE, &freqMode, 1);
            
            _detectMode = DETECT_MODE_LEVELDETECTE;
            oldDetectMode = _detectMode;

            HalMgr* halMgr = EapGetSys().GetHalMgr();
            Drv_SetAntennaMode antennaMode = {IfChangeAntenna, amp};
            Hal_Data inData = {sizeof(Drv_SetAntennaMode), (uint8_t*)&antennaMode};
            if (nullptr != halMgr) halMgr->OnSet(EAP_DRVCODE_SET_ANTENNA_MODE, inData);

            _groupCnt = 0;
            _state = ORI_STATE_SCANNING_HORI;
        }
    }
    else
    {
        _state = ORI_STATE_NO_ELIGIBLE_ENTRY_DIR;
    }
}

void OrientationProc::ShowInfo(uint8_t level)
{
    cout << EAP_BLANK_PREFIX(level) << "_state: " << (uint16_t)_state;
    cout << EAP_BLANK_PREFIX(level) << "_step: " << (uint16_t)_step;
    cout << EAP_BLANK_PREFIX(level) << "_adminEntryOri: " << _adminEntryOri;
    cout << EAP_BLANK_PREFIX(level) << "_detectMode: " << (uint16_t)_detectMode;
    cout << EAP_BLANK_PREFIX(level) << "_groupCnt: " << (uint16_t)_groupCnt;
    cout << EAP_BLANK_PREFIX(level) << "_levelAmpMax: " << _levelAmpMax;
    cout << EAP_BLANK_PREFIX(level) << "_pitchAmpMax: " << _pitchAmpMax;
    cout << EAP_BLANK_PREFIX(level) << "_levelAngleCen: " << _levelAngleCen;
    cout << EAP_BLANK_PREFIX(level) << "_pitchAngleCen: " << _pitchAngleCen;
    cout << EAP_BLANK_PREFIX(level) << "_droneLocationMode: " << (uint16_t)_droneLocationMode;
    cout << EAP_BLANK_PREFIX(level) << "_droneAngle: " << _droneAngle;
    cout << EAP_BLANK_PREFIX(level) << "_dronePitch: " << _dronePitch;
    cout << EAP_BLANK_PREFIX(level) << "_levelDetects: ";
    int i = 0;
    for (; i < 3; ++i)
    {
        cout << EAP_BLANK_PREFIX(level + 1) << "_levelDetects[" << i << "]: ";
        for (int j = 0; j < LEVEL_CNT_MAX; ++j)
        {
            if (j % 16 == 0) 
            {
                cout << EAP_BLANK_PREFIX(level + 2) << "[";
            }
            cout << _levelDetects[i][j] << ", ";
            if (j % 16 == 15) 
            {
                cout << EAP_OUT_BACKSPACE << "]";
            }
        }
    }
    cout << EAP_BLANK_PREFIX(level) << "_pitchAngles: ";
    for (i = 0; i < 3; ++i)
    {
        cout << EAP_BLANK_PREFIX(level + 1) << "_pitchAngles[" << i << "]: ";
        for (int j = 0; j < PITCH_CNT_MAX; ++j)
        {
            if (j % 16 == 0) 
            {
                cout << EAP_BLANK_PREFIX(level + 2) << "[";
            }
            cout << _pitchAngles[i][j] << ", ";
            if (j % 16 == 15) 
            {
                cout << EAP_OUT_BACKSPACE << "]";
            }
        }
    }
}