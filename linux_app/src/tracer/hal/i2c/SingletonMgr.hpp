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

#pragma once

#ifndef _SINGLETONMGR_HPP_
#define _SINGLETONMGR_HPP_

#include <mutex>

namespace hal
{
namespace driver
{

class SingletonMgr
{
public:
    static SingletonMgr* instance();
    static void deleteInstance();

protected:

private:
    SingletonMgr();
    virtual ~SingletonMgr();

    SingletonMgr(const SingletonMgr&) = delete;
    const SingletonMgr &operator=(const SingletonMgr&) = delete;

    static SingletonMgr* _instance;
    static std::mutex _mutex;
};

}  // ~namespace driver
}  // ~namespace hal

#endif
