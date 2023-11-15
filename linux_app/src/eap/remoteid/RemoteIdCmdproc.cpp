#include "RemoteIdCmdproc.h"
#include "RemoteIdBufScanStrategy.h"
#include "RemoteIdModule.h"
#include "AlinkCmdproc.h"
#include "HalMgr.h"
#include "SysBase.h"
#include <sys/time.h>
#include <stdio.h>

#ifndef __UNITTEST__
#include "../../tracer/hal/remoteid_wifi/rid_capture.h"
#endif
#include <stdio.h>
#include <unistd.h>

int g_scan_channels_all[3] = {1,6,11};
extern struct timeval glast_tv;

static Drv_RemoteIdChnSet RemoteIdChnValue = {0};
uint8_t test_data[] = { 0x80,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0xEB,0x51,0x6F,0x9A,0x01,0x66,0x66,0x51,0x6F,0x9A,0x01,0x88,0x88,0x80,0x50,
                            0x90,0x37,0xAB,0x46,0x00,0x00,0x00,0x00,0xFF,0xFF,0x21,0x04,
                            0x00,0x0C,0x64,0x65,0x66,0x61,0x75,0x6C,0x74,0x2D,0x73,0x73,0x69,0x64,
                            0xDD,0xB7,0xFA,0x0B,0xBC,0x0D,
                            0x00,0xF0,0x19,0x07,
                            0x02,0x13,0x4C,0x54,0x4F,0x39,0x32,0x32,0x32,0x38,0x35,0x34,0x39,0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x02,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x12,0x12,0xB4,0x00,0x00,0x35,0x29,0x78,0x0D,0x71,0xAB,0xF2,0x43,0x48,0x09,0x48,0x09,0x6A,0x07,0x6C,0x26,0x54,0x38,0x05,0x00,
                            0x22,0x10,0x00,0x00,0x90,0x6E,0xD6,0x1A,0x00,0x32,0xAA,0x66,0x9D,0x76,0x60,0x34,0x27,0xD9,0x4A,0x80,0xF3,0x00,0x00,0x00,0x00,
                            0x32,0x00,0x75,0x6E,0x73,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x42,0x00,0x90,0x08,0x78,0x0D,0x00,0xBA,0xF2,0x43,0x01,0x00,0x00,0x6A,0x07,0x6A,0x07,0x00,0x48,0x09,0x90,0x6E,0xD6,0x1A,0x00,
                            0x52,0x00,0x75,0x6E,0x73,0x65,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                          };
int Wifi_CaptureMain_test();

void Remoteid_SetRemoteIdChnValue(int value)
{
    if (1 == value)
    {
        RemoteIdChnValue.chn = 2412;
    }
    else  if (6 == value)
    {
        RemoteIdChnValue.chn = 2437;
    }
    else  if (11 == value)
    {
        RemoteIdChnValue.chn = 2462;
    }
    else
    {
        RemoteIdChnValue.chn = 2437;
    }
}

std::uint32_t GetFreqByChannel(const std::uint32_t channel)
{
    std::uint32_t freq = 2437;

    if (1 == channel)
    {
        freq = 2412;
    }
    else if (6 == channel)
    {
        freq = 2437;
    }
    else if (11 == channel)
    {
        freq = 2462;
    }
    else
    {
        freq = 2437;
    }

    return freq;
}

uint32_t Remoteid_GetRemoteIdChnValue()
{
    return RemoteIdChnValue.chn;
}

RemoteIdCmdproc::RemoteIdCmdproc() : _scanStrategy(nullptr)
{

}

void RemoteIdCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_REMOTEID, (Cmdproc_Func)&RemoteIdCmdproc::_OnRemoteIdCmd);
    Register(EAP_CMD_SHOW_REMOTEID_MODULE, (Cmdproc_Func)&RemoteIdCmdproc::_OnShowRemoteIdModuleCmd);
    Register(EAP_CMD_REMOTEID_RID_CAPTURE, (Cmdproc_Func)&RemoteIdCmdproc::_OnRemoteIdCmdRidCapture);
}

void RemoteIdCmdproc::_OnRemoteIdCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    uint8_t* data = nullptr;
    int32_t dataLen = inMsg.dataLen;

    if (EAP_SUCCESS == _GetRemoteIdData(inMsg, data, dataLen))
    {
        RemoteIdModule* remoteIdModule = dynamic_cast<RemoteIdModule*>(_module);
        if (nullptr != remoteIdModule)
        {
            remoteIdModule->BeaconFreamProcess(data, dataLen);
        }
    }
}

void RemoteIdCmdproc::_OnRemoteIdCmdRidCapture(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
	RemoteId_Info *rid_info = (RemoteId_Info*)inMsg.data;
	//printf("****chn:%d,uavid:%s\n",rid_info->uChn,rid_info->uavId);

    RemoteIdModule* remoteIdModule = dynamic_cast<RemoteIdModule*>(_module);
    if (nullptr != remoteIdModule)
    {
        remoteIdModule->RemoteIdCmdRidCapture(rid_info);
    }	
}

