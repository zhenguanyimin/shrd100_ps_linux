#include "AlinkCmdproc.h"
#include "checksum.h"
#include "SysBase.h"
#include "HalMgr.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

#include "../alink_pb/TracerUavMsg.pb.h"
#include "sfAirBorneSniffer.h"
#include "AirBorneProc.h"

#if 0
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif


Alink_Func AlinkCmdproc::s_alinkFuns[EAP_ALINK_CMD_MAX] = {0};

AlinkCmdproc::AlinkCmdproc() : CmdprocBase()
{
    RegisterAlinkCmd(EAP_ALINK_CMD_TRACER_UAV_MESSAGE, (Alink_Func)&AlinkCmdproc::_ProcUavALinkCmd);
    RegisterAlinkCmd(EAP_ALINK_CMD_SET_RF_ENABLE, (Alink_Func)&AlinkCmdproc::_ProcSetRFEnableALinkCmd);
}

void AlinkCmdproc::_RegisterCmd()
{
    Register(EAP_CMD_ALINK, (Cmdproc_Func)&AlinkCmdproc::_OnAlinkCmd);
}

void AlinkCmdproc::_OnAlinkCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg)
{
    alink_msg_t* msg = (alink_msg_t*)(inMsg.data);

    if (_IsHeaderValid(msg->msg_head))
    {
        uint8_t destId = msg->msg_head.destid;
        if (destId != EapGetSys().GetDeviceType() && destId != 0xFF)
        {
#if 0			        
            EAP_LOG_VERBOSE("sourceid 0x%x!=0x%x,msgid:%hhx\r\n", destId, EapGetSys().GetDeviceType(),(int)msg->msg_head.msgid);
            return;
#endif
        }
        alink_msg_t* responseMsg = (alink_msg_t*)(outMsg.data);
        uint16_t length = _ProcAlinkCmd(msg->msg_head.msgid, (uint8_t *)msg, (uint8_t *)responseMsg);
		if (msg->msg_head.ans == 1 && length > 0)
        {      
        	uint32_t msg_len;
            responseMsg->channelType = msg->channelType;
            responseMsg->channelId   = msg->channelId;
            responseMsg->msg_head.magic     = msg->msg_head.magic;
            responseMsg->msg_head.len_lo    = length & 0xFF;
            responseMsg->msg_head.len_hi    = (length & 0xFF00) >> 8;
            responseMsg->msg_head.destid    = msg->msg_head.sourceid;
            responseMsg->msg_head.sourceid  = msg->msg_head.destid;
            responseMsg->msg_head.msgid     = msg->msg_head.msgid;
            responseMsg->msg_head.ans       = 0;
    		responseMsg->msg_head.checksum = _GetHeaderChecksum(responseMsg->msg_head);			
			FillCRC(*responseMsg, (uint16_t)length); //msg crc is 16bits. append to the end of data.
			msg_len = sizeof(alink_msg_t)-sizeof(msg->buffer) +length + 2; //the 2bytes added is msg crc
            HalMgr* halMgr = EapGetSys().GetHalMgr();
            if (nullptr != halMgr)
            {
                Hal_Data inData = {msg_len, (uint8_t*)responseMsg};
                halMgr->OnSet(EAP_DRVCODE_ALINK_RESPONSE, inData);
            }
        }
        else
        {
            responseMsg->msg_head.len_hi = 0;
            responseMsg->msg_head.len_lo = 0;
        }
    }
    else
    {
        cout << "\n _IsHeaderValid Error!" << "AlinkMsg ID: " << std::hex << msg->msg_head.msgid << std::endl;
    }
}

uint16_t AlinkCmdproc::_ProcAlinkCmd(uint8_t cmd, uint8_t *request, uint8_t *response)
{
    if (cmd >= EAP_ALINK_CMD_MAX) return 0;
    if (nullptr == s_alinkFuns[cmd]) return 0;

    return (*(s_alinkFuns[cmd]))(request, response);
}

void AlinkCmdproc::FillHeader(alink_msg_head_t& header, uint16_t length, uint8_t msgId, uint8_t destId,uint8_t seq,uint8_t ans)
{
    header.magic    = 0xFD;
    header.len_lo   = (length & 0x00FF);
	header.len_hi   = (length & 0xFF00) >> 8;
	header.seq      = seq;
    header.destid   = destId;
    header.sourceid = EapGetSys().GetDeviceType();
    header.msgid    = msgId;
    header.ans      = ans;
    header.checksum = _GetHeaderChecksum(header);
}

