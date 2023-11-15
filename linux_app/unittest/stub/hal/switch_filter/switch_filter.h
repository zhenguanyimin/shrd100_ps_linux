#include <stdint.h>

enum RF5250_CHANNEL
{
	eRF5250_CLOSE,
	eRF5250_CHANNEL1,
	eRF5250_CHANNEL2,
	eRF5250_CHANNEL3,
	eRF5250_CHANNEL4,
	eRF5250_CHANNEL5,
};


enum HMC345ALP3E_CHANNEL
{
	HMC345ALP3E_CLOSE,
	HMC345ALP3E_400M_1000M,
	HMC345ALP3E_2400M_2483M,
	HMC345ALP3E_5100M_5300M,
	HMC345ALP3E_5650M_5850M,
};

enum T3_RF_CHANNEL
{
	T3_RF_CLOSE,	// <SHRD100_RF_T3开关控制文档V1.2.docx>
	BFCN_4800,		// BFCN-4800+: 5.15~5.25GHz
	TA1038A,		// TA1038A:    2.4~2.46GHz
	TA1059A,		// TA1059A:    2.45~2.51GHz
	BFCN_5750,		// BFCN-5750+: 5.65~5.85GHz
	LFCN_6000		// LFCN-6000+: 0.4~6GHz
};

enum HMC1119_SERNIN
{
	eHMC1119_SERNIN0, // attenuation 0 db
	eHMC1119_SERNIN1, // attenuation 0.25
	eHMC1119_SERNIN2, // attenuation 0.5
	eHMC1119_SERNIN3, // attenuation 1
	eHMC1119_SERNIN4, // attenuation 2
	eHMC1119_SERNIN5, // attenuation 4
	eHMC1119_SERNIN6, // attenuation 8
	eHMC1119_SERNIN7, // attenuation  16
	eHMC1119_SERNIN8, // attenuation 31.75
};

uint8_t SetSwitchFilterGain(uint16_t value);
void Adrf5250Ctrl(uint8_t channel);
void Adrf5250Disable();

int32_t SwitchFilterGainIoCtrl(uint8_t value);
