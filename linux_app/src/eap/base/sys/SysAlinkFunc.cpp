#include "SysBase.h"
#include "AlinkCmdproc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include "../../include/module/VehicleMountedModule.h"
#include "../../include/cmdproc/VehicleMountedCmdproc.h"
#include "HalMgr.h"
#include "eap_pub.h"
#include "WhitelistManager.h"

using namespace std;

SysBase& EapGetSys();

extern "C"
{

#if 0
#define DEBUG_SYSALINKFUN_PRINTF(arg...) printf(arg)
#else
#define DEBUG_SYSALINKFUN_PRINTF(arg...) 
#endif



#include "FreeRTOS_CLI.h"

uint16_t Alink_GetSoftwareVersion_Func(uint8_t *request, uint8_t *response)
{
    alink_msg_t *reponse_msg = (alink_msg_t *)response;
	AlinkDevInfoResp *data = (AlinkDevInfoResp *)reponse_msg->buffer;
    
    uint16_t dataLen = strlen(EAP_EMBED_SOFTWARE_PS_VERSION_STR);
    memcpy(reponse_msg->buffer, EAP_EMBED_SOFTWARE_PS_VERSION_STR, dataLen);
	return dataLen;
}

uint16_t Alink_SystemReset_Func(uint8_t *request, uint8_t *response)
{
    return 0;
}

uint16_t Alink_GetBoardSN_Func(uint8_t *request, uint8_t *response)
{
    alink_msg_t *request_msg = (alink_msg_t *)request;
    alink_msg_t *response_msg = (alink_msg_t *)response;
	uint8_t len = 0;
    uint8_t boardSN[EAP_SN_LENGTH] = {0};
    uint32_t respDataLen = EAP_SN_LENGTH;
    SendSynData(EAP_THREAD_ID_ALINK, EAP_CMD_GET_SN_CODE, nullptr, 0, boardSN, &respDataLen);
    if (0 == respDataLen || respDataLen > EAP_SN_LENGTH)
	{
		response_msg->buffer[0] = 0;
		len = 1;
	}
	else
	{
		memcpy(response_msg->buffer, boardSN, respDataLen);
		len = respDataLen;
	}
	return len;
}

uint16_t Alink_SetBoardSN_Func(uint8_t *request, uint8_t *response)
{
    alink_msg_t *request_msg = (alink_msg_t *)request;
    alink_msg_t *response_msg = (alink_msg_t *)response;
	uint32_t len = (request_msg->msg_head.len_hi << 8) | request_msg->msg_head.len_lo;
	if ((len <= 1) || (len > EAP_SN_LENGTH) )
	{
		response_msg->buffer[0] = 0;
		return 1;
	}
    SendAsynData(EAP_CMD_SET_SN_CODE, (uint8_t*)request_msg->buffer, len);
	response_msg->buffer[0] = 1;
	return 1;
}

uint16_t Alink_Get_Log_List_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *reponse_msg = (alink_msg_t *)response;
	uint32_t pkg_total_num = 0x00;
	uint32_t pkg_cur_num = 0x00;
	FILE *fp;
	uint32_t buffer_len = 0;
    char command[16];
    char line[EAP_MAX_RESULT_SIZE];
    uint32_t file_num = 0;
	bool is_subcontract = false;
	sprintf(command, "ls -l /media/sd-mmcblk0p1");
	AlinkLogListResp* data = (AlinkLogListResp*)reponse_msg->buffer;
	// 执行命令并读取输出结果
    fp = popen(command, "r");
    if (fp == NULL) 
	{
        printf("Failed to run command %s\n",command);
        return -1;
    }

    // 解析输出结果
    while (fgets(line, sizeof(line), fp) != NULL) 
	{
        // 去除换行符
        line[strcspn(line, "\n")] = '\0';

        // 解析每行结果
        char *token = strtok(line, " "); // 使用空格作为分隔符
        int count = 0;
		string file_name_str;
		string len_str;
		
        while (token != NULL) 
		{
			if(count == 4)
			{
				//第四个字段是文件长度
				len_str = token;
			}
            else if (count == 8) 
			{
                // 第8个字段是文件名    debug error info data 
				file_name_str = token;
				// if(strcmp(file_name,"debug_") == 0 || strcmp(file_name,"data__") == 0 
				// 	|| strcmp(file_name,"info__") == 0 || strcmp(file_name,"error_") == 0)
				if(file_name_str.find("debug_") != std::string::npos || file_name_str.find("data__") != std::string::npos
				   || file_name_str.find("info__") != std::string::npos || file_name_str.find("error_") != std::string::npos)
				{
					
					data->Log_data_len = atoi(len_str.c_str());
					memcpy(data->Log_name,token,std::min<uint32_t>(strlen(token),EAP_LOG_FILE_NAME_LEN));
					data->Log_name_len = strlen(token);
					data++;
					file_num++;
					buffer_len = sizeof(data->Log_data_len) + sizeof(data->Log_name_len) + strlen(data->Log_name);
				}
            }
            token = strtok(NULL, " ");
            count++;
        }
		if((file_num > 0 && file_num%EAP_PACKAGE_MAX_NUM == 0x00)) //超过30个文件，进行分包发送
		{
			pkg_total_num = file_num/EAP_PACKAGE_MAX_NUM;
			memcpy(&reponse_msg->buffer[0],&pkg_total_num,sizeof(uint32_t));
			memcpy(&reponse_msg->buffer[4],&pkg_cur_num,sizeof(uint32_t));
			pkg_cur_num++;
			file_num -= EAP_PACKAGE_MAX_NUM;
			data = (AlinkLogListResp*)reponse_msg->buffer;
			is_subcontract = true;
			buffer_len = sizeof(pkg_total_num) + sizeof(pkg_cur_num);
			AlinkCmdproc::SendAlinkCmd(*reponse_msg, buffer_len, EAP_ALINK_CMD_C2_GET_LOG_LIST);
			eap_thread_delay(5);	
			memset(reponse_msg->buffer,0,EAP_ALINK_MSG_MAX_BUFF);
		}
    }

	if(file_num < 30)
	{
		if(!is_subcontract)
		{
			pkg_total_num = 1;
			pkg_cur_num = 0;
		}
		memcpy(&reponse_msg->buffer[0],&pkg_total_num,sizeof(uint32_t));
		memcpy(&reponse_msg->buffer[4],&pkg_cur_num,sizeof(uint32_t));
		buffer_len = sizeof(pkg_total_num) + sizeof(pkg_cur_num);
		AlinkCmdproc::SendAlinkCmd(*reponse_msg, buffer_len, EAP_ALINK_CMD_C2_GET_LOG_LIST);
	}

    pclose(fp);
    return 1;
	//return sizeof(reponse_msg->buffer);
}

