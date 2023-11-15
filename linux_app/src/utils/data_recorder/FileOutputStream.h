#pragma once

#include <stdio.h>
#include <string>
#include "eap_pub.h"

using namespace std;

class FileOutputStream
{
public:
    FileOutputStream();
    ~FileOutputStream();
    void Open();
    void Write(uint8_t* buf, uint32_t length);
    void Close();
    void SetFileName(string path, string name, string extName);
    void SetFileMaxSize(uint32_t maxSize){ _maxSize = maxSize; }
protected:
private:
    FILE* _OpenFile(bool isCreate = false);
    void _DelFile();
    void _DelFile(string pathName);
    void _Compress();
    void _RefreshFileNoName(uint8_t curFileNo);
    void _CalcCurFile();
    uint8_t _GetEarliestFileNo();
    string _filePath;
    string _fileName;
    string _fileExtName;
    string _fileFullName;
    string _fileRarFullName;
    uint32_t _maxSize;
    FILE* _file;
    uint8_t _fileNum;
    uint8_t _curFileNo;
};