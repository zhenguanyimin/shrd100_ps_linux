#include "gtest/gtest.h"

class ThreadTest : public testing::Test 
{
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};