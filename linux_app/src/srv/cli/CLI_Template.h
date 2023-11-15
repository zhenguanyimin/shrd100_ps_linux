#include "eap_pub.h"
#include "log.h"
#include "FreeRTOS_CLI.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CliCmd_TwoDatas
{
    uint32_t value1;
    uint32_t value2;
} CliCmd_TwoDatas;

typedef struct CliCmd_ThreeDatas
{
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
} CliCmd_ThreeDatas;

#define DEFINE_CLI_HANDLER1(cmd_name, cmd_code, help)                                                                           \
BaseType_t cmd_name##_handler(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)                         \
{                                                                                                                               \
	BaseType_t len1 = 0;                                                                                                        \
	const char *param1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);                                                   \
                                                                                                                                \
	if (len1 > 0)                                                                                                               \
	{                                                                                                                           \
		uint32_t value1 = strtoul(param1, NULL, 0);                                                                             \
		                                                                                                                        \
        SendAsynData(cmd_code, (uint8_t*)&value1, sizeof(uint32_t));                                                            \
                                                                                                                                \
		EAP_LOG_DEBUG("value1:%d\n", value1);                                                                                   \
		snprintf(pcWriteBuffer, xWriteBufferLen, "OK");                                                                         \
	}                                                                                                                           \
	else                                                                                                                        \
	{                                                                                                                           \
		snprintf(pcWriteBuffer, xWriteBufferLen, "ERROR");                                                                      \
	}                                                                                                                           \
                                                                                                                                \
	return pdFALSE;                                                                                                             \
}                                                                                                                               \
static const CLI_Command_Definition_t cmd_name##_cmd =                                                                          \
{                                                                                                                               \
	#cmd_name,                                                                                                                  \
	help,                                                                                                                       \
	cmd_name##_handler,                                                                                                         \
	1                                                                                                                           \
};

#define DEFINE_CLI_HANDLER2(cmd_name, cmd_code, help)                                                                           \
BaseType_t cmd_name##_handler(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)                         \
{                                                                                                                               \
	BaseType_t len1 = 0;                                                                                                        \
	BaseType_t len2 = 0;                                                                                                        \
	const char *param1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);                                                   \
	const char *param2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &len2);                                                   \
                                                                                                                                \
	if ((len1 > 0) && (len2 > 0))                                                                                               \
	{                                                                                                                           \
		CliCmd_TwoDatas data;                                                                                                   \
        data.value1 = strtoul(param1, NULL, 0);                                                                                 \
		data.value2 = strtoul(param2, NULL, 0);                                                                                 \
		                                                                                                                        \
        SendAsynData(cmd_code, (uint8_t*)&data, sizeof(CliCmd_TwoDatas));                                                       \
                                                                                                                                \
		EAP_LOG_DEBUG("value1:%d\n", data.value1);                                                                              \
		EAP_LOG_DEBUG("value2:%d\n", data.value2);                                                                              \
		snprintf(pcWriteBuffer, xWriteBufferLen, "OK");                                                                         \
	}                                                                                                                           \
	else                                                                                                                        \
	{                                                                                                                           \
		snprintf(pcWriteBuffer, xWriteBufferLen, "ERROR");                                                                      \
	}                                                                                                                           \
                                                                                                                                \
	return pdFALSE;                                                                                                             \
}                                                                                                                               \
static const CLI_Command_Definition_t cmd_name##_cmd =                                                                          \
{                                                                                                                               \
	#cmd_name,                                                                                                                  \
	help,                                                                                                                       \
	cmd_name##_handler,                                                                                                         \
	2                                                                                                                           \
};


