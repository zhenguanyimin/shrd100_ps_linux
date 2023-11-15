#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "FreeRTOS_CLI.h"
#include "polit_tcp_cfg.h"
#include "eap_sys_cmds.h"

#define CFG_DEBUG_CREATE_TCP_CLIENT_IP  
#if 1
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif

#ifdef CFG_DEBUG_CREATE_TCP_CLIENT_IP

#define FLAG_DEBUG_TCP_CLIENT_IP_FILE  "/run/media/mmcblk0p1/flag_debug_tcp_client_ip"

BaseType_t cli_polit_tcp_cfg_cmd_handler(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString_orig)                         
{                                                                                                                              
	BaseType_t len1 = 0;                                                                                                        
	BaseType_t len2 = 0;                                                                                                        
	BaseType_t len3 = 0; 
	char pcCommandString[256];
	
	printf("%s[%d] cmd:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig);
	snprintf(pcCommandString,sizeof(pcCommandString),"%s",pcCommandString_orig);	
	printf("%s[%d] cmd:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig);

	char *op = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);                                                   
	printf("%s[%d] cmd:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig);

	char *ip = FreeRTOS_CLIGetParameter(pcCommandString, 2, &len2);                                                   
	printf("%s[%d] cmd:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig);

	char *port = FreeRTOS_CLIGetParameter(pcCommandString, 3, &len3);
	char buf[512];
	BaseType_t ret;	
	FILE *file;
	int len;
	char *data;

	if(len1>0 && op)
		op[len1] = 0;
	if(len2>0 && ip)
		ip[len2] = 0;
	if(len3>0 && port)
		port[len3] = 0;
	
	printf("%s[%d] cmd:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig);
	if (len1 > 0){                                                                                                                           
        if(!strcmp(op,"get")){
			FILE *file;
			char *buffer;
			long file_size;
			DEBUG_PRINTF("%s[%d] cmd:%s, op:%s,ip:%s,port:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig,op,ip,port);
			/* 打开文件 */
			file = fopen(FLAG_DEBUG_TCP_CLIENT_IP_FILE, "r");
			if (file == NULL) {
				snprintf(pcWriteBuffer, xWriteBufferLen, "no wificfg info");
				ret  = pdTRUE;
				goto out;
			}

			/* 获取文件大小 */
			fseek(file, 0, SEEK_END);
			file_size = ftell(file);
			fseek(file, 0, SEEK_SET);

			/* 分配内存缓冲区 */
			buffer = (char *)malloc(file_size + 1);
			if (buffer == NULL) {
				snprintf(pcWriteBuffer, xWriteBufferLen, "read wificfg info err");
				ret  = pdTRUE;
				fclose(file);
				goto out;
			}

			/* 读取文件内容到缓冲区 */
			len = fread(buffer, file_size, 1, file);
			buffer[file_size] = '\0'; // 添加字符串结束符

			/* 关闭文件 */
			fclose(file);

			snprintf(pcWriteBuffer, xWriteBufferLen, "%s",buffer);
			/* 释放内存 */
			free(buffer);
			ret  = pdTRUE;
			DEBUG_PRINTF("%s[%d] cmd:%s, op:%s,ip:%s,port:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig,op,ip,port);
        }else if(!strcmp(op,"clear")){	
			DEBUG_PRINTF("%s[%d] cmd:%s, op:%s,ip:%s,port:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig,op,ip,port);
			unlink(FLAG_DEBUG_TCP_CLIENT_IP_FILE);
			sync();
			snprintf(pcWriteBuffer, xWriteBufferLen, "OK,Please reboot to make effect"); 
			ret = pdTRUE;
        }else if(!strcmp(op,"set") && len2>0 && len3>0){
			DEBUG_PRINTF("%s[%d] cmd:%s, op:%s,ip:%s,port:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig,op,ip,port);
		    /* 打开文件，如果文件不存在则创建，如果文件已存在则覆盖 */
		    file = fopen(FLAG_DEBUG_TCP_CLIENT_IP_FILE, "w");
		    if (file == NULL) {
				snprintf(pcWriteBuffer, xWriteBufferLen, "set fail");																		
				ret  = pdFALSE;
		        goto out;
		    }
		    /* 写入字符串到文件 */
			snprintf(buf,sizeof(buf),"%s:%s",ip,port);
		    fprintf(file, "%s",buf);
			
		    /* 关闭文件 */
		    fclose(file);	
			snprintf(pcWriteBuffer, xWriteBufferLen, "%s",buf); 
			sync();
			ret = pdTRUE;
		}else{
			snprintf(pcWriteBuffer, xWriteBufferLen, "param invalid");	
			ret = pdFALSE;
		}
	}   else{                                                                                                                          
		snprintf(pcWriteBuffer, xWriteBufferLen, "param invalid");   
		ret = pdFALSE;
	}                                                                                                                         
out:     

	printf("%s[%d] ret:%ld,cmd_output:%s\n", __FUNCTION__, __LINE__,(long)ret,pcWriteBuffer);
	return ret;                                                                                                            
}  

static const CLI_Command_Definition_t _politcfg_cli_cmd =                                                                          \
{                                                                                                                               \
	"polit_tcp_cfg",                                          
	"polit_tcp_cfg   {set|clear|get}  {IP}   {PORT}\r\n",
	cli_polit_tcp_cfg_cmd_handler, 
	3                                                                                                                           \
};

#endif


int polit_tcp_cfg_init()
{

#ifdef CFG_DEBUG_CREATE_TCP_CLIENT_IP
	FreeRTOS_CLIRegisterCommand(&_politcfg_cli_cmd);
#endif
	return 0;
}

