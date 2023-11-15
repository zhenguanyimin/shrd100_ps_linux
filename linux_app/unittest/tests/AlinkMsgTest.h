#include "gtest/gtest.h"

class AlinkMsgTest : public testing::Test 
{
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    virtual void SetUp();
    virtual void TearDown();
};