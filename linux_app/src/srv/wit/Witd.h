/**
 * WIT header file
 *
 * @author: chenyili@autel.com
 **/

#ifndef __WITD__H_
#define __WITD__H_

#ifdef __cplusplus
extern "C" {
#endif


#define WIT_MSG_CNT (10)
#define WIT_MSG_MAX_SIZE (512)

enum WIT_REQ_CMD
{
	WIT_REQ_CMD_NET_CONNECT,
	WIT_REQ_CMD_NET_CHECK,
	WIT_REQ_CMD_POWERON,
	WIT_REQ_CMD_POWEROFF,
	WIT_REQ_CMD_READY_CHECK,
};

enum WIT_RSP_RESULT
{
	WIT_RSP_RESULT_POWEROFF=-4,  //图传模块已经下电
	WIT_RSP_RESULT_UNSUPPORTED = -3, //不支持的指令
	WIT_RSP_RESULT_HW_ERROR = -2, //网卡不通或者不存在
	WIT_RSP_RESULT_ERROR = -1,    //图传模块能应答，但是数据异常
	WIT_RSP_RESULT_OK = 0,        //WIT_REQ_CMD_CONNECT的返回值
	WIT_RSP_RESULT_CONNECTED,     //WIT_REQ_CMD_CHECK的返回值
	WIT_RSP_RESULT_CONNECTING,    //WIT_REQ_CMD_CHECK的返回值
	WIT_RSP_RESULT_DISCONNECT,    //WIT_REQ_CMD_CHECK的返回值
};


struct wit_msg_req
{
	int req_cmd;
	int param_len;
	char param[WIT_MSG_MAX_SIZE];
};

struct wit_msg_rsp
{
	int req_cmd;
	int rsp_result;
	int param_len;
	char param[WIT_MSG_MAX_SIZE];
};


#ifdef __cplusplus
}
#endif



#endif

