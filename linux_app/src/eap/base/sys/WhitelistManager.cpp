#include <sys/stat.h>
#include "WhitelistManager.h"
#include "eap_pub.h"
#include "eap_os_adapter.h"

#define SQLITEDB_PATH "/run/media/mmcblk0p3/database"
#define SQLITEDB_NAME "/run/media/mmcblk0p3/database/tracer.db"
#define TABLE_NAME "WarningWhitelist"
#define KEY_SNCODE "SNCode"

vector<string> TestWhitelist = {"TestDroneID0001", "TestDroneID0002", "TestDroneID0003", "TestDroneID0004"};

int Sqlite3Callback(void *data, int count, char **colText, char **colName)
{
    if (0 >= count)
    {
        return -1;
    }

    vector<string> list;
    WhitelistManager *obj = (WhitelistManager *)data;
    for (int i = 0; i < count; i++)
    {
        list.push_back(string((colText[i])));
    }

    return obj->InsertWhitelist(list);
}

WhitelistManager::WhitelistManager()
{
    _wlMutex = eap_create_mutex();
}

WhitelistManager::~WhitelistManager()
{
#ifndef __UNITTEST__
    if (nullptr != _sqlite3DB)
    {
        sqlite3_close(_sqlite3DB);
        _sqlite3DB = nullptr;
    }
    eap_destroy_mutex(_wlMutex);
#endif
}