uint16_t Alink_Get_Log_Data_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t* request_msg = (alink_msg_t *)request;
	alink_msg_t* reponse_msg = (alink_msg_t *)response;
	uint32_t index = 0x00 ;
	bool is_subcontract = false;
	uint32_t buffer_len = 0;
	uint32_t log_id,log_name_len,Pkg_total_num,Pkg_cur_num;
	uint32_t log_num = (uint32_t)request_msg->buffer[index]
				+ ((uint32_t)request_msg->buffer[index+1] << 8)
				+ ((uint32_t)request_msg->buffer[index+2] << 16)
				+ ((uint32_t)request_msg->buffer[index+3] << 24);
	uint8_t* p_request = request_msg->buffer;
	p_request += 4;
	for(int num = 0; num < log_num; ++num)
	{
		AlinkLogReq* request_data = (AlinkLogReq*)p_request;
		log_id = request_data->Log_id;
		log_name_len = request_data->Log_name_len;
		string file_path = (char*)request_data->Log_name;
		file_path += "/media/sd-mmcblk0p1";
		struct stat st;
		stat(file_path.c_str(), &st);
		uint32_t file_len = st.st_size;
		AlnikLogResp* response_data = (AlnikLogResp*)reponse_msg->buffer;
		if(file_len > 0  && file_len%EAP_LOG_FILE_LEN_MAX ==0) 
		{
			//超长数据分包发送
			Pkg_total_num = file_len/EAP_LOG_FILE_LEN_MAX;
			for(int sub_package_num = 0; sub_package_num < file_len/EAP_LOG_FILE_LEN_MAX; ++sub_package_num)
			{
				response_data->Log_id = request_data->Log_id;
				response_data->Pkg_total_num = Pkg_total_num;
				response_data->Pkg_cur_num = Pkg_cur_num++;
				response_data->Data_len = EAP_LOG_FILE_LEN_MAX;
				response_data++;
				FILE *p_file = fopen(file_path.c_str(), "rb");
				if (p_file != NULL) 
				{
					//char buffer[EAP_LOG_FILE_LEN_MAX] = {0}; // 用于存储读取的内容的缓冲区
					uint32_t bytesRead = fread(response_data, sizeof(char), EAP_LOG_FILE_LEN_MAX, p_file);
					if(bytesRead == file_len)
					{
						//发送响应
						buffer_len = sizeof(response_data->Log_id) + sizeof(response_data->Pkg_total_num) + sizeof(response_data->Pkg_cur_num) 
										+ response_data->Data_len;
						AlinkCmdproc::SendAlinkCmd(*reponse_msg, buffer_len, EAP_ALINK_CMD_C2_GET_LOG);
						eap_thread_delay(5);				
					}
				}
				memset(reponse_msg->buffer,0,EAP_ALINK_MSG_MAX_BUFF);	
				response_data = (AlnikLogResp*)reponse_msg->buffer;			
			}
			continue;
		}
		if(!is_subcontract)
		{
			response_data->Log_id = request_data->Log_id;
			response_data->Pkg_total_num = 1;
			response_data->Pkg_cur_num = 0;
			response_data->Data_len = file_len;
			response_data++;
			FILE *p_file = fopen(file_path.c_str(), "rb");
			if (p_file != NULL) 
			{
				//char buffer[EAP_LOG_FILE_LEN_MAX] = {0}; // 用于存储读取的内容的缓冲区
				uint32_t bytesRead = fread(response_data, sizeof(char), file_len, p_file);
				if(bytesRead == file_len)
				{
					//发送响应
					buffer_len = sizeof(response_data->Log_id) + sizeof(response_data->Pkg_total_num) + sizeof(response_data->Pkg_cur_num) 
										+ response_data->Data_len;
						AlinkCmdproc::SendAlinkCmd(*reponse_msg, buffer_len, EAP_ALINK_CMD_C2_GET_LOG);					
				}
				memset(reponse_msg->buffer,0,EAP_ALINK_MSG_MAX_BUFF);	
				response_data = (AlnikLogResp*)reponse_msg->buffer;	
			}
		}	
	}
    return 1;
}

