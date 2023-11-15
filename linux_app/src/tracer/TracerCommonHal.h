#pragma once


#include <stdint.h>
#include <unistd.h>
#include <mutex>
#include "HalBase.h"
#include "eap_pub.h"
#include "../eap/include/cmdproc/CmdprocBase.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
#include "hal.h"
#include "gpio/gpioapp.h"
#include "output/output.h"
#include "input/input.h"
#include "hal/eth_link/socket/tcp_client.h"
#ifdef __cplusplus
}
#endif

#define INDATA_U8(inData)  *((uint8_t *)(inData.buf))
#define INDATA_U16(inData) *((uint16_t *)(inData.buf))
#define INDATA_U32(inData) *((uint32_t *)(inData.buf))
#define INDATA2_U32(inData) *((uint32_t *)(inData.buf)+1)
#define INDATA_INT(inData) *((int *)(inData.buf))
#define INDATA_FLOAT(inData) *((float *)(inData.buf))

#define OUTDATA_U8(outData) ((uint8_t *)(outData.buf)) 
#define OUTDATA_U16(outData) ((uint16_t *)(outData.buf))
#define OUTDATA_U32(outData) ((uint32_t *)(outData.buf))
#define OUTDATA_INT(outData) ((int *)(outData.buf))
#define OUTDATA_FLOAT(outData) ((float *)(outData.buf))

typedef enum
{
    GPIO_INPUT = 0,
    GPIO_OUTPUT,
} gpio_direction_t;

typedef struct gpio_ctrl
{
    uint16_t name;
    uint8_t direction;
    uint8_t value;
} gpio_ctrl_t;

typedef struct register_ctrl
{
    uint32_t addr;
    int32_t value;
} register_ctrl_t;

class TracerCommonHal : public HalBase
{
public:
    TracerCommonHal(HalMgr* halMgr);
    void Init() override;
    void TcpConnect(tcp_client_t &tcp);
	void TypeCConnect(tcp_client_t &tcp);
    void TcpDisconnect(tcp_client_t &tcp);
    bool TcpOk(tcp_client_t &tcp);

protected:
    int _GetPlVersion(const Hal_Data& inData, Hal_Data& outData);
    int _GetGpioValue(const Hal_Data& inData, Hal_Data& outData);
    int _SetGpioValue(const Hal_Data& inData, Hal_Data& outData);
    int _GetRegisterValue(const Hal_Data& inData, Hal_Data& outData);
    int _SetRegisterValue(const Hal_Data& inData, Hal_Data& outData);
    int _ReportAlinkInfo(const Hal_Data& inData, Hal_Data& outData);
	//typec
	void TypeCSetRevHeartBeatFlag(const Hal_Data& inData, Hal_Data& outData);

	//RemoteId section begin
	int _SetRemoteIdBufUseFinish(const Hal_Data& inData, Hal_Data& outData);
	int _SetRemotIdChn(const Hal_Data& inData, Hal_Data& outData);
	//RemoteId section end
    int _C2HeartBeatTimeOutProc(const Hal_Data& inData, Hal_Data& outData);
    void _SetTcpState(link_c2_t link_type, bool state);

	int _SendMsgToLinkChannel(const Hal_Data& inData, Hal_Data& outData);
	int _DumpAlinkMsgHead(alink_msg_t *msg);


	//for NET_PROTO_TYPE_TRACER_UAVCONTROLLER
    int _LoginToRemoteControlUnit(const Hal_Data& inData, Hal_Data& outData);
    int _SendHeartBeatToRemoteControlUnit(const Hal_Data& inData, Hal_Data& outData);
    int _SendDroneInfoToRemoteControlUnit(const char* buf, uint32_t len);

    int _GetSerialNumber(const Hal_Data& inData, Hal_Data& outData);
    int _SetSerialNumber(const Hal_Data& inData, Hal_Data& outData);

    int _GetRemoteAddress(const Hal_Data& inData, Hal_Data& outData);

	//for bluetooth
	int _BlueToothSetNetwork(const Hal_Data& inData, Hal_Data& outData);
    void _BlueTooth_BD_Pro_Analyz(uint8_t *pbuff, int length);
	void _BlueToothGetPathofCfg(void);
	int _BlueToothSetWpaConfig(void);
	void _BlueToothStopNetwork(void);
	void _BlueToothStartNetwortk(void);
	void _BlueToothSetWifiConnectServer(void);
	void _BlueToothDisableWifiNetwork(void);
	void _BlueToothEnableWifiNetwork(void);
	int _BlueToothCreateWifiConnectService(void);
	int _getBoardType(void);
private:
    tcp_client_t _tcps[TCP_MAX_CONNECT_NUM];
	tcp_client_t _typeC;
    mutex _tcpMtx;
	mutex _buletoothMtx;
	uint32_t _netProtoType;

    //for bluetooth
    char _name[33];
	char _secret[33];
	char _auth;
	char _path[128];
};