int WhitelistManager::Init()
{
    struct stat st;
    int result = 0;
    char *sql;
    char *errmsg = NULL;

#ifndef __UNITTEST__
    result = stat(SQLITEDB_PATH, &st);
    if (0 != result || !S_ISDIR(st.st_mode))
    {
        result = mkdir(SQLITEDB_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (0 != result)
        {
            EAP_LOG_ERROR("create_directory fialed! result: %d", result);
            goto exitPoint;
        }
    }

    result = sqlite3_open(SQLITEDB_NAME, &_sqlite3DB);
    if (SQLITE_OK != result || nullptr == _sqlite3DB)
    {
        EAP_LOG_ERROR("sqlite3_open fialed! %d", result);
        result = -result;
        goto exitPoint;
    }

    sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %q (id INTEGER PRIMARY KEY, %q TEXT);", TABLE_NAME, KEY_SNCODE);
    result = sqlite3_exec(_sqlite3DB, sql, NULL, NULL, &errmsg);
    if (SQLITE_OK != result)
    {
        EAP_LOG_ERROR("sqlite3_exec: %s, err_msg=%s", sql, errmsg);
        sqlite3_free(errmsg);
        result = -result;
        goto exitPoint;
    }

    sql = sqlite3_mprintf("SELECT %q FROM %q;", KEY_SNCODE, TABLE_NAME);
    result = sqlite3_exec(_sqlite3DB, sql, Sqlite3Callback, this, &errmsg);
    if (SQLITE_OK != result)
    {
        EAP_LOG_ERROR("sqlite3_exec: %s, err_msg=%s", sql, errmsg);
        sqlite3_free(errmsg);
        result = -result;
        goto exitPoint;
    }
/*
    sql = sqlite3_mprintf("SELECT name FROM sqlite_master WHERE type='table' AND name='%q';", TABLE_NAME);
    result = sqlite3_exec(pSqliteDB, sql, NULL, NULL, &errmsg);
    if (SQLITE_OK == result)
    {
        sql = sqlite3_mprintf("SELECT %q FROM %q;", KEY_SNCODE, TABLE_NAME);
        result = sqlite3_exec(pSqliteDB, sql, Sqlite3Callback, this, &errmsg);
        if (SQLITE_OK != result)
        {
            EAP_LOG_DEBUG("sqlite3_exec: %s, err_msg=%s", sql, errmsg);
            sqlite3_free(errmsg);
            result = -result;
            goto exitPoint;
        }
    }
    else
    {
        EAP_LOG_DEBUG("sqlite3_exec: %s, errmsg=%s", sql, errmsg);
        sqlite3_free(errmsg);

        sql = sqlite3_mprintf("CREATE TABLE %q (id INTEGER PRIMARY KEY, %q TEXT);", TABLE_NAME, KEY_SNCODE);
        result = sqlite3_exec(pSqliteDB, sql, NULL, NULL, &errmsg);
        if (SQLITE_OK != result)
        {
            EAP_LOG_DEBUG("sqlite3_exec: %s, err_msg=%s", sql, errmsg);
            sqlite3_free(errmsg);
            result = -result;
            goto exitPoint;
        }
    }
*/
exitPoint:
    if (nullptr != _sqlite3DB)
    {
        sqlite3_close(_sqlite3DB);
        _sqlite3DB = nullptr;
    }

    if (_whiteList.empty()) //just for test
    {
        SetWhitelist(TestWhitelist);
    }
#endif
    return result;
}
/*
int WhitelistManager::InitWhitelist(vector<string> &list)
{
    int result = 0;

    if (!list.empty() && list != mWhiteList)
    {
        mWhiteList.swap(list);
    }
    else
    {
        result = -1;
    }

    return result;
}
*/
int WhitelistManager::InsertWhitelist(const vector<string> &list)
{
    eap_mutex_lock(_wlMutex);
    for (const string& item : list)
    {
        _whiteList.push_back(item);
    }
    eap_mutex_unlock(_wlMutex);

    DumpWhitelist();

    return 0;
}

vector<string> WhitelistManager::GetWhitelist()
{
    eap_mutex_lock(_wlMutex);
    vector<string> list(_whiteList);
    eap_mutex_unlock(_wlMutex);
    return list;
}

int WhitelistManager::SetWhitelist(const vector<string> &list)
{
    int result = 0;
    char *sql;
    char *errmsg = NULL;
    vector<string> l(list);

#ifndef __UNITTEST__
    if (list == _whiteList)
    {
        EAP_LOG_DEBUG("Whitelist is same!");
        goto exitPoint;
    }

    result = sqlite3_open(SQLITEDB_NAME, &_sqlite3DB);
    if (SQLITE_OK != result || nullptr == _sqlite3DB)
    {
        EAP_LOG_ERROR("sqlite3_open fialed! %d", result);
        result = -1;
        goto exitPoint;
    }

    sql = sqlite3_mprintf("DELETE FROM %q;", TABLE_NAME);
    result = sqlite3_exec(_sqlite3DB, sql, NULL, NULL, &errmsg);
    if (SQLITE_OK != result)
    {
        EAP_LOG_ERROR("sqlite3_exec: %s, err_msg=%s", sql, errmsg);
        sqlite3_free(errmsg);
        result = -result;
        goto exitPoint;
    }

    if (list.empty())
    {
        EAP_LOG_DEBUG("Whitelist is empty!");
        goto emptyList;
    }

    sql = sqlite3_mprintf("BEGIN TRANSACTION;");
    result = sqlite3_exec(_sqlite3DB, sql, NULL, NULL, &errmsg);
    if (SQLITE_OK != result)
    {
        EAP_LOG_ERROR("sqlite3_exec: %s, err_msg=%s", sql, errmsg);
        sqlite3_free(errmsg);
        result = -result;
        goto exitPoint;
    }

    for (vector<string>::iterator it = l.begin(); it != l.end(); it++)
    {
        sql = sqlite3_mprintf("INSERT INTO %q (%q) VALUES ('%q');", TABLE_NAME, KEY_SNCODE, it->c_str());
        result = sqlite3_exec(_sqlite3DB, sql, NULL, NULL, &errmsg);
        if (SQLITE_OK != result)
        {
            EAP_LOG_ERROR("sqlite3_exec: %s, err_msg=%s", sql, errmsg);
            sqlite3_free(errmsg);
            result = -result;
            goto exitPoint;
        }
    }

    sql = sqlite3_mprintf("COMMIT;");
    result = sqlite3_exec(_sqlite3DB, sql, NULL, NULL, &errmsg);
    if (SQLITE_OK != result)
    {
        EAP_LOG_ERROR("sqlite3_exec: %s, err_msg=%s", sql, errmsg);
        sqlite3_free(errmsg);
        result = -result;
        goto exitPoint;
    }

emptyList:
    eap_mutex_lock(_wlMutex);
    _whiteList.swap(l);
    eap_mutex_unlock(_wlMutex);
    DumpWhitelist();

exitPoint:
    if (nullptr != _sqlite3DB)
    {
        sqlite3_close(_sqlite3DB);
        _sqlite3DB = nullptr;
    }
#endif
    return result;
}

void WhitelistManager::DumpWhitelist()
{
    eap_mutex_lock(_wlMutex);
    EAP_LOG_DEBUG("WhiteList size=%d:", _whiteList.size());
    for (const string& item : _whiteList)
    {
        EAP_LOG_DEBUG("%s", item.c_str());
    }
    eap_mutex_unlock(_wlMutex);
}

