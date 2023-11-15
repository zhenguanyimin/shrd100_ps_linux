/*****************************************************************************
 * Copyright (c) 2023
 * Skyfend Technology Co., Ltd
 *
 * All rights reserved. Any unauthorized disclosure or publication of the
 * confidential and proprietary information to any other party will constitute
 * an infringement of copyright laws.
 *
 * $Id$
 * Author  : JiaWei, jiawei@autel.com
 * Created : 22 September 2023
 * Purpose : Provide Singleton management class
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */

#include "SingletonMgr.hpp"

#include <memory>

namespace hal
{
namespace driver
{

SingletonMgr* SingletonMgr::_instance = nullptr;
std::mutex SingletonMgr::_mutex;

SingletonMgr* SingletonMgr::instance()
{
    if (nullptr == _instance)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (nullptr == _instance)
        {
            volatile auto temp = new(std::nothrow)SingletonMgr();
            _instance = temp;
        }
    }

    return _instance;
}

void SingletonMgr::deleteInstance()
{
    std::unique_lock<std::mutex> lock(_mutex);

    if (nullptr != _instance)
    {
        delete _instance;
        _instance = nullptr;
    }
}

}  // ~namespace driver
}  // ~namespace hal


