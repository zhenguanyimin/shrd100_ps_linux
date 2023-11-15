#include <memory.h>
#include "RemoteIdBufScanDefaultStrategy.h"

int32_t RemoteIdBufScanDefaultStrategy::Scan(uint8_t* data, uint32_t dataLen, uint32_t& start)
{
	uint8_t* checkData = nullptr;
    start = 0;
	for (int i = 0; i < dataLen; i++)
	{		
        if (data[i] == 0x80)
		{
            checkData = data + i;
			if ((checkData[0] == 0x80) && (checkData[1] == 0x0) && (checkData[2] == 0x0) && (checkData[3] == 0x0) 
                && (checkData[4] == 0xFF) && (checkData[5] == 0xFF) && (checkData[6] == 0xFF) && (checkData[7] == 0xFF))
			{
                start = i;
				return EAP_SUCCESS;
			}
		}
	}

	return EAP_FAIL;
}