int32_t RemoteIdCmdproc::_GetRemoteIdData(const Cmdproc_Data& inMsg, uint8_t* &data, int32_t& dataLen)
{
    if (nullptr == _scanStrategy) return EAP_FAIL;

    uint32_t start = 0;
    if (EAP_SUCCESS != _scanStrategy->Scan(inMsg.data, inMsg.dataLen, start))
    {
        return EAP_FAIL;
    }
    data = inMsg.data + start;

    return EAP_SUCCESS;
}

void RemoteIdCmdproc::_OnTimer(uint8_t timerType)
{
	static unsigned int scan_idx = 0;
    static unsigned int scan_idx_test = 0;
    static int last_channel = 0;
	int ret;
    int chn;
    if (EAP_CMDTYPE_TIMER_1 == timerType)
    {
        _ReportRemoteIdsInfo();
    }
    else if (EAP_CMDTYPE_TIMER_2 == timerType)
    {
		HalMgr* halMgr = EapGetSys().GetHalMgr();
#ifndef __UNITTEST__
		if (nullptr != halMgr)
		{
            RemoteIdModule* remoteIdModule = dynamic_cast<RemoteIdModule*>(_module);
            if (nullptr != remoteIdModule)
            {
                struct timeval tv;
                gettimeofday(&tv, NULL);

                std::uint64_t diffInMs = 0;
                if (tv.tv_sec > glast_tv.tv_sec)
                {
                   if (tv.tv_usec >= glast_tv.tv_usec)
                   {
                       diffInMs = (tv.tv_sec - glast_tv.tv_sec) * 1000 + (tv.tv_usec - glast_tv.tv_usec) / 1000;
                   }
                   else
                   {
                       diffInMs = (tv.tv_sec - glast_tv.tv_sec - 1) * 1000 + (1000000 + tv.tv_usec - glast_tv.tv_usec) / 1000;            
                   }
                }
                else if (tv.tv_sec == glast_tv.tv_sec)
                {
                   if (tv.tv_usec >= glast_tv.tv_usec)
                   {
                       diffInMs = (tv.tv_usec - glast_tv.tv_usec) / 1000;
                   }
                }

                if ((diffInMs > 700) && (diffInMs < 5000)
                    && (remoteIdModule->IsActiveTargetInChannel(GetFreqByChannel(last_channel))))
                {
                    return; // not switch channel
                }
            }

            if(rid_checkfilewhetherexist(FLAG_DEBUG_REMOTEID_GET_UAV_DATA))
            {
                // int scan_channels_all[] = {1,6,11};
                int scan_channel_allnums = sizeof(g_scan_channels_all)/sizeof(int);
                chn = g_scan_channels_all[scan_idx_test % scan_channel_allnums];

                if (last_channel == chn)
                {
                    return; // not switch channel
                }
                else
                {
                    last_channel = chn;
                }
                scan_idx_test++; 
            }
            else
            {
                //printf("RemoteIdSetChannel calling\n");
                int scan_channels[] = {1,6,11};
                int scan_channel_nums = sizeof(scan_channels)/sizeof(int);
                chn = scan_channels[scan_idx%scan_channel_nums];
                scan_idx++; 

                last_channel = chn;
            }
					
	        Drv_RemoteIdChnSet info = {0};
			info.chn = chn;
	        Hal_Data inData = {sizeof(Drv_RemoteIdChnSet), (uint8_t*)&info};
	        ret = halMgr->OnSet(EAP_DRVCODE_REMOTEID_SET_CHANNEL, inData);
            Remoteid_SetRemoteIdChnValue(chn);
		}	
#endif
        // Wifi_CaptureMain_test();
    }
}

void RemoteIdCmdproc::_ReportRemoteIdsInfo()
{
    RemoteIdModule* remoteIdModule = dynamic_cast<RemoteIdModule*>(_module);
    if (nullptr != remoteIdModule)
    {
        alink_msg_t msg = {0};
        msg.channelType = EAP_CHANNEL_TYPE_TCP | EAP_CHANNEL_TYPE_TYPEC;
        msg.channelId   = UINT16_ALL;
        int16_t dataLen = remoteIdModule->FillRemoteIdInfo(msg.buffer, sizeof(msg.buffer) - 2);
        if (-1 == dataLen) return;
        AlinkCmdproc::SendAlinkCmd(msg, dataLen, EAP_ALINK_RPT_REMOTEID);
        
        remoteIdModule->OldRemoteIdInfo();
    }
}

void RemoteIdCmdproc::_OnShowRemoteIdModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    _DoShowModuleInfo();
}

int Wifi_CaptureMain_test()
{
	// memcpy(msg_test.buf, test_data, sizeof(test_data));
	SendAsynData(EAP_CMD_REMOTEID, test_data, sizeof(test_data));
    return 0;
}

int rid_checkfilewhetherexist(char *filename)
{
  if (access(filename, 0) == 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
