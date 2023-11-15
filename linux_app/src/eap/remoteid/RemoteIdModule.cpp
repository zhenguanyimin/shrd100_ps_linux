#include "RemoteIdModule.h"
#include "RemoteIdCmdproc.h"
#include "RemoteIdThread.h"
#include "RemoteIdBufScanDefaultStrategy.h"
#include "SysBase.h"
#include "HalMgr.h"
#include "../../tracer/hal/remoteid_wifi/RemoteIdWifiThread.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>

#include "json_pb_converter.h"
#include "WhitelistManager.h"

extern "C"
{
#include "droneID_system_param.h"
}

#define DEBUG_PRINT  1
#define RID_PRINTF(arg...) if(DEBUG_PRINT)printf(arg)

#define FLAG_DEBUG_RUN_RID_PARSE_VALUE_PRINT 
#define MANUFACTURER_NAME_LEN  4

struct timeval glast_tv;

RemoteIdModule::RemoteIdModule(uint8_t coreId) : ModuleReal(coreId)
{
    if (access("/run/media/mmcblk0p1/flag_debug_remoteid_get_uav_data", 0) != 0)
    {
        _debugLogEnable = false;
    }
    else
    {
        _debugLogEnable = true;
    }
    if (ProductSwCfgGet().issupportwarningwhitelist())
    {
        _enemyTargetNum = 0;
    }
}

uint8_t RemoteIdModule::GetModuleId()
{
    return EAP_MODULE_ID_REMOTEID;
}

CmdprocBase* RemoteIdModule::_CreateCmdproc()
{
    RemoteIdCmdproc* remoteIdCmdproc = new RemoteIdCmdproc();
    remoteIdCmdproc->SetRecvBuf(4096);
    remoteIdCmdproc->SetScanStrategy(new RemoteIdBufScanDefaultStrategy);
    return remoteIdCmdproc;
}

ThreadBase* RemoteIdModule::_CreateThread()
{
    return ADD_THREAD(RemoteIdThread, _coreId);
}

int8_t RemoteIdModule::_GetDeviceType(uint8_t* revBuff)
{
    if ((*(revBuff + 32) == 0x71) && (*(revBuff + 33) == 0x02))
    {
    	return DEVICE_TYPE_LITE; // lite
    }
    else if ((*(revBuff + 32) == 0x64) && (*(revBuff + 33) == 0x0))
    {
    	return DEVICE_TYPE_PARROT; // parrot
    }
    return UINT8_INVALID;
}

typedef struct {
	UAV_DEVICE_TYPE device_type; 
	char            mac[6];
}UavDeviceMacItem;

static UavDeviceMacItem uavDeviceMacTable[] = {
{ DEVICE_TYPE_LITE,		{'\x51','\x6F','\x9A','\x01','\x66','\x66'}	}, //51 6F 9A 01 66 66
{ DEVICE_TYPE_PARROT,	{'\x90','\x3a','\xe6','\x5b','\xb6','\x08'}}, //90:3a:e6:5b:b6:08
};
typedef struct {
	UAV_DEVICE_TYPE_BY_SN device_type; 
	char            manufacturer[8];
    char            modelname[16];
    uint16_t        modellen;
    uint16_t        unitylen;
}UavDeviceSnItem;