uint8_t AlinkCmdproc::_GetHeaderChecksum(const alink_msg_head_t& header)
{
	uint8_t *pkt = (uint8_t *)&header;
	uint8_t checksum = 0;
	uint8_t i = 0;

	for (i = 0; i < ALINK_CORE_HEADER_LEN; i++)
	{
		checksum += pkt[i];
	}

	return checksum;
}

void AlinkCmdproc::FillCRC(alink_msg_t& msg, uint16_t dataLen)
{
    uint8_t* buf = ((uint8_t*)&msg) + 4;
    uint16_t crc = crc_calculate(buf + 1, dataLen + sizeof(alink_msg_head_t) - 1); // 计算校验值不包含帧头和channel，校验从字节1至字节N+8
    msg.buffer[dataLen] = crc & 0xFF;
    msg.buffer[dataLen + 1] = (crc & 0xFF00) >> 8;
}

bool AlinkCmdproc::_IsHeaderValid(const alink_msg_head_t& header)
{
	bool ret = true;
	uint32_t len_msg_payload = 0;
	uint8_t checksum = 0;

    do
	{
        // check the magic
        if (header.magic != ALINK_MSG_MAGIC_VAL)
        {
            cout << "\n header.magic Error: " << (uint16_t)header.magic;
            EAP_LOG_VERBOSE("0x%x!=0x%x\r\n", header.magic, ALINK_MSG_MAGIC_VAL);
            ret = false;
            break;
        }

        // check the length
        len_msg_payload = (header.len_hi << 8) + header.len_lo;
        if (len_msg_payload > ALINK_MAX_PAYLOAD_LEN)
        {
            cout << "\n len_msg_payload Error: " << len_msg_payload;
            EAP_LOG_VERBOSE("%u > %u\r\n", len_msg_payload, ALINK_MAX_PAYLOAD_LEN);
            ret = false;
            break;
        }

        // check the checksum
        checksum = _GetHeaderChecksum(header);
        if (header.checksum != checksum)
        {
            cout << "\n checksum Error: " << (uint16_t)header.checksum;
            ret = false;
            break;
        }
    } while(0);

	return ret;
}

void AlinkCmdproc::RegisterAlinkCmd(uint16_t cmd, Alink_Func func)
{
    if (cmd >= EAP_ALINK_CMD_MAX) return;
    s_alinkFuns[cmd] = func;
}

void AlinkCmdproc::ShowRegisterInfo()
{
    cout << hex;
    cout << "\n" << "AlinkCmdMap: ";
    cout << "\n" << "  No.  | Code                                            | Func";
    for (int i = 0; i < EAP_ALINK_CMD_MAX; ++i)
    {
        cout << "\n  " << right << setw(4) << i;
        cout << " | " << left << setw(47) << _ConvertStr(i);
        cout << " | " << right << setw(14) << reinterpret_cast<void*>(s_alinkFuns[i]);
    }
}

string AlinkCmdproc::_ConvertStr(uint16_t drvCode)
{
    EAP_GET_ALINKCMD_STR(drvCode);
    return "unknown";
}

void AlinkCmdproc::SendAlinkCmd(alink_msg_t &msg, int16_t dataLen, uint8_t alinkCmd)
{
    if (dataLen > ALINK_MAX_PAYLOAD_LEN) return;

    if (alinkCmd != EAP_ALINK_RPT_TRACER_UAV_MESSAGE)  // 调试接口联调稳定后再删除
    {
        FillHeader(msg.msg_head, (uint16_t)dataLen, alinkCmd, DEV_C2,0,0);
    }
    else
    {
        FillHeader(msg.msg_head, (uint16_t)dataLen, alinkCmd, DEV_UAV,0,0);
    }

    FillCRC(msg, (uint16_t)dataLen);
    Hal_Data inData = {sizeof(alink_msg_head_t) + dataLen + 6, (uint8_t*)&msg};

    if (alinkCmd == EAP_ALINK_RPT_TRACER_UAV_MESSAGE)  // TBD... 调试接口联调稳定后再删除
    {
        msg.msg_head.destid = DEV_UAV;
        /*
        std::ostringstream out;
        for (uint32_t pos = 0; pos < inData.length; pos++)
        {
            out << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(inData.buf[pos]);
        }
        std::cout << "AlinkCmdproc::SendAlinkCmd dataLen = " << std::dec << dataLen << ", " << out.str() << std::endl;
        */
    }

    HalMgr *halMgr = EapGetSys().GetHalMgr();
    if (nullptr != halMgr) halMgr->OnSet(EAP_DRVCODE_RPT_INFO_BY_ALINK, inData);
}

