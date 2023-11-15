#pragma once

#include <map>
#include <string.h>

namespace testing {

namespace internal {

using namespace std;

enum
{
    AUTHOR_PASS = 0,
    AUTHOR_SKIP,
    AUTHOR_FAIL
};

typedef struct Counter_Info
{
    int total;
    int passed;
    int skipped;
    int failed;
} Counter_Info;

class CounterByAuthor
{
public:
    static void Clear();
    static void Incre(const string& author, int type);
    static void Print();
private:
    static map<string, Counter_Info> _counters;
};

}
}