static UavDeviceSnItem uavDeviceSnTable[] = {
    // Autel
    {DEVICE_TYPE_AUTEL_EVO_NANO,                    "1748", "NAG",       3, 12},
    {DEVICE_TYPE_AUTEL_EVO_NANO,                    "1748", "NAO",       3, 12},
    {DEVICE_TYPE_AUTEL_EVO_NANO,                    "1748", "NAW",       3, 12},
    {DEVICE_TYPE_AUTEL_EVO_NANO,                    "1748", "NAR",       3, 12},
    {DEVICE_TYPE_AUTEL_EVO_LITE,                    "1748", "LTC",       3, 12},
    {DEVICE_TYPE_AUTEL_EVO_LITE,                    "1748", "LTO",       3, 12},	
    {DEVICE_TYPE_AUTEL_EVO_LITE,                    "1748", "LTW",       3, 12},
    {DEVICE_TYPE_AUTEL_EVO_LITE,                    "1748", "LTG",       3, 12},
	{DEVICE_TYPE_AUTEL_EVO_MAX,                     "1748", "EV3",       3, 15},
    {DEVICE_TYPE_AUTEL_EVO_II,                      "1748", "HL",        2, 12},
    {DEVICE_TYPE_AUTEL_EVO_II,                      "1748", "HM",        2, 12},
    {DEVICE_TYPE_AUTEL_EVO_II,                      "1748", "HN",        2, 12},
	{DEVICE_TYPE_AUTEL_EVO_DRAGFISH_PRO,            "1748", "JD3",       3, 12},
	{DEVICE_TYPE_AUTEL_EVO_DRAGFISH_LITE,           "1748", "JD2",       3, 12},
	{DEVICE_TYPE_AUTEL_EVO_DRAGFISH_STD,            "1748", "JD1",       3, 12},

    // Parrot
    {DEVICE_TYPE_PARROT_ANAFI_USA,                  "1588", "0405",      4, 14},
    {DEVICE_TYPE_PARROT_ANAFI_AI,                   "1588", "0404",      4, 14},

    // Skydio
    {DEVICE_TYPE_SKYDIO_X2E,                        "1668", "E10",       3, 11},
    {DEVICE_TYPE_SKYDIO_2,                          "1668", "S20",       3, 11},

    // Percepto AirMax
    {DEVICE_TYPE_PERCEPTO_AIRMAX,                   "1701", "SPR",       3, 10},
    {DEVICE_TYPE_PERCEPTO_AIRMAX,                   "1701", "GAS",       3, 13},

    // Microdrones
    {DEVICE_TYPE_MICRODRONES_MD4_1000,              "1644", "MD41",      4, 15},
    {DEVICE_TYPE_MICRODRONES_MD4_3000,              "1644", "MD43",      4, 15},
    {DEVICE_TYPE_MICRODRONES_EASY_ONE,              "1644", "MD4E",      4, 15},

    // Wingtra
    {DEVICE_TYPE_WINGTRA_GEN_II,                    "1744", " ",         0, 4},

    // AgEagle(senseFly)
    {DEVICE_TYPE_AGEAGLE_EBEE_X_SERIES,             "1587", "05",        2, 10},
    {DEVICE_TYPE_AGEAGLE_EBEE_X_SERIES,             "1587", "54",       2, 10},

    // Sony
    {DEVICE_TYPE_SONY_ARS_S1,                       "1728", "2",         1, 7},
    {DEVICE_TYPE_SONY_ARS_S1,                       "1728", "9",         1, 7},
    
    // Commaris
    {DEVICE_TYPE_COMMARIS_SEEKER,                   "1787", "03B",       3, 15},

    // Elsight Ltd.
    {DEVICE_TYPE_ELSIGHT_HNF4W,                     "1902", "N0",        2, 7},
    {DEVICE_TYPE_ELSIGHT_HNF4W,                     "1902", "A0",        2, 7},
    {DEVICE_TYPE_ELSIGHT_HNF4W,                     "1902", "B0",        2, 7},
    {DEVICE_TYPE_ELSIGHT_HNF4W,                     "1902", "C0",        2, 7},
    {DEVICE_TYPE_ELSIGHT_HNF4W,                     "1902", "D0",        2, 7},
    {DEVICE_TYPE_ELSIGHT_HNF4W,                     "1902", "K0",        2, 7},

    // Dronetag
    {DEVICE_TYPE_DRONETAG_BEACON,                   "1596", "34",        2, 10},
    {DEVICE_TYPE_DRONETAG_BS,                       "1596", "31",        2, 15},
    {DEVICE_TYPE_DRONETAG_DRI,                      "1596", "33",        2, 15},
    {DEVICE_TYPE_DRONETAG_MINI,                     "1596", "35",        2, 15},

    // EXO
    {DEVICE_TYPE_EXO_X7_RANGER_PLUS,                "1895", "X7R",       3, 11},
    {DEVICE_TYPE_EXO_BLACKHAWK_3_PRO,               "1895", "BC0",       3, 15},

    // Zephyr Systems
    {DEVICE_TYPE_ZEPHYR_SYSTEMS_MODULE_100,         "1795", "01Z",       3, 15},
    {DEVICE_TYPE_ZEPHYR_SYSTEMS_ARK_350,            "1795", "350",       3, 15},
    {DEVICE_TYPE_ZEPHYR_SYSTEMS_OKO_250,            "1795", "250",       3, 15},

    // Inspired Flight
    {DEVICE_TYPE_INSPIRED_FLIGHT_IF1200A,           "1791", "5",         1, 5},
    {DEVICE_TYPE_INSPIRED_FLIGHT_IF1200A,           "1791", "6",         1, 5},

    // BlueMark
    {DEVICE_TYPE_BLUEMARK_DB100,                    "1787", "01",        2, 15},
    {DEVICE_TYPE_BLUEMARK_DB121,                    "1787", "05",        2, 15},
    {DEVICE_TYPE_BLUEMARK_DB121PCB,                 "1787", "06",        2, 15},
    {DEVICE_TYPE_BLUEMARK_DB122FPV,                 "1787", "07",        2, 15},
    {DEVICE_TYPE_BLUEMARK_DB120,                    "1787", "04",        2, 15},

    // Drone Defence
    {DEVICE_TYPE_DRONE_DEFENCE_AEROPING,            "1796", "001",       3, 15},

    // Harris Aerial
    {DEVICE_TYPE_HARRIS_AERIAL_CARRIER_H6,          "1813", "H6",        2, 6},
    {DEVICE_TYPE_HARRIS_AERIAL_CARRIER_H8E,         "1813", "H8",        2, 6},

    // Wing Aviation LLC
    {DEVICE_TYPE_WING_AVIATION_HUMMINGBIRD_V2,      "1633", "A",         1, 5},
    {DEVICE_TYPE_WING_AVIATION_HUMMINGBIRD_V2,      "1633", "V",         1, 5},
    {DEVICE_TYPE_WING_AVIATION_HUMMINGBIRD_V2,      "1633", "M",         1, 5},
    {DEVICE_TYPE_WING_AVIATION_HUMMINGBIRD_V2,      "1633", "D",         1, 5},

    // SJRC
    {DEVICE_TYPE_SJRC_F5S_PRO,                      "1892", "F5SA",      4, 12},    
    {DEVICE_TYPE_SJRC_F5S_PRO_PLUS,                 "1892", "F5SP",      4, 12}, 
    {DEVICE_TYPE_SJRC_F7_4K_PRO,                    "1892", "F7A",       3, 11}, 
    {DEVICE_TYPE_SJRC_F11_4K_PRO,                   "1892", "F110",      4, 12}, 
    {DEVICE_TYPE_SJRC_F11S_4K_PRO,                  "1892", "F11S",      4, 12}, 
    {DEVICE_TYPE_SJRC_F22_4K_PRO,                   "1892", "F22A",      4, 12}, 
    {DEVICE_TYPE_SJRC_F22S_4K_PRO,                  "1892", "F22B",      4, 12}, 

    // Aurelia
    {DEVICE_TYPE_AURELIA_TAROT_650,                 "1906", "B00",       3, 9},
    {DEVICE_TYPE_AURELIA_X4,                        "1906", "B01",       3, 9},
    {DEVICE_TYPE_AURELIA_X6_STANDARD,               "1906", "B02",       3, 9},
    {DEVICE_TYPE_AURELIA_X6_PRO,                    "1906", "B03",       3, 9},
    {DEVICE_TYPE_AURELIA_X6_MAX,                    "1906", "B04",       3, 9},
    {DEVICE_TYPE_AURELIA_X6_PRO_V2,                 "1906", "B10",       3, 9},
    {DEVICE_TYPE_AURELIA_X8_STANDARD,               "1906", "B05",       3, 9},
    {DEVICE_TYPE_AURELIA_X8_MAX,                    "1906", "B06",       3, 9},

    // Vision Aerial
    {DEVICE_TYPE_VISION_AERIAL_DRONE,               "1788", "2",         1, 5},

    // Watts Innovations
    {DEVICE_TYPE_WATTS_INNOVATIONS_PRISM_SKY,       "1785", "000000006", 9, 12},
    {DEVICE_TYPE_WATTS_INNOVATIONS_PRISM_SKY,       "1785", "000000007", 9, 12},
    {DEVICE_TYPE_WATTS_INNOVATIONS_PRISM_LITE,      "1785", "000000002", 9, 12},
    {DEVICE_TYPE_WATTS_INNOVATIONS_PRISM_LITE,      "1785", "000000003", 9, 12},

    // TRIPLEFINE
    {DEVICE_TYPE_TRIPLEFINE_TF35_PRO,               "1922", "TF35",      4, 11},
    {DEVICE_TYPE_TRIPLEFINE_TF_15EX,                "1922", "TF15",      4, 13},

    // Leica Geosystems
    {DEVICE_TYPE_LEICA_GEOSYSTEMS_BLK2FLY,          "1702", "3",         1, 7},
    {DEVICE_TYPE_LEICA_GEOSYSTEMS_BLK2FLY,          "1702", "4",         1, 7},
    
    // Bwine
    {DEVICE_TYPE_BWINE_F7GB2,                       "1875", "F7GB2",     5, 11}, 

    // XM2 Labs
    {DEVICE_TYPE_XM2_LABS_XTD_120,                  "1928", "XTD",       3, 15}, 

    // Drone Volt
    {DEVICE_TYPE_HERCULES_20,                       "1801", "DV",        2, 11}, 

    // Flytrex
    {DEVICE_TYPE_FLYTREX_FTX_M600P,                 "1886", "FTXM600P",  8, 15}, 

    // Hylio
    {DEVICE_TYPE_HYLIO_AG_210,                      "1790", "08",        2, 6},
    {DEVICE_TYPE_HYLIO_AG_216,                      "1790", "09",        2, 6},
    {DEVICE_TYPE_HYLIO_AG_230,                      "1790", "10",        2, 6},
    {DEVICE_TYPE_HYLIO_AG_272,                      "1790", "07",        2, 6},

    // Coretronic
    {DEVICE_TYPE_CORETRONIC_SIRAS,                  "1836", "F1E",       3, 15},

    // Ascent AeroSystems
    {DEVICE_TYPE_ASCENT_AEROSYSTEMS_NX30,           "1812", "NX3",       3, 15},
    {DEVICE_TYPE_ASCENT_AEROSYSTEMS_SPIRIT,         "1812", "SPR",       3, 15},

    // uAvionix
    {DEVICE_TYPE_UAVIONIX_PINGRID,                  "1792", "00",        2, 12},

    // XAG
    {DEVICE_TYPE_XAG_P100,                          "1863", "PEG8218",   7, 15},
    {DEVICE_TYPE_XAG_P100_PRO,                      "1863", "PEG8219",   7, 15},

    // Event38 Unmanned Systems
    {DEVICE_TYPE_EVENT38_UNMANNED_SYSTEMS_E455,     "1807", "455",       3, 7},
    {DEVICE_TYPE_EVENT38_UNMANNED_SYSTEMS_E400,     "1807", "400",       3, 7},

    // Hoverfly Technologies
    {DEVICE_TYPE_HOVERFLY_SENTRY_SPECTRE,           "1867", "180",       3, 15},

    // SkyRanger / SkyRaider
    {DEVICE_TYPE_SKYRANGER_SKYRAIDER_R70_R80D,      "1767", "SRH",       3, 12},

    // UAS
    {DEVICE_TYPE_UAS_RC,                            "1774", "REM",       3, 15},

    // Censys Technologies
    {DEVICE_TYPE_CENSYS_SENTAERO_5,                 "1718", "042",       3, 7},

    // Flightware
    {DEVICE_TYPE_FLIGHTWARE_EDGE_130,               "1879", "DV",        2, 7},
    {DEVICE_TYPE_FLIGHTWARE_EDGE_130,               "1879", "PV",        2, 7},
    {DEVICE_TYPE_FLIGHTWARE_EDGE_130,               "1879", "MV",        2, 7},

    // Cider Copter
    {DEVICE_TYPE_CIDER_COPTER_H10,                  "1911", "00",        2, 4},

    // Teal Drones
    {DEVICE_TYPE_TEAL_DRONES_TEAL_2,                "1839", "TD5",       3, 15},

    // Camflite
    {DEVICE_TYPE_CAMFLITE_AURORA_ASCENT,            "1871", "00",        2, 12},

    // SmartDrone
    {DEVICE_TYPE_SMARTDRONE_DISCOVERY_2,            "1866", "SDD",       3, 10},

    // WISPR
    {DEVICE_TYPE_WISPR_RANGER_PRO_1100,             "1889", "RP11",      3, 14},
    {DEVICE_TYPE_WISPR_RANGER_ELITE,                "1889", "RE",        2, 14},

    // Holy Stone
    {DEVICE_TYPE_HOLY_STONE_HSRID01,                "1865", "A",         1, 6},
    {DEVICE_TYPE_HOLY_STONE_HS720G,                 "1865", "HS720",     5, 12},
    {DEVICE_TYPE_HOLY_STONE_HS720R,                 "1865", "000002",    6, 12},
    {DEVICE_TYPE_HOLY_STONE_HS720,                  "1865", "000000",    6, 12},
    {DEVICE_TYPE_HOLY_STONE_HS700E,                 "1865", "000003",    6, 12},
    {DEVICE_TYPE_HOLY_STONE_HS720E,                 "1865", "000001",    6, 12},
    {DEVICE_TYPE_HOLY_STONE_HS600,                  "1865", "HS600",     5, 10},

    // Workhorse Aero
    {DEVICE_TYPE_WORKHORSE_AERO_WA4_100,            "1884", "00",        2, 7},

    // Draganfly Innovations
    {DEVICE_TYPE_DRAGANFLY_COMMANDER3_XL,           "1808", "DAE",       3, 8},

    // Ruko
    {DEVICE_TYPE_RUKO_F11GIM,                       "1869", "F11GM0",    6, 11},
    {DEVICE_TYPE_RUKO_U11S,                         "1869", "U11S",      4, 10},
    {DEVICE_TYPE_RUKO_U11PRO,                       "1869", "U11P",      4, 10},
    {DEVICE_TYPE_RUKO_F11,                          "1869", "F110",      4, 9},
    {DEVICE_TYPE_RUKO_F11GIM2,                      "1869", "F11GM2",    6, 12},
    {DEVICE_TYPE_RUKO_R111,                         "1869", "F111",      4, 10},
    {DEVICE_TYPE_RUKO_F11PRO,                       "1869", "F11P",      4, 10},

    // Matternet
    {DEVICE_TYPE_MATTERNET_M2_BLOCK_2,              "1810", "M22",       3, 6},

    // Skyfront
    {DEVICE_TYPE_SKYFRONT_PERIMETER_8,              "1926", "P8",        2, 15},

    // Tucok
    {DEVICE_TYPE_TUCOK_193MAX2S,                    "1900", "MAX2S",     5, 12},
    {DEVICE_TYPE_TUCOK_T70,                         "1900", "T70",       3, 10},
    {DEVICE_TYPE_TUCOK_012S,                        "1900", "12S",       3, 10},

    // Aero Systems West
    {DEVICE_TYPE_AERO_SYS_WEST_INTER_LIFT,          "1620", "11",        2, 12},
    {DEVICE_TYPE_AERO_SYS_WEST_HEAVY_LIFT,          "1620", "10",        2, 12},

    // Lucid Bots
    {DEVICE_TYPE_LUCID_BOTS_SHERPA,                 "1877", "SHP",       3, 14},
    {DEVICE_TYPE_LUCID_BOTS_SHERPA,                 "1877", "C123",      4, 10},
    {DEVICE_TYPE_LUCID_BOTS_SHERPA,                 "1877", "C123",      4, 11},

    // PrecisionVision
    {DEVICE_TYPE_PRECISIONVISION_PV40X,             "1937", "114",       3, 5},

    // Drone-Clone Xperts
    {DEVICE_TYPE_DRONE_CLONE_XPERTS_LLIMITLESS_4S,  "1894", "L4S",       3, 11},

    // Spektrum
    {DEVICE_TYPE_SPEKTRUM_SPMA9500_SKYID,           "1849", "084",       3, 14},
    {DEVICE_TYPE_SPEKTRUM_SPMA9500_SKYID,           "1849", "030",       3, 14},

    // C100
    {DEVICE_TYPE_C100_BLOCK_1C,                     "1934", "C",         1, 6},

    // Zipline
    {DEVICE_TYPE_ZIPLINE_SPARROW,                   "1873", "1",         1, 4},
    {DEVICE_TYPE_ZIPLINE_SPARROW,                   "1873", "6",         1, 3},
    {DEVICE_TYPE_ZIPLINE_SPARROW,                   "1873", "8",         1, 3},
    {DEVICE_TYPE_ZIPLINE_SPARROW,                   "1873", "9",         1, 3},

    // Veeniix
    {DEVICE_TYPE_VEENIIX_V11,                       "1872", "V11",       3, 9},

    // Fotokite
    {DEVICE_TYPE_FOTOKITE_SIGMA,                    "1898", "002",       3, 11},

    // DIY-D LLC
    {DEVICE_TYPE_DIY_D_LLC_ESP32_C3_MINI_1,         "1925", "00",        2, 12},

    // Xil
    {DEVICE_TYPE_XIL_193E,                          "1860", "193E",      4, 11},

    // Sentera
    {DEVICE_TYPE_SENTERA_PHX,                       "1798", "11",        2, 15},

    // ELIOS 3 UAV
    {DEVICE_TYPE_ELIOS_3_UAV_107000,                "1826", "E3R",       3, 14},

    // Arcsky
    {DEVICE_TYPE_ARCSKY_X55,                        "1924", "01",        2, 10},

    // Amazon Prime Air
    {DEVICE_TYPE_AMAZON_PRIME_AIR_MK27_2,           "1786", "SN",        2, 8},

    // Quantum-Systems
    {DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R6,        "1656", "TYR02",     5, 13},
    {DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R6,        "1656", "TYR03",     5, 13},
    {DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R6,        "1656", "TYR06",     5, 13},
    {DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R7,        "1656", "TYR07",     5, 13},
    {DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R8,        "1656", "TYR08",     5, 13},
    {DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R10,       "1656", "TYR10",     5, 13},
    {DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R14,       "1656", "TYR14",     5, 13},

    // idME
    {DEVICE_TYPE_IDME_PRO,                          "1809", "30",        2, 9},

    // Pierce Aerospace
    {DEVICE_TYPE_PIERCE_AEROSPACE_B1,               "1893", "B1",        2, 15},

};