// Parameters request and response actual type is alink_msg_t* 
std::uint16_t AlinkCmdproc::_ProcUavALinkCmd(std::uint8_t *request, std::uint8_t *)
{
    std::uint16_t ret = -1;
    alink_msg_t *alink_msg = (alink_msg_t *)request;

    if (nullptr != request)
    {
        std::uint8_t *data = alink_msg->buffer;
        std::uint32_t dataLength = (static_cast<std::uint16_t>(alink_msg->msg_head.len_hi) << 8) | alink_msg->msg_head.len_lo;

        protobuf::traceruav::TracerUavMessagePB uavMsgPb;
        bool isParseOK = uavMsgPb.ParseFromArray(data, dataLength);

        // TBD... 联调暂时加打印
        std::ostringstream out;
        for (std::uint32_t pos = 0; pos < dataLength; pos++)
        {
            out << " " << std::hex << std::setw(2) << std::setfill('0') << (std::uint16_t)(data[pos]);
        }
        std::cout << "AlinkCmdproc::_ProcUavALinkCmd ParseFromArray : " << std::boolalpha << isParseOK << ", " << out.str() << std::endl;

        if (false == isParseOK)
        {
            std::ostringstream out;

            for (std::uint32_t pos = 0; pos < dataLength; pos++)
            {
                out << " " << std::hex << std::setw(2) << std::setfill('0') << (std::uint16_t)(data[pos]);
            }

            std::cout << "ParseFromArray : " << std::boolalpha << isParseOK << ", " << out.str() << std::endl;
        }
        else
        {
            std::cout << "AlinkCmdproc::_ProcUavALinkCmd uavMsgPb.msgtype() = " << uavMsgPb.msgtype() << std::endl;

            if (uavMsgPb.msgtype() == protobuf::traceruav::TRACERUAVMSG_FLY_STATUS_TYPE)
            {
                if (uavMsgPb.has_flystatus())
                {
                    const protobuf::traceruav::FlyStatusPB& flyStatusData = uavMsgPb.flystatus();

                    const std::uint32_t maxLength = 192;
                    std::uint8_t buffer[maxLength] = {0};
                    const std::uint32_t dataLength = flyStatusData.ByteSizeLong();

                    // std::cout << "dataLength = " << std::dec << dataLength << ", " << std::hex << flyStatusData.yaw() << ", " << flyStatusData.vz() << std::endl;

                    if (dataLength <= maxLength)
                    {
                        if (false == flyStatusData.SerializeToArray(buffer, dataLength))
                        {
                            EAP_LOG_ERROR("FlyStatusPB is failed to SerializeToArray, size = %d\n", dataLength);
                        }
                        else
                        {
                            // save one copy to HAL
                            HalMgr *halMgr = EapGetSys().GetHalMgr();
                            if (nullptr != halMgr)
                            {
                                Hal_Data inData = {dataLength, buffer};
                                halMgr->OnSet(EAP_DRVCODE_SET_UAV_STATUS, inData);
                            }
                        }
                    }
                }
            }
            else if (uavMsgPb.msgtype() == protobuf::traceruav::TRACERUAVMSG_WORK_MODE_TYPE)
            {
                if (uavMsgPb.has_workmode())
                {
                    const protobuf::traceruav::CtrlWorkModePB& workModeData = uavMsgPb.workmode();

                    ioController_t ctrl_state;
                    ctrl_state.workMode = workModeData.workmode();
                    ctrl_state.dxNum    = workModeData.dxnum();
                    ctrl_state.dxFreq   = workModeData.dxfreq();
                    std::cout << "ctrl_state.workMode = " << ctrl_state.workMode
                              << ", ctrl_state.dxNum = " << ctrl_state.dxNum
                              << ", ctrl_state.dxFreq = " << ctrl_state.dxFreq
                              << std::endl;

                    const std::uint32_t maxLength = 192;
                    std::uint8_t buffer[maxLength] = {0};

                    if ((ARBN_SET_WIFI_ENABLE != ctrl_state.workMode)
                        && (ARBN_SET_WIFI_DISABLE != ctrl_state.workMode))
                    {
                        const std::size_t dataLength = sizeof(ioController_t);
                        memcpy(buffer, &ctrl_state, dataLength);

                        SendAsynData(EAP_CMD_AIRBORNE_STATE_CTRL, buffer, dataLength);
                    }
                    else
                    {
                        HalMgr *halMgr = EapGetSys().GetHalMgr();
                        if (nullptr != halMgr)
                        {
                            buffer[0] = (ARBN_SET_WIFI_ENABLE == ctrl_state.workMode) ? 1 : 0;
                            Hal_Data inData = {1, buffer};
                            halMgr->OnSet(EAP_DRVCODE_SET_WIFI_STATUS, inData);
                        }
                    }
                }
            }
            else if (uavMsgPb.msgtype() == protobuf::traceruav::TRACERUAVMSG_MEASURE_INFO_TYPE)
            {
                if (uavMsgPb.has_measureinfo())
                {
                    const protobuf::traceruav::MeasureInfoPb& meaData = uavMsgPb.measureinfo();
                    const std::uint32_t dataSize = meaData.meainfodata_size();
                    out << "meaData.workstatus() = " << meaData.workstatus()
                        << ", meaData.tarnum() = " << meaData.tarnum()
                        << ", info data size = " << dataSize;

                    // TBD... 调试接口联调稳定后再删除
                    std::ostringstream out;
                    for (const auto& minfoData : meaData.meainfodata())
                    {
                        out << "tnum: " << std::dec << minfoData.tnum()
                            << "tfreq: " << minfoData.tfreq()
                            << "tamp: " << minfoData.tamp()
                            << "tmfw: " << minfoData.tmfw()
                            << "tmfy: " << minfoData.tmfy()
                            << "tffw: " << minfoData.tffw()
                            << "tffy: " << minfoData.tffy();
                    }
                    std::cout << out.str() << std::endl;
                }
            }
            else
            {
                // do nothing
            }
        }
    }

    return ret;
}