#define DEFINE_CLI_HANDLER3(cmd_name, cmd_code, help)                                                                           \
BaseType_t cmd_name##_handler(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)                         \
{                                                                                                                               \
	BaseType_t len1 = 0;                                                                                                        \
	BaseType_t len2 = 0;                                                                                                        \
	BaseType_t len3 = 0;                                                                                                        \
	Thread_Msg msg;                                                                                                             \
	const char *param1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);                                                   \
	const char *param2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &len2);                                                   \
	const char *param3 = FreeRTOS_CLIGetParameter(pcCommandString, 3, &len3);                                                   \
                                                                                                                                \
	if ((len1 > 0) && (len2 > 0) && (len3 > 0))                                                                                 \
	{                                                                                                                           \
        CliCmd_ThreeDatas data;                                                                                                 \
		data.value1 = strtoul(param1, NULL, 0);                                                                                 \
		data.value2 = strtoul(param2, NULL, 0);                                                                                 \
		data.value3 = strtoul(param3, NULL, 0);                                                                                 \
		                                                                                                                        \
        SendAsynData(cmd_code, (uint8_t*)&CliCmd_ThreeDatas, sizeof(CliCmd_ThreeDatas));                                        \
                                                                                                                                \
		LOG_DEBUG("value1:%d\n", data.value1);                                                                                  \
		LOG_DEBUG("value2:%d\n", data.value2);                                                                                  \
		LOG_DEBUG("value3:%d\n", data.value3);                                                                                  \
		snprintf(pcWriteBuffer, xWriteBufferLen, "OK");                                                                         \
	}                                                                                                                           \
	else                                                                                                                        \
	{                                                                                                                           \
		snprintf(pcWriteBuffer, xWriteBufferLen, "ERROR");                                                                      \
	}                                                                                                                           \
                                                                                                                                \
	return pdFALSE;                                                                                                             \
}                                                                                                                               \
static const CLI_Command_Definition_t cmd_name##_cmd =                                                                          \
{                                                                                                                               \
	#cmd_name,                                                                                                                  \
	help,                                                                                                                       \
	cmd_name##_handler,                                                                                                         \
	3                                                                                                                           \
};

#define DEFINE_CLI_HANDLER1_SYNC(cmd_name, cmd_code, help, success_msg, fail_msg)                                                                           \
BaseType_t cmd_name##_handler(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)                         \
{                                                                                                                               \
	BaseType_t len1 = 0;                                                                                                        \
	const char *param1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);                                                   \
                                                                                                                                \
	if (len1 > 0)                                                                                                               \
	{                                                                                                                           \
		uint32_t value1 = strtoul(param1, NULL, 0);                                                                             \
		                                                                                                                        \
        uint8_t ret = EAP_SUCCESS;                                                                                              \
        uint32_t respDataLen = 1;                                                                                               \
        SendSynData(EAP_THREAD_ID_CLI, cmd_code, (uint8_t*)&value1, sizeof(uint32_t), &ret, &respDataLen);                      \
        if (1 == respDataLen && EAP_SUCCESS == ret)                                                                             \
        {                                                                                                                       \
			snprintf(pcWriteBuffer, xWriteBufferLen, success_msg, value1);                                                      \
        }                                                                                                                       \
        else                                                                                                                    \
        {                                                                                                                       \
			snprintf(pcWriteBuffer, xWriteBufferLen, fail_msg, value1);                                                         \
        }                                                                                                                       \
	}                                                                                                                           \
	else                                                                                                                        \
	{                                                                                                                           \
		snprintf(pcWriteBuffer, xWriteBufferLen, "ERROR");                                                                      \
	}                                                                                                                           \
                                                                                                                                \
	return pdFALSE;                                                                                                             \
}                                                                                                                               \
static const CLI_Command_Definition_t cmd_name##_cmd =                                                                          \
{                                                                                                                               \
	#cmd_name,                                                                                                                  \
	help,                                                                                                                       \
	cmd_name##_handler,                                                                                                         \
	1                                                                                                                           \
};

#define DEFINE_CLI_HANDLER_FUNC(cmd_name, help, func)                                                                           \
BaseType_t cmd_name##_handler(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)                         \
{                                                                                                                               \
	func(pcWriteBuffer, xWriteBufferLen);                                                                                       \
	return pdFALSE;                                                                                                             \
}                                                                                                                               \
static const CLI_Command_Definition_t cmd_name##_cmd =                                                                          \
{                                                                                                                               \
	#cmd_name,                                                                                                                  \
	help,                                                                                                                       \
	cmd_name##_handler,                                                                                                         \
	1                                                                                                                           \
};

#define DEFINE_CLI_HANDLER1_FUNC(cmd_name, help, func)                                                                          \
BaseType_t cmd_name##_handler(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)                         \
{                                                                                                                               \
	BaseType_t len1 = 0;                                                                                                        \
	const char *param1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len1);                                                   \
                                                                                                                                \
	func(len1, param1, pcWriteBuffer, xWriteBufferLen);                                                                         \
                                                                                                                                \
	return pdFALSE;                                                                                                             \
}                                                                                                                               \
static const CLI_Command_Definition_t cmd_name##_cmd =                                                                          \
{                                                                                                                               \
	#cmd_name,                                                                                                                  \
	help,                                                                                                                       \
	cmd_name##_handler,                                                                                                         \
	1                                                                                                                           \
};

#ifdef __cplusplus
}
#endif