uint16_t Alink_Delete_Log_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t* reponse_msg = (alink_msg_t *)response;
	system("rm /media/sd-mmcblk0p1/debug_* /media/sd-mmcblk0p1/info__* /media/sd-mmcblk0p1/data__* /media/sd-mmcblk0p1/error_");
	reponse_msg->buffer[0] = 1;
    return 1;
}

uint16_t Alink_SetOrientationWorkMode_Func(uint8_t *request, uint8_t *response)
{
    alink_msg_t *request_msg = (alink_msg_t *)request;
    alink_msg_t *response_msg = (alink_msg_t *)response;
	uint8_t is_enter_orientation = request_msg->buffer[0];
	uint8_t work_mode = WORKMODE_SPECTRUM_OMNI;

	SendAsynData(EAP_CMD_SET_WORK_MODE, (uint8_t*)&is_enter_orientation, 1);
	response_msg->buffer[0] = 1;
	if(is_enter_orientation)
	{
		work_mode = WORKMODE_SPECTRUM_OMNI_ORIE;
	}
	else
	{
		work_mode = WORKMODE_SPECTRUM_OMNI;
	}
	EapGetSys().SetWorkMode(work_mode);
	EAP_LOG_DEBUG("work_mode = %d(0:DRONEID_MODE,1:SPECTRUM1_MODE,2:SPECTRUM2_MODE)\r\n",work_mode);

	return 1;
}