uint16_t AlinkCmdproc::_ProcSetRFEnableALinkCmd(uint8_t *request, uint8_t *response)
{
    if ((nullptr != request) && (nullptr != response))
    {
        alink_msg_t *request_msg = (alink_msg_t *)request;
        alink_msg_t *response_msg = (alink_msg_t *)response;

        std::uint8_t inBuffer[2] = {0};
        inBuffer[0] = request_msg->buffer[0];

        response_msg->buffer[0] = 0;  // result 0: 失败, 1:成功
        response_msg->buffer[1] = inBuffer[0];

        HalMgr *halMgr = EapGetSys().GetHalMgr();
        if (nullptr != halMgr)
        {
            Hal_Data inData = {1, inBuffer};
            if (EAP_SUCCESS == halMgr->OnSet(EAP_DRVCODE_SET_RF_ENABLE, inData))
            {
                response_msg->buffer[0] = 1;
            }

            std::uint8_t outBuffer[2] = {0};
            Hal_Data outData = {1, outBuffer};

            halMgr->OnGet(EAP_DRVCODE_GET_RF_ENABLE, inData, outData);
            response_msg->buffer[1] = outData.buf[0];
        }
    }

	return 2;  // response message length
}


extern "C"
{
    void register_alink_cmd(uint16_t cmd, Alink_Func func)
    {
        AlinkCmdproc::RegisterAlinkCmd(cmd, func);
    }

    void ShowAlinkCmdRegisterInfo()
    {
        AlinkCmdproc::ShowRegisterInfo();
    }

	void alink_msg_fill_crc(void * alink_msg, uint16_t dataLen)
	{
		alink_msg_t * msg = (alink_msg_t *)alink_msg;
		AlinkCmdproc::FillCRC(*msg, dataLen);
	}

	void alink_msg_fill_header(void *msg_header, uint16_t length, uint8_t msgId, uint8_t destId,uint8_t seq,uint8_t ans)
	{

		alink_msg_head_t * h = (alink_msg_head_t *)msg_header;
		AlinkCmdproc::FillHeader(*h,length,msgId,destId,seq,ans);
	}
	
}
