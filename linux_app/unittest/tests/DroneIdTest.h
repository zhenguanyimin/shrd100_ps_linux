#include "gtest/gtest.h"

class DroneIdTest : public testing::Test 
{
protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    virtual void SetUp();
    virtual void TearDown();
};