static UavDeviceSnItem uavDeviceSnTable_irrgular[] = {
    // Autel test
    {DEVICE_TYPE_AUTEL_EVO_LITE,                    "LTO",  " ",        3, 12},
};



UAV_DEVICE_TYPE RemoteIdModule::_GetDeviceTypeByMac(uint8_t* mac)
{
	int n = sizeof(uavDeviceMacTable)/sizeof(UavDeviceMacItem);
	int i;
	for(i = 0; i < n ; i++){
		//printf("mac:%hhx:%hhx:%hhx\n",mac[0],mac[1],mac[2]);
		//the high 3 bytes of mac indicate factory info.
		if(!memcmp(uavDeviceMacTable[i].mac,mac,3)){
			//printf("match => %d\n",uavDeviceMacTable[i].device_type);
			return uavDeviceMacTable[i].device_type;
		}
	}
    return DEVICE_TYPE_UNKNOWN;
}

UAV_DEVICE_TYPE_BY_SN RemoteIdModule::_GetDeviceTypeBySn(char* sn)
{
	int n = sizeof(uavDeviceSnTable)/sizeof(UavDeviceSnItem);
	int i;
    char *uavid = NULL;

    uavid = sn;
	for(i = 0; i < n ; i++)
    {       
        if((!strncmp(uavDeviceSnTable[i].manufacturer, uavid, MANUFACTURER_NAME_LEN)) && (!strncmp(uavDeviceSnTable[i].modelname, &uavid[MANUFACTURER_NAME_LEN + 1], uavDeviceSnTable[i].modellen)))
        {
			return uavDeviceSnTable[i].device_type;
		}    
	}
    for(i = 0; i < n ; i++)
    {
        if (!strncmp(uavDeviceSnTable_irrgular[i].manufacturer, uavid, uavDeviceSnTable_irrgular[i].modellen))
        {
            return uavDeviceSnTable_irrgular[i].device_type;
        }
    }
    return DEVICE_TYPE_UNKNOWN_UAV;
}




