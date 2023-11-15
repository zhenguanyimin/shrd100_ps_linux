
#include "ad9361_config.h"
#include "zynq_bram.h"
#include <math.h>
// #include "xgpio.h"
// #include "xscugic.h"
// #include "xparameters.h"

#include "../../../srv/log/log.h"
#include "../output/output.h"


// #include "freeRTOSConfig.h"
#define AD9361_FRE_HOP_TIME (124)
// extern XScuGic xInterruptController;
uint8_t uGps915mHitFlag = Freq_TX_GPS_915m;

static uint8_t Ad9361GainBuf[5] = {0x32, 0x28, 0x00, 0x00, 0x00};

enum Freq_CFG_ADDR
{
	fre_433m_addr = 0x00,
	//	fre_1931m_addr = 0x00
	fre_842m_addr,
	fre_915m_addr,
	fre_1433m_addr,
//	fre_1185m_addr,
	fre_1225m_addr,
//	fre_1187m_addr,
//	fre_1212m_addr,
//	fre_1253m_addr,
	fre_1265m_addr,
	fre_1572m_addr,
	fre_1597m_addr,
	fre_max,
};

enum Freq_CFG1_ADDR
{
	fre1_842m_addr,
	fre1_915m_addr,
	fre1_1187m_addr,
	fre1_1212m_addr,
	fre1_1253m_addr,
	fre1_1265m_addr,
	fre1_1572m_addr,
	fre1_1597m_addr,
	fre1_max,
};

enum Freq_CFG2_ADDR
{
	fre2_828m_addr,
	fre2_838m_addr,
	fre2_848m_addr,
	fre2_858m_addr,
	fre2_900m_addr,
	fre2_910m_addr,
	fre2_920m_addr,
	fre2_930m_addr,
	fre2_max,
};

enum Freq_CFG3_ADDR
{
	fre3_1087m_addr,
	fre3_1127m_addr,
	fre3_1167m_addr,
	fre3_1207m_addr,
	fre3_1247m_addr,
	fre3_1287m_addr,
	fre3_1327m_addr,
	fre3_1367m_addr,
	fre3_max,
};

enum Freq_CFG4_ADDR
{
//	fre4_433m_addr,
	fre4_842m_addr,
	fre4_915m_addr,
	fre4_1775m_addr,
	fre4_1185m_addr,
	fre4_1225m_addr,
	fre4_1265m_addr,
	fre4_1572m_addr,
	fre4_1597m_addr,
	fre4_max,
};

RfTxProperty_t RfTxProperty[fre_max] =
{
		[fre_433m_addr]   = {ENABLE, 0},
		[fre_1433m_addr]  = {ENABLE, 0},
		[fre_842m_addr]   = {ENABLE, 0},
		[fre_915m_addr]   = {ENABLE, 0},
		[fre_1225m_addr]   = {ENABLE, 1},
		[fre_1265m_addr]   = {ENABLE, 1},
		[fre_1572m_addr]  = {ENABLE, 1},
		[fre_1597m_addr]  = {ENABLE, 1}
};

RfTxProperty_t RfTxProperty1[fre1_max] =
{
		[fre1_842m_addr]   = {ENABLE, 0},
		[fre1_915m_addr]   = {ENABLE, 0},
		[fre1_1187m_addr]  = {ENABLE, 1},
		[fre1_1212m_addr]  = {ENABLE, 1},
		[fre1_1253m_addr]  = {ENABLE, 1},
		[fre1_1265m_addr]  = {ENABLE, 1},
		[fre1_1572m_addr]  = {ENABLE, 1},
		[fre1_1597m_addr]  = {ENABLE, 1}
};

RfTxProperty_t RfTxProperty2[fre2_max] =
{
		[fre2_828m_addr]  = {ENABLE, 0},
		[fre2_838m_addr]  = {ENABLE, 0},
		[fre2_848m_addr]  = {ENABLE, 0},
		[fre2_858m_addr]  = {ENABLE, 0},
		[fre2_900m_addr]  = {ENABLE, 0},
		[fre2_910m_addr]  = {ENABLE, 0},
		[fre2_920m_addr]  = {ENABLE, 0},
		[fre2_930m_addr]  = {ENABLE, 0}
};

RfTxProperty_t RfTxProperty3[fre3_max] =
{
		[fre3_1087m_addr] = {ENABLE, 0},
		[fre3_1127m_addr] = {ENABLE, 0},
		[fre3_1167m_addr] = {ENABLE, 0},
		[fre3_1207m_addr] = {ENABLE, 0},
		[fre3_1247m_addr] = {ENABLE, 0},
		[fre3_1287m_addr] = {ENABLE, 0},
		[fre3_1327m_addr] = {ENABLE, 0},
		[fre3_1367m_addr] = {ENABLE, 0},
};
RfTxProperty_t RfTxProperty4[fre4_max] =
{
//		[fre4_842m_addr] = {ENABLE, 0},
//		[fre4_915m_addr] = {ENABLE, 0},
		[fre4_1775m_addr] = {ENABLE, 0},
		[fre4_1185m_addr] = {ENABLE, 1},
		[fre4_1225m_addr] = {ENABLE, 1},
		[fre4_1265m_addr] = {ENABLE, 1},
		[fre4_1572m_addr] = {ENABLE, 1},
		[fre4_1597m_addr] = {ENABLE, 1},
};

void SetHitFlag( uint8_t type )
{

}

int32_t InitAd9361(void)
{
	return 0;
}

int32_t InitAd9361_Rx(void)
{
	return 0;
}


int32_t InitAd9361_Rx_droneid(void)
{
	return 0;
}

/* Sets the TX frequency. */
void set_tx_freq(float freq)
{
}
/* Sets the RX frequency. */

#define EQUAL( x , y )  fabs((x) - (y)) < 0.000001

float SetRxFreq(float uLocalOscFreq)
{
	return 0;
}

uint32_t set_rx_freq(uint32_t freqItem)
{
	return 0;
}

/* Gets the RX frequency. */
uint32_t get_rx_freq(uint32_t freqItem)
{
	return 0;
}

void ad9361_generate_fast_lock_profile(uint8_t profile_num, uint8_t *reg_value_list)
{

}

void enable_fast_lock_profile()
{

}

#define HOP_BY_GPIO

static void fre_hop_handle(void *param)
{}

int32_t Ad9361_fre_hop_init(void)
{
	return 0;
}

int32_t Ad9361_fre_hop_start(void)
{
	return 0;
}

int32_t Ad9361_fre_hop_stop(void)
{
	return 0;
}

int32_t Ad9361_fre_hop_set(int32_t rfRang)
{
	return 0;
}

void SetAD9361Gpio(uint32_t value)
{

}

void SetAd9361Gain(uint8_t omnidirect_value, uint8_t orientation_value)
{

}

static uint8_t Ad9361GainCutoverFlag = 0;
void SetAd9361GainCutoverFlag(uint8_t value)
{

}
uint8_t GetAd9361GainCutoverFlag(void)
{
	return 0;
}
void SetAd9361GainCutoverValue(uint8_t value1, uint8_t value2)
{

}

void SetAd9361GainAGC(uint8_t value)
{

}

void SetAd9361Reg(uint8_t addr, uint8_t addr2, uint8_t value)
{

}
