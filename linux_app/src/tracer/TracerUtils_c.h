/*****************************************************************************
 * Copyright (c) 2023
 * Skyfend Technology Co., Ltd
 *
 * All rights reserved. Any unauthorized disclosure or publication of the
 * confidential and proprietary information to any other party will constitute
 * an infringement of copyright laws.
 *
 * $Id$
 * Author  : Fang kai, fangkai@autel.com
 * Created : 20 September 2023
 * Purpose : Declare util functions for tracer in C format
 *
 * $Rev$
 *
 * Notes: This file is compiled in C, C++ is forbidden here!!!!!
 *
 *****************************************************************************
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>


#define TACER_UTILS_PRINTF_RAW(...) printf(__VA_ARGS__) 
#define TACER_UTILS_PRINTF_ERR(...) \
{\
    TACER_UTILS_PRINTF_RAW("%s():###Error*****", __FUNCTION__);\
    TACER_UTILS_PRINTF_RAW(__VA_ARGS__);\
}

#define TACER_UTILS_PRINTF_INFO(...) \
{\
    TACER_UTILS_PRINTF_RAW("%s():****Info*****", __FUNCTION__);\
    TACER_UTILS_PRINTF_RAW(__VA_ARGS__);\
}

#define USER_CFG_FILE "/run/media/mmcblk0p3/usercfg.json"

bool IsBoardType_T3();
bool IsBoardType_T2();
const char* GetTtyName_Mcu2Ps();
const char* GetTtyName_BlueTooth();

int readWarningLevelFromUserCfg();
void saveWarningLevelToUserCfg(int warningLevel);

#ifdef __cplusplus
}
#endif