uint16_t Alink_DeviceStatusInfo_Func(uint8_t *request, uint8_t *response)
{
    alink_msg_t *request_msg = (alink_msg_t *)request;
    alink_msg_t *response_msg = (alink_msg_t *)response;
	AlinkDupBroadcastReq *data = (AlinkDupBroadcastReq *)request_msg->buffer;

	// printf("\n Alink_DeviceStatusInfo_Func %x\n", data->protocol);
	//EapGetSys().SetDupInfo(data->protocol);
	//printf("data->protocol is %d\n",data->protocol);
	EapGetSys().DupResponse();

	response_msg->buffer[0] = 1;
	return 1;
}

uint16_t Alink_GetDevInfo_Func(uint8_t *request, uint8_t *response)
{
    alink_msg_t *reponse_msg = (alink_msg_t *)response;
	AlinkDevInfoResp *data = (AlinkDevInfoResp *)reponse_msg->buffer;
    char str[32];

    memcpy(data->Company_name, EAP_EMBED_COMPANY_NAME, min(strlen(EAP_EMBED_COMPANY_NAME), sizeof(data->Company_name)));
    memcpy(data->Device_name, EAP_EMBED_DEVICE_NAME, min(strlen(EAP_EMBED_DEVICE_NAME), sizeof(data->Device_name)));
    memcpy(data->PS_version, EAP_EMBED_SOFTWARE_PS_VERSION_STR, min(strlen(EAP_EMBED_SOFTWARE_PS_VERSION_STR), sizeof(data->PS_version)));
    uint64_t plVersion = ((uint64_t)EapGetSys().GetPlVersionDate() << 32) | (EapGetSys().GetPlVersionTime());
    sprintf(str, "%d", plVersion);
    memcpy(data->PL_version, str, min(strlen(str), sizeof(data->PL_version)));
    memcpy(data->Device_sn, EapGetSys().GetSnName(), min((int)EapGetSys().GetSnNameLen(), (int)sizeof(data->Device_sn)));
    memcpy(data->Device_ip, EapGetSys().GetRemoteAddr(), min((int)EAP_REMOTE_ADDR_LEN, (int)sizeof(data->Device_ip)));
	return sizeof(AlinkDevInfoResp);
}

uint16_t Alink_GetWorkMode_Func(uint8_t *request, uint8_t *response)
{
	 alink_msg_t *response_msg = (alink_msg_t *)response;
	 uint8_t work_mode = EapGetSys().GetWorkMode();
	 if(work_mode == WORKMODE_SPECTRUM_OMNI_ORIE)
	 {
		response_msg->buffer[0] = 1;
	 }
	 else if(work_mode == WORKMODE_SPECTRUM_OMNI)
	 {
		response_msg->buffer[0] = 0;
	 }
	return 1;
}

uint16_t Alink_C2HeartBeatPacket_Func(uint8_t *request, uint8_t *response)
{
	//SendAsynData(EAP_CMD_UPDATE_C2_HEARTBEAT_TIME, 0, 0);
	alink_msg_t *request_msg = (alink_msg_t *)request;
	SendAsynData(EAP_CMD_UPDATE_C2_HEARTBEAT_TIME, (uint8_t *)request_msg, ALINK_MSG_T_HEADER_SIZE);
	//EAP_LOG_DEBUG("recv Alink_C2HeartBeatPacket_Func value %d %s:%d\n",request_msg->buffer[4],__FILE__,__LINE__);
	return 1;
}

