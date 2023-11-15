#include "gtest/gtest.h"

class TracerEnvironment : public testing::Environment
{
public:
	virtual void SetUp();
	virtual void TearDown();
};