void RemoteIdModule::RemoteIdCmdRidCapture(RemoteId_Info *rid_info)
{
	RemoteIdKey key ;
	RemoteId_Info info = *rid_info;
	char mac_str[64];
	char *mac = (char*)rid_info->mac;
	int len = sprintf(mac_str,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	key.mac.assign(mac_str);
#if CFG_NO_USING_RID_CAPTURE
	key.deviceType = _GetDeviceTypeByMac(info.mac);
#else
    key.deviceType = _GetDeviceTypeBySn(info.uavId);
#endif

/*
	typedef struct _remoteId_Info
	{
		uint32_t pilotLongitude;
		uint32_t pilotLatitude;
		uint32_t uavLongitude;
		uint32_t uavLatitude;
		uint16_t uavAtmosAltitude;
		uint16_t uavEarthAltitude;
		uint16_t uavHeightAgl;
		uint16_t uavSpeedMultip;
		uint8_t uavDirection;
		uint8_t uavAbsolSpeed;
		uint8_t uavVertSpeed;
		uint32_t uavRecordTime;
		uint32_t uFreq;
		uint32_t uChn;
		uint8_t  mac[6];
		char dbm_ant_signal;
		char dbm_ant_noise;
		int32_t  rssi;
		uint32_t operatorIdType;
		char operatorId[OPERATORID_ID_SIZE+1];
		char uavId[UAV_ID_SIZE+1];
		char uasid[OPERATORID_ID_SIZE+1];
		char selfid[OPERATORID_ID_SIZE+1];
		double latitude;
		double longitude;
		float altitude;
		float direction;
		float speedHorizontal;
		float speedVertical;
	} RemoteId_Info;

*/
	if(info.longitude)info.uavLongitude = (uint32_t)info.longitude;
	if(info.latitude)info.uavLatitude = (uint32_t)info.latitude;	
	info.uavHeightAgl = (uint32_t)info.altitude;
	if(info.pilot_latitude)info.pilotLatitude = (uint32_t)info.pilot_latitude;
	if(info.pilot_longitude)info.pilotLongitude = (uint32_t)info.pilot_longitude;	
	info.uavDirection = (uint32_t)info.direction;	
	info.uavAbsolSpeed = (uint32_t)info.speedHorizontal;
	info.uavVertSpeed = (uint32_t)info.speedVertical;	
	info.uavAtmosAltitude = info.atmosaltitude;
    info.uavEarthAltitude = info.earthaltitude;

   
#ifdef FLAG_DEBUG_RUN_RID_PARSE_VALUE_PRINT
    printf("pilotLongitude: %d   pilotLatitude: %d   uavLongitude: %d   uavLatitude: %d   uavAtmosAltitude: %d   uavEarthAltitude: %d\r\n  \
        uavHeightAgl: %d  uavSpeedMultip: %d   uavDirection: %d   uavAbsolSpeed: %d   uavVertSpeed: %d   uavRecordTime: %d\r\n     \
        uFreq: %d   uChn: %d   mac: %s   dbm_ant_signal: %hhd   dbm_ant_noise: %hhd  rssi: %d   operatorIdType: %d   operatorId: %s\r\n   \
        uavId: %s   uasid: %s   selfid: %s   HeightType: %d\r\n",    \
        info.pilotLongitude, info.pilotLatitude, info.uavLongitude, info.uavLatitude, info.uavAtmosAltitude, info.uavEarthAltitude, info.uavHeightAgl, 
        info.uavSpeedMultip, info.uavDirection, info.uavAbsolSpeed, info.uavVertSpeed, info.uavRecordTime, info.uFreq, info.uChn,
        info.mac, info.dbm_ant_signal, info.dbm_ant_noise, info.rssi, info.operatorIdType, info.operatorId, info.uavId, 
        info.uasid, info.selfid, info.HeightType);
#endif

    std::lock_guard<std::mutex> lock(_remoteIdInfoMutex);

    uint16_t oldTargetNum = _remoteIdInfos.size();
    auto result = _remoteIdInfos.find(key);
    if (result != _remoteIdInfos.end())
    {
        result->second.uChn  = Remoteid_GetRemoteIdChnValue();
        result->second.uFreq = rid_info->uFreq;
        result->second.uavRecordTime = eap_get_cur_time_ms();
		_RefreshUavMesssgeInfo(info, result->second);
		//if (isHasSysInfo) _RefreshSysMesssgeInfo(info, result->second);
    }
    else
    {
    	info.uavRecordTime = eap_get_cur_time_ms();
        _remoteIdInfos.insert(make_pair(key, info));
    }
    if (ProductSwCfgGet().issupportwarningwhitelist())
    {
        _TargetWarningProcWhitelist();
    }
    else
    {
        _TargetWarningProc(oldTargetNum);
    }
    _rptCtrl.Refresh(oldTargetNum, _remoteIdInfos.size());

    _debugRecord(info);
}


void RemoteIdModule::BeaconFreamProcess(uint8_t* revBuff, uint32_t len)
{
    bool valid = false;
    RemoteIdKey key;
    RemoteId_Info info = {0};
    bool isHasUavInfo = false;
    bool isHasSysInfo = false;
	char mac_str[32];
	do
    {
        // key.deviceType = _GetDeviceType(revBuff);
        // if (UINT8_INVALID == key.deviceType) break;
        info.uChn = Remoteid_GetRemoteIdChnValue();
        memcpy(info.mac,(char*)(revBuff+10),6);
		char *mac = (char*)info.mac;
		sprintf(mac_str,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		key.mac.assign(mac_str);

        uint8_t* data = nullptr;
        int i = 0;
        for(; i < len; i++)
        {
            if (*(revBuff + i) == 0xDD)
            {
                data = revBuff + i;
                if ((*(data + 2) == 0xFA) && (*(data + 3) == 0x0B) && (*(data + 4) == 0xBC))
                {
                    break;
                }
            }
        }
        if (i == len || nullptr == data)
        {
            break;
        }

        uint16_t remoteLen = *(data + 1);
        uint16_t messageLen = *(data + 8);
        uint16_t messageNum = *(data + 9);
        if ((remoteLen - 8) != (messageLen * messageNum))
        {
            break;
        }

        uint8_t* messageStart = data + 10;

        bool isDataValid = false;
        int j = 0;
        for (; j < messageNum; j++)
        {
            if (((*(messageStart + j * messageLen) & 0xf0) == 0x0) && ((*(messageStart + j * messageLen + 1) & 0xf0) == 0x10))
            {
                info.uavRecordTime = eap_get_cur_time_ms();
                _ParseSnCodeMesssgeInfo(messageStart + j * messageLen, messageLen, key,info);  // get sn
                isDataValid = true;
            }
            else if ((*(messageStart + j * messageLen) & 0xf0) == 0x10)
            {
                _ParseUavMesssgeInfo(messageStart + j * messageLen, messageLen, info);  // get uav information
                isHasUavInfo = true;
            }
            else if ((*(messageStart + j * messageLen) & 0xf0) == 0x40)
            {
                _ParseSysMesssgeInfo(messageStart + j * messageLen, messageLen, info);  // get pilot informaiton
                isHasSysInfo = true;
            }
        }
        if (!isDataValid)
        {
            EAP_LOG_DEBUG("RemoteIdModule::BeaconFreamProcess isDataValid=%d, isHasUavInfo=%d, isHasSysInfo=%d\n",
                isDataValid, isHasUavInfo, isHasSysInfo);
            break;
        }
        valid = true;
    } while(0);

    key.deviceType = _GetDeviceTypeBySn(info.uavId);
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    Hal_Data inData = {0, nullptr};
    if (nullptr != halMgr) halMgr->OnSet(EAP_DRVCODE_REMOTEID_BUF_USE_FINISH, inData);

    if (!valid) 
    {
        return;
    }

    std::lock_guard<std::mutex> lock(_remoteIdInfoMutex);

    uint16_t oldTargetNum = _remoteIdInfos.size();
    auto result = _remoteIdInfos.find(key);
    if (result != _remoteIdInfos.end())
    {
        result->second.uChn = info.uChn;
        result->second.uFreq = Remoteid_GetRemoteIdChnValue();
        result->second.uavRecordTime = info.uavRecordTime;
        if (isHasUavInfo) _RefreshUavMesssgeInfo(info, result->second);
        if (isHasSysInfo) _RefreshSysMesssgeInfo(info, result->second);
    }
    else
    {
        info.uFreq = Remoteid_GetRemoteIdChnValue();
        _remoteIdInfos.insert(make_pair(key, info));
    }
    if (ProductSwCfgGet().issupportwarningwhitelist())
    {
        _TargetWarningProcWhitelist();
    }
    else
    {
        _TargetWarningProc(oldTargetNum);
    }
    _rptCtrl.Refresh(oldTargetNum, _remoteIdInfos.size());

    _debugRecord(info);
}

void RemoteIdModule::_debugRecord(const RemoteId_Info& info)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    static struct timeval last_tv = tv;

    glast_tv = tv;

    std::time_t currentTime = std::time(nullptr);
    char* timeString = std::ctime(&currentTime);

    std::uint64_t diffInMs = 0;
    if (tv.tv_sec > last_tv.tv_sec)
    {
        if (tv.tv_usec >= last_tv.tv_usec)
        {
            diffInMs = (tv.tv_sec - last_tv.tv_sec) * 1000 + (tv.tv_usec - last_tv.tv_usec) / 1000;
        }
        else
        {
            diffInMs = (tv.tv_sec - last_tv.tv_sec - 1) * 1000 + (1000000 + tv.tv_usec - last_tv.tv_usec) / 1000;            
        }
    }
    else if (tv.tv_sec == last_tv.tv_sec)
    {
        if (tv.tv_usec >= last_tv.tv_usec)
        {
            diffInMs = (tv.tv_usec - last_tv.tv_usec) / 1000;
        }
    }
    last_tv = tv;

    std::string prefix("normal ");
    if ((0 == diffInMs) || (diffInMs > 4000)) // 3 channels switch, each 700ms
    {
        prefix = "long ";
    	EAP_LOG_DEBUG("Error long diffInMs %d, current time:%s, second = %d, microseconds = %d, last second = %d, last microseconds = %d\r\n",
            diffInMs, timeString, tv.tv_sec, tv.tv_usec, last_tv.tv_sec, last_tv.tv_usec);
    }

    if (false == _debugLogEnable)
    {
        return;
    }

    std::ofstream debugOutputFile("/run/media/mmcblk0p1/remoteid_debug_record.txt", std::ofstream::app);
    if (debugOutputFile.is_open()) 
    {
        std::string strUavId(info.uavId);

        debugOutputFile << "***current time = " << timeString
                        << prefix << "diffInMs = " << std::dec << diffInMs
                        << ", second = " << tv.tv_sec
                        << ", microseconds = " << tv.tv_usec
                        << ", strUavId = " << strUavId
                        << ", pilotLongitude = " << info.pilotLongitude
                        << ", pilotLatitude = " << info.pilotLatitude
                        << ", uavLongitude = " << info.uavLongitude
                        << ", uavLatitude = " << info.uavLatitude
                        << ", uavSpeedMultip = " << info.uavSpeedMultip
                        << ", uavDirection = " << (std::uint32_t)info.uavDirection
                        << ", uavAbsolSpeed = " << (std::uint32_t)info.uavAbsolSpeed
                        << ", uavVertSpeed = " << (std::uint32_t)info.uavVertSpeed
                        << ", uavAtmosAltitude = " << info.uavAtmosAltitude
                        << ", uavEarthAltitude = " << info.uavEarthAltitude
                        << ", channel = " << info.uChn
                        << ", freq = " << info.uFreq
                        << std::endl << std::endl;

        debugOutputFile.close();
    }
    else
    {
    }
}

void RemoteIdModule::_debugRecordReport(const Rpt_RemoteInfo& info)
{
    if (false == _debugLogEnable)
    {
        return;
    }

    std::time_t currentTime = std::time(nullptr);
    char* timeString = std::ctime(&currentTime);

    std::ofstream debugOutputFile("/run/media/mmcblk0p1/remoteid_debug_record.txt", std::ofstream::app);
    if (debugOutputFile.is_open()) 
    {
        debugOutputFile << "@@@current time = " << timeString
                        << ", productType = " << info.productType
                        << ", droneName = " << info.droneName
                        << ", serialNum = " << info.serialNum
                        << ", droneLongitude = " << info.droneLongitude
                        << ", droneLatitude = " << info.droneLatitude
                        << ", droneHeight = " << info.droneHeight
                        << ", droneDirection = " << info.droneDirection
                        << ", droneSpeed = " << info.droneSpeed
                        << ", speedderection = " << (std::uint32_t)info.speedderection
                        << ", droneVerticalSpeed = " << (std::uint32_t)info.droneVerticalSpeed
                        << ", droneSailLongitude = " << (std::uint32_t)info.droneSailLongitude
                        << ", droneSailLatitude = " << info.droneSailLatitude
                        << ", uFreq = " << info.uFreq
                        << ", uDistance = " << info.uDistance
                        << ", dangerLevels = " << info.dangerLevels
                        << std::endl << std::endl;

        debugOutputFile.close();
    }
    else
    {
    }
}

void RemoteIdModule::_TargetWarningProc(uint16_t oldTargetNum)
{
    uint16_t newTargetNum = _remoteIdInfos.size();
    bool isTargetAppear = (0 == oldTargetNum && newTargetNum > 0);
    bool isTargetDisAppear = (0 < oldTargetNum && 0 == newTargetNum);
    if (isTargetAppear || isTargetDisAppear)
    {
        TargetWarning warning = {EAP_TARGET_SCENE_REMOTEID, (uint8_t)(isTargetAppear ? 1 : 0)};
        SendAsynData(EAP_CMD_TARGET_WARNING, (uint8_t*)&warning, sizeof(TargetWarning));
    }
}

void RemoteIdModule::_RefreshUavMesssgeInfo(const RemoteId_Info& from, RemoteId_Info& to)
{
	to.uavSpeedMultip = from.uavSpeedMultip;
	to.uavDirection = from.uavDirection;
	to.uavAbsolSpeed = from.uavAbsolSpeed;
	to.uavVertSpeed = from.uavVertSpeed;
	to.uavLatitude = from.uavLatitude;
	to.uavLongitude = from.uavLongitude;
	to.uavAtmosAltitude = from.uavAtmosAltitude;
	to.uavEarthAltitude = from.uavEarthAltitude;
	to.uavHeightAgl = from.uavHeightAgl;

	if(!to.uavId[0] && from.uavId[0]){
		memcpy(to.uavId,from.uavId,sizeof(from.uavId));
	}
}

void RemoteIdModule::_RefreshSysMesssgeInfo(const RemoteId_Info& from, RemoteId_Info& to)
{
	to.pilotLatitude = from.pilotLatitude;
	to.pilotLongitude = from.pilotLongitude;
}

void RemoteIdModule::_ParseUavMesssgeInfo(uint8_t* revBuff, uint16_t len, RemoteId_Info& info)
{
	info.uavSpeedMultip = (*(revBuff + 1)) & 0x01;
	info.uavDirection = *(revBuff + 2);
	info.uavAbsolSpeed = *(revBuff + 3);
	info.uavVertSpeed = *(revBuff + 4);
	info.uavLatitude = CONVERT_32_REV(*((uint32_t*)(revBuff + 5)));
	info.uavLongitude = CONVERT_32_REV(*((uint32_t*)(revBuff + 9)));
	info.uavAtmosAltitude = CONVERT_16_REV(*(uint16_t*)(revBuff + 13));
	info.uavEarthAltitude = CONVERT_16_REV(*(uint16_t*)(revBuff + 15));
	info.uavHeightAgl = CONVERT_16_REV(*(uint16_t*)(revBuff + 17));

	// EAP_LOG_DEBUG("Multip:%d   Dire:%d  bsolSpe:%d VertSp:%d Lati:%d Long:%d Atmos:%d Earth:%d Heig:%d \r\n",\
	// 		info.uavSpeedMultip, info.uavDirection, info.uavAbsolSpeed, info.uavVertSpeed, info.uavLatitude,
	// 		info.uavLongitude, info.uavAtmosAltitude, info.uavEarthAltitude, info.uavHeightAgl);
    RID_PRINTF("Multip:%d   Dire:%d  bsolSpe:%d VertSp:%d Lati:%d Long:%d Atmos:%d Earth:%d Heig:%d \r\n",\
	 		info.uavSpeedMultip, info.uavDirection, info.uavAbsolSpeed, info.uavVertSpeed, info.uavLatitude,
	 		info.uavLongitude, info.uavAtmosAltitude, info.uavEarthAltitude, info.uavHeightAgl);
}


void RemoteIdModule::_ParseSysMesssgeInfo(uint8_t* revBuff, uint16_t len, RemoteId_Info& info)
{
	info.pilotLatitude = CONVERT_32_REV(*((uint32_t*)(revBuff + 2)));
	info.pilotLongitude = CONVERT_32_REV(*((uint32_t*)(revBuff + 6)));

	// EAP_LOG_DEBUG("PiLat:%d   PiLon:%d\r\n", info.pilotLatitude, info.pilotLongitude);
    RID_PRINTF("PiLat:%d   PiLon:%d\r\n", info.pilotLatitude, info.pilotLongitude);
}

void RemoteIdModule::_ParseSnCodeMesssgeInfo(uint8_t* revBuff, uint16_t len, RemoteIdKey& key,RemoteId_Info& info)
{
    memcpy(info.uavId,(char*)(revBuff + 2),20);
	int i;
	// EAP_LOG_DEBUG("SnCode:");
    RID_PRINTF("SnCode:");
	for (i = 0; i < 20; i++)
	{
		// EAP_LOG_DEBUG("%c ", revBuff[2 + i]);
        RID_PRINTF("%c ", revBuff[2 + i]);
	}
	// EAP_LOG_DEBUG("\r\n");
    RID_PRINTF("\r\n");
}

void RemoteIdModule::OldRemoteIdInfo()
{
    std::lock_guard<std::mutex> lock(_remoteIdInfoMutex);

    uint16_t oldTargetNum = _remoteIdInfos.size();
	uint32_t curTime = eap_get_cur_time_ms();
    for (auto iter = _remoteIdInfos.begin(); iter != _remoteIdInfos.end();)
    {
        if (curTime - iter->second.uavRecordTime >= EAP_MAX_KEEP_REMOTEID_TIME)
        {
            _remoteIdInfos.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
    if (ProductSwCfgGet().issupportwarningwhitelist())
    {
        _TargetWarningProcWhitelist();
    }
    else
    {
        _TargetWarningProc(oldTargetNum);
    }
}

bool RemoteIdModule::IsActiveTargetInChannel(const std::uint32_t& channel)
{
    std::lock_guard<std::mutex> lock(_remoteIdInfoMutex);

    for (auto iter = _remoteIdInfos.begin(); iter != _remoteIdInfos.end(); iter++)
    {
        const auto& info = iter->second;
        if (info.uChn == channel)
        {
            return true;
        }
    }

    return false;
}

int16_t RemoteIdModule::FillRemoteIdInfo(uint8_t* buf, uint16_t bufLen)
{
    std::lock_guard<std::mutex> lock(_remoteIdInfoMutex);

	int n = 0;
    int8_t count = _remoteIdInfos.size();
    if (!_rptCtrl.Report(count)) return -1;
    int16_t dataLen = EAP_SN_LENGTH + 1 + count * sizeof(Rpt_RemoteInfo);
    if (dataLen > bufLen) return -1;

    uint8_t *data = buf;
    memcpy(buf, EapGetSys().GetSnName(), EAP_SN_LENGTH);
    data += EAP_SN_LENGTH;
    data++;//skip count position, fill it later.
    for (auto& entry : _remoteIdInfos)
    {
        Rpt_RemoteInfo* info = (Rpt_RemoteInfo *)data;

		RemoteId_Info &ridInfo = entry.second;

		if(ridInfo.uavId[0] == 0x0){
			continue;
		}
		n++;
        info->productType = entry.first.deviceType;
        int16_t droneAltitude = _GetDroneAltitude(info->productType, entry.second.uavHeightAgl);
        int16_t droneSpeed = _GetDroneSpeed(info->productType, entry.second);
        int16_t droneVerticalSpeed = _GetDroneVerticalSpeed(info->productType, entry.second);
        // _FillDroneName(info->productType, info->droneName);
        _FillDroneNameBySn(info->productType, info->droneName);
        memcpy(info->serialNum, ridInfo.uavId, strlen(ridInfo.uavId));
		info->serialNum[strlen(ridInfo.uavId)] = 0;
		//printf("**FillRemoteIdInfo:sn:%s\n",info->serialNum);
        _FillPilotInfo(info->productType, entry.second, *info);
        info->droneLongitude     = (int32_t)CONVERT_32_REV(entry.second.uavLongitude);
        info->droneLatitude      = (int32_t)CONVERT_32_REV(entry.second.uavLatitude);
        info->droneHeight        = (int16_t)CONVERT_16_REV(droneAltitude);
        info->droneDirection     = (int16_t)CONVERT_16_REV(entry.second.uavDirection);
        info->droneSpeed         = (int16_t)CONVERT_16_REV(droneSpeed);
        info->speedderection     = (int8_t)CONVERT_16_REV(droneSpeed);
        info->droneVerticalSpeed = (int16_t)CONVERT_16_REV(droneVerticalSpeed);
        info->uFreq              = CONVERT_32_REV(entry.second.uFreq);
        // printf("droneLongitude %d droneLatitude %d droneHeight %d droneDirection %d [%s:%d]\n"
		// 				,info->droneLongitude,info->droneLatitude,info->droneHeight,info->droneDirection,__func__,__LINE__);
        data += sizeof(Rpt_RemoteInfo);
    }
	buf[EAP_SN_LENGTH] = n;
	dataLen = data - buf;
    _rptCtrl.Clear();
    return dataLen;
}

void RemoteIdModule::_FillDroneName(uint8_t type, char* droneName)
{
    if (DEVICE_TYPE_LITE == type)
    {
        char remote_lite[] =  "lite rid1";
        memcpy(droneName, remote_lite, sizeof(remote_lite));
    }
    else if (DEVICE_TYPE_PARROT == type)
    {
        char remote_parrot[] =  "parrot rid1";
        memcpy(droneName, remote_parrot, sizeof(remote_parrot));
    }
    else
    {
        char remote_other[] =  "unknow rid1";
        memcpy(droneName, remote_other, sizeof(remote_other));
    }
}
void RemoteIdModule::_FillDroneNameBySn(uint8_t type, char* droneName)
{   
    // Autel
    if (DEVICE_TYPE_AUTEL_EVO_LITE == type)
    {
        char displayName[] = "Autel EVO Lite";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AUTEL_EVO_NANO == type)
    {
        char displayName[] = "Autel EVO Nano";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AUTEL_EVO_MAX == type)
    {
        char displayName[] = "Autel EVO Max";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AUTEL_EVO_DRAGFISH_PRO == type)
    {
        char displayName[] = "Autel Dragonfish Pro";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AUTEL_EVO_DRAGFISH_LITE == type)
    {
        char displayName[] = "Autel Dragonfish Lite";
        memcpy(droneName, displayName, sizeof(displayName));
    }    
    else if (DEVICE_TYPE_AUTEL_EVO_DRAGFISH_STD == type)
    {
        char displayName[] = "Autel Dragonfish Standard";
        memcpy(droneName, displayName, sizeof(displayName));
    } 
    else if (DEVICE_TYPE_AUTEL_EVO_II == type)
    {
        char displayName[] = "Autel EVO II";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Parrot
    else if (DEVICE_TYPE_PARROT_ANAFI_USA == type)
    {
        char displayName[] = "Parrot Anafi USA";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_PARROT_ANAFI_AI == type)
    {
        char displayName[] = "Parrot Anafi Ai";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Skydio
    else  if (DEVICE_TYPE_SKYDIO_X2E == type)
    {
        char displayName[] = "Skydio X2E";
        memcpy(droneName, displayName, sizeof(displayName));
    }    
    else if (DEVICE_TYPE_SKYDIO_2 == type)
    {
        char displayName[] = "Skydio 2";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Percepto AirMax
    else if (DEVICE_TYPE_PERCEPTO_AIRMAX == type)
    {
        char displayName[] = "Percepto AirMax";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Microdrones
    else if (DEVICE_TYPE_MICRODRONES_MD4_1000 == type)
    {
        char displayName[] = "Microdrones md4-1000";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_MICRODRONES_MD4_3000 == type)
    {
        char displayName[] = "Microdrones md4-3000";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_MICRODRONES_EASY_ONE == type)
    {
        char displayName[] = "Microdrones EasyOne";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // WingtraOne
    else if (DEVICE_TYPE_WINGTRA_GEN_II == type)
    {
        char displayName[] = "WingtraOne Gen II";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // AgEagle(senseFly)
    else if (DEVICE_TYPE_AGEAGLE_EBEE_X_SERIES == type)
    {
        char displayName[] = "AgEagle eBee X series";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Sony
    else if (DEVICE_TYPE_SONY_ARS_S1 == type)
    {
        char displayName[] = "Sony ARS-S1";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Commaris
    else if (DEVICE_TYPE_COMMARIS_SEEKER == type)
    {
        char displayName[] = "Commaris SEEKER";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Elsight Ltd.
    else if (DEVICE_TYPE_ELSIGHT_HNF4W == type)
    {
        char displayName[] = "Elsight HNF4W";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Dronetag
    else if (DEVICE_TYPE_DRONETAG_BEACON == type)
    {
        char displayName[] = "Dronetag Beacon";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_DRONETAG_BS == type)
    {
        char displayName[] = "Dronetag BS";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_DRONETAG_DRI == type)
    {
        char displayName[] = "Dronetag DRI";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_DRONETAG_MINI == type)
    {
        char displayName[] = "Dronetag Mini";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // EXO
    else if (DEVICE_TYPE_EXO_X7_RANGER_PLUS == type)
    {
        char displayName[] = "EXO X7 Ranger Plus";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_EXO_BLACKHAWK_3_PRO == type)
    {
        char displayName[] = "EXO Blackhawk 3 PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Zephyr Systems
    else if (DEVICE_TYPE_ZEPHYR_SYSTEMS_MODULE_100 == type)
    {
        char displayName[] = "Zephyr Module 100";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_ZEPHYR_SYSTEMS_ARK_350 == type)
    {
        char displayName[] = "Zephyr ARK-350";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_ZEPHYR_SYSTEMS_OKO_250 == type)
    {
        char displayName[] = "Zephyr OKO-250";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Inspired Flight
    else if (DEVICE_TYPE_INSPIRED_FLIGHT_IF1200A == type)
    {
        char displayName[] = "Inspired Flight IF1200A";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // BlueMark
    else if (DEVICE_TYPE_BLUEMARK_DB100 == type)
    {
        char displayName[] = "BlueMark db100";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_BLUEMARK_DB121 == type)
    {
        char displayName[] = "BlueMark db121";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_BLUEMARK_DB121PCB == type)
    {
        char displayName[] = "BlueMark db121pcb";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_BLUEMARK_DB122FPV == type)
    {
        char displayName[] = "BlueMark db122fpv";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_BLUEMARK_DB120 == type)
    {
        char displayName[] = "BlueMark db120";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Drone Defence
    else if (DEVICE_TYPE_DRONE_DEFENCE_AEROPING == type)
    {
        char displayName[] = "Drone Defence AeroPing";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Harris Aerial
    else if (DEVICE_TYPE_HARRIS_AERIAL_CARRIER_H6 == type)
    {
        char displayName[] = "Harris Carrier H6";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HARRIS_AERIAL_CARRIER_H8E == type)
    {
        char displayName[] = "Harris Carrier H8E";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Wing Aviation LLC
    else if (DEVICE_TYPE_WING_AVIATION_HUMMINGBIRD_V2 == type)
    {
        char displayName[] = "Wing Avi Hummingbird v2";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // SJRC
    else if (DEVICE_TYPE_SJRC_F5S_PRO == type)
    {
        char displayName[] = "SJRC F5s PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    } 
    else if (DEVICE_TYPE_SJRC_F5S_PRO_PLUS == type)
    {
        char displayName[] = "SJRC F5s Pro+";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_SJRC_F7_4K_PRO == type)
    {
        char displayName[] = "SJRC F7 4K PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_SJRC_F11_4K_PRO == type)
    {
        char displayName[] = "SJRC F11 4K PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_SJRC_F11S_4K_PRO == type)
    {
        char displayName[] = "SJRC F11s 4K PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_SJRC_F22_4K_PRO == type)
    {
        char displayName[] = "SJRC F22 4K PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_SJRC_F22S_4K_PRO == type)
    {
        char displayName[] = "SJRC F22s 4K PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Aurelia
    else if (DEVICE_TYPE_AURELIA_TAROT_650 == type)
    {
        char displayName[] = "Aurelia Tarot 650";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AURELIA_X4 == type)
    {
        char displayName[] = "Aurelia X4";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AURELIA_X6_STANDARD == type)
    {
        char displayName[] = "Aurelia X6 Standard";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AURELIA_X6_PRO == type)
    {
        char displayName[] = "Aurelia X6 Pro";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AURELIA_X6_MAX == type)
    {
        char displayName[] = "Aurelia X6 MAX";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AURELIA_X6_PRO_V2 == type)
    {
        char displayName[] = "Aurelia X6 Pro V2";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AURELIA_X8_STANDARD == type)
    {
        char displayName[] = "Aurelia X8 Standard";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AURELIA_X8_MAX == type)
    {
        char displayName[] = "Aurelia X8 MAX";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Vision Aerial
    else if (DEVICE_TYPE_VISION_AERIAL_DRONE == type)
    {
        char displayName[] = "Vision Aerial Drone";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Watts Innovations
    else if (DEVICE_TYPE_WATTS_INNOVATIONS_PRISM_SKY == type)
    {
        char displayName[] = "Watts PRISM Sky";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_WATTS_INNOVATIONS_PRISM_LITE == type)
    {
        char displayName[] = "Watts PRISM Lite";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // TRIPLEFINE
    else if (DEVICE_TYPE_TRIPLEFINE_TF35_PRO == type)
    {
        char displayName[] = "TRIPLEFINE TF35 PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_TRIPLEFINE_TF_15EX == type)
    {
        char displayName[] = "TRIPLEFINE TF-15EX";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Leica Geosystems
    else if (DEVICE_TYPE_LEICA_GEOSYSTEMS_BLK2FLY == type)
    {
        char displayName[] = "Leica Geosys BLK2FLY";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Bwine
    else if (DEVICE_TYPE_BWINE_F7GB2 == type)
    {
        char displayName[] = "Bwine F7GB2";
        memcpy(droneName, displayName, sizeof(displayName));
    }    

    // XM2 Labs
    else if (DEVICE_TYPE_XM2_LABS_XTD_120 == type)
    {
        char displayName[] = "XM2 Labs XTD-120";
        memcpy(droneName, displayName, sizeof(displayName));
    }     

    // Drone Volt
    else if (DEVICE_TYPE_HERCULES_20 == type)
    {
        char displayName[] = "Drone Volt Hercules 20";
        memcpy(droneName, displayName, sizeof(displayName));
    } 

    // Flytrex
    else if (DEVICE_TYPE_FLYTREX_FTX_M600P == type)
    {
        char displayName[] = "Flytrex FTX-M600P";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Hylio
    else if (DEVICE_TYPE_HYLIO_AG_210 == type)
    {
        char displayName[] = "Hylio AG-210";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HYLIO_AG_216 == type)
    {
        char displayName[] = "Hylio AG-216";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HYLIO_AG_230 == type)
    {
        char displayName[] = "Hylio AG-230";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HYLIO_AG_272 == type)
    {
        char displayName[] = "Hylio AG-272";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Coretronic
    else if (DEVICE_TYPE_CORETRONIC_SIRAS == type)
    {
        char displayName[] = "Coretronic SIRAS";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Ascent AeroSystems
    else if (DEVICE_TYPE_ASCENT_AEROSYSTEMS_NX30 == type)
    {
        char displayName[] = "Ascent AeroSys NX30";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_ASCENT_AEROSYSTEMS_SPIRIT == type)
    {
        char displayName[] = "Ascent AeroSys Spirit";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // uAvionix
    else if (DEVICE_TYPE_UAVIONIX_PINGRID == type)
    {
        char displayName[] = "uAvionix pingRID";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // XAG
    else if (DEVICE_TYPE_XAG_P100 == type)
    {
        char displayName[] = "XAG P100";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_XAG_P100_PRO == type)
    {
        char displayName[] = "XAG P100 Pro";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Event38 Unmanned Systems
    else if (DEVICE_TYPE_EVENT38_UNMANNED_SYSTEMS_E455 == type)
    {
        char displayName[] = "Event38 Unmanned E455";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_EVENT38_UNMANNED_SYSTEMS_E400 == type)
    {
        char displayName[] = "Event38 Unmanned E400";
        memcpy(droneName, displayName, sizeof(displayName));
    }   

    // Hoverfly Technologies
    else if (DEVICE_TYPE_HOVERFLY_SENTRY_SPECTRE == type)
    {
        char displayName[] = "Hoverfly Sentry/Spectre";
        memcpy(droneName, displayName, sizeof(displayName));
    }  

    // SkyRanger / SkyRaider
    else if (DEVICE_TYPE_SKYRANGER_SKYRAIDER_R70_R80D == type)
    {
        char displayName[] = "SkyRanger R70/R80D";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // UAS
    else if (DEVICE_TYPE_UAS_RC == type)
    {
        char displayName[] = "UAS RC";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Censys Technologies
    else if (DEVICE_TYPE_CENSYS_SENTAERO_5 == type)
    {
        char displayName[] = "Censys Sentaero 5";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Flightware
    else if (DEVICE_TYPE_FLIGHTWARE_EDGE_130 == type)
    {
        char displayName[] = "Flightware Edge 130";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Cider Copter
    else if (DEVICE_TYPE_CIDER_COPTER_H10 == type)
    {
        char displayName[] = "Cider Copter H10";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Teal Drones
    else if (DEVICE_TYPE_TEAL_DRONES_TEAL_2 == type)
    {
        char displayName[] = "Teal Drones Teal 2";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Camflite
    else if (DEVICE_TYPE_CAMFLITE_AURORA_ASCENT == type)
    {
        char displayName[] = "Camflite Aurora/Ascent";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // SmartDrone
    else if (DEVICE_TYPE_SMARTDRONE_DISCOVERY_2 == type)
    {
        char displayName[] = "SmartDrone Discovery 2";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // WISPR
    else if (DEVICE_TYPE_WISPR_RANGER_PRO_1100 == type)
    {
        char displayName[] = "WISPR Ranger Pro 1100";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_WISPR_RANGER_ELITE == type)
    {
        char displayName[] = "WISPR Ranger Elite";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Holy Stone
    else if (DEVICE_TYPE_HOLY_STONE_HSRID01 == type)
    {
        char displayName[] = "Holy Stone HSRID01";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HOLY_STONE_HS720G == type)
    {
        char displayName[] = "Holy Stone HS720G";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HOLY_STONE_HS720R == type)
    {
        char displayName[] = "Holy Stone HS720R";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HOLY_STONE_HS720 == type)
    {
        char displayName[] = "Holy Stone HS720";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HOLY_STONE_HS700E == type)
    {
        char displayName[] = "Holy Stone HS700E";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HOLY_STONE_HS720E == type)
    {
        char displayName[] = "Holy Stone HS720E";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_HOLY_STONE_HS600 == type)
    {
        char displayName[] = "Holy Stone HS600";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Workhorse Aero
    else if (DEVICE_TYPE_WORKHORSE_AERO_WA4_100 == type)
    {
        char displayName[] = "Workhorse Aero WA4-100";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Draganfly Innovations
    else if (DEVICE_TYPE_DRAGANFLY_COMMANDER3_XL == type)
    {
        char displayName[] = "Draganfly Commander3 XL";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Ruko
    else if (DEVICE_TYPE_RUKO_F11GIM == type)
    {
        char displayName[] = "Ruko F11GIM";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_RUKO_U11S == type)
    {
        char displayName[] = "Ruko U11S";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_RUKO_U11PRO == type)
    {
        char displayName[] = "Ruko U11PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_RUKO_F11 == type)
    {
        char displayName[] = "Ruko F11";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_RUKO_F11GIM2 == type)
    {
        char displayName[] = "Ruko F11GIM2";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_RUKO_R111 == type)
    {
        char displayName[] = "Ruko R111";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_RUKO_F11PRO == type)
    {
        char displayName[] = "Ruko F11PRO";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Matternet
    else if (DEVICE_TYPE_MATTERNET_M2_BLOCK_2 == type)
    {
        char displayName[] = "Matternet M2 Block 2";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Skyfront
    else if (DEVICE_TYPE_SKYFRONT_PERIMETER_8 == type)
    {
        char displayName[] = "Skyfront Perimeter 8";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Tucok
    else if (DEVICE_TYPE_TUCOK_193MAX2S == type)
    {
        char displayName[] = "Tucok 193MAX2S";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_TUCOK_T70 == type)
    {
        char displayName[] = "Tucok T70";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_TUCOK_012S == type)
    {
        char displayName[] = "Tucok 12S";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Aero Systems West
    else if (DEVICE_TYPE_AERO_SYS_WEST_INTER_LIFT == type)
    {
        char displayName[] = "Aero Sys West Inter Lift";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_AERO_SYS_WEST_HEAVY_LIFT == type)
    {
        char displayName[] = "Aero Sys West Heavy Lift";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Lucid Bots
    else if (DEVICE_TYPE_LUCID_BOTS_SHERPA == type)
    {
        char displayName[] = "Lucid Bots Sherpa";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // PrecisionVision
    else if (DEVICE_TYPE_PRECISIONVISION_PV40X == type)
    {
        char displayName[] = "PrecisionVision PV40X";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Drone-Clone Xperts
    else if (DEVICE_TYPE_DRONE_CLONE_XPERTS_LLIMITLESS_4S == type)
    {
        char displayName[] = "Drone-Clone LIMITLESS 4S";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Spektrum
    else if (DEVICE_TYPE_SPEKTRUM_SPMA9500_SKYID == type)
    {
        char displayName[] = "Spektrum SPMA9500_SKYID";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // C100
    else if (DEVICE_TYPE_C100_BLOCK_1C == type)
    {
        char displayName[] = "C100 Block 1C";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Zipline
    else if (DEVICE_TYPE_ZIPLINE_SPARROW == type)
    {
        char displayName[] = "Zipline Sparrow";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Veeniix
    else if (DEVICE_TYPE_VEENIIX_V11 == type)
    {
        char displayName[] = "Veeniix V11";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Fotokite
    else if (DEVICE_TYPE_FOTOKITE_SIGMA == type)
    {
        char displayName[] = "Fotokite Sigma";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // DIY-D LLC
    else if (DEVICE_TYPE_DIY_D_LLC_ESP32_C3_MINI_1 == type)
    {
        char displayName[] = "DIY-D ESP32-C3-MINI-1";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Xil
    else if (DEVICE_TYPE_XIL_193E == type)
    {
        char displayName[] = "Xil 193E";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Sentera
    else if (DEVICE_TYPE_SENTERA_PHX == type)
    {
        char displayName[] = "Sentera PHX";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // ELIOS 3 UAV
    else if (DEVICE_TYPE_ELIOS_3_UAV_107000 == type)
    {
        char displayName[] = "ELIOS 3 UAV 107000";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Arcsky
    else if (DEVICE_TYPE_ARCSKY_X55 == type)
    {
        char displayName[] = "Arcsky X55";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Amazon Prime Air
    else if (DEVICE_TYPE_AMAZON_PRIME_AIR_MK27_2 == type)
    {
        char displayName[] = "Amazon Prime Air MK27-2";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Quantum-Systems
    else if (DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R6 == type)
    {
        char displayName[] = "Quantum Sys Trinity R6";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R7 == type)
    {
        char displayName[] = "Quantum Sys Trinity R7";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R8 == type)
    {
        char displayName[] = "Quantum Sys Trinity R8";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R10 == type)
    {
        char displayName[] = "Quantum Sys Trinity R10";
        memcpy(droneName, displayName, sizeof(displayName));
    }
    else if (DEVICE_TYPE_QUANTUM_SYSTEMS_TRINITY_R14 == type)
    {
        char displayName[] = "Quantum Sys Trinity R14";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // idME
    else if (DEVICE_TYPE_IDME_PRO == type)
    {
        char displayName[] = "idME Pro";
        memcpy(droneName, displayName, sizeof(displayName));
    }

    // Pierce Aerospace
    else if (DEVICE_TYPE_PIERCE_AEROSPACE_B1 == type)
    {
        char displayName[] = "Pierce Aerospace B1";
        memcpy(droneName, displayName, sizeof(displayName));
    }


    else
    {
        char displayName[] = "unknow rid1";
        memcpy(droneName, displayName, sizeof(displayName));
    } 
}

int16_t RemoteIdModule::_GetDroneAltitude(uint8_t type, uint16_t uavHeightAgl)
{
#if CFG_NO_USING_RID_CAPTURE
   if (DEVICE_TYPE_AUTEL_EVO_LITE == type)
    {
        return (int16_t)((2000 - uavHeightAgl) / 2 * 10);
    }
    else if (DEVICE_TYPE_PARROT_ANAFI_AI == type)
    {
        return (int16_t)((uavHeightAgl - 2000) / 2 * 10);
    }
    return 0;
#else
    return  uavHeightAgl;
#endif
}

int16_t RemoteIdModule::_GetDroneSpeed(uint8_t type, const RemoteId_Info& info)
{
#if CFG_NO_USING_RID_CAPTURE
    if (DEVICE_TYPE_AUTEL_EVO_LITE == type)
    {
		if(info.uavSpeedMultip == 0x00)
		{
            return (int16_t)(info.uavAbsolSpeed / 4 * 100);
		}
		else
		{
            return (int16_t)(info.uavAbsolSpeed / 4 * 300);
		}
    }
    else if (DEVICE_TYPE_PARROT == type)
    {
		if(info.uavSpeedMultip == 0x00)
		{
           return (int16_t)(info.uavAbsolSpeed / 4 * 100);
		}
		else
		{
           return (int16_t)(info.uavAbsolSpeed / 4 * 300);
		}
    }
    return 0;
#else    
    return info.uavAbsolSpeed;
#endif    
}

int16_t RemoteIdModule::_GetDroneVerticalSpeed(uint8_t type, const RemoteId_Info& info)
{
#if CFG_NO_USING_RID_CAPTURE
    if (DEVICE_TYPE_PARROT_ANAFI_AI == type)
    {
		if(info.uavSpeedMultip == 0x00)
		{
           return (int16_t)(info.uavAbsolSpeed / 2 * 100);
		}
		else
		{
           return (int16_t)(info.uavAbsolSpeed / 2 * 300);
		}
    }
    return 0;
# else
    return info.uavAbsolSpeed;
#endif 
    
}

void RemoteIdModule::_FillPilotInfo(uint8_t type, const RemoteId_Info& info, Rpt_RemoteInfo& fillInfo)
{
    if (DEVICE_TYPE_AUTEL_EVO_LITE == type || DEVICE_TYPE_PARROT_ANAFI_AI == type)
    {
        uint32_t pilotLongitude = (uint32_t)(info.pilotLongitude);
        fillInfo.droneSailLongitude = (int32_t)CONVERT_32_REV(pilotLongitude);
        uint32_t pilotLatitude = (uint32_t)(info.pilotLatitude);
        fillInfo.droneSailLatitude = (int32_t)CONVERT_32_REV(pilotLatitude);
    }
}

uint16_t RemoteIdModule::GetRemoteIdNum()
{
    std::lock_guard<std::mutex> lock(_remoteIdInfoMutex);

    return _remoteIdInfos.size();
}

void RemoteIdModule::Clear()
{
    std::lock_guard<std::mutex> lock(_remoteIdInfoMutex);

    _remoteIdInfos.clear();
}

void RemoteIdModule::RequestShowInfo()
{
    SendAsynCmdCode(EAP_CMD_SHOW_REMOTEID_MODULE);
}

void RemoteIdModule::ShowInfo(uint8_t level)
{
    std::lock_guard<std::mutex> lock(_remoteIdInfoMutex);

    ModuleReal::ShowInfo(level);
    cout << EAP_BLANK_PREFIX(level) << "RemoteIdNum: " << _remoteIdInfos.size();
    cout << EAP_BLANK_PREFIX(level) << "RptIntervalTimes: " << (uint16_t)_rptCtrl.GetInterval();
    cout << EAP_BLANK_PREFIX(level) << "_remoteIdInfos: ";
    int i = 0;
    for (auto remoteId : _remoteIdInfos)
    {
        cout << EAP_BLANK_PREFIX(level + 1) << "_remoteIdInfos[" << i << "]: ";
        cout << EAP_BLANK_PREFIX(level + 2) << "Key: ";
        cout << EAP_BLANK_PREFIX(level + 3) << "deviceType: " << remoteId.first.deviceType;
        cout << EAP_BLANK_PREFIX(level + 3) << "mac: " << remoteId.first.mac;
        cout << EAP_BLANK_PREFIX(level + 2) << "Value: ";
        cout << EAP_BLANK_PREFIX(level + 3) << "uavSnCode: " << remoteId.second.uavId;
        cout << EAP_BLANK_PREFIX(level + 3) << "pilotLongitude: " << remoteId.second.pilotLongitude;
        cout << EAP_BLANK_PREFIX(level + 3) << "pilotLatitude: " << remoteId.second.pilotLatitude;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavLongitude: " << remoteId.second.uavLongitude;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavLatitude: " << remoteId.second.uavLatitude;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavAtmosAltitude: " << remoteId.second.uavAtmosAltitude;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavEarthAltitude: " << remoteId.second.uavEarthAltitude;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavHeightAgl: " << remoteId.second.uavHeightAgl;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavSpeedMultip: " << remoteId.second.uavSpeedMultip;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavDirection: " << (uint16_t)remoteId.second.uavDirection;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavAbsolSpeed: " << (uint16_t)remoteId.second.uavAbsolSpeed;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavVertSpeed: " << (uint16_t)remoteId.second.uavVertSpeed;
        cout << EAP_BLANK_PREFIX(level + 3) << "uavRecordTime: " << remoteId.second.uavRecordTime;
        cout << EAP_BLANK_PREFIX(level + 3) << "uFreq: " << remoteId.second.uFreq;
        ++i;
    }
}

void RemoteIdModule::_TargetWarningProcWhitelist()
{
    uint32_t oldEnemyNum = _enemyTargetNum;
    vector<string> list = EapGetSys().GetWhitelistManager()->GetWhitelist();
    bool inWhitelist = false;

    EAP_LOG_DEBUG("enter: oldEnemyNum=%d, whitelist size=%d", oldEnemyNum, list.size());
   _enemyTargetNum = _remoteIdInfos.size();
    for (const auto &info : _remoteIdInfos)
    {
        inWhitelist = false;
        for (vector<string>::iterator it = list.begin(); it != list.end(); it++)
        {
            if (0 == it->compare((const char*)(info.second.uavId)))
            {
                inWhitelist = true;
                EAP_LOG_DEBUG("RemoteID in Whitelist: serial_num=%s, _enemyTargetNum=%d", info.second.uavId, _enemyTargetNum);
                break;
            }
        }
        if (inWhitelist)
        {
            _enemyTargetNum--;
        }
    }
 
    bool isTargetAppear = (0 == oldEnemyNum && 0 < _enemyTargetNum);
    bool isTargetDisAppear = (0 < oldEnemyNum && 0 == _enemyTargetNum);
    if (isTargetAppear || isTargetDisAppear)
    {
        TargetWarning warning = {EAP_TARGET_SCENE_REMOTEID, (uint8_t)(isTargetAppear ? 1 : 0)};
        SendAsynData(EAP_CMD_TARGET_WARNING, (uint8_t*)&warning, sizeof(TargetWarning));
    }
}


