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
	WIT_RSP_RESULT_POWEROFF=-4,  //ͼ��ģ���Ѿ��µ�
	WIT_RSP_RESULT_UNSUPPORTED = -3, //��֧�ֵ�ָ��
	WIT_RSP_RESULT_HW_ERROR = -2, //������ͨ���߲�����
	WIT_RSP_RESULT_ERROR = -1,    //ͼ��ģ����Ӧ�𣬵��������쳣
	WIT_RSP_RESULT_OK = 0,        //WIT_REQ_CMD_CONNECT�ķ���ֵ
	WIT_RSP_RESULT_CONNECTED,     //WIT_REQ_CMD_CHECK�ķ���ֵ
	WIT_RSP_RESULT_CONNECTING,    //WIT_REQ_CMD_CHECK�ķ���ֵ
	WIT_RSP_RESULT_DISCONNECT,    //WIT_REQ_CMD_CHECK�ķ���ֵ
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

