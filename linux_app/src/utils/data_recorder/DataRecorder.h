#pragma once

#include <mutex>
#include <list>
#include <string>
#include "eap_pub.h"

using namespace std;

class FileOutputStream;
class RecorderFormatter;

class DataRecorder
{
    DECLARE_SINGLETON(DataRecorder);
public:
    void Save(RecorderFormatter& formatter);
    void SetRecordFileName(string path, string name, string extName);
    void SetFileMaxItemNum(uint32_t maxItemNum) { _maxItemNum = maxItemNum; }
    void SetFileMaxSize(uint32_t maxSize);
    uint32_t GetFileCurItemNum();
    void ShowCmdInfos();
protected:
    void _Flush();
private:
    uint32_t _maxItemNum;
    list<RecorderFormatter*> _items;
    FileOutputStream* _fileStream;
    recursive_mutex _mutex;
};