uint16_t Alink_GetC2TimeInfo_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *request_msg = (alink_msg_t *)request;
	char timeBuffer[20] = {0};
	memcpy(timeBuffer,request_msg->buffer,sizeof(timeBuffer));
	std::string timeStr = timeBuffer;
    std::string year = timeStr.substr(0, 4);
    std::string month = timeStr.substr(4, 2);
    std::string day = timeStr.substr(6, 2);
    std::string hour = timeStr.substr(8, 2);
    std::string minute = timeStr.substr(10, 2);
    std::string second = timeStr.substr(12, 2);
    
	static int32_t time = 0;
	if(time == 0)
	{
		char timeCmd[52] = {0};
		sprintf(timeCmd,"date -s \"%s-%s-%s %s:%s:%s\"",year.c_str(),month.c_str(),day.c_str(),hour.c_str(),minute.c_str(),second.c_str());
		system(timeCmd);
		//EAP_LOG_DEBUG("recv Alink_GetC2TimeInfo_Func response time info is %s fromat timestamp is %s",timeBuffer,timeCmd);
		time++;
	}
	return 1;
}

uint16_t Alink_GetSelfInspectInfo_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *response_msg = (alink_msg_t *)response;
	SelfInspectInfoResp* selfInspectInfo = (SelfInspectInfoResp *)response_msg->buffer;

	HalMgr* halMgr = EapGetSys().GetHalMgr();
	Hal_Data inData = {0, nullptr};
    Drv_HardWareInfo hard_ware_info = {0};
    Hal_Data outData = {sizeof(Drv_HardWareInfo), (uint8_t*)&hard_ware_info};
    if (nullptr != halMgr && EAP_SUCCESS != EapGetSys().GetHalMgr()->OnGet(EAP_DRVCODE_GET_DEVICE_HARDWARE_INFO, inData, outData))
	{
		EAP_LOG_ERROR("get device hardware info error!");
		return 0;
	}
	
	selfInspectInfo->version = EapGetSys().GetDupProtocol();
	selfInspectInfo->tempFpga = hard_ware_info.tempFpga;
	selfInspectInfo->tempBatBoard1 = hard_ware_info.tempBatBoard1;
	selfInspectInfo->tempBatBoard2 = hard_ware_info.tempBatBoard2;
	selfInspectInfo->batteryCap = hard_ware_info.batteryCap;
	selfInspectInfo->workMode = EapGetSys().GetWorkMode();
	selfInspectInfo->digitComStatus = hard_ware_info.digitComStatus;
	selfInspectInfo->fanRateStatus = hard_ware_info.fanRateStatus;
	selfInspectInfo->fanRate = hard_ware_info.fanRate;
	selfInspectInfo->tcDevStatus = hard_ware_info.tcDevStatus;
	selfInspectInfo->tcConnectStatus = hard_ware_info.tcConnectStatus;
	selfInspectInfo->tcFrequence = hard_ware_info.tcFrequence;
	selfInspectInfo->wifiRemoteIdDevStatus = hard_ware_info.wifiRemoteIdDevStatus;
	selfInspectInfo->wifiRemoteIdConnectStatus = hard_ware_info.wifiRemoteIdConnectStatus;
	selfInspectInfo->wifiNetDevStatus = hard_ware_info.wifiNetDevStatus;
	selfInspectInfo->wifiNetConnectStatus = hard_ware_info.wifiNetConnectStatus;
	return sizeof(SelfInspectInfoResp);
}

