#include "gtest/gtest.h"
#include "TracerEnvironment.h"

using namespace std;

int main() {
    testing::AddGlobalTestEnvironment(new TracerEnvironment());
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
