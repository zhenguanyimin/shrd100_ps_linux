#include "FileOutputStream.h"
#include "RecorderFormatter.h"
#include "eap_os_adapter.h"

extern "C"
{
    int lzmaCompress(const uint8_t dwEncodeMode, const char *pcInputFile, const char *pcOutputFile);
}

FileOutputStream::FileOutputStream()
{
    _file = nullptr;
    _maxSize = 0;
    _fileNum = 20;
    _curFileNo = 0;
}

FileOutputStream::~FileOutputStream()
{

}

void FileOutputStream::Open()
{
    _file = _OpenFile();
    if (nullptr == _file) return;
    if (fseek(_file, 0L, SEEK_END) != 0) return;
    uint32_t fileSize = ftell(_file);
    if (fileSize > _maxSize)
    {
        fclose(_file);
        _Compress();
        _file = _OpenFile(true);
    }
}

void FileOutputStream::_Compress()
{
    _RefreshFileNoName(_curFileNo);
    lzmaCompress(1, _fileFullName.c_str(), _fileRarFullName.c_str());
    ++_curFileNo;
    if (_curFileNo >= _fileNum) _curFileNo = 0;
}

void FileOutputStream::_RefreshFileNoName(uint8_t curFileNo)
{
    _fileRarFullName = _filePath;
    _fileRarFullName.append("/").append(_fileName).append("_").append(to_string(curFileNo)).append(".rar");
}

void FileOutputStream::_DelFile()
{
    _DelFile(_fileFullName);
}

void FileOutputStream::_DelFile(string pathName)
{
    eap_del_file(pathName.c_str());
}

void FileOutputStream::_CalcCurFile()
{
    uint8_t i = 0;
    for (; i < _fileNum; ++i)
    {
        _RefreshFileNoName(i);
        FILE* file = fopen(_fileRarFullName.c_str(), "rb");
        if (nullptr == file) break;
        fclose(file);
    }
    if (i == _fileNum)
    {
        _curFileNo = _GetEarliestFileNo();
        _RefreshFileNoName(_curFileNo);
    }
    else
    {
        _curFileNo = i;
    }
}

uint8_t FileOutputStream::_GetEarliestFileNo()
{
    uint8_t earliestNo = 0;
    _RefreshFileNoName(0);
    int64_t earlierTime = eap_get_modity_time(_fileRarFullName.c_str());
    for (uint8_t i = 1; i < _fileNum; ++i)
    {
        _RefreshFileNoName(i);
        int64_t time = eap_get_modity_time(_fileRarFullName.c_str());
        if (time < earlierTime)
        {
            earlierTime = time;
            earliestNo = i;
        }
    }
    return earliestNo;
}

void FileOutputStream::SetFileName(string path, string name, string extName)
{
    _filePath = path;
    _fileName = name;
    _fileExtName = extName;
    _fileFullName = _filePath;
    _fileFullName.append("/").append(_fileName).append(".").append(_fileExtName);
    _CalcCurFile();
    _file = _OpenFile(true);
    fclose(_file);
}

void FileOutputStream::Close()
{
    if (nullptr != _file)
    {
        fflush(_file);
        fclose(_file);
        _file = nullptr;
    }
}

FILE* FileOutputStream::_OpenFile(bool isCreate)
{
    FILE* file = fopen(_fileFullName.c_str(), isCreate ? "wb+" : "ab+");
    if (nullptr == file)
    {
        eap_mkdir(_filePath.c_str());
        file = fopen(_fileFullName.c_str(), isCreate ? "wb+" : "ab+");
    }
    return file;
}

void FileOutputStream::Write(uint8_t* buf, uint32_t length)
{
    if (nullptr == _file) return;
    fwrite(buf, length, 1, _file);
}