//huaguoqing add,2023.09.05
uint16_t Alink_C2CliCmd_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	BaseType_t ret;	
	
	alink_cli_cmd_t     *cmd_msg = (alink_cli_cmd_t *)req->buffer;
	alink_cli_cmd_ack_t *ack = (alink_cli_cmd_ack_t *)rsp->buffer;
	alink_msg_head_t  *h = &req->msg_head;

	uint32_t payload_len = ((uint32_t)h->len_hi) <<8 | (uint32_t)h->len_lo;
	int cmd_str_len = payload_len - sizeof(alink_cli_cmd_t);
	int output_len = 0;
	uint16_t ret_len;
	//eap_print_memory(&req->msg_head, sizeof(req->msg_head)+payload_len);
	if(cmd_str_len == 0)
		return 0;
	DEBUG_SYSALINKFUN_PRINTF("%s[%d] enter\n", __FUNCTION__, __LINE__);
	if(cmd_msg->cmd[cmd_str_len - 1] != 0){
		ack->cmd_parse_retcode = -1;
		strcpy(ack->cmd_exec_output,"cmd_invalid");
	}else{
		memset((void*)ack, 0, sizeof(*ack)+1);
		DEBUG_SYSALINKFUN_PRINTF("%s[%d] cmd:%s\n", __FUNCTION__, __LINE__, cmd_msg->cmd);
    	ret = FreeRTOS_CLIProcessCommand(cmd_msg->cmd, ack->cmd_exec_output, sizeof(rsp->buffer) - sizeof(alink_cli_cmd_ack_t));
		DEBUG_SYSALINKFUN_PRINTF("%s[%d] \n", __FUNCTION__, __LINE__);

		if(ret == pdFALSE){
			ack->cmd_parse_retcode = -2;
			ack->cmd_exec_retcode = -1;
		}else{
			ack->cmd_parse_retcode = 0;
			ack->cmd_exec_retcode = 0;
		}
	}
	ret_len = sizeof(alink_cli_cmd_ack_t) + strlen(ack->cmd_exec_output)+1;
	DEBUG_SYSALINKFUN_PRINTF("%s[%d] exit,cmd_parse_retcode:%d,cmd_exec_retcode:%d,ret_len:%d,output:%s \n", __FUNCTION__, __LINE__
			,ack->cmd_parse_retcode,ack->cmd_exec_retcode,ret_len,ack->cmd_exec_output);
	return ret_len;
}



void dump_alink_msg(alink_msg_t *msg)
{
	if(msg){
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->channelType = %d\n", __func__, __LINE__, msg->channelType);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->channelId = %d\n", __func__, __LINE__, msg->channelId);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.magic = %d\n", __func__, __LINE__, msg->msg_head.magic);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.len_lo = %d\n", __func__, __LINE__, msg->msg_head.len_lo);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.len_hi = %d\n", __func__, __LINE__, msg->msg_head.len_hi);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.seq = %d\n", __func__, __LINE__, msg->msg_head.seq);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.destid = %d\n", __func__, __LINE__, msg->msg_head.destid);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.sourceid = %d\n", __func__, __LINE__, msg->msg_head.sourceid);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.msgid = 0x%x\n", __func__, __LINE__, msg->msg_head.msgid);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.ans = 0x%x\n", __func__, __LINE__, msg->msg_head.ans);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.checksum = 0x%x\n", __func__, __LINE__, msg->msg_head.checksum);
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d]msg->msg_head.payload = %s\n", __func__, __LINE__, msg->msg_head.payload);
	}
	return;
}



uint16_t Alink_C2BlueToothCmd_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	int data_length = 0;

	dump_alink_msg(req);

	data_length = (req->msg_head.len_hi << 8) + req->msg_head.len_lo;

	if(!data_length){
		rsp->buffer[0] = -1;
	}else{
		SendAsynData(EAP_CMD_SET_WIFI_NAME_AND_SECRET, (uint8_t *)req, ALINK_MSG_T_HEADER_SIZE + ALINK_MSG_BD_PRO_PAYLOAD_SIZE);
		rsp->buffer[0] = 0;
	}
	
	return 1;
}

// 2023-11-10 拨码开关由4档改为2档并由C2控制，且仅当物理档位非0-NONE时C2设置才生效，在DeviceHal实现
uint16_t Alink_SetAlarmMode_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	uint8_t alarmMode = req->buffer[0];

	dump_alink_msg(req);

	HalMgr* halMgr = EapGetSys().GetHalMgr();
	if(nullptr != halMgr && alarmMode < EAP_WARNING_LEVEL_MAX)
	{
		EAP_LOG_DEBUG("Set alarmMode=%d\n", alarmMode);
		Hal_Data inData = {sizeof(uint8_t), &alarmMode};
		halMgr->OnSet(EAP_DRVCODE_SET_ALARM_MODE, inData);
		rsp->buffer[0] = 1;
	}
	else
		rsp->buffer[0] = 0;

	return 1;
}

