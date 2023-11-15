#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "FreeRTOS_CLI.h"
#include "wificfg.h"
#include "eap_sys_cmds.h"
// #include "TracerDataPath.h"
extern void SetOutputSpecMat(uint8_t value);
extern uint8_t GetOutputSpecMat(void);

#if 0
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...)
#endif




#define CFG_ENABLE_SPECTRUM_CLI_CMD

#ifdef CFG_ENABLE_SPECTRUM_CLI_CMD

BaseType_t cli_spectrum_dump_cmd_handler(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString_orig)
{
	BaseType_t len1 = 0;
	BaseType_t len2 = 0;
	char pcCommandString[256];

	// printf("%s[%d] cmd:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig);
	snprintf(pcCommandString,sizeof(pcCommandString),"%s",pcCommandString_orig);
	printf("%s[%d] cmd:%s\n", __FUNCTION__, __LINE__,pcCommandString);

	char *op = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);
	char *num = FreeRTOS_CLIGetParameter(pcCommandString, 2, &len2);

	BaseType_t ret;

	if(len1>0 && op)
		op[len1] = 0;
	if(len2>0 && num)
		num[len2] = 0;

	printf("%s[%d] op=%s, len1=%d, num=%s, len2=%d\n", __FUNCTION__, __LINE__, op, len1, num, len2);
	if (len1 > 0){
        if(!strcmp(op,"get")){
			DEBUG_PRINTF("%s[%d] cmd:%s, op:%s,num:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig,op,num);
			uint8_t remains = GetOutputSpecMat();
			snprintf(pcWriteBuffer, xWriteBufferLen, "GetOutputSpecMat()=%u", remains); 
			ret = pdTRUE;
        }else if(!strcmp(op,"clear")){
			DEBUG_PRINTF("%s[%d] cmd:%s, op:%s,num:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig,op,num);
			system("rm -rf /run/media/mmcblk0p3/spectrum/*/*.dat");
			sync();
			snprintf(pcWriteBuffer, xWriteBufferLen, "OK, delete all *.dat in /run/media/mmcblk0p3/spectrum/");
			ret = pdTRUE;
        }else if(!strcmp(op,"set") && len2>=0){
			DEBUG_PRINTF("%s[%d] cmd:%s, op:%s,num:%s\n", __FUNCTION__, __LINE__,pcCommandString_orig,op,num);
			int dump_num = atoi(num);
			if (dump_num >= 0 && dump_num <= 100) {
				SetOutputSpecMat(dump_num);
				snprintf(pcWriteBuffer, xWriteBufferLen, "SetOutputSpecMat(%d)", dump_num);
				ret = pdTRUE;
			}
			else {
				snprintf(pcWriteBuffer, xWriteBufferLen, "invalid num, should be 0~100!");
				ret = pdFALSE;
			}
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

static const CLI_Command_Definition_t _spectrum_cli_cmd =
{
	"spectrum_dump",
	"spectrum_dump   {set|clear|get}  {num}\r\n",
	cli_spectrum_dump_cmd_handler,
	2
};

#endif


int spectrum_dump_init()
{

#ifdef CFG_ENABLE_SPECTRUM_CLI_CMD
	FreeRTOS_CLIRegisterCommand(&_spectrum_cli_cmd);
#endif
	return 0;
}

