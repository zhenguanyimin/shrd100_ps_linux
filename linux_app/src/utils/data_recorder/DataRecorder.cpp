#include "DataRecorder.h"
#include "RecorderFormatter.h"
#include "FileOutputStream.h"

DEFINE_SINGLETON_NO_CONSTRUCT(DataRecorder)

DataRecorder::DataRecorder()
{
    _maxItemNum = 0;
    _fileStream = new FileOutputStream();
}

void DataRecorder::Save(RecorderFormatter& formatter)
{
    lock_guard<recursive_mutex> lock(_mutex);
    _items.emplace_back(&formatter);
    if (_items.size() >= _maxItemNum)
    {
        _Flush();
    }
}

void DataRecorder::_Flush()
{
    lock_guard<recursive_mutex> lock(_mutex);
    if (nullptr == _fileStream) return;
    _fileStream->Open();
    for (auto item : _items)
    {
        if (nullptr == item) continue;
        uint8_t* serialBuf = item->GetSerialBuf();
        if (nullptr == serialBuf) continue;
        _fileStream->Write(serialBuf, item->GetSerialLength());
        EAP_DELETE(item);
    }
    _items.clear();
    _fileStream->Close();
}

void DataRecorder::SetRecordFileName(string path, string name, string extName)
{
    if (nullptr == _fileStream) return;
    _fileStream->SetFileName(path, name, extName);
}

void DataRecorder::SetFileMaxSize(uint32_t maxSize)
{
    if (nullptr == _fileStream) return;
    _fileStream->SetFileMaxSize(maxSize);
}

void DataRecorder::ShowCmdInfos()
{
    lock_guard<recursive_mutex> lock(_mutex);
    for (auto item : _items)
    {
        if (nullptr == item) continue;
        uint8_t* serialBuf = item->GetSerialBuf();
        if (nullptr == serialBuf) continue;
        printf("\n");
        printf(" %s", (char*)serialBuf);
    }
    printf("\n Total: %d\n", _items.size());
}