uint16_t Alink_SetCovertMode_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	uint8_t covertMode = req->buffer[0];

	dump_alink_msg(req);

	HalMgr* halMgr = EapGetSys().GetHalMgr();
	if(nullptr != halMgr && covertMode < 2)
	{
		EAP_LOG_DEBUG("Set covertMode=%d\n", covertMode);
		Hal_Data inData = {sizeof(uint8_t), &covertMode};
		halMgr->OnSet(EAP_DRVCODE_SET_COVERT_MODE, inData);
		SendAsynData(EAP_CMD_SET_COVERT_MODE, &covertMode, 1);
		rsp->buffer[0] = 1;
	}
	else
		rsp->buffer[0] = 0;

	return 1;
}


/*for tracers shrd106l */
extern VehicleMountedModule* GetVehicleMountedModuler();
uint16_t Alink_C2SetFiredMode_Func(uint8_t *request, uint8_t *response)
{
	VehicleMountedModule *pvehMod = NULL;
	VehicleMountedCmdproc *pvehCmd = NULL;
	uint8_t firemode = 0;
	
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	int data_length = 0;

	dump_alink_msg(req);

	data_length = (req->msg_head.len_hi << 8) + req->msg_head.len_lo;

	if(!data_length || (EAP_ALINK_CMD_C2_SET_FIRED_MODE != req->msg_head.msgid)){
		rsp->buffer[0] = 0;
	}else{
	
		pvehMod = GetVehicleMountedModuler();
		if(!pvehMod){
			EAP_LOG_ERROR("err,  pvehMod is NULL!\n");
		}
		
		pvehCmd = (VehicleMountedCmdproc *)pvehMod->getCmdProc();
		if(!pvehCmd){
			EAP_LOG_ERROR("err,  pvehCmd is NULL!\n");
		}

		firemode = req->buffer[0];
		
		pvehCmd->VehicleMountedSetFiredMode(&firemode);
		
		rsp->buffer[0] = 1;
	}
	
	return 1;
}

uint16_t Alink_C2GetFiredMode_Func(uint8_t *request, uint8_t *response)
{
	VehicleMountedModule *pvehMod = NULL;
	VehicleMountedCmdproc *pvehCmd = NULL;
	uint8_t firemode = 0;
	
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	uint32_t data_length = sizeof(uint8_t) + ALINK_MSG_T_HEADER_SIZE;
	
	dump_alink_msg(req);


	if(EAP_ALINK_CMD_C2_GET_FIRED_MODE != req->msg_head.msgid){
		rsp->buffer[0] = 0;
	}else{
		pvehMod = GetVehicleMountedModuler();
		if(!pvehMod){
			EAP_LOG_ERROR("err,  pvehMod is NULL!\n");
		}

		pvehCmd = (VehicleMountedCmdproc *)pvehMod->getCmdProc();
		if(!pvehCmd){
			EAP_LOG_ERROR("err,  pvehCmd is NULL!\n");
		}
		
		pvehCmd->VehicleMountedGetFiredMode(&firemode);

		rsp->buffer[0] = firemode;
		DEBUG_SYSALINKFUN_PRINTF("[%s, %d] mode = %d\n", __func__, __LINE__, rsp->buffer[0]);
	}
	
	return 1;
}


uint16_t Alink_C2StartFiredALL_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;

	dump_alink_msg(req);


	if(EAP_ALINK_CMD_C2_START_FIRED_ALL != req->msg_head.msgid){
		rsp->buffer[0] = 0;
	}else{
		SendAsynData(EAP_CMD_SET_VEHICLE_MOUNTED_START_FIRED_ALL, NULL, 0);
		rsp->buffer[0] = 1;
	}
	
	return 1;
}
	
uint16_t Alink_C2StopFiredALL_Func(uint8_t *request, uint8_t *response)
{
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;

	dump_alink_msg(req);


	if(EAP_ALINK_CMD_C2_STOP_FIRED_ALL != req->msg_head.msgid){
		rsp->buffer[0] = 0;
	}else{
		SendAsynData(EAP_CMD_SET_VEHICLE_MOUNTED_STOP_FIRED_ALL, NULL, 0);
		rsp->buffer[0] = 1;
	}
	
	return 1;
}

uint16_t Alink_C2SetFiredTime_Func(uint8_t *request, uint8_t *response)
{
	VehicleMountedModule *pvehMod = NULL;
	VehicleMountedCmdproc *pvehCmd = NULL;
	
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	int data_length = 0;
	uint8_t firetime = 0;

	dump_alink_msg(req);

	if(EAP_ALINK_CMD_C2_SET_FIRED_TIME != req->msg_head.msgid){
		rsp->buffer[0] = 0;
	}else{
		pvehMod = GetVehicleMountedModuler();
		if(!pvehMod){
			EAP_LOG_ERROR("err,  pvehMod is NULL!\n");
		}
		
		pvehCmd = (VehicleMountedCmdproc *)pvehMod->getCmdProc();
		if(!pvehCmd){
			EAP_LOG_ERROR("err,  pvehCmd is NULL!\n");
		}

		firetime = req->buffer[0];

		DEBUG_SYSALINKFUN_PRINTF("%s, %d, firetime = %d\n", __func__, __LINE__, firetime);
		
		pvehCmd->VehicleMountedSetFiredTime(&firetime);
		
		rsp->buffer[0] = 1;
	}
	
	return 1;
}


uint16_t Alink_C2GetFiredTime_Func(uint8_t *request, uint8_t *response)
{
	VehicleMountedModule *pvehMod = NULL;
	VehicleMountedCmdproc *pvehCmd = NULL;
	
	alink_msg_t *req = (alink_msg_t *)request;
	alink_msg_t *rsp = (alink_msg_t *)response;
	uint8_t firetime = 0;

	dump_alink_msg(req);

	if(EAP_ALINK_CMD_C2_GET_FIRED_TIME != req->msg_head.msgid){
		rsp->buffer[0] = 0;
	}else{
		pvehMod = GetVehicleMountedModuler();
		if(!pvehMod){
			EAP_LOG_ERROR("err,  pvehMod is NULL!\n");
		}
		
		pvehCmd = (VehicleMountedCmdproc *)pvehMod->getCmdProc();
		if(!pvehCmd){
			EAP_LOG_ERROR("err,  pvehCmd is NULL!\n");
		}

		pvehCmd->VehicleMountedGetFiredTime(&firetime);
		
		rsp->buffer[0] = firetime;
	}
	
	return 1;
}

uint16_t Alink_SetWhiteList_Func(uint8_t *request, uint8_t *response)
{
    alink_msg_t *req = (alink_msg_t *)request;
    alink_msg_t *rsp = (alink_msg_t *)response;
    uint16_t total = (uint16_t)(req->buffer[1] << 8) + req->buffer[0];
    vector<string> list;
    char snCode[32];
    int result = -1;

    EAP_LOG_DEBUG("Set Whitelist: total=%d", total);
    for (uint16_t i = 0; i < total; i++)
    {
        memcpy(snCode, (char*)(req->buffer + 2 + 32*i), sizeof(snCode));
        string snStr(snCode);
        list.push_back(snStr);
        EAP_LOG_DEBUG("Set Whitelist: %s", snStr);
    }

    result = EapGetSys().GetWhitelistManager()->SetWhitelist(list);
    memset(rsp->buffer, 0, sizeof(rsp->buffer));
    if (0 == result)
    {
        rsp->buffer[0] = 1;
    }
    else
    {
        rsp->buffer[0] = 0;
    }

    return 1;
}


}

