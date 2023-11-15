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
 * Created : 07 Octomber 2023
 * Purpose : Provide util functions for GPIO init, this files is written by C++
 *
 * $Rev$
 *
 * Notes: 
 *
 *****************************************************************************
 */


#include <string>
#include <map>

#include "TracerUtils_c.h"
#include "json_pb_converter.h"
#include "gpioapp.h"

extern "C" {
}


using namespace std;

namespace tracer
{
namespace gpio
{

#define DEFINE_GPIO_ITEM(pGPIO_NAME) {pGPIO_NAME, #pGPIO_NAME} ,
map<INPUT_NAME, string> _sGpioInputEnumNameMap = {
    #include "hal/gpio/gpio_def_input.h"
};

map<OUTPUT_NAME, string> _sGpioOutputEnumNameMap = {
    #include "hal/gpio/gpio_def_output.h"
};

#undef DEFINE_GPIO_ITEM

}
}


void showAllGpioIDs(DIRECTION direction)
{
    if(INPUT == direction)
    {
        TACER_UTILS_PRINTF_INFO("Dislay all Input GPIOs");
        for(const auto& lIterMap : tracer::gpio::_sGpioInputEnumNameMap)
        {
            TACER_UTILS_PRINTF_INFO("INPUT[%s]=%d\n", lIterMap.second.c_str(), lIterMap.first);
        }
    }
    else if(OUTPUT == direction)
    {
        TACER_UTILS_PRINTF_INFO("Dislay all Output GPIOs");
        for(const auto& lIterMap : tracer::gpio::_sGpioOutputEnumNameMap)
        {
            TACER_UTILS_PRINTF_INFO("OUTPUT[%s]=%d\n", lIterMap.second.c_str(), lIterMap.first);
        }
    }
    else
    {
        TACER_UTILS_PRINTF_ERR("Invalid direction %d\n", (uint32_t)direction);
    }
}
