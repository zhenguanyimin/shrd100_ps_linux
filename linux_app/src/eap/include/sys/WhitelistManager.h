#pragma once

//#include <iostream>
//#include <iomanip>
#include <string>
#include <vector>
#ifndef __UNITTEST__
#include "sqlite3.h"
#endif

using namespace std;

class WhitelistManager
{
public:
    WhitelistManager();
    ~WhitelistManager();
    int Init();
//    int InitWhitelist(vector<string> &list);
    int InsertWhitelist(const vector<string> &list);
    vector<string> GetWhitelist();
    int SetWhitelist(const vector<string> &list);
protected:
//    int Sqlite3Callback(void *data, int count, char **colText, char **colName);
private:
    void DumpWhitelist();
#ifndef __UNITTEST__
    sqlite3 *_sqlite3DB = nullptr;
#endif
    vector<string> _whiteList;
    void *_wlMutex = nullptr;
};
