/** 
 * wireless image trasition service
 *
 * author: chenyili@autel.com
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <mqueue.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "Witd.h"
#include "WitService.h"

#ifdef __cplusplus
}
#endif

enum 
{
	WIT_LINK_START,
	WIT_LINK_STOP,
	WIT_LINK_EXIT,
};

static struct wit_service_s
{
	bool start;
	mqd_t req_mq;
	mqd_t rsp_mq;
	pthread_t tid;
	int link_action;
	//sem_t link_signal;
	int link_connected;
	int error;
	
	pthread_mutex_t api_lock;
} wit_service = {
	.start = false,
	.req_mq = (mqd_t)-1,
	.rsp_mq = (mqd_t)-1,
	.tid = -1,
	.link_action = WIT_LINK_START,
	.link_connected = 0,
};

#define WIT_CONNECT_TIMEOUT_SECOND (1 * 60)


static int wit_send_cmd(mqd_t req_mq, mqd_t rsp_mq, int cmd, int *result)
{
	int ret;
	struct wit_msg_req req;
	struct wit_msg_rsp rsp;
	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

	req.req_cmd = cmd;
	req.param_len = 0;

	pthread_mutex_lock(&lock);
	
	ret = mq_send(req_mq, (char *)&req, sizeof(req), 0);
//	printf("mq_send:%d\n", ret);
	
	if (ret != 0)
	{
		pthread_mutex_unlock(&lock);
		return -1;
	}
	
	ret = mq_receive(rsp_mq, (char *)&rsp, sizeof(rsp), NULL);
//	printf("mq_receive:%d\n", ret);
	
	if (ret <= 0)
	{
		pthread_mutex_unlock(&lock);
		return -2;
	}

	*result = rsp.rsp_result;
	pthread_mutex_unlock(&lock);
	
	return 0;
}

static void *wit_link_task(void *param)
{
	struct wit_service_s *pwit = (struct wit_service_s *)param;
	int ret;
	int cmd_result;
	int i;

	while(1)
	{
		if (WIT_LINK_START == pwit->link_action)
		{
			pwit->link_connected = 0;
			
			ret = wit_send_cmd(pwit->req_mq, pwit->rsp_mq, WIT_REQ_CMD_READY_CHECK, &cmd_result);
			if (ret || (cmd_result != WIT_RSP_RESULT_OK))
			{
				pwit->error = 1;
				
				sleep(2);
				continue;
			}
			pwit->error = 0;

			while(WIT_LINK_START == pwit->link_action)
			{
				printf("WIT start to connect\n");

				ret = wit_send_cmd(pwit->req_mq, pwit->rsp_mq, WIT_REQ_CMD_READY_CHECK, &cmd_result);
				if (ret || (cmd_result != WIT_RSP_RESULT_OK))
				{
					break;
				}
					
				ret = wit_send_cmd(pwit->req_mq, pwit->rsp_mq, WIT_REQ_CMD_NET_CONNECT, &cmd_result);
				if (ret || (cmd_result != WIT_RSP_RESULT_OK))
				{				
					sleep(2);
					continue;
				}

				printf("WIT connecting...\n");
				for (int i=0; i < WIT_CONNECT_TIMEOUT_SECOND && (WIT_LINK_START == pwit->link_action); i++)
				{
					ret = wit_send_cmd(pwit->req_mq, pwit->rsp_mq, WIT_REQ_CMD_NET_CHECK, &cmd_result);
					if ((0 == ret) && (WIT_RSP_RESULT_CONNECTED == cmd_result))
					{
						pwit->link_connected = 1;
						break;
					}
					else
					{
						sleep(1);
					}
				}

				if (pwit->link_connected)
				{
					printf("WIT connected!\n");
					while(WIT_LINK_START == pwit->link_action)
					{
						sleep(1);
						
						ret = wit_send_cmd(pwit->req_mq, pwit->rsp_mq, WIT_REQ_CMD_NET_CHECK, &cmd_result);
						if (ret || (cmd_result != WIT_RSP_RESULT_CONNECTED))
						{
							printf("WIT disconnect, reconnect now\n");
							pwit->link_connected = 0;
							break;
						}
					}
				}
				else
				{
					printf("WIT connect timeout! start again\n");
				}
			}
		}
		else if (WIT_LINK_STOP == pwit->link_action)
		{
//			printf("WIT link stop\n");
			sleep(1);
		}
		else if (WIT_LINK_EXIT == pwit->link_action)
		{
//			printf("WIT link thread exit!!\n");
			break;
		}
		else
		{
//			printf("wit link action error:%d\n", pwit->link_action);
			sleep(1);
		}
	}

	return NULL;
}

static int is_wit_ready(struct wit_service_s *pwit, int *is_error)
{
	int ret;
	int cmd_result;
	
	ret = wit_send_cmd(pwit->req_mq, pwit->rsp_mq, WIT_REQ_CMD_READY_CHECK, &cmd_result);
	if (ret || (cmd_result != WIT_RSP_RESULT_OK))
	{
		if ((cmd_result < 0) && (is_error))
		{
			*is_error = 1;	
		}
		return 0;
	}

	return 1;
}


int wit_service_start(const char *msg_prefix_name)
{
	int ret;
	struct wit_service_s *pwit = &wit_service;
	char req_msg_name[256];
	char rsp_msg_name[256];
	int is_error;
	int req_flag = 1;
	int rsp_flag = 1;

	if (pwit->start)
	{
		return -1;
	}

	if (NULL == msg_prefix_name)
	{
		return -1;
	}

	snprintf(req_msg_name, sizeof(req_msg_name)-1, "/%s-req.msg", msg_prefix_name);
	snprintf(rsp_msg_name, sizeof(rsp_msg_name)-1, "/%s-rsp.msg", msg_prefix_name);

	do{	
		pwit->req_mq = mq_open(req_msg_name, O_RDWR); 
		if (pwit->req_mq == (mqd_t)-1)
		{
			//perror("mq open error\n");
			//goto error;
		}else{
			req_flag = 0;
		}
		sleep(1);
	}while(req_flag);

	printf("req_mq:%d\n", pwit->req_mq);

	do{
		pwit->rsp_mq = mq_open(rsp_msg_name, O_RDWR);
		if (pwit->rsp_mq == (mqd_t)-1)
		{
			//perror("mq open error\n");
			//goto error;
		}else{
			rsp_flag = 0;
		}
		sleep(1);
	}while(rsp_flag);

	printf("req_mq:%d\n", pwit->req_mq);

	pwit->link_connected = 0;
	if (is_wit_ready(pwit, &is_error))
	{
	//	pwit->error = 0;
		pwit->error = is_error;
		pwit->link_action = WIT_LINK_START;
	}
	//else
	//{
	//	pwit->error = is_error;
	//	pwit->link_action = WIT_LINK_STOP;
	//}

	ret = pthread_create(&pwit->tid, NULL, wit_link_task, pwit);
	if (ret)
	{
		goto error;
	}
	pthread_setname_np(pwit->tid, "Wit");
	
	pthread_mutex_init(&pwit->api_lock, NULL);

	pwit->start = true;
	return 0;

	error:
	if (pwit->req_mq != (mqd_t)-1)
	{
		mq_close(pwit->req_mq);
		pwit->req_mq = (mqd_t)-1;
	}

	if (pwit->rsp_mq != (mqd_t)-1)
	{
		mq_close(pwit->rsp_mq);
		pwit->rsp_mq = (mqd_t)-1;
	}
	return -1;
}

int wit_service_stop(void)
{
	struct wit_service_s *pwit = &wit_service;

	if (pwit->start)
	{
		pwit->link_action = WIT_LINK_EXIT;
		pthread_join(pwit->tid, NULL);
		
		pthread_mutex_destroy(&pwit->api_lock);
		
		if (pwit->req_mq != (mqd_t)-1)
		{
			mq_close(pwit->req_mq);
			pwit->req_mq = (mqd_t)-1;
		}

		if (pwit->rsp_mq != (mqd_t)-1)
		{
			mq_close(pwit->rsp_mq);
			pwit->rsp_mq = (mqd_t)-1;
		}

		pwit->start = false;
	}
	
	return 0;
}


int wit_disable(void)
{
	struct wit_service_s *pwit = &wit_service;
	int ret = 0;
	int cmd_result;
	
	pthread_mutex_lock(&pwit->api_lock);
	
	if (WIT_LINK_START == pwit->link_action)
	{
		pwit->link_action = WIT_LINK_STOP;

		ret = wit_send_cmd(pwit->req_mq, pwit->rsp_mq, WIT_REQ_CMD_POWEROFF, &cmd_result);
		if (ret || (cmd_result != WIT_RSP_RESULT_OK))
		{
			printf("WIT poweroff fail:%d\n", ret);
			ret = -1;
		}
		else
		{
			ret = 0;
		}
	}

	pthread_mutex_unlock(&pwit->api_lock);
	return ret;
}

int wit_enable(void)
{
	struct wit_service_s *pwit = &wit_service;
	int ret = 0;
	int cmd_result;

	pthread_mutex_lock(&pwit->api_lock);

	if (WIT_LINK_STOP == pwit->link_action)
	{
		ret = wit_send_cmd(pwit->req_mq, pwit->rsp_mq, WIT_REQ_CMD_POWERON, &cmd_result);
		if (ret || (cmd_result != WIT_RSP_RESULT_OK))
		{
			printf("WIT poweron fail:%d\n", ret);
			ret = -1;
		}
		else
		{
			pwit->link_action = WIT_LINK_START;
			ret = 0;
		}
	}

	pthread_mutex_unlock(&pwit->api_lock);
	return ret;
}

WIT_STATUS_t wit_status(void)
{
	WIT_STATUS_t result = WIT_STATUS_ERROR;
	struct wit_service_s *pwit = &wit_service;
	
	pthread_mutex_lock(&pwit->api_lock);
	if (WIT_LINK_STOP == pwit->link_action)
	{
		result = WIT_STATUS_DISABLE;
	}
	else if (WIT_LINK_START == pwit->link_action)
	{
		if (pwit->error)
			result = WIT_STATUS_ERROR;
		else if (pwit->link_connected)
			result = WIT_STATUS_ENABLE_CONNECTED;
		else 
			result = WIT_STATUS_ENABLE_NO_CONNECT;
	}
	else
	{
		result = WIT_STATUS_ERROR;
	}

	pthread_mutex_unlock(&pwit->api_lock);

	return result;
}




