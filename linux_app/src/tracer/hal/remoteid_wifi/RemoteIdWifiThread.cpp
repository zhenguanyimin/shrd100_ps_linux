#include "RemoteIdWifiThread.h"
#include "eap_sys_cmds.h"
#include "pcap.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "RemoteIdInfo.h"

extern "C" {
#include "rid_capture.h"
#include "parse.h"
}

#define CHECK_PAYLOAD_HEADER_START 36

DEFINE_THREAD(RemoteIdWifiThread, REMOTEIDWIFI)
//DEFINE_TIMER1(RemoteIdThread, EAP_REMOTEID_REP_TIMER, true)

void getPacket(u_char * arg, const struct pcap_pkthdr * pkthdr, const u_char * packet);
int Wifi_CaptureMain();
int get_remodeid_info_callback(char *data, int data_len);


#define DEBUG_GET_PACKAGE  0
#define RID_PRINTF(arg...) if(DEBUG_GET_PACKAGE)printf(arg)

struct ether_header
{
	unsigned char ether_dhost[6];	//目的mac
	unsigned char ether_shost[6];	//源mac
	unsigned short ether_type;		//以太网类型
};
 
void RemoteIdWifiThread::_InitQueue(uint16_t& num)
{
    num = 10;
}

void RemoteIdWifiThread::_OnCommand(const Thread_Msg& msg)
{
#if 0
    //Wifi_CaptureMain();
#else
	int  argc;
	char *argv[5];
	argv[0] = "rid_capture_main";
	argv[1] = "-w";
	//FIXME, 应该通过函数获取remoteid使用网卡名
	char ifname[32] = {'\0'};
	FILE* fp = fopen(IFNAME_REMOTEID, "r");
	if (fp)
	{
		fscanf(fp, "%s", ifname);
		fclose(fp);
	}
	if (strlen(ifname) > 0)
		argv[2] = ifname;
	else
		argv[2] = "wlp1s0";
	argc = 3;
	printf("*******************************\n");
	printf("starting remoteid_main_rid_capture\n");
	printf("*******************************\n");
	
#if CFG_NO_USING_RID_CAPTURE	
    remoteid_main_rid_capture(argc,argv,getPacket,NULL);
#else
    remoteid_main_rid_capture(argc,argv,NULL,get_remodeid_info_callback);
#endif

#endif    
}


void getPacket(u_char * arg, const struct pcap_pkthdr * pkthdr, const u_char * packet)
{
	  int			 i, offset = 36, length, typ, len;
	  char			 ssid_tmp[32], text[128];
	  u_char		*payload, *val, mac[6];
	  u_int16_t 	radiotap_len; 
	  uint8_t*      data = nullptr;
	  static u_char  nan_cluster[6]  = {0x50, 0x6f, 0x9a, 0x01, 0x00, 0xff},
					 nan_service[6]  = {0x88, 0x69, 0x19, 0x9d, 0x92, 0x09},
					 oui_alliance[3] = {0x50, 0x6f, 0x9a};
	
	  ssid_tmp[0]  =
	  ssid_tmp[32] = 0;
	  radiotap_info info = {0};
		
	  length	   = pkthdr->len;
	  radiotap_len = *(u_int16_t *) &packet[2];
	
	  if (radiotap_len < length) {
		
		payload = (u_char *)  &packet[radiotap_len];
		radiotap_parse_header((void*)packet, radiotap_len,&info);
	  } else {
		return;
	  }

	if (payload[0] == 0x80) {
		for(i = CHECK_PAYLOAD_HEADER_START; i < (length - radiotap_len); i++)
        {
            if (*(payload + i) == 0xDD)
            {
                data = payload + i;
                if ((*(data + 2) == 0xFA) && (*(data + 3) == 0x0B) && (*(data + 4) == 0xBC))
                {
                    //struct timeval tv1;
                    //gettimeofday(&tv1, NULL);
                    //printf("received 0xFA 0x0B 0xBC header, second = %d, microseconds = %d \n", tv1.tv_sec, tv1.tv_usec);

					SendAsynData(EAP_CMD_REMOTEID, (uint8_t*)payload, length - radiotap_len );
                    break;
                }
            }
        }   	
	}

}

int get_remodeid_info_callback(char *data, int data_len)
{
	//printf("**%s:%d\n",__FUNCTION__,__LINE__);
	RemoteId_Info *info = (RemoteId_Info *)data;

	SendAsynData(EAP_CMD_REMOTEID_RID_CAPTURE, (uint8_t*)data, data_len);
	return 0;
}


int RemoteIdWifiThread::remoteIdSwitchChannel(uint32_t chn)
{
	return remoteid_switch_channel(chn);
}


int Wifi_CaptureMain()
{
	char errBuf[PCAP_ERRBUF_SIZE], * devStr;
	pcap_t *device;
	devStr = "wlp1s0";
	// system("ifconfig wlp1s0 up");
	// system("iwconfig wlp1s0 mode monitor");
	// system("iwconfig wlp1s0 channel 1");
	// system("iwlist   wlp1s0 channel");
	printf("***************Wifi_CaptureMain ******************  1111  \r\n");
	// devStr = "wlan0";
	
	/* open a device, wait until a packet arrives */
	// pcap_t * device = pcap_open_live(devStr, 65535, 1, 1, errBuf); //设置超时时间为1ms，超时后获取数据包的函数就会立即返回
	device = pcap_open_live(devStr, 65535, 1, 1, errBuf); //设置超时时间为1ms，超时后获取数据包的函数就会立即返回

	if(!device)
	{
		printf("error: pcap_open_live(): %s\n", errBuf);
		exit(1);
	}

	/* construct a filter */
	struct bpf_program filter;
	// pcap_compile(device, &filter, PACP_FILTER, 1, 0);
	// pcap_setfilter(device, &filter);

	/* wait loop forever */
	int id = 0;
	pcap_loop(device, -1, getPacket, (u_char*)&id);// -1表示无限循环捕获

//	while(1)
//	{
//		getOnePacket(device);
//	}
	pcap_close(device);

	return 0;
}


bool RemoteIdWifiThread::Run()
{
	Thread_Msg msg;
	printf("*******************************\n");
	printf("starting RemoteIdWifiThread:run\n");
	printf("*******************************\n");
	
	SetSleepMs(5);	
	_OnCommand(msg);

    return true;
}
