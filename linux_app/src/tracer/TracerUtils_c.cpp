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
 * Purpose : Provide util functions for tracer
 *
 * $Rev$
 *
 * Notes: 
 *
 *****************************************************************************
 */


#include <string>

#include "TracerUtils_c.h"
#include "json_pb_converter.h"

#include <iostream>
#include <fstream>
#ifndef __UNITTEST__
#include <json-c/json.h>
#endif

using namespace std;

static const string sBoardTypeName_T3 = "SHRD100-T3";
static const string sBoardTypeName_T2 = "SHRD100-T2";


namespace tracer
{
namespace utils_c
{
void TestGpioJson()
{
    const auto& lGpioCfg = ProductHwCfgGet().gpioconfig();
    TACER_UTILS_PRINTF_RAW("%s():********'miobasepinnum=%d.\n", __FUNCTION__, lGpioCfg.miobasepinnum());
    TACER_UTILS_PRINTF_RAW("%s():********'emiobasepinnum=%d.\n", __FUNCTION__, lGpioCfg.emiobasepinnum());
    TACER_UTILS_PRINTF_RAW("%s():********'linuxbasepinnum=%d.\n", __FUNCTION__, lGpioCfg.linuxbasepinnum());
    auto lSize = lGpioCfg.gpiocfglist_size();
    TACER_UTILS_PRINTF_RAW("%s():****'gpiocfglist_size=%d.\n", __FUNCTION__, lSize);
    for(int i = 0 ; i < lSize ; ++i)
    {
        const auto& lGpioDataItem = lGpioCfg.gpiocfglist(i);
        {
            TACER_UTILS_PRINTF_RAW("%s():********'index=%d.\n", __FUNCTION__, i);
            TACER_UTILS_PRINTF_RAW("%s():********'name=%s.\n", __FUNCTION__, lGpioDataItem.name().c_str());
            TACER_UTILS_PRINTF_RAW("%s():********'direction=%d.\n", __FUNCTION__, lGpioDataItem.direction());
            TACER_UTILS_PRINTF_RAW("%s():********'enabled=%d.\n", __FUNCTION__, lGpioDataItem.enabled());
            TACER_UTILS_PRINTF_RAW("%s():********'mode=%d.\n", __FUNCTION__, lGpioDataItem.mode());
            TACER_UTILS_PRINTF_RAW("%s():********'deviceid=%d.\n", __FUNCTION__, lGpioDataItem.deviceid());
            TACER_UTILS_PRINTF_RAW("%s():********'axichl=%d.\n", __FUNCTION__, lGpioDataItem.axichl());
            TACER_UTILS_PRINTF_RAW("%s():********'pinnum=%d.\n", __FUNCTION__, lGpioDataItem.pinnum());
            TACER_UTILS_PRINTF_RAW("%s():********'initlevel=%d.\n", __FUNCTION__, lGpioDataItem.initlevel());
        }
    }
}



}  // ~namespace utils_c
}  // ~namespace tracer

using namespace tracer::utils_c;

bool IsBoardType_T3()
{
	int ret = 0;
	int len = 0;
    //showAllGpioIDs(INPUT); // TODO: Used for debug
    //showAllGpioIDs(OUTPUT); // TODO: Used for debug
    // TestGpioJson(); // TODO: Used for debug

	len = strlen(sBoardTypeName_T3.c_str());
	
    EAP_LOG_DEBUG("%s\n", ProductHwCfgGet().producthwtype().c_str());

	
	ret = strncmp(ProductHwCfgGet().producthwtype().c_str(), sBoardTypeName_T3.c_str(), len);
	if(ret){
			return 0;
	}else{
			return 1;
	}
	
    //return ProductHwCfgGet().producthwtype() == sBoardTypeName_T3;
}
bool IsBoardType_T2()
{
    return ProductHwCfgGet().producthwtype() == sBoardTypeName_T2;
}
const char* GetTtyName_Mcu2Ps()
{
    return ProductHwCfgGet().devttymcu2ps().c_str();
}
const char* GetTtyName_BlueTooth()
{
    return ProductHwCfgGet().devttywifibluetooth().c_str();
}

#ifndef __UNITTEST__

// 用户配置 - C2设置告警级别
int readWarningLevelFromUserCfg() {
    std::ifstream file(USER_CFG_FILE);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << USER_CFG_FILE << std::endl;
        return 1; // 默认为低
    }

    std::string jsonData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    json_object* root = json_tokener_parse(jsonData.c_str());
    if (root == nullptr) {
        std::cerr << "Failed to parse UserCfg.json" << std::endl;
        return 1; // 默认为低
    }

    json_object* warningLevelObj = json_object_object_get(root, "WarningLevel");
    int warningLevel = json_object_get_int(warningLevelObj);

    json_object_put(root);

    return warningLevel;
}

void saveWarningLevelToUserCfg(int warningLevel) {
    std::ifstream file(USER_CFG_FILE);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << USER_CFG_FILE << std::endl;
        return;
    }

    std::string jsonData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    json_object* root = json_tokener_parse(jsonData.c_str());
    if (root == nullptr) {
        std::cerr << "Failed to parse UserCfg.json" << std::endl;
        return;
    }

    json_object* warningLevelObj = json_object_new_int(warningLevel);
    json_object_object_add(root, "WarningLevel", warningLevelObj);

    std::ofstream outFile("UserCfg.json");
    if (!outFile.is_open()) {
        std::cerr << "Failed to open UserCfg.json for writing" << std::endl;
        return;
    }

    outFile << json_object_to_json_string(root);
    outFile.close();

    json_object_put(root);
}

#else

int readWarningLevelFromUserCfg()
{
    return 1;
}

void saveWarningLevelToUserCfg(int warningLevel)
{
    return;
}

#endif
