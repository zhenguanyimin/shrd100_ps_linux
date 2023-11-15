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
 * Created : 20 September 2023
 * Purpose : Unit test
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */
#pragma once

#include "gtest/gtest.h"

class ProtoBufTest : public testing::Test 
{
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    virtual void SetUp();
    virtual void TearDown();
};
