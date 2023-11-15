#include "gtest/gtest.h"

class SysTest : public testing::Test 
{
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};