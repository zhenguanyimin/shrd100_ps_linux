#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sfDroneSniffer.h"

// 数字图传特征库定义
static struct DroneLib droneDigiLIB[40] =
{
	{"DJI OcuSync2         ",{0,1,2,3,5,6},6,0,{2400,2483.5},2,0,0,{2.1,3.1,4.1,5.1},4,0,{4,6},2,{10,18},2,{0.2,0.2,0.2,0.2},{1,3,0,3},0.2,8,0,1,0,0.758,{20,33,20,33,20}},
	{"DJI OcuSync2         ",{0,1,2,3,5,6},6,0,{5725,5850},2,0,0,{2.1,3.1,4.1,5.1},4,0,{4,6},2,{10,18},2,{0.2,0.2,0.2,0.2},{1,3,0,3},0.2,8,0,1,0,0.758,{30,33,14,33,14}},

	{"DJI OcuSync3         ",{1,2,5,6},4,0,{2400,2483.5},2,0,0,{1.1,2.1},2,0,{2,4,4},3,{10,18},2,{0.2,0.2},{14,4},0.2,30,0,1,0,0.41,{20,33,20,33,20}},
	{"DJI OcuSync3         ",{1,2,5,6},4,0,{5725,5850},2,0,0,{1.1,2.1},2,0,{2,4,4},3,{10,18},2,{0.2,0.2},{14,4},0.2,20,0,1,0,0.41,{30,33,14,33,14}},

	{"Autel EVO II         ",{14},1,0,{2400,2483.5},2,0,0,{4},1,1,{5},1,{9.4},1,{0.15},{12},0.2,16,0,1,0,0.5,{20,27,20,27,20}},	// 20230624更改数据库参数
	{"Autel EVO II         ",{14},1,0,{5725,5850},2,0,0,{4},1,1,{5},1,{9.4},1,{0.15},{12},0.2,16,0,1,0,0.5,{27,27,14,27,14}},	// 20230624更改数据库参数

	{"Autel EVO II V2      ",{15},1,0,{2400,2483.5},2,0,0,{10.27},1,1,{14},1,{10},1,{0.2},{5},0.2,4,0,1,0,0.7,{20,27,20,27,20}}, // 20230911与DJI WIFI拉开
	{"Autel EVO II V2      ",{15},1,0,{5725,5850},2,0,0,{10.27},1,1,{14},1,{10},1,{0.2},{5},0.2,4,0,1,0,0.7,{27,27,14,27,14}}, // 20230911与DJI WIFI拉开


	{"Autel Max4T          ",{20},2,0,{1427.9,1447.9},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{20,20,20,20,20}},// 20230624更改数据库参数
	{"Autel V3/M4T         ",{18,20},2,0,{832,862},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{31,31,31,31,31}},// 20230624更改数据库参数
	{"Autel V3/M4T         ",{18,20},2,0,{902,928},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{31,31,31,31,31}},// 20230624更改数据库参数
	{"Autel Lt/V3/M4T      ",{18,18,20},1,0,{2400,2483.5},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{20,32,20,32,20}},// 20230624更改数据库参数
	{"Autel Lt/V3/M4T      ",{18,18,20},1,0,{5725,5850},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{33,33,14,33,14}},// 20230624更改数据库参数
	{"Autel Lt/V3/M4T      ",{18,18,20},1,0,{5150,5250},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{30,30,20,30,20}},// 20230624更改数据库参数
	{"Autel V3/M4T         ",{18,20},1,0,{4850,5150},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{20,20,20,20,20}},// 20230624更改数据库参数
	{"Autel V3/M4T         ",{18,20},1,0,{5250,5725},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{20,20,20,20,20}},// 20230624更改数据库参数
	{"Autel V3/M4T         ",{18,20},1,0,{5850,6000},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{20,20,20,20,20}},// 20230624更改数据库参数

	{"Autel EVO Nano       ",{18,19},2,0,{2400,2483.5},2,0,0,{0.9},1,1,{1,1,3},3,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{20,27,20,27,20}},// 20230624更改数据库参数
	{"Autel EVO Nano       ",{18,19},2,0,{5725,5850},2,0,0,{0.9},1,1,{1,1,3},3,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{27,27,20,27,20}},// 20230624更改数据库参数
	{"Autel EVO Nano       ",{18,19},2,0,{5150,5250},2,0,0,{0.9},1,1,{1,1,3},3,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{27,27,20,27,20}},// 20230624更改数据库参数

	//20230901增加数字图传报文
	{"Autel LongYu         ",{18,20},2,0,{902,928},2,0,0,{0.9},1,1,{1,1,3},3,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5},

	{"DJI Phantom4 V2      ",{12},1,0,{2400,2483.5},2,0,0,{3.5,5.5},2,0,{4,6},2,{10,18},2,{0.15,0.15},{8,8},0.2,16,0,1,0,0.758,{20,27,20,27,20}},//David20230807增加
	{"DJI Phantom4 V2      ",{12},1,0,{5725,5850  },2,0,0,{3.5,5.5},2,0,{4,6},2,{10,18},2,{0.15,0.15},{8,8},0.2,16,0,1,0,0.758,{27,26,14,27,14}},//David20230807增加
	{"DJI Phantom4 RTK     ",{13},1,0,{2400,2483.5},2,0,0,{1,2,4,5},4,0,{3,17},2,{8.5},1,{0.2,0.2,0.2,0.2},{1,2,0,3},0.5,8,0,1,0,0.758,{20,20,20,20,20}},//David20230808增加
	{"DJI Phantom4 RTK     ",{13},1,0,{5725,5850  },2,0,0,{1,2,4,5},4,0,{3,17},2,{8.5},1,{0.2,0.2,0.2,0.2},{1,2,0,3},0.5,8,0,1,0,0.758,{26,26,14,26,14}},//David20230808增加

	//20230821增加数字图传报文
	{"Digital FPV-1        ",{21},1,0,{5635,5950  },2,0,0,{5.3},1,1,{7.2},1,{18},1,{0.2},{12},0.2,12,0,1,0,0.5,{26,26,14,26,14}},
	//20230901增加数字图传报文
	{"DJI Phantom4 Wifi    ",{12},1,0,{2400,2483.5},2,0,0,{9.76},1,1,{14},1,{10},1,{0.2},{5},0.2,4,0,1,0,0.7},
	
	//2023/10/16增加新MAXT4
	//{"Autel V3/M4T(New)    ",{18,20},2,0,{832,862},2,0,0,{0.8,1.6},2,1,{2,1,2,15.1},4,{9.4},1,{0.15,0.3},{15},0.2,10,0,1,0,0.5,{31,31,31,31,31}},// 20230624更改数据库参数  2023/10/14更改验证MAXT4
	//20231010增加数字图传报文
	//{"xiaomi               ",{1,2,5,6},4,0,{5740,5830},2,0,0,{1},1,1,{3.06},1,{10},1,{0.2},{5},0.3,5,1.5,2,0,0.7,{31,31,31,31,31}},
	//{"udi(youdi)           ",{1,2,5,6},4,0,{2409,2436},2,0,0,{11.5},1,1,{13.5},1,{6},1,{0.2},{3},1,4,2,2,0,0.7,{31,31,31,31,31}},
	//{"AMIMON               ",{1,2,5,6},4,0,{5736,5814},2,0,0,{14.2},1,1,{16},1,{40},1,{0.2},{5},1,5,1,1,0,0.7,{31,31,31,31,31}},  //40
	//{"DIY_zhibao	       ",{1,2,5,6},4,0,{2392,2492},2,0,0,{3},1,1,{5},1,{18},1,{0.1},{18},0.2,18,1,1,0,0.7,{31,31,31,31,31}},
	//{"SKYDROID	           ",{1,2,5,6},4,0,{2411,2507},2,0,0,{2.5},1,1,{3},1,{1.2},1,{0.2},{6},0.5,6,1,1,0,0.7,{31,31,31,31,31}}, //最低跳数较小
	//{"AVATA	               ",{1,2,5,6},4,0,{5700,5800},2,0,0,{1.1},1,0,{10},1,{9,18,36},3,{0.2},{10},0.5,8,0,1,5,0.7,{31,31,31,31,31}}, //
	//{"swellpro	           ",{1,2,5,6},4,0,{2425.5,2468.5},2,0,0,{1.12},1,1,{4},1,{1},1,{0.15},{14},0.4,10,1,2,0,0.7,{31,31,31,31,31}}, //带宽小1M
	//{"fengying_PIX	       ",{1,2,5,6},4,0,{2418.7,2515.3},2,0,0,{1.4},1,1,{2,3},2,{0.6},1,{0.1},{20},0.3,22,1,1,0,0.7,{31,31,31,31,31}}, //带宽小0.6M
};
// 特征库内图传种类
static int nDigiLib = 27;

//注释模拟特征库
/*
// 模拟图传特征库定义
static struct VideoLib droneMoniLIB[30] =
{
	//20230805添加-自采FPV数据库-仅用于数据调用
	{"FPV1-Band A         ",{51},1,{5865,5845,5825,5805,5785,5765,5745,5725},8,-2.5,12,8,12,20,0.9,0.13},// 20230805更改数据库参数
	{"FPV1-Band B         ",{51},1,{5733,5752,5771,5790,5809,5828,5847,5866},8,-2.5,12,8,12,20,0.9,0.13},// 20230805更改数据库参数
	{"FPV1-Band E         ",{51},1,{5705,5685,5665,5645,5885,5905,5925,5945},8,-2.5,12,8,12,20,0.9,0.13},// 20230805更改数据库参数
	{"FPV1-Band F         ",{51},1,{5740,5760,5780,5800,5820,5840,5860,5880},8,-2.5,12,8,12,20,0.9,0.13},// 20230805更改数据库参数
	{"FPV1-Band C         ",{51},1,{5658,5695,5732,5769,5806,5843,5880,5917},8,-2.5,12,8,12,20,0.9,0.13},// 20230805更改数据库参数

	//20230821添加-R国采集数据
	{"FPV2-Band A         ",{51},1,{5865,5845,5825,5805,5785,5765,5745,5725},8,0,6,8,12,17,0.9,0.13},// 20230805更改数据库参数
	{"FPV2-Band B         ",{51},1,{5733,5752,5771,5790,5809,5828,5847,5866},8,0,6,8,12,17,0.9,0.13},// 20230805更改数据库参数
	{"FPV2-Band E         ",{51},1,{5705,5685,5665,5645,5885,5905,5925,5945},8,0,6,8,12,17,0.9,0.13},// 20230805更改数据库参数
	{"FPV2-Band F         ",{51},1,{5740,5760,5780,5800,5820,5840,5860,5880},8,0,6,8,12,17,0.9,0.13},// 20230805更改数据库参数
	{"FPV2-Band C         ",{51},1,{5658,5695,5732,5769,5806,5843,5880,5917},8,0,6,8,12,17,0.9,0.13},// 20230805更改数据库参数


	//20230815添加-自采FPV数据库-仅用于数据调用
	{"FPV3-Band 1.2       ",{52},1,{1080,1120,1160,1200,1240,1280,1320,1360},8,0,7,14,20,17,0.9,0.13},// 20230815更改数据库参数
};
// 特征库内图传种类
static int nMoniLib = 11;
*/


// 频点相关计算信息
static float listFrequence[30] = { 842,915,1080,1120,1160,1200,1240,1280,1320,1360,1437,2422,2437,2462,5160,5180,5200,5220,5240,5655,5695,5725,5745,5765,5785,5805,5825,5850,5890,5930 };
static float listNoice[30] = { 14000,14000,13600,13600,13600,13600,13600,13600,13600,13600,13600,12100,12100,12100,12000,12000,12000,12000,12000,11850,11850,11850,11850,11800,11800,11800,11800,11750,11750,11750 };
static int   listGain1[30] = { 50,50,50,50,50,50,50,50,50,50,50,56,56,56,61,61,61,61,61,66,66,66,66,66,66,66,66,66,66,66 };
static int   listNum = 30;
static float list2DNoice[30][128] = { 0 };// 软件启动时从FLASH加载，否则算法会出错

// 定向功率参数,定向路与全向路增益差，定向路高则为正
static float corQD = 0 * 85;
// 无人机典型信噪比
static float uavTipicalSNR = 0.0f;


// 侦测函数部分-------------------------------------------------

// 主函数入口
void  sfV2DroneSniffer(struct droneResult *droneInfo, int *nDrone, unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], unsigned short upMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], float cenFreq, float fs, float bw, float gain, int calRows, unsigned short *output, unsigned int *flagMark)
{
	int nRows = calRows; //确定本次计算用了何种大小的数据
	int nCols = NFFT / N;
	float dF = fs / NFFT * N;
	float dT = M * NFFT / fs / 1000.0f;

	unsigned short(*pBuf)[NRow / M][NFFT / N] = NULL;
	pBuf = (unsigned short*)downMat;

	// 数字图传部分定义
	static struct blockGroup uavPulseDigi[MaxDrone];
	int nUavPulseDigi = 0;
	// 模拟图传部分定义
	static struct blockGroup uavPulseMoni[MaxDrone];
	int nUavPulseMoni = 0;

	// 待输出数据初始化
	static unsigned short workSpectrum1[128] = { 0 };
	static unsigned short workSpectrum2[128] = { 0 };
	static unsigned short workSpectrum3[128] = { 0 };
	static unsigned short workStatus[128] = { 0 };
	static unsigned short workPowerN[128] = { 0 };
	static unsigned short workPowerS[128] = { 0 };
	// 数据初始化
	for (int i = 0; i < 128; i++)
	{
		workSpectrum1[i] = 0;
		workSpectrum2[i] = 0;
		workSpectrum3[i] = 0;
		workStatus[i] = 0;
		workPowerN[i] = 0;
		workPowerS[i] = 0;
	}
	// 执行次数计数
	static unsigned short worknum1 = 0;
	static unsigned short worknum2 = 0;

	// 信号频谱获取
	sfV2GetSpectrum(pBuf, 0, workSpectrum1, workSpectrum2, workSpectrum3);
	// 数字图传
	*flagMark = 0;
	if ((cenFreq >= 842 && cenFreq <= 915) || (cenFreq >= 1437 && cenFreq <= 5240) || (cenFreq >= 5725 && cenFreq <= 5930))
	{
		// 信号处理S2模式-自动门限控制方案
		worknum1 = worknum1 + 1;
		sfV2S2DownLoadSniffer(uavPulseDigi, &nUavPulseDigi, pBuf, 0, nRows, nCols, cenFreq, gain, dT, dF, droneDigiLIB, nDigiLib, worknum1, workStatus, workPowerN, workPowerS, flagMark);
		// S1模式无目标则计算S2模式(固定噪底已被更新)
		if (uavPulseDigi == 0 && list2DNoice[0][0] != 0)
		{
			// 信号处理S1模式-经验噪底固定方案
			worknum2 = worknum2 + 1;
			sfV2S1DownLoadSniffer(uavPulseDigi, &nUavPulseDigi, pBuf, 0, nRows, nCols, cenFreq, gain, dT, dF, droneDigiLIB, nDigiLib, worknum2, workStatus, workPowerN, workPowerS);
		}
	}
	// 模拟图传-David20230807/20231020修改检测频点
	if ((cenFreq >= 842 && cenFreq <= 1360) || (cenFreq >= 5655 && cenFreq <= 5930))
	{
		sfMoniSniffer(uavPulseMoni, &nUavPulseMoni, pBuf, 0, nRows, nCols, cenFreq);
	}

	// 数据输出
	for (int i = 0; i < 128; i++)
	{
		output[i + 128 * 0] = workSpectrum1[i];
		output[i + 128 * 1] = workSpectrum2[i];
		output[i + 128 * 2] = workSpectrum3[i];
		output[i + 128 * 3] = workStatus[i];
		output[i + 128 * 4] = workPowerN[i];
		output[i + 128 * 5] = workPowerS[i];
	}
	output[1022] = worknum1;
	output[1023] = worknum2;
	// 写入处理信息
	sfSetDigiResult(droneInfo, nDrone, droneDigiLIB, uavPulseDigi, nUavPulseDigi, 1);
	sfSetMoniResult(droneInfo, nDrone, uavPulseMoni, nUavPulseMoni, 2);
}

// 获取频率功率情况
void sfV2GetSpectrum(unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chnum, unsigned short *pHist1, unsigned short *pHist2, unsigned short *pHist3)
{
	static float line1[128];// 均值
	static float line2[128];// 最小值
	static float line3[128];// 最大值

	int i, j = 0;

	for (j = 0; j < 128; j++)
	{
		line1[j] = 0;
		line2[j] = 50000;
		line3[j] = 0;

		for (i = 0 + 5; i < 3000 - 5; i++)
		{
			// 求和
			line1[j] += downMat[chnum][i][j];
			// 最小值
			if (line2[j] > downMat[chnum][i][j])
			{
				line2[j] = downMat[chnum][i][j];
			}
			// 最大值
			if (line3[j] < downMat[chnum][i][j])
			{
				line3[j] = downMat[chnum][i][j];
			}
		}
		line1[j] = line1[j] / 2990;

		pHist1[j] = (unsigned short)line1[j]; // 对外输出使用
		pHist2[j] = (unsigned short)line2[j]; // 对外输出使用
		pHist3[j] = (unsigned short)line3[j]; // 对外输出使用
	}
}

// V2版本主处理函数S1
void sfV2S1DownLoadSniffer(struct blockGroup *droneBlock, int *nBlock, unsigned short ***downMat, int chn, int nRows, int nCols, float cenFreq, float gain, float dT, float dF, struct DroneLib *UAVtypes, int nUav, unsigned short worknum, unsigned short *workStatus, unsigned short *workPowerN, unsigned short *workPowerS)
{
	// 常用变量定义
	int i = 0, j = 0, k = 0;
	// 状态数据记录
	workStatus[0] = worknum;
	workStatus[1] = (unsigned short)cenFreq;
	workStatus[2] = (unsigned short)nRows;
	workStatus[3] = (unsigned short)nCols;
	workStatus[4] = (unsigned short)gain;

	// 获取计算基准频率范围
	int sCol = 0;
	int eCol = 127;
	if (cenFreq == 842)
	{
		sCol = 39;
		eCol = 114;
	}
	else if (cenFreq == 915)
	{
		sCol = 32;
		eCol = 96;
	}
	else
	{
		sCol = 0 + 2;
		eCol = 127 - 2;
	}
	// 状态数据记录
	workStatus[5] = (unsigned short)sCol;
	workStatus[6] = (unsigned short)eCol;

	//-----双门限计算法S1部分------//

	// 状态数据记录
	workStatus[7] = 0;
	// 计算频率维度过门限的点
	static int mPass1[sfM1];
	for (i = 0; i < sfM1; i++)
	{
		mPass1[i] = 0;
	}
	sfV2S1FrequenceHist(downMat, 0, mPass1, cenFreq);
	// 边带处理
	for (i = 0; i <= sCol; i++)
	{
		mPass1[i] = 0;
	}
	for (i = eCol; i <= 127; i++)
	{
		mPass1[i] = 0;
	}
	// 获取可能的频率
	static int fStart[sfM1];
	static int fEnd[sfM1];
	int fnum = 0;
	fnum = sfV2S1CalculateFrequenceSE(mPass1, 0.25, fStart, fEnd);
	// 状态数据记录
	workStatus[8] = 0;
	workStatus[9] = 0;
	workStatus[10] = (unsigned short)fnum;
	// 循环计算脉冲
	static float mPass2[sfM2];
	static int q1[sfM2];
	static int w[sfM2];
	static float colMeanAmp[sfM2];
	static float colSnr[sfM2];
	if (fnum > 0)
	{
		for (i = 0; i < fnum; i++)
		{
			// 生成合并脉冲列
			for (j = 0; j < sfM2; j++)
			{
				mPass2[j] = 0;
				for (k = fStart[i]; k <= fEnd[i]; k++)
				{
					mPass2[j] = mPass2[j] + downMat[chn][j][k];
				}
				mPass2[j] = mPass2[j] / (fEnd[i] - fStart[i] + 1);
			}
			// 计算幅度门限
			float threshold3 = sfAutoThreshOnLineF(mPass2, nRows, 1, 6);
			// 门限增加固定值保护
			sfThresholdCompare(&threshold3, cenFreq, 0, 3 + 6);
			// 获取脉冲描述信息
			int	nw;
			int Delt = 1;
			int MinW = 0.5f / dT; //0.7f / dT; // 20230626,窄脉冲识别门限由0.7下降至0.5
			nw = sfThresholdToPulse(mPass2, nRows, threshold3, Delt, MinW, q1, w, colSnr, colMeanAmp);
			// 进行目标数据库匹配
			static struct BlockRow aBlkRow;
			int det = sfVidBlocksDetect(&aBlkRow, mPass2, nRows, q1, w, nw, colSnr, chn, fStart[i], fEnd[i], cenFreq, dT, dF, UAVtypes, nUav);
			// 状态数据记录
			if (i < 6)
			{
				workStatus[16 + 0 + 16 * i] = (unsigned short)fStart[i];
				workStatus[16 + 1 + 16 * i] = (unsigned short)fEnd[i];
				workStatus[16 + 2 + 16 * i] = (unsigned short)threshold3;
				workStatus[16 + 3 + 16 * i] = (unsigned short)nw;
				workStatus[16 + 4 + 16 * i] = (unsigned short)det;
				if (det == 1)
				{
					workStatus[16 + 5 + 16 * i] = (unsigned short)aBlkRow.uavIndex;
					workStatus[16 + 6 + 16 * i] = (unsigned short)aBlkRow.snr;
					workStatus[16 + 7 + 16 * i] = (unsigned short)aBlkRow.meanColAmp;
					workStatus[16 + 8 + 16 * i] = (unsigned short)aBlkRow.nw;
					workStatus[16 + 9 + 16 * i] = (unsigned short)aBlkRow.nCol;

					workStatus[11] = (unsigned short)(i + 1);
					workStatus[12] = (unsigned short)nw;
					for (j = 0; j < workStatus[11]; j++)
					{
						if (j >= 128)
							break;

						workPowerS[j] = (unsigned short)colMeanAmp[j];
						workPowerN[j] = (unsigned short)colSnr[j];
					}
				}
			}
			// 最后检测判定
			if (det == 1)
			{
				// 数据完成ablock至droneBlock的添加
				int tmpBlock = *nBlock;
				sfPulseToTarget(droneBlock, tmpBlock, aBlkRow, fStart[i], fEnd[i], cenFreq, dT, dF, UAVtypes);
				*nBlock = *nBlock + 1;
			}
			else
			{
				continue;
			}
		}
	}
}
// 频率直方图统计
void sfV2S1FrequenceHist(unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chnum, int* hist, float cenFreq)
{
	// 查找频点索引
	int fIndex = 0;
	for (int i = 0; i < listNum; i++)
	{
		if (listFrequence[i] == cenFreq)
		{
			fIndex = i;
			break;
		}
	}
	// 统计过门限点数
	int i = 0, j = 0;
	for (i = 0; i < 3000; i++)
	{
		for (j = 0; j < 128; j++)
		{
			if (downMat[chnum][i][j] > list2DNoice[fIndex][j] + 125)// 高过均值1.5dB则进行统计
			{
				hist[j] = hist[j] + 1;
			}
		}
	}
}
// 计算需要进行脉冲计算的频率行起始信息
int  sfV2S1CalculateFrequenceSE(int *line, float dT, int *lineS, int *lineE)
{
	static int calM1[sfM1];
	static int calM2[sfM1];
	static int calM3[sfM1];
	static int calM4[sfM1];
	int calCnt1 = 0;
	int calCnt2 = 0;
	int i = 0;

	for (i = 0; i < 128; i++)
	{
		calM1[i] = 0;
		calM2[i] = 0;
	}
	int threshold = dT * 3000;
	for (i = 0; i < 128; i++)
	{
		if (line[i] > threshold)
		{
			calM1[i] = 1;
		}
	}
	// 数据结构上下为0，保证了在频率维度上检测一定有始有终
	calCnt1 = 0;
	for (i = 1; i < 128; i++)
	{
		if ((calM1[i] - calM1[i - 1]) == 1)
		{
			calM2[calCnt1] = i;
		}
		else if ((calM1[i] - calM1[i - 1]) == -1)
		{
			calM3[calCnt1] = i;
			calCnt1++;
		}
		else
		{

		}
	}
	// 短带宽检测-4M以下则不认为是有效的
	calCnt2 = 0;
	for (i = 0; i < calCnt1; i++)
	{
		if ((calM3[i] - calM2[i]) > 10)
		{
			lineS[calCnt2] = calM2[i];
			lineE[calCnt2] = calM3[i] - 1;
			calCnt2++;
		}
	}
	return calCnt2;
}

// V2版本主处理函数S2
void  sfV2S2DownLoadSniffer(struct blockGroup *droneBlock, int *nBlock, unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chn, int nRows, int nCols, float cenFreq, float gain, float dT, float dF, struct DroneLib *UAVtypes, int nUav, unsigned short worknum, unsigned short *workStatus, unsigned short *workPowerN, unsigned short *workPowerS, unsigned int *flagMark)
{
	// 常用变量定义
	int i = 0, j = 0, k = 0;
	// 疑似图传识别标记
	unsigned int flagTC = 0;

	// 状态数据记录
	workStatus[0] = worknum;
	workStatus[1] = (unsigned short)cenFreq;
	workStatus[2] = (unsigned short)nRows;
	workStatus[3] = (unsigned short)nCols;
	workStatus[4] = (unsigned short)gain;

	// 获取计算基准频率范围
	int sCol = 0;
	int eCol = 127;
	if (cenFreq == 842)
	{
		sCol = 39;
		eCol = 114;
	}
	else if (cenFreq == 915)
	{
		sCol = 32;
		eCol = 96;
	}
	else
	{
		sCol = 0 + 2;
		eCol = 127 - 2;
	}
	// 状态数据记录
	workStatus[5] = (unsigned short)sCol;
	workStatus[6] = (unsigned short)eCol;

	// 计算全图门限
	float maxT = 0;
	sfThresholdCompare(&maxT, cenFreq, 18, 18 + 6);
	static float DT = 0.1;
	float threshold1 = sfV2S2AutoThreshOnMap(downMat, chn, sCol, eCol, nRows, DT, 6, maxT); // 20230912需更改-增益未调整
	// 状态数据记录
	workStatus[7] = (unsigned short)threshold1;
	sfThresholdCompare(&threshold1, cenFreq, 0, 3 + 6);
	// 计算频率维度过门限的点
	static int mPass1[sfM1];
	for (i = 0; i < sfM1; i++)
	{
		mPass1[i] = 0;
	}
	sfV2S2FrequenceHist(downMat, 0, mPass1, sCol, eCol, nRows, threshold1);
	// 边带处理
	for (i = 0; i <= sCol; i++)
	{
		mPass1[i] = (int)(nRows*0.2);
	}
	for (i = eCol; i <= 127; i++)
	{
		mPass1[i] = (int)(nRows*0.2);
	}
	// 计算频率维点数门限
	float threshold2 = sfV2S2AutoThreshOnLineI(mPass1, sCol, eCol, DT, 6);
	// 如果点数超过1500，占空比过半，则进行检测
	if (threshold2 > 1500)
	{
		threshold2 = 1500;
	}
	// 获取可能的频率
	static int fStart[sfM1];
	static int fEnd[sfM1];
	int fnum = 0;
	fnum = sfV2S2CalculateFrequenceSE(mPass1, threshold2, 6, fStart, fEnd);
	// 状态数据记录
	workStatus[8] = (unsigned short)threshold1;
	workStatus[9] = (unsigned short)threshold2;
	workStatus[10] = (unsigned short)fnum;

	// 循环计算脉冲
	static float mPass2[sfM2];
	static int q1[sfM2];
	static int w[sfM2];
	static float colMeanAmp[sfM2];
	static float colSnr[sfM2];
	if (fnum > 0)
	{
		for (i = 0; i < fnum; i++)
		{
			// 识别标志清零
			flagTC = 0;
			// 生成合并脉冲列
			for (j = 0; j < sfM2; j++)
			{
				mPass2[j] = 0;
				for (k = fStart[i]; k <= fEnd[i]; k++)
				{
					mPass2[j] = mPass2[j] + downMat[chn][j][k];
				}
				mPass2[j] = mPass2[j] / (fEnd[i] - fStart[i] + 1);
			}
			// 计算幅度门限
			float threshold3 = sfAutoThreshOnLineF(mPass2, nRows, DT, 6);
			// 门限增加固定值保护
			sfThresholdCompare(&threshold3, cenFreq, 0, 3 + 6);
			// 获取脉冲描述信息
			int	nw;
			int Delt = 1;
			int MinW = 0.5f / dT; //0.7f / dT; // 20230626,窄脉冲识别门限由0.7下降至0.5
			nw = sfThresholdToPulse(mPass2, nRows, threshold3, Delt, MinW, q1, w, colSnr, colMeanAmp);
			// 疑似图传信号识别计算
			flagTC = sfDigiRecognition(q1, w, nw);
			// 进行目标数据库匹配
			static struct BlockRow aBlkRow;
			int det = sfVidBlocksDetect(&aBlkRow, mPass2, nRows, q1, w, nw, colSnr, chn, fStart[i], fEnd[i], cenFreq, dT, dF, UAVtypes, nUav);
			// 状态数据记录
			if (i < 6)
			{
				workStatus[16 + 0 + 16 * i] = (unsigned short)fStart[i];
				workStatus[16 + 1 + 16 * i] = (unsigned short)fEnd[i];
				workStatus[16 + 2 + 16 * i] = (unsigned short)threshold3;
				workStatus[16 + 3 + 16 * i] = (unsigned short)nw;
				workStatus[16 + 4 + 16 * i] = (unsigned short)det;
				if (det == 1)
				{
					workStatus[16 + 5 + 16 * i] = (unsigned short)aBlkRow.uavIndex;
					workStatus[16 + 6 + 16 * i] = (unsigned short)aBlkRow.snr;
					workStatus[16 + 7 + 16 * i] = (unsigned short)aBlkRow.meanColAmp;
					workStatus[16 + 8 + 16 * i] = (unsigned short)aBlkRow.nw;
					workStatus[16 + 9 + 16 * i] = (unsigned short)aBlkRow.nCol;

					workStatus[11] = (unsigned short)(i + 1);
					workStatus[12] = (unsigned short)nw;
					for (j = 0; j < workStatus[11]; j++)
					{
						if (j >= 128)
							break;

						workPowerS[j] = (unsigned short)colMeanAmp[j];
						workPowerN[j] = (unsigned short)colSnr[j];
					}
				}
			}
			// 最后检测判定
			if (det == 1)
			{
				// 最后进行检测幅度校验
				float threshold4 = 0;
				sfThresholdCompare(&threshold4, cenFreq, -3, 3 + 6);
				if (aBlkRow.meanColAmp < threshold4)
				{
					continue;
				}
				// 数据完成ablock至droneBlock的添加
				int tmpBlock = *nBlock;
				sfPulseToTarget(droneBlock, tmpBlock, aBlkRow, fStart[i], fEnd[i], cenFreq, dT, dF, UAVtypes);
				*nBlock = *nBlock + 1;
			}
			else
			{
				// 未识别为目标，且当前识别为1，总识别为0
				if (*flagMark == 0 && flagTC == 1)
				{
					*flagMark = 1;
				}
				continue;
			}
		}
	}
}

// 获取区分信号和噪声的阈值（二维）
float sfV2S2AutoThreshOnMap(unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chnum, int d128s, int d128e, int d3000, float dT, int maxIter, float baseT)
{
	// 先做加和，再做自动门限
	int   i = 0, j = 0, k = 0;
	int   n1 = 0;
	float t, tnew = 0.0f;
	float t1 = 0.0f, t2 = 0.0f;

	static float line1[128];

	// 20230706-除数值域限制
	if (d3000 < 100 || d128e - d128s < 32)
	{
		return 0;
	}

	for (j = 0; j < 128; j++)
	{
		line1[j] = 0;
		for (i = 0; i < d3000; i++)
		{

			if (downMat[chnum][i][j] > baseT)
			{
				line1[j] += baseT;
			}
			else
			{
				line1[j] += downMat[chnum][i][j];
			}
		}
		line1[j] = line1[j] / d3000;
	}

	// 计算功率值总和
	for (j = d128s; j <= d128e; j++)
	{
		tnew += line1[j] / (d128e - d128s + 1);
	}
	t = tnew + 2 * dT;

	while (fabs(tnew - t) > dT && k < maxIter)
	{
		t = tnew;

		for (j = d128s; j <= d128e; j++)
		{
			if (line1[j] > t)
			{
				t1 += line1[j];
				n1++;
			}
			else
				t2 += line1[j];
		}

		// 20230706-修正除零风险
		if (n1 == 0)
			tnew = t;
		else if (d128e - d128s + 1 > n1)
			tnew = t1 / n1 / 4 * 3 + t2 / (d128e - d128s + 1 - n1) / 4 * 1;// 门限要往大值倾斜
		else
			tnew = t;

		t1 = 0.0f;
		t2 = 0.0f;
		n1 = 0;
		k++;
	}
	return tnew;
}
// 频率直方图统计
void  sfV2S2FrequenceHist(unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chnum, int* hist, int d128s, int d128e, int d3000, float threshold)
{
	int i = 0, j = 0;

	for (i = 0; i < d3000; i++)
	{
		for (j = 0; j < 128; j++)
		{
			if (downMat[chnum][i][j] > threshold)
			{
				hist[j] = hist[j] + 1;
			}
		}
	}
	// 补最少点值-防止部分零结果将整个门限拉低
	for (i = 0; i < 128; i++)
	{
		if (hist[i] < (int)(d3000*0.2))
		{
			hist[i] = (int)(d3000*0.2);
		}
	}
}
// 点数门限计算
float sfV2S2AutoThreshOnLineI(int *line, int d128s, int d128e, float dT, int maxIter)
{
	int   i = 0, j = 0, k = 0;
	int   n1 = 0;
	float t, tnew = 0.0f;
	float t1 = 0.0f, t2 = 0.0f;


	// 除数值域限制
	if (d128e - d128s + 1 < 32)
	{
		return 0;
	}
	// 计算功率值总和
	for (j = d128s; j <= d128e; j++)
	{
		tnew += line[j] / (d128e - d128s + 1);
	}
	t = tnew + 2 * dT;

	while (fabs(tnew - t) > dT && k < maxIter)
	{
		t = tnew;

		for (j = d128s; j <= d128e; j++)
		{
			if (line[j] > t)
			{
				t1 += line[j];
				n1++;
			}
			else
				t2 += line[j];
		}

		// 20230706-修正除零风险
		if (n1 == 0)
			tnew = t;
		if (d128e - d128s + 1 > n1)
			tnew = (t1 / n1 + t2 / (d128e - d128s + 1 - n1)) / 2.0f;
		else
			tnew = t;

		t1 = 0.0f;
		t2 = 0.0f;
		n1 = 0;
		k++;
	}
	return tnew;
}
// 计算需要进行脉冲计算的频率行起始信息
int   sfV2S2CalculateFrequenceSE(int *line, float dT, int lineWidthT, int *lineS, int *lineE)
{
	static int calM1[sfM1];
	static int calM2[sfM1];
	static int calM3[sfM1];
	static int calM4[sfM1];
	static int calCnt1 = 0;
	static int calCnt2 = 0;
	int i = 0;

	for (i = 0; i < 128; i++)
	{
		calM1[i] = 0;
		calM2[i] = 0;
	}

	for (i = 0; i < 128; i++)
	{
		if (line[i] > dT)
		{
			calM1[i] = 1;
		}
	}
	// 数据结构上下为0，保证了在频率维度上检测一定有始有终
	calCnt1 = 0;
	for (i = 1; i < 128; i++)
	{
		if ((calM1[i] - calM1[i - 1]) == 1)
		{
			calM2[calCnt1] = i;
		}
		else if ((calM1[i] - calM1[i - 1]) == -1)
		{
			calM3[calCnt1] = i;
			calCnt1++;
		}
		else
		{

		}
	}
	// 短带宽检测-4M以下则不认为是有效的
	calCnt2 = 0;
	for (i = 0; i < calCnt1; i++)
	{
		if ((calM3[i] - calM2[i]) > 10)
		{
			lineS[calCnt2] = calM2[i];
			lineE[calCnt2] = calM3[i] - 1;
			calCnt2++;
		}
	}
	return calCnt2;
}

// 分选脉冲流
int sfThresholdToPulse(float *s, int n, float thresh, int delt, int minw, int *q1, int *w, float *colSnr, float *colAm)
{
	int i, j;
	char  flag = 1;
	int   nFall = 0;
	int   nw = 0;
	float signalAmp = 0.0f, noiseAmp = 0.0f;

	q1[0] = -1;
	for (i = 0; i < n; i++)
	{
		if (s[i] > thresh)
		{
			if (!flag)
			{
				q1[nw] = i;
				flag = 1;
			}
			nFall = 0;
		}
		else
		{
			if (flag)
			{
				nFall++;
				if (nFall >= delt)
				{
					if (q1[0] > 0)
					{
						// dingyu, 20230202
						w[nw] = i - delt - q1[nw];
						if (w[nw] > minw)
						{
							nw++;
						}
					}
					flag = 0;
				}
			}
		}
	}

	if (nw > 0)
	{
		for (i = 0; i < nw; i++)
		{
			signalAmp = 0.0f;
			noiseAmp = 0.0f;
			for (j = q1[i]; j <= q1[i] + w[i]; j++)
			{
				signalAmp += s[j];
			}
			signalAmp /= w[i] + 1;
			if (i == 0)
			{
				if (q1[i] - 3 < 1)
				{
					noiseAmp = 0;
				}
				else
				{
					for (j = q1[i] - 3; j < q1[i]; j++)
					{
						noiseAmp += s[j];
					}
					noiseAmp /= 3.0f;
				}
			}
			else
			{
				if (q1[i] - q1[i - 1] - w[i - 1] - 1 < 14)
				{
					noiseAmp = 0;
				}
				else
				{
					for (j = q1[i - 1] + w[i - 1] + 1 + 2; j < q1[i] - 2; j++)
					{
						noiseAmp += s[j];
					}
					noiseAmp /= q1[i] - q1[i - 1] - w[i - 1] - 1 - 4;
				}
			}
			colSnr[i] = noiseAmp;
			colAm[i] = signalAmp;

		}
	}
	return nw;
}
// 计算疑似图传信号可能性-20230905
int sfDigiRecognition(int *q1, int *w, int nw)
{
	//q1:pulse start point 
	//w:pulse hold points 
	//nw:pulse number

	return 0;
}

// 图传图块检测
int sfVidBlocksDetect(struct BlockRow *aBlkRow, float *s, int n, int *q1, int *w, int nw, float *colSnr, int antIndex, int colIndexS, int colIndexE, float cenFreq, float dT, float dF, struct DroneLib *UAVtypes, int nUAV)
{
	int   i, j, k;
	char  isUAV;
	char  isInFreqSpan;
	int   mPulseW[maxPulseWNumInLib];
	int * Indexq = (int *)malloc(n * sizeof(int));
	int   nIndex;
	float freqL, freqR;
	int   freqIndexL, freqIndexR;
	int   count = 0;

	int **match = (int**)malloc(MaxVidPulse * sizeof(int*));
	for (i = 0; i < MaxVidPulse; i++)
		match[i] = (int*)malloc(MaxNumVidT * sizeof(int));

	int   meetPulseTCount = 0;
	float minSnr = 0.0;

	float *t2 = (float *)malloc(nw * sizeof(float));
	for (i = 0; i < nw; i++)
		t2[i] = (q1[i] + w[i]) * dT;

	for (i = 0; i < nUAV; i++)
	{
		if (UAVtypes[i].method != 1)
			continue;

		isUAV = 1;

		if (UAVtypes[i].isFixedFreq)
		{
			isInFreqSpan = 0;
			for (j = 0; j < UAVtypes[i].nfreq; j++)
			{
				freqL = UAVtypes[i].freqPoints[j] - UAVtypes[i].pulseBW[UAVtypes[i].nPulseBW - 1] / 2;
				freqR = UAVtypes[i].freqPoints[j] + UAVtypes[i].pulseBW[UAVtypes[i].nPulseBW - 1] / 2;
				freqIndexL = (freqL - cenFreq) / dF + NFFT / N / 2;
				freqIndexR = (freqR - cenFreq) / dF + NFFT / N / 2;
				if (colIndexS > freqIndexL && colIndexE <= freqIndexR)
				{
					isInFreqSpan = 1;
					break;
				}
			}
			if (!isInFreqSpan)
				continue;
		}
		else
		{
			freqL = UAVtypes[i].freqPoints[0];
			freqR = UAVtypes[i].freqPoints[1];
			freqIndexL = (freqL - cenFreq) / dF + NFFT / N / 2;
			freqIndexR = (freqR - cenFreq) / dF + NFFT / N / 2;

			//上下界有交叠则认为该目标可参与计算-此处后续可考虑做更精细的处理
			if (colIndexE <= freqIndexL || colIndexS > freqIndexR)
				continue;
		}

		for (j = 0; j < maxPulseWNumInLib; j++)
			mPulseW[j] = 0;
		nIndex = 0;

		for (j = 0; j < nw; j++)
		{
			for (k = 0; k < UAVtypes[i].nPulseW; k++)
			{
				if (fabs(UAVtypes[i].pulseW[k] - w[j] * dT) < UAVtypes[i].pulseWErr[k])
				{
					Indexq[nIndex] = j;
					nIndex++;
					mPulseW[k]++;
					break;
				}
			}
		}

		for (j = 0; j < UAVtypes[i].nPulseW; j++)
		{
			if (UAVtypes[i].meetPulseW[j] > 0 && mPulseW[j] < UAVtypes[i].meetPulseW[j])
			{
				isUAV = 0;
				break;
			}
			if (UAVtypes[i].meetPulseW[j] < 0 && mPulseW[j] >= -UAVtypes[i].meetPulseW[j])
			{
				isUAV = 0;
				break;
			}
		}

		if (isUAV && UAVtypes[i].nPulseT > 0 && UAVtypes[i].pulseT[0] > 0)
		{
			isUAV = 0;

			sfMatchPulseT(match, t2, Indexq, nIndex, UAVtypes[i].pulseT, UAVtypes[i].nPulseT, UAVtypes[i].pulseTErr);

			for (j = 0; j < nIndex; j++)
			{
				if (j > MaxNumVidT - 1)
					break;
				meetPulseTCount = 0;
				for (k = 0; k < MaxNumVidT; k++)
				{
					if (match[j][k] > 0)
					{
						meetPulseTCount++;
					}
				}


				if (meetPulseTCount > 0 && meetPulseTCount >= UAVtypes[i].hoppCnt)
				{

					//-----修改从此开始

					// 20230523误报处理：针对O3误报做一个最大脉冲数与满足分选的比较
					// 风险点为如果有干扰，可能就无法分选出来了
					if (i == 2)
					{
						// 求最大脉冲编号
						int maxpn = -1;
						for (int tmi = 0; tmi < 50; tmi++)
						{
							if (match[j][tmi] > maxpn)
							{
								maxpn = match[j][tmi];
							}
						}
						// 计算本参数平均脉冲个数
						float sumPulseT = 0.0f;
						for (int tmi = 0; tmi < UAVtypes[i].nPulseT; tmi++)
							sumPulseT += UAVtypes[i].pulseT[tmi];
						int tmpn = (120 / sumPulseT)*UAVtypes[i].nPulseT;
						if (tmpn<50 && maxpn>tmpn*1.75)
						{
							continue;// 脉冲过密则认为不判断
						}
					}

					// 20230624误报处理：针对EVOII误报做一个频率分选比较统计
					// 风险点为如果有干扰，可能就无法分选出来了
					if (i >= 4 && i <= 5)
					{
						// 求信号脉宽
						float tmpbw = (colIndexE - colIndexS)*0.4;
						if (tmpbw > 16)
						{
							continue;// 信号太宽则认为不判断
						}
					}

					// 20230522误报处理：针对8-16 Autel Lt/V3/M4T 误报处理
					// P1：检测120ms是否存在一次及以上的7ms中断
					// P2：检测[1 1 1 2]序列是否存在一次及以上
					// P3：[1 1 1 2]序列中，2ms周期中间应不存在其他脉冲
					// P4：7ms中断不足一次，严格的[1 1 1 2]逻辑不足3个，则认为是虚警
					if (i >= 8 && i <= 13) // 索引号严格核对库信息
					{
						float timePRT[50];
						int timecnt = 0;
						int prtcnt = 0;
						int numPw[50]; // 记录所有满足分选的有效脉冲编号

						for (int tmi = 0; tmi < 50; tmi++)//获取有效到达时间信息队列
						{
							if (match[j][tmi] > 0)
							{
								timePRT[timecnt] = t2[match[j][tmi]];
								numPw[timecnt] = match[j][tmi];
								timecnt++;
							}
						}
						//计算周期与7ms的对应关系-6~11，同时记录重复周期的变化规律
						float timeJG[50];
						for (int tmi = 1; tmi < timecnt; tmi++)
						{
							timeJG[tmi - 1] = (timePRT[tmi] - timePRT[tmi - 1]);
							if ((timePRT[tmi] - timePRT[tmi - 1]) > 6 && (timePRT[tmi] - timePRT[tmi - 1]) < 11)
							{
								prtcnt++; // 正常应该有3次，要不要检测两次-50里
							}

						}
						// 计算重复周期满足[1 1 1 2] 规律，且比较2ms周期中间是否有脉冲
						int jgcnt1 = 0;
						int jgcnt2 = 0;
						for (int tmi = 0; tmi < timecnt - 1 - 3; tmi++)
						{
							jgcnt2 = 0;
							for (int tmj = 0; tmj < 4; tmj++)
							{
								if (tmj < 3 && (timeJG[tmi + tmj] - 1) < 0.2)
								{
									jgcnt2++;
								}
								else if (tmj == 3 && (timeJG[tmi + tmj] - 2) < 0.2)
								{
									jgcnt2++;
								}
							}
							if (jgcnt2 == 4 && (numPw[tmi + 4] - numPw[tmi + 3]) == 1) // 判第四个周期间有没有脉冲
							{
								jgcnt1++;
							}
						}
						if (prtcnt < 1 || jgcnt1 < 5)//如果7ms大周期不足1个（只有50个脉冲）或者满足的[1112]周期数量不足3个，则认为是虚警
						{
							continue;
						}
						else
						{

						}
					}

					// 20230522误报处理：针对17-19 非常规Autel V3/M4T 误报处理-后续可能会单独修改
					// P1：检测120ms是否存在一次及以上的7ms中断
					// P2：检测[1 1 1 2]序列是否存在一次及以上
					// P3：[1 1 1 2]序列中，2ms周期中间应不存在其他脉冲
					// P4：7ms中断不足一次，严格的[1 1 1 2]逻辑不足3个，则认为是虚警
					if (i >= 14 && i <= 16) // 索引号严格核对库信息
					{
						float timePRT[50];
						int timecnt = 0;
						int prtcnt = 0;
						int numPw[50]; // 记录所有满足分选的有效脉冲编号

						for (int tmi = 0; tmi < 50; tmi++)//获取有效到达时间信息队列
						{
							if (match[j][tmi] > 0)
							{
								timePRT[timecnt] = t2[match[j][tmi]];
								numPw[timecnt] = match[j][tmi];
								timecnt++;
							}
						}
						//计算周期与7ms的对应关系-6~11，同时记录重复周期的变化规律
						float timeJG[50];
						for (int tmi = 1; tmi < timecnt; tmi++)
						{
							timeJG[tmi - 1] = (timePRT[tmi] - timePRT[tmi - 1]);
							if ((timePRT[tmi] - timePRT[tmi - 1]) > 6 && (timePRT[tmi] - timePRT[tmi - 1]) < 10)
							{
								prtcnt++; // 正常应该有1-2次
							}

						}
						// 计算重复周期满足[1 1 1 2] 规律，且比较2ms周期中间是否有脉冲
						int jgcnt1 = 0;
						int jgcnt2 = 0;
						for (int tmi = 0; tmi < timecnt - 1 - 3; tmi++)
						{
							jgcnt2 = 0;
							for (int tmj = 0; tmj < 4; tmj++)
							{
								if (tmj < 3 && (timeJG[tmi + tmj] - 1) < 0.2)
								{
									jgcnt2++;
								}
								else if (tmj == 3 && (timeJG[tmi + tmj] - 2) < 0.2)
								{
									jgcnt2++;
								}
							}
							if (jgcnt2 == 4 && (numPw[tmi + 4] - numPw[tmi + 3]) == 1) // 判第四个周期间有没有脉冲
							{
								jgcnt1++;
							}
						}
						if (prtcnt < 1 || jgcnt1 < 5)//如果7ms大周期不足1个（只有50个脉冲）或者满足的[1112]周期数量不足3个，则认为是虚警
						{
							continue;
						}
						else
						{

						}
					}

					// 20230522误报处理：针对17-19 nano 误报处理
					// P1：检测120ms是否存在一次及以上的7ms中断
					// P2：检测[1 1 3]序列是否存在一次及以上
					// P3：[1 1 3]序列中，3ms周期中间应不存在其他脉冲
					// P4：7ms中断不足一次，严格的[1 1 3]逻辑不足3个，则认为是虚警
					if (i >= 17 && i <= 20) // 索引号严格核对库信息-20230905扩展至龙鱼
					{
						float timePRT[50];
						int timecnt = 0;
						int prtcnt = 0;
						int numPw[50]; // 记录所有满足分选的有效脉冲编号

						for (int tmi = 0; tmi < 50; tmi++)//获取有效到达时间信息队列
						{
							if (match[j][tmi] > 0)
							{
								timePRT[timecnt] = t2[match[j][tmi]];
								numPw[timecnt] = match[j][tmi];
								timecnt++;
							}
						}
						//计算周期与7ms的对应关系-6~11，同时记录重复周期的变化规律
						float timeJG[50];
						for (int tmi = 1; tmi < timecnt; tmi++)
						{
							timeJG[tmi - 1] = (timePRT[tmi] - timePRT[tmi - 1]);
							if ((timePRT[tmi] - timePRT[tmi - 1]) > 6 && (timePRT[tmi] - timePRT[tmi - 1]) < 10)
							{
								prtcnt++; // 正常应该有1-2次
							}

						}
						// 计算重复周期满足[1 1 3] 规律，且比较3ms周期中间是否有脉冲
						int jgcnt1 = 0;
						int jgcnt2 = 0;
						for (int tmi = 0; tmi < timecnt - 1 - 2; tmi++)
						{
							jgcnt2 = 0;
							for (int tmj = 0; tmj < 3; tmj++)
							{
								if (tmj < 2 && (timeJG[tmi + tmj] - 1) < 0.2)
								{
									jgcnt2++;
								}
								else if (tmj == 2 && (timeJG[tmi + tmj] - 3) < 0.2)
								{
									jgcnt2++;
								}
							}
							if (jgcnt2 == 3 && (numPw[tmi + 3] - numPw[tmi + 2]) == 1) // 判第三个周期间有没有脉冲
							{
								jgcnt1++;
							}
						}
						if (prtcnt < 1 || jgcnt1 < 5)//如果7ms大周期不足1个（只有50个脉冲）或者满足的[113]周期数量不足3个，则认为是虚警
						{
							continue;
						}
						else
						{

						}
					}

					//-----修改到此结束

					isUAV = 1;
					break;
				}

			}

			if (isUAV)
			{
				nIndex = 0;
				for (k = 0; k < MaxNumVidT; k++)
				{
					if (match[j][k] >= 0)
					{
						Indexq[nIndex] = match[j][k];
						nIndex++;
					}
				}
			}
		}

		if (isUAV)
		{
			minSnr = colSnr[Indexq[0]];
			for (j = 0; j < nIndex; j++)
			{
				if (colSnr[Indexq[j]] < minSnr)
					minSnr = colSnr[Indexq[j]];
				if (colSnr[Indexq[j]] < uavTipicalSNR)
				{
					isUAV = 0;
					break;
				}
			}
		}

		if (isUAV)
		{
			(*aBlkRow).id = 0;
			(*aBlkRow).uavIndex = i;
			(*aBlkRow).antIndex = antIndex;
			(*aBlkRow).colIndex = (int)((colIndexE - colIndexS) / 2);
			count = 0;
			for (j = colIndexS; j <= colIndexE; j++)
			{
				(*aBlkRow).allColIndex[count] = j;
				count++;
			}
			(*aBlkRow).snr = minSnr;
			(*aBlkRow).nCol = colIndexE - colIndexS + 1;
			(*aBlkRow).meanColAmp = 0.0;
			count = 0;
			for (j = 0; j < nIndex; j++)
			{
				(*aBlkRow).q1[j] = q1[Indexq[j]];
				(*aBlkRow).w[j] = w[Indexq[j]];
				for (k = (*aBlkRow).q1[j]; k < (*aBlkRow).q1[j] + (*aBlkRow).w[j]; k++)
				{
					(*aBlkRow).meanColAmp += s[k];
				}
				count += (*aBlkRow).w[j];
			}
			(*aBlkRow).meanColAmp /= (float)count;
			(*aBlkRow).nw = nIndex;
			free(Indexq);
			free(t2);

			for (i = 0; i < MaxVidPulse; i++)
				free(match[i]);
			free(match);

			return 1;
		}
	}
	free(Indexq);
	free(t2);

	for (i = 0; i < MaxVidPulse; i++)
		free(match[i]);
	free(match);

	return 0;
}
// 周期匹配
void sfMatchPulseT(int **match, float *t2, int *index, int n, float *pulseT, int nPulseT, float pulseTErr)
{
	float diffTime, resTime, TErr;
	float sumPulseT = 0.0f;
	int   nt = 0, nT = 0, nBigT = 0;
	int   count = 0, maxCount = 0, maxIndex = 0;
	int i, j, k, p, m;
	static int   oneMatch[5][MaxNumVidT];
	static float oneMatchTimeErr[5][MaxNumVidT];

	for (i = 0; i < nPulseT; i++)
		sumPulseT += pulseT[i];

	for (i = 0; i < n; i++)
	{
		if (i > MaxNumVidT - 1)
			break;
		for (k = 0; k < nPulseT; k++)
		{
			oneMatch[k][0] = index[i];
			for (m = 1; m < MaxNumVidT; m++)
				oneMatch[k][m] = -1;

			for (j = 0; j < n; j++)
			{
				if (j > MaxNumVidT - 1)
					break;
				if (i == j)
					continue;

				diffTime = t2[index[j]] - t2[index[i]];
				if (diffTime < 0)
					continue;

				nBigT = diffTime / sumPulseT;
				resTime = diffTime - nBigT * sumPulseT;

				nt = 0;
				while (1)
				{
					TErr = fabs(resTime);
					if (TErr < pulseTErr)
					{
						nT = nPulseT * nBigT + nt;
						if (nT > 0 && nT < MaxNumVidT)
						{
							if (oneMatch[k][nT] < 0)
							{
								oneMatch[k][nT] = index[j];
								oneMatchTimeErr[k][nT] = TErr;
							}
							else if (TErr < oneMatchTimeErr[k][nT])
							{
								oneMatch[k][nT] = index[j];
								oneMatchTimeErr[k][nT] = TErr;
							}
						}
						break;
					}
					if (resTime < 0)
						break;
					resTime -= pulseT[(k + nt) % nPulseT];
					nt++;
				}
			}
		}

		maxCount = 0;
		maxIndex = 0;
		for (k = 0; k < nPulseT; k++)
		{
			count = 0;
			for (p = 0; p < MaxNumVidT; p++)
			{
				if (oneMatch[k][p] > 0)
				{
					count++;
				}
			}
			if (count > maxCount)
			{
				maxCount = count;
				maxIndex = k;
			}
		}

		for (k = 0; k < MaxNumVidT; k++)
		{
			match[i][k] = oneMatch[maxIndex][k];
		}
	}
}
// 脉冲参数转目标参数
void sfPulseToTarget(struct blockGroup *droneBlock, int nBlock, struct BlockRow aBlkRow, int fstart, int fend, float cenFreq, float dT, float dF, struct DroneLib *UAVtypes)
{
	int i = 0, j = 0, k = 0;
	if (nBlock >= MaxDrone)
		return;

	droneBlock[nBlock].id = nBlock;
	droneBlock[nBlock].index = aBlkRow.uavIndex;
	droneBlock[nBlock].range = 0;
	droneBlock[nBlock].bw = (fend - fstart)*dF;

	float freq[MaxPulseInGroup];
	float bw;

	droneBlock[nBlock].n = aBlkRow.nw;
	for (j = 0; j < NCh; j++)
	{
		droneBlock[nBlock].inCh[j] = aBlkRow.inCh[j];
	}
	for (j = 0; j < aBlkRow.nw; j++)
	{
		droneBlock[nBlock].freq[j] = cenFreq + ((float)(fend + fstart) / 2 - (NFFT / N / 2))*dF;
		droneBlock[nBlock].burstTime[j] = aBlkRow.q1[j] * dT;
		droneBlock[nBlock].width[j] = aBlkRow.w[j] * dT;
		droneBlock[nBlock].dist[j] = 0;
		for (k = 0; k < NCh; k++)
		{
			droneBlock[nBlock].amp[k][j] = aBlkRow.meanColAmp;
		}
	}
	nBlock++;
}
// 求幅度门限
float sfAutoThreshOnLineF(float *line, int d128, float dT, int maxIter)
{
	int   i = 0, j = 0, k = 0;
	int   n1 = 0;
	float t, tnew = 0.0f;
	float t1 = 0.0f, t2 = 0.0f;


	// 20230706-除数值域限制
	if (d128 < 32)
	{
		return 0;
	}

	// 计算功率值总和

	for (j = 0; j < d128; j++)
	{
		tnew += line[j] / d128;
	}
	t = tnew + 2 * dT;

	while (fabs(tnew - t) > dT && k < maxIter)
	{
		t = tnew;

		for (j = 0; j < d128; j++)
		{
			if (line[j] > t)
			{
				t1 += line[j];
				n1++;
			}
			else
				t2 += line[j];
		}

		// 20230706-修正除零风险
		if (n1 == 0)
			tnew = t;
		if (d128 > n1)
			tnew = t1 / n1 / 4 * 2 + t2 / (d128 - n1) / 4 * 2; // 改了门限分配比例
		else
			tnew = t;

		t1 = 0.0f;
		t2 = 0.0f;
		n1 = 0;
		k++;
	}
	return tnew;
}
// 幅度阈值限定
void  sfThresholdCompare(float* thresh, float cenFreq, int tdB, int mdB)
{
	// 根据频点判定固定门限保护值
	for (int i = 0; i < listNum; i++)
	{
		if (listFrequence[i] == cenFreq)
		{
			if (*thresh < (listNoice[i] + tdB * 85))
			{
				*thresh = listNoice[i] + tdB * 85;
			}
			else if (*thresh > (listNoice[i] + mdB * 85))
			{
				*thresh = listNoice[i] + mdB * 85;
			}
			break;
		}
	}
}
// 对外接口
void sfSetDigiResult(struct droneResult *droneInfo, int *nDrone, struct DroneLib *droneDigiLIB, struct blockGroup *uavPulse, int nUavPulse, char type)
{
	float maxAmp = 0.0f;
	for (int i = 0; i < nUavPulse; i++)
	{
		// 类型与库编号
		droneInfo[*nDrone + i].type = type;
		droneInfo[*nDrone + i].index = uavPulse[i].index;
		// ID号录入
		droneInfo[*nDrone + i].ID = uavPulse[i].id;
		// 名称录入
		memcpy(droneInfo[*nDrone + i].name, droneDigiLIB[uavPulse[i].index].name, 50);
		// psbID录入
		for (int j = 0; j < droneDigiLIB[uavPulse[i].index].nPsbID; j++)
		{
			droneInfo[*nDrone + i].psbID[j] = droneDigiLIB[uavPulse[i].index].psbID[j];
		}
		droneInfo[*nDrone + i].nPsbID = droneDigiLIB[uavPulse[i].index].nPsbID;
		// 编号录入
		droneInfo[*nDrone + i].flag = uavPulse[i].index;

		droneInfo[*nDrone + i].amp = 0.0f;
		for (int j = 0; j < uavPulse[i].n; j++)
		{
			droneInfo[*nDrone + i].freq[j] = uavPulse[i].freq[j];
			droneInfo[*nDrone + i].burstTime[j] = uavPulse[i].burstTime[j];
			droneInfo[*nDrone + i].width[j] = uavPulse[i].width[j];
			maxAmp = uavPulse[i].amp[0][j];
			for (int k = 0; k < NCh; k++)
			{
				if (uavPulse[i].amp[k][j] > maxAmp)
					maxAmp = uavPulse[i].amp[k][j];
			}
			droneInfo[*nDrone + i].amp += maxAmp;
		}
		droneInfo[*nDrone + i].bw = uavPulse[i].bw;
		droneInfo[*nDrone + i].amp = droneInfo[*nDrone + i].amp / (float)uavPulse[i].n;
		droneInfo[*nDrone + i].range = 0;
		// 20230818-补充数字图传信号距离估计算法
		if (droneInfo[*nDrone + i].type == 1)
		{
			float refPtGt = 0;
			float refR = 0;
			float refF = 0;
			float refBW = 0;
			float refPr = 0;

			float tmp1 = 0;

			if (droneInfo[*nDrone + i].freq[0] >= 2400 && droneInfo[*nDrone + i].freq[0] <= 2483.5)
			{
				// 参考标定定义
				refPtGt = 30;
				refR = 1000;
				refF = 5745;
				refBW = 20;
			}
			else if (droneInfo[*nDrone + i].freq[0] >= 5150 && droneInfo[*nDrone + i].freq[0] <= 5250)
			{


			}
			else if (droneInfo[*nDrone + i].freq[0] >= 5725 && droneInfo[*nDrone + i].freq[0] <= 5850)
			{
				// 参考标定定义
				refPtGt = 30;
				refR = 1020;
				refF = 5745;
				refBW = 20;
				refPr = 12340 / 85;

				tmp1 = refPr + 20 * log10(3E8 / 5745E6) + 20 * log10(refR) - refPtGt;
				tmp1 = tmp1 - droneInfo[*nDrone + i].amp / 85 - 20 * log10(3E8 / (droneInfo[*nDrone + i].freq[0] * 1E6)) + droneDigiLIB[uavPulse[i].index].cPower[0];
				tmp1 = pow(10, tmp1 / 20);
				droneInfo[*nDrone + i].range = tmp1;
			}
		}

	}

	*nDrone = *nDrone + nUavPulse;
}


// 模拟图传检测主函数
void sfMoniSniffer(struct blockGroup *droneBlock, int *nBlock, unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chn, int nRows, int nCols, float cenFreq)
{
	int i = 0, j = 0, k = 0, flag = 0;

	float fieldMaxPower = 0;
	float fieldMinPower = 0;
	float listnoice = 0;
	float powerGate = 0;
	float normal = 0;
	static int indexField[3000] = { 0 };
	static int indexAll[3000] = { 0 };
	static int diff[2999] = { 0 };
	static float CenF[20] = { 0 };
	static float T_uav[20] = { 0 };
	static float Amp_uav[20] = { 0 };
	static int pulseW[3000] = { 0 };
	static int pulseStart[3000] = { 0 };

	int count = 0;
	int count1 = 0;
	int N1 = 0;
	int N2 = 0;
	int N_uav = 0;
	int indexS = 14;
	int indexE = 114;

	// 新增，用于统计通过的目标上下信号是否截止
	float staThd = 0;
	int staCnt1 = 0;
	int staCnt2 = 0;


	for (i = 0; i < 30; i++)
	{
		if (listFrequence[i] == cenFreq)
		{
			listnoice = listNoice[i];
			break;
		}
	}

	if (cenFreq == 842)
	{
		indexS = (int)((832 - 842) / 0.4 + 64);
		indexE = (int)((862 - 842) / 0.4 + 64);
	}
	if (cenFreq == 915)
	{
		indexS = (int)((902 - 915) / 0.4 + 64);
		indexE = (int)((928 - 915) / 0.4 + 64);
	}

	//检测可疑场信号40Mhz带宽内是否满足周期性-20的考虑为信号上边带至少有6M
	for (j = indexS; j < indexE - 15; j++)
	{

		fieldMaxPower = downMat[chn][1][j];
		fieldMinPower = fieldMaxPower;
		powerGate = 0;
		count = 0;
		count1 = 0;
		N1 = 0;
		N2 = 0;

		//判断幅度最大值是否小于底噪
		for (int m = 0 + 1; m < 3000 - 1; m++)
		{

			if (downMat[chn][m][j] > fieldMaxPower)
			{
				fieldMaxPower = downMat[chn][m][j];
			}
		}
		if (fieldMaxPower < listnoice)
		{
			continue;
		}

		count = 0;
		//每500个采样点作为一个区间，将时域分为6个区间
		for (k = 0; k < 6; k++)
		{
			fieldMaxPower = downMat[chn][k * 500 + 1][j];
			fieldMinPower = fieldMaxPower;
			//首尾不参与比较
			for (int m = 0 + 1; m < 500 - 1; m++)
			{
				if (fieldMaxPower < downMat[chn][k * 500 + m][j])
				{
					fieldMaxPower = downMat[chn][k * 500 + m][j];
				}
				if (fieldMinPower > downMat[chn][k * 500 + m][j])
				{
					fieldMinPower = downMat[chn][k * 500 + m][j];
				}
			}

			if (fieldMaxPower - 85 * 6 > listnoice + 85 * 3)
			{
				powerGate = fieldMaxPower - 85 * 6;
			}
			else
			{
				powerGate = listnoice + 85 * 3;
			}

			//
			for (int m = 0; m < 500; m++)
			{
				if (downMat[chn][k * 500 + m][j] > powerGate)
				{
					indexField[count] = k * 500 + m;
					count++;
					indexAll[k * 500 + m] = 1;
				}
				else
				{
					indexAll[k * 500 + m] = 0;
				}
			}
		}
		// 点数超过太多直接退出
		count = 0;
		for (k = 0; k < 3000; k++)
		{
			if (indexAll[k] == 1)
			{
				count++;
			}
		}
		if (count > 100)
		{
			continue;
		}

		count = 0;
		//获取差分序列，并记录脉宽及起始位置(保留脉宽在4-6的脉冲)
		for (int m = 0; m < 2999; m++)
		{
			diff[m] = indexAll[m + 1] - indexAll[m];
			if (diff[m] == 1)
			{
				pulseStart[count1] = m + 1;
				count1++;
			}
			if (diff[m] == -1)
			{
				if (count1 > 0)
				{
					int w = m + 1 - pulseStart[count1 - 1];
					if (w < 4 || w > 6)
					{
						pulseStart[count1 - 1] = 0;
						count1--;
					}
					else
					{
						pulseW[count1 - 1] = w;
						count++;
					}
				}
			}
		}

		if (count < 5 || count > 15)
		{
			continue;
		}

		for (k = 0; k < count - 1; k++)
		{
			if ((pulseStart[k + 1] - pulseStart[k]) < 505 && (pulseStart[k + 1] - pulseStart[k]) > 495)
			{
				N1++;
			}
			if ((pulseStart[k + 1] - pulseStart[k]) < 422 && (pulseStart[k + 1] - pulseStart[k]) > 412)
			{
				N2++;
			}
		}

		fieldMaxPower = 0;
		for (k = 0; k < 3000; k++)
		{
			if (indexAll[k] == 1)
			{
				if (downMat[chn][k][j] > fieldMaxPower)
				{
					fieldMaxPower = downMat[chn][k][j];
				}
			}
		}

		if (N1 >= 4)
		{
			// 统计下5行以及上25行信号
			staThd = 0;
			staCnt1 = 0;
			staCnt2 = 0;
			if (fieldMaxPower - 85 * 20 > listnoice + 85 * 3)
			{
				staThd = fieldMaxPower - 85 * 20;
			}
			else
			{
				staThd = listnoice + 85 * 3;
			}
			for (k = 0; k < 3000; k++)
			{
				if (downMat[chn][k][j - 5] > staThd)
				{
					staCnt1++;
				}
				// 上边带
				if (downMat[chn][k][j + 25] > staThd)
				{
					staCnt2++;
				}
			}
			if (staCnt1 > 300 || staCnt2 > 300)
			{
				continue;
			}

			CenF[N_uav] = cenFreq + (j - 64)*0.4;
			T_uav[N_uav] = 20;
			Amp_uav[N_uav] = fieldMaxPower;
			N_uav++;
			//printf("PAL  :CF:%f   TF:%f\n", cenFreq, cenFreq + (j - 64)*0.4);
		}
		if (N2 >= 5)
		{
			// 统计下5行以及上25行信号
			staThd = 0;
			staCnt1 = 0;
			staCnt2 = 0;
			if (fieldMaxPower - 85 * 20 > listnoice + 85 * 3)
			{
				staThd = fieldMaxPower - 85 * 20;
			}
			else
			{
				staThd = listnoice + 85 * 3;
			}
			for (k = 0; k < 3000; k++)
			{
				if (downMat[chn][k][j - 5] > staThd)
				{
					staCnt1++;
				}
				// 上边带
				if (downMat[chn][k][j + 25] > staThd)
				{
					staCnt2++;
				}
			}
			if (staCnt1 > 300 || staCnt2 > 300)
			{
				continue;
			}
			CenF[N_uav] = cenFreq + (j - 64)*0.4;
			T_uav[N_uav] = 16.67;
			Amp_uav[N_uav] = fieldMaxPower;
			N_uav++;
			//printf("NTSC  :CF:%f   TF:%f\n", cenFreq, cenFreq + (j - 64)*0.4);
		}
	}



	fieldMaxPower = Amp_uav[0];
	k = 0;
	for (i = 0; i < N_uav; i++)
	{
		if (Amp_uav[i] > fieldMaxPower)
		{
			k = i;
			fieldMaxPower = Amp_uav[i];
		}
	}

	if (N_uav > 0)
	{
		int tmpBlock = *nBlock;
		sfMoniToTarget(droneBlock, tmpBlock, cenFreq, CenF[k], 10, Amp_uav[k], T_uav[k]);
		*nBlock = *nBlock + 1;
	}
}
// 模拟图传转目标参数
void sfMoniToTarget(struct blockGroup *droneBlock, int nBlock, float cenFreq, float tarFreq, float tarBW, float tarAm, float T)
{
	int i = 0, j = 0, k = 0;
	if (nBlock >= MaxDrone)
		return;

	droneBlock[nBlock].id = nBlock;
	droneBlock[nBlock].range = 100;
	droneBlock[nBlock].bw = tarBW;

	droneBlock[nBlock].n = 2;
	for (j = 0; j < NCh; j++)
	{
		droneBlock[nBlock].inCh[j] = 0;
	}
	for (j = 0; j < 2; j++)
	{
		droneBlock[nBlock].freq[j] = tarFreq;
		droneBlock[nBlock].burstTime[j] = 0;
		droneBlock[nBlock].width[j] = T;
		droneBlock[nBlock].dist[j] = 0;
		for (k = 0; k < NCh; k++)
		{
			droneBlock[nBlock].amp[k][j] = tarAm;
		}
	}
	nBlock++;
}
// 对外接口
void sfSetMoniResult(struct droneResult *droneInfo, int *nDrone, struct blockGroup *uavPulse, int nUavPulse, char type)
{
	float maxAmp = 0.0f;
	for (int i = 0; i < nUavPulse; i++)
	{
		// 类型与库编号
		droneInfo[*nDrone + i].type = type;
		droneInfo[*nDrone + i].index = uavPulse[i].index;
		// ID号录入
		droneInfo[*nDrone + i].ID = uavPulse[i].id;
		// 名称录入
		memcpy(droneInfo[*nDrone + i].name, "FPV               ", 50);
		// 编号录入
		droneInfo[*nDrone + i].flag = uavPulse[i].index;

		droneInfo[*nDrone + i].amp = 0.0f;
		for (int j = 0; j < uavPulse[i].n; j++)
		{
			droneInfo[*nDrone + i].freq[j] = uavPulse[i].freq[j];
			droneInfo[*nDrone + i].burstTime[j] = uavPulse[i].burstTime[j];
			droneInfo[*nDrone + i].width[j] = uavPulse[i].width[j];
			maxAmp = uavPulse[i].amp[0][j];
			for (int k = 0; k < NCh; k++)
			{
				if (uavPulse[i].amp[k][j] > maxAmp)
					maxAmp = uavPulse[i].amp[k][j];
			}
			droneInfo[*nDrone + i].amp += maxAmp;
		}
		droneInfo[*nDrone + i].bw = uavPulse[i].bw;
		droneInfo[*nDrone + i].amp = droneInfo[*nDrone + i].amp / (float)uavPulse[i].n;
		droneInfo[*nDrone + i].range = 0;
	}
	*nDrone = *nDrone + nUavPulse;
}



// 目标跟踪部分-------------------------------------------------

void sfTargetManageCluster(struct targetInfo *tar, int *tarNum, struct droneResult *drone, int droneNum, unsigned int calCnt, float cenFreq)
{
	// 全局用于新建目标计数，程序全程不清零，下电清零
	static unsigned short g_tarNum = 0;

	int i = 0, j = 0, k = 0;
	float freqMeet = 0;
	float tmp1S = 0, tmp1E = 0;
	float tmp2S = 0, tmp2E = 0;
	float tmp3S = 0, tmp3E = 0;
	int ppFlag = 0;

	// 测量信息分类计算
	for (i = 0; i < droneNum; i++)
	{
		ppFlag = 0;
		// 逐一匹配计算
		for (j = 0; j < *tarNum; j++)
		{
			//数字图传类别匹配-图传库索引一致
			if (tar[j].type == drone[i].type && tar[j].type == 1)
			{
				// Autel Lt/V3/M4T数字图传机型三合一，直接更新目标
				if ((drone[i].index > 10 && drone[i].index < 14) && (tar[j].typeIndex > 10 && tar[j].typeIndex < 14))
				{
					tmp1S = (tar[j].freqSMean + tar[j].freqEMean) / 2;
					tmp1E = tar[j].freqEMean - tar[j].freqSMean;

					// 信息合并-原始数据录入
					for (k = 0; k < TarDataLength - 1; k++)
					{
						tar[j].sqTime[TarDataLength - 1 - k] = tar[j].sqTime[TarDataLength - 1 - k - 1];
						tar[j].sqAmp[TarDataLength - 1 - k] = tar[j].sqAmp[TarDataLength - 1 - k - 1];
						tar[j].sqFreq[TarDataLength - 1 - k] = tar[j].sqFreq[TarDataLength - 1 - k - 1];
						tar[j].sqBw[TarDataLength - 1 - k] = tar[j].sqBw[TarDataLength - 1 - k - 1];
					}
					tar[j].sqTime[0] = calCnt;
					tar[j].sqAmp[0] = drone[i].amp;
					tar[j].sqFreq[0] = drone[i].freq[0];
					tar[j].sqBw[0] = drone[i].bw;
					if (tar[j].cnt >= 4E9)
					{
						tar[j].cnt = 4E9;
					}
					else
					{
						tar[j].cnt = tar[j].cnt + 1;
					}
					// 跟踪参数更新
					if (calCnt - tar[j].timeLock < 5)
					{
						// 一轮以内-中心频率离得近的优先
						tmp1E = abs(tar[j].trackFreq - tar[j].trackCenterFreq);
						tmp2E = abs(drone[i].freq[0] - cenFreq);
						if (tmp2E < tmp1E)
						{
							tar[j].trackFreq = drone[i].freq[0];
							tar[j].trackBw = drone[i].bw;
							tar[j].trackCenterFreq = cenFreq;
							tmp1S = 64 + ((tar[j].trackFreq - tar[j].trackCenterFreq) / sfdF);
							tmp2S = ((tar[j].trackBw / sfdF) / 2);
							tar[j].trackBwS = (tmp1S - tmp2S);
							tar[j].trackBwE = (tmp1S + tmp2S);
							tar[j].timeLock = calCnt;
						}
					}
					else
					{
						// 超过一轮后跟踪新的目标
						tar[j].trackFreq = drone[i].freq[0];
						tar[j].trackBw = drone[i].bw;
						tar[j].trackCenterFreq = cenFreq;
						tmp1S = 64 + ((tar[j].trackFreq - tar[j].trackCenterFreq) / sfdF);
						tmp2S = ((tar[j].trackBw / sfdF) / 2);
						tar[j].trackBwS = (tmp1S - tmp2S);
						tar[j].trackBwE = (tmp1S + tmp2S);
						tar[j].timeLock = calCnt;
					}
					tar[j].timeLast = calCnt;
					// 计算参数更新
					tmp1S = 0;
					tmp1E = 0;
					tmp2S = 20000;
					tmp2E = 0;
					tmp3S = 20000;
					tmp3E = 0;
					if (tar[j].cnt >= TarDataLength)
					{
						for (k = 0; k < TarDataLength; k++)
						{
							tmp1S = tmp1S + tar[j].sqAmp[k];
							if (tmp2S > tar[j].sqAmp[k])
							{
								tmp2S = tar[j].sqAmp[k];
							}
							if (tmp2E < tar[j].sqAmp[k])
							{
								tmp2E = tar[j].sqAmp[k];
							}
							if (tmp3S > tar[j].sqFreq[k] - tar[j].sqBw[k] / 2)
							{
								tmp3S = tar[j].sqFreq[k] - tar[j].sqBw[k] / 2;
							}
							if (tmp3E < tar[j].sqFreq[k] + tar[j].sqBw[k] / 2)
							{
								tmp3E = tar[j].sqFreq[k] + tar[j].sqBw[k] / 2;
							}

						}
						tar[j].update = (tar[j].sqTime[0] - tar[j].sqTime[TarDataLength - 1]) / (TarDataLength - 1);
						tar[j].ampMean = tmp1S / TarDataLength;
						tar[j].ampQF = tmp2E - tmp2S;
						tar[j].freqSMean = tmp3S;
						tar[j].freqEMean = tmp3E;
					}
					else if (tar[j].cnt < TarDataLength && tar[j].cnt > 1)
					{
						for (k = 0; k < tar[j].cnt; k++)
						{
							tmp1S = tmp1S + tar[j].sqAmp[k];
							if (tmp2S > tar[j].sqAmp[k])
							{
								tmp2S = tar[j].sqAmp[k];
							}
							if (tmp2E < tar[j].sqAmp[k])
							{
								tmp2E = tar[j].sqAmp[k];
							}
							if (tmp3S > tar[j].sqFreq[k] - tar[j].sqBw[k] / 2)
							{
								tmp3S = tar[j].sqFreq[k] - tar[j].sqBw[k] / 2;
							}
							if (tmp3E < tar[j].sqFreq[k] + tar[j].sqBw[k] / 2)
							{
								tmp3E = tar[j].sqFreq[k] + tar[j].sqBw[k] / 2;
							}
						}
						tar[j].update = (tar[j].sqTime[0] - tar[j].sqTime[tar[j].cnt - 1]) / (tar[j].cnt - 1);
						tar[j].ampMean = tmp1S / tar[j].cnt;
						tar[j].ampQF = tmp2E - tmp2S;
						tar[j].freqSMean = tmp3S;
						tar[j].freqEMean = tmp3E;
					}
					else
					{
						tar[j].update = 0;
						tar[j].ampMean = tar[j].sqAmp[0];
						tar[j].ampQF = 0;
						tar[j].freqSMean = tar[j].sqFreq[0] - tar[j].sqBw[0] / 2;
						tar[j].freqEMean = tar[j].sqFreq[0] + tar[j].sqBw[0] / 2;
					}
					// 距离信息计算
					float refPtGt = 0;
					float refR = 0;
					float refF = 0;
					float refBW = 0;
					float refPr = 0;
					float tmp1 = 0;
					float tmp2 = tar[j].freqEMean - tar[j].freqSMean;

					if (tar[j].trackFreq >= 2400 && tar[j].trackFreq <= 2483.5)
					{
						// 参考标定定义
						refPtGt = 30;
						refR = 1000;
						refF = 5745;
						refBW = 20;

						tar[j].dist = 0;
					}
					else if (tar[j].trackFreq >= 5150 && tar[j].trackFreq <= 5250)
					{
						tar[j].dist = 0;
					}
					else if (tar[j].trackFreq >= 5725 && tar[j].trackFreq <= 5850)
					{
						// 参考标定定义
						refPtGt = 30;
						refR = 1020;
						refF = 5745;
						refBW = 20;
						refPr = 12340 / 85;

						tmp1 = refPr + 20 * log10(3E8 / 5745E6) + 20 * log10(refR) - refPtGt;
						tmp1 = tmp1 - tar[j].ampMean / 85 - 20 * log10(3E8 / (tar[j].trackFreq * 1E6)) + droneDigiLIB[tar[j].typeIndex].cPower[0];
						tmp1 = pow(10, tmp1 / 20);
						tar[j].dist = tmp1;
						if (tmp2 >= 30)
						{
							tar[j].dist = tmp1 * 1.414;
						}
						else if (tmp2 < 30 && tmp2 >= 15)
						{
							tar[j].dist = tmp1 * 1;
						}
						else if (tmp2 < 15 && tmp2 >= 5)
						{
							tar[j].dist = tmp1 * 0.707;
						}
					}
					// 匹配后退出循环
					ppFlag = 1;
					break;
				}
				// 数字图传类别匹配-图传库索引一致（除Autel Lt/V3/M4T外的数字图传机型）
				else if (tar[j].typeIndex == drone[i].index)
				{
					// 计算交集-交集大于2.5M则认为是同一目标
					tmp1S = (tar[j].freqSMean + tar[j].freqEMean) / 2;
					tmp1E = tar[j].freqEMean - tar[j].freqSMean;

					freqMeet = tmp1E / 2 + drone[i].bw / 2 - abs(tmp1S - drone[i].freq[0]);

					if (freqMeet < 2.5)
					{
						continue;
					}
					// 信息合并-原始数据录入
					for (k = 0; k < TarDataLength - 1; k++)
					{
						tar[j].sqTime[TarDataLength - 1 - k] = tar[j].sqTime[TarDataLength - 1 - k - 1];
						tar[j].sqAmp[TarDataLength - 1 - k] = tar[j].sqAmp[TarDataLength - 1 - k - 1];
						tar[j].sqFreq[TarDataLength - 1 - k] = tar[j].sqFreq[TarDataLength - 1 - k - 1];
						tar[j].sqBw[TarDataLength - 1 - k] = tar[j].sqBw[TarDataLength - 1 - k - 1];
					}
					tar[j].sqTime[0] = calCnt;
					tar[j].sqAmp[0] = drone[i].amp;
					tar[j].sqFreq[0] = drone[i].freq[0];
					tar[j].sqBw[0] = drone[i].bw;
					if (tar[j].cnt >= 4E9)
					{
						tar[j].cnt = 4E9;
					}
					else
					{
						tar[j].cnt = tar[j].cnt + 1;
					}
					// 跟踪参数更新
					if (calCnt - tar[j].timeLock < listNum + 5)
					{
						// 一轮以内-中心频率离得近的优先
						tmp1E = abs(tar[j].trackFreq - tar[j].trackCenterFreq);
						tmp2E = abs(drone[i].freq[0] - cenFreq);
						if (tmp2E < tmp1E)
						{
							tar[j].trackFreq = drone[i].freq[0];
							tar[j].trackBw = drone[i].bw;
							tar[j].trackCenterFreq = cenFreq;
							tmp1S = 64 + ((tar[j].trackFreq - tar[j].trackCenterFreq) / sfdF);
							tmp2S = ((tar[j].trackBw / sfdF) / 2);
							tar[j].trackBwS = (tmp1S - tmp2S);
							tar[j].trackBwE = (tmp1S + tmp2S);
							tar[j].timeLock = calCnt;
						}
					}
					else
					{
						// 超过一轮后跟踪新的目标
						tar[j].trackFreq = drone[i].freq[0];
						tar[j].trackBw = drone[i].bw;
						tar[j].trackCenterFreq = cenFreq;
						tmp1S = 64 + ((tar[j].trackFreq - tar[j].trackCenterFreq) / sfdF);
						tmp2S = ((tar[j].trackBw / sfdF) / 2);
						tar[j].trackBwS = (tmp1S - tmp2S);
						tar[j].trackBwE = (tmp1S + tmp2S);
						tar[j].timeLock = calCnt;
					}
					tar[j].timeLast = calCnt;
					// 计算参数更新
					tmp1S = 0;
					tmp1E = 0;
					tmp2S = 20000;
					tmp2E = 0;
					tmp3S = 20000;
					tmp3E = 0;
					if (tar[j].cnt >= TarDataLength)
					{
						for (k = 0; k < TarDataLength; k++)
						{
							tmp1S = tmp1S + tar[j].sqAmp[k];
							if (tmp2S > tar[j].sqAmp[k])
							{
								tmp2S = tar[j].sqAmp[k];
							}
							if (tmp2E < tar[j].sqAmp[k])
							{
								tmp2E = tar[j].sqAmp[k];
							}
							if (tmp3S > tar[j].sqFreq[k] - tar[j].sqBw[k] / 2)
							{
								tmp3S = tar[j].sqFreq[k] - tar[j].sqBw[k] / 2;
							}
							if (tmp3E < tar[j].sqFreq[k] + tar[j].sqBw[k] / 2)
							{
								tmp3E = tar[j].sqFreq[k] + tar[j].sqBw[k] / 2;
							}

						}
						tar[j].update = (tar[j].sqTime[0] - tar[j].sqTime[TarDataLength - 1]) / (TarDataLength - 1);
						tar[j].ampMean = tmp1S / TarDataLength;
						tar[j].ampQF = tmp2E - tmp2S;
						tar[j].freqSMean = tmp3S;
						tar[j].freqEMean = tmp3E;
					}
					else if (tar[j].cnt < TarDataLength && tar[j].cnt > 1)
					{
						for (k = 0; k < tar[j].cnt; k++)
						{
							tmp1S = tmp1S + tar[j].sqAmp[k];
							if (tmp2S > tar[j].sqAmp[k])
							{
								tmp2S = tar[j].sqAmp[k];
							}
							if (tmp2E < tar[j].sqAmp[k])
							{
								tmp2E = tar[j].sqAmp[k];
							}
							if (tmp3S > tar[j].sqFreq[k] - tar[j].sqBw[k] / 2)
							{
								tmp3S = tar[j].sqFreq[k] - tar[j].sqBw[k] / 2;
							}
							if (tmp3E < tar[j].sqFreq[k] + tar[j].sqBw[k] / 2)
							{
								tmp3E = tar[j].sqFreq[k] + tar[j].sqBw[k] / 2;
							}
						}
						tar[j].update = (tar[j].sqTime[0] - tar[j].sqTime[tar[j].cnt - 1]) / (tar[j].cnt - 1);
						tar[j].ampMean = tmp1S / tar[j].cnt;
						tar[j].ampQF = tmp2E - tmp2S;
						tar[j].freqSMean = tmp3S;
						tar[j].freqEMean = tmp3E;
					}
					else
					{
						tar[j].update = 0;
						tar[j].ampMean = tar[j].sqAmp[0];
						tar[j].ampQF = 0;
						tar[j].freqSMean = tar[j].sqFreq[0] - tar[j].sqBw[0] / 2;
						tar[j].freqEMean = tar[j].sqFreq[0] + tar[j].sqBw[0] / 2;
					}
					// 距离信息计算
					float refPtGt = 0;
					float refR = 0;
					float refF = 0;
					float refBW = 0;
					float refPr = 0;
					float tmp1 = 0;
					float tmp2 = tar[j].freqEMean - tar[j].freqSMean;

					if (tar[j].trackFreq >= 2400 && tar[j].trackFreq <= 2483.5)
					{
						// 参考标定定义
						refPtGt = 30;
						refR = 1000;
						refF = 5745;
						refBW = 20;

						tar[j].dist = 0;
					}
					else if (tar[j].trackFreq >= 5150 && tar[j].trackFreq <= 5250)
					{
						tar[j].dist = 0;
					}
					else if (tar[j].trackFreq >= 5725 && tar[j].trackFreq <= 5850)
					{
						// 参考标定定义
						refPtGt = 30;
						refR = 1020;
						refF = 5745;
						refBW = 20;
						refPr = 12340 / 85;

						tmp1 = refPr + 20 * log10(3E8 / 5745E6) + 20 * log10(refR) - refPtGt;
						tmp1 = tmp1 - tar[j].ampMean / 85 - 20 * log10(3E8 / (tar[j].trackFreq * 1E6)) + droneDigiLIB[tar[j].typeIndex].cPower[0];
						tmp1 = pow(10, tmp1 / 20);
						tar[j].dist = tmp1;
						if (tmp2 >= 30)
						{
							tar[j].dist = tmp1 * 1.414;
						}
						else if (tmp2 < 30 && tmp2 >= 15)
						{
							tar[j].dist = tmp1 * 1;
						}
						else if (tmp2 < 15 && tmp2 >= 5)
						{
							tar[j].dist = tmp1 * 0.707;
						}
					}
					// 匹配后退出循环
					ppFlag = 1;
					break;
				}
			}
			// 模拟图传类别匹配-图传库索引一致
			if (tar[j].type == drone[i].type && tar[j].type == 2)
			{
				// 频率相差10M以内则认为是同一目标
				if (fabs(tar[j].trackFreq - drone[i].freq[0]) > 10.0)
				{
					continue;
				}
				// 信息合并-原始数据录入
				for (k = 0; k < TarDataLength - 1; k++)
				{
					tar[j].sqTime[TarDataLength - 1 - k] = tar[j].sqTime[TarDataLength - 1 - k - 1];
					tar[j].sqAmp[TarDataLength - 1 - k] = tar[j].sqAmp[TarDataLength - 1 - k - 1];
					tar[j].sqFreq[TarDataLength - 1 - k] = tar[j].sqFreq[TarDataLength - 1 - k - 1];
					tar[j].sqBw[TarDataLength - 1 - k] = tar[j].sqBw[TarDataLength - 1 - k - 1];
				}
				tar[j].sqTime[0] = calCnt;
				tar[j].sqAmp[0] = drone[i].amp;
				tar[j].sqFreq[0] = drone[i].freq[0];
				tar[j].sqBw[0] = drone[i].bw;
				if (tar[j].cnt >= 4E9)
				{
					tar[j].cnt = 4E9;
				}
				else
				{
					tar[j].cnt = tar[j].cnt + 1;
				}
				// 跟踪参数更新
				if (calCnt - tar[j].timeLock < listNum + 5)
				{
					// 一轮以内-中心频率离得近的优先
					tmp1E = abs(tar[j].trackFreq - tar[j].trackCenterFreq);
					tmp2E = abs(drone[i].freq[0] - cenFreq);
					if (tmp2E < tmp1E)
					{
						tar[j].trackFreq = drone[i].freq[0];
						tar[j].trackBw = drone[i].bw;
						tar[j].trackCenterFreq = cenFreq;
						tmp1S = 64 + ((tar[j].trackFreq - tar[j].trackCenterFreq) / sfdF);
						tmp2S = ((tar[j].trackBw / sfdF) / 2);
						tar[j].trackBwS = (tmp1S - tmp2S);
						tar[j].trackBwE = (tmp1S + tmp2S);
						tar[j].timeLock = calCnt;
					}
				}
				else
				{
					// 超过一轮后跟踪新的目标
					tar[j].trackFreq = drone[i].freq[0];
					tar[j].trackBw = drone[i].bw;
					tar[j].trackCenterFreq = cenFreq;
					tmp1S = 64 + ((tar[j].trackFreq - tar[j].trackCenterFreq) / sfdF);
					tmp2S = ((tar[j].trackBw / sfdF) / 2);
					tar[j].trackBwS = (tmp1S - tmp2S);
					tar[j].trackBwE = (tmp1S + tmp2S);
					tar[j].timeLock = calCnt;
				}
				tar[j].timeLast = calCnt;
				// 计算参数更新
				tmp1S = 0;
				tmp1E = 0;
				tmp2S = 20000;
				tmp2E = 0;
				tmp3S = 20000;
				tmp3E = 0;
				if (tar[j].cnt >= TarDataLength)
				{
					for (k = 0; k < TarDataLength; k++)
					{
						tmp1S = tmp1S + tar[j].sqAmp[k];
						if (tmp2S > tar[j].sqAmp[k])
						{
							tmp2S = tar[j].sqAmp[k];
						}
						if (tmp2E < tar[j].sqAmp[k])
						{
							tmp2E = tar[j].sqAmp[k];
						}
						if (tmp3S > tar[j].sqFreq[k] - tar[j].sqBw[k] / 2)
						{
							tmp3S = tar[j].sqFreq[k] - tar[j].sqBw[k] / 2;
						}
						if (tmp3E < tar[j].sqFreq[k] + tar[j].sqBw[k] / 2)
						{
							tmp3E = tar[j].sqFreq[k] + tar[j].sqBw[k] / 2;
						}

					}
					tar[j].update = (tar[j].sqTime[0] - tar[j].sqTime[TarDataLength - 1]) / (TarDataLength - 1);
					tar[j].ampMean = tmp1S / TarDataLength;
					tar[j].ampQF = tmp2E - tmp2S;
					tar[j].freqSMean = tmp3S;
					tar[j].freqEMean = tmp3E;
				}
				else if (tar[j].cnt < TarDataLength && tar[j].cnt > 1)
				{
					for (k = 0; k < tar[j].cnt; k++)
					{
						tmp1S = tmp1S + tar[j].sqAmp[k];
						if (tmp2S > tar[j].sqAmp[k])
						{
							tmp2S = tar[j].sqAmp[k];
						}
						if (tmp2E < tar[j].sqAmp[k])
						{
							tmp2E = tar[j].sqAmp[k];
						}
						if (tmp3S > tar[j].sqFreq[k] - tar[j].sqBw[k] / 2)
						{
							tmp3S = tar[j].sqFreq[k] - tar[j].sqBw[k] / 2;
						}
						if (tmp3E < tar[j].sqFreq[k] + tar[j].sqBw[k] / 2)
						{
							tmp3E = tar[j].sqFreq[k] + tar[j].sqBw[k] / 2;
						}
					}
					tar[j].update = (tar[j].sqTime[0] - tar[j].sqTime[tar[j].cnt - 1]) / (tar[j].cnt - 1);
					tar[j].ampMean = tmp1S / tar[j].cnt;
					tar[j].ampQF = tmp2E - tmp2S;
					tar[j].freqSMean = tmp3S;
					tar[j].freqEMean = tmp3E;
				}
				else
				{
					tar[j].update = 0;
					tar[j].ampMean = tar[j].sqAmp[0];
					tar[j].ampQF = 0;
					tar[j].freqSMean = tar[j].sqFreq[0] - tar[j].sqBw[0] / 2;
					tar[j].freqEMean = tar[j].sqFreq[0] + tar[j].sqBw[0] / 2;
				}
				tar[j].dist = 0;
				// 匹配后退出循环
				ppFlag = 1;
				break;
			}
			// Wifi图传类别匹配-图传库索引一致
			if (tar[j].type == drone[i].type && tar[j].type == 3 && tar[j].typeIndex == drone[i].index)
			{
				// 计算mac地址，完全一致则属于同一目标
				int idcnt = 0;
				for (k = 0; k < 8; k++)
				{
					if (tar[j].ID[k] == drone[i].uni[k])
					{
						idcnt = idcnt + 1;
					}
				}
				if (idcnt < 8)
				{
					continue;
				}
				// 信息合并-原始数据录入
				for (k = 0; k < TarDataLength - 1; k++)
				{
					tar[j].sqTime[TarDataLength - 1 - k] = tar[j].sqTime[TarDataLength - 1 - k - 1];
					tar[j].sqAmp[TarDataLength - 1 - k] = tar[j].sqAmp[TarDataLength - 1 - k - 1];
					tar[j].sqFreq[TarDataLength - 1 - k] = tar[j].sqFreq[TarDataLength - 1 - k - 1];
					tar[j].sqBw[TarDataLength - 1 - k] = tar[j].sqBw[TarDataLength - 1 - k - 1];
				}
				tar[j].sqTime[0] = calCnt;
				tar[j].sqAmp[0] = drone[i].amp;
				tar[j].sqFreq[0] = drone[i].freq[0];
				tar[j].sqBw[0] = drone[i].bw;
				if (tar[j].cnt >= 4E9)
				{
					tar[j].cnt = 4E9;
				}
				else
				{
					tar[j].cnt = tar[j].cnt + 1;
				}
				// 跟踪参数更新
				if (calCnt - tar[j].timeLock < listNum + 5)
				{
					// 一轮以内-中心频率离得近的优先
					tmp1E = abs(tar[j].trackFreq - tar[j].trackCenterFreq);
					tmp2E = abs(drone[i].freq[0] - cenFreq);
					if (tmp2E < tmp1E)
					{
						tar[j].trackFreq = drone[i].freq[0];
						tar[j].trackBw = drone[i].bw;
						tar[j].trackCenterFreq = cenFreq;
						tmp1S = 64 + ((tar[j].trackFreq - tar[j].trackCenterFreq) / sfdF);
						tmp2S = ((tar[j].trackBw / sfdF) / 2);
						tar[j].trackBwS = (tmp1S - tmp2S);
						tar[j].trackBwE = (tmp1S + tmp2S);
						tar[j].timeLock = calCnt;
					}
				}
				else
				{
					// 超过一轮后跟踪新的目标
					tar[j].trackFreq = drone[i].freq[0];
					tar[j].trackBw = drone[i].bw;
					tar[j].trackCenterFreq = cenFreq;
					tmp1S = 64 + ((tar[j].trackFreq - tar[j].trackCenterFreq) / sfdF);
					tmp2S = ((tar[j].trackBw / sfdF) / 2);
					tar[j].trackBwS = (tmp1S - tmp2S);
					tar[j].trackBwE = (tmp1S + tmp2S);
					tar[j].timeLock = calCnt;
				}
				tar[j].timeLast = calCnt;
				// 计算参数更新
				tmp1S = 0;
				tmp1E = 0;
				tmp2S = 20000;
				tmp2E = 0;
				tmp3S = 20000;
				tmp3E = 0;
				if (tar[j].cnt >= TarDataLength)
				{
					for (k = 0; k < TarDataLength; k++)
					{
						tmp1S = tmp1S + tar[j].sqAmp[k];
						if (tmp2S > tar[j].sqAmp[k])
						{
							tmp2S = tar[j].sqAmp[k];
						}
						if (tmp2E < tar[j].sqAmp[k])
						{
							tmp2E = tar[j].sqAmp[k];
						}
						if (tmp3S > tar[j].sqFreq[k] - tar[j].sqBw[k] / 2)
						{
							tmp3S = tar[j].sqFreq[k] - tar[j].sqBw[k] / 2;
						}
						if (tmp3E < tar[j].sqFreq[k] + tar[j].sqBw[k] / 2)
						{
							tmp3E = tar[j].sqFreq[k] + tar[j].sqBw[k] / 2;
						}

					}
					tar[j].update = (tar[j].sqTime[0] - tar[j].sqTime[TarDataLength - 1]) / (TarDataLength - 1);
					tar[j].ampMean = tmp1S / TarDataLength;
					tar[j].ampQF = tmp2E - tmp2S;
					tar[j].freqSMean = tmp3S;
					tar[j].freqEMean = tmp3E;
				}
				else if (tar[j].cnt < TarDataLength && tar[j].cnt > 1)
				{
					for (k = 0; k < tar[j].cnt; k++)
					{
						tmp1S = tmp1S + tar[j].sqAmp[k];
						if (tmp2S > tar[j].sqAmp[k])
						{
							tmp2S = tar[j].sqAmp[k];
						}
						if (tmp2E < tar[j].sqAmp[k])
						{
							tmp2E = tar[j].sqAmp[k];
						}
						if (tmp3S > tar[j].sqFreq[k] - tar[j].sqBw[k] / 2)
						{
							tmp3S = tar[j].sqFreq[k] - tar[j].sqBw[k] / 2;
						}
						if (tmp3E < tar[j].sqFreq[k] + tar[j].sqBw[k] / 2)
						{
							tmp3E = tar[j].sqFreq[k] + tar[j].sqBw[k] / 2;
						}
					}
					tar[j].update = (tar[j].sqTime[0] - tar[j].sqTime[tar[j].cnt - 1]) / (tar[j].cnt - 1);
					tar[j].ampMean = tmp1S / tar[j].cnt;
					tar[j].ampQF = tmp2E - tmp2S;
					tar[j].freqSMean = tmp3S;
					tar[j].freqEMean = tmp3E;
				}
				else
				{
					tar[j].update = 0;
					tar[j].ampMean = tar[j].sqAmp[0];
					tar[j].ampQF = 0;
					tar[j].freqSMean = tar[j].sqFreq[0] - tar[j].sqBw[0] / 2;
					tar[j].freqEMean = tar[j].sqFreq[0] + tar[j].sqBw[0] / 2;
				}
				tar[j].dist = 0;
				// 匹配后退出循环
				ppFlag = 1;
				break;
			}
		}

		// 若未匹配，且当前队列仍有位置，则新建目标
		if (*tarNum < TarLength && ppFlag == 0)
		{
			g_tarNum = g_tarNum + 1;
			tar[*tarNum].num = g_tarNum;
			tar[*tarNum].type = drone[i].type;
			tar[*tarNum].typeIndex = drone[i].index;

			tar[*tarNum].nPsbID = drone[i].nPsbID;
			for (k = 0; k < 10; k++)
			{
				tar[*tarNum].psbID[k] = drone[i].psbID[k];
			}

			for (k = 0; k < 50; k++)
			{
				tar[*tarNum].name[k] = drone[i].name[k];
			}
			if (tar[*tarNum].type == 1)
			{
				// 缺省填1
				for (k = 0; k < 32; k++)
				{
					tar[*tarNum].ID[k] = 1;
				}
			}
			else if (tar[*tarNum].type == 2)
			{
				// 缺省填2
				for (k = 0; k < 32; k++)
				{
					tar[*tarNum].ID[k] = 2;
				}
			}
			else if (tar[*tarNum].type == 3)
			{
				// 填Mac地址
				for (k = 0; k < 32; k++)
				{
					tar[*tarNum].ID[k] = drone[i].uni[k];
				}
			}
			tar[*tarNum].cnt = 1;
			tar[*tarNum].status = 1;			// 目标状态：1发现、2跟踪、3记忆
			tar[*tarNum].timeCreate = calCnt;	// 目标建立时间
			tar[*tarNum].timeLast = calCnt;		// 目标最近测量时间
			tar[*tarNum].timeLock = calCnt;		// 目标锁定时间
			tar[*tarNum].trackFreq = drone[i].freq[0];	// 跟踪频率
			tar[*tarNum].trackBw = drone[i].bw;			// 跟踪带宽
			tar[*tarNum].trackCenterFreq = cenFreq;		// 跟踪中心频点
			tmp1S = 64 + ((tar[*tarNum].trackFreq - tar[*tarNum].trackCenterFreq) / sfdF);
			tmp2S = ((tar[*tarNum].trackBw / sfdF) / 2);
			tar[*tarNum].trackBwS = (tmp1S - tmp2S);	// 跟踪频率起始索引
			tar[*tarNum].trackBwE = (tmp1S + tmp2S);	// 跟踪频率终止索引
			tar[*tarNum].update = 0;					// 数据更新率
			tar[*tarNum].ampMean = drone[i].amp;		// 目标平均幅度
			tar[*tarNum].ampQF = 0;						// 目标幅度起伏
			tar[*tarNum].freqSMean = tar[*tarNum].trackFreq - tar[*tarNum].trackBw / 2;	// 起始频率均值
			tar[*tarNum].freqEMean = tar[*tarNum].trackFreq + tar[*tarNum].trackBw / 2;	// 终止频率均值
			tar[*tarNum].dist = 0;						// 目标距离估计
			tar[*tarNum].sqTime[0] = calCnt;			// 到达时间序列
			tar[*tarNum].sqAmp[0] = drone[i].amp;		// 幅度序列
			tar[*tarNum].sqFreq[0] = drone[i].freq[0];	// 频率序列
			tar[*tarNum].sqBw[0] = drone[i].bw;			// 带宽序列

			tar[*tarNum].trackPriority = 0;	// 跟踪优先级

			*tarNum = *tarNum + 1;
		}
	}
}

void sfTargetManageStatus(struct targetInfo *tar, int *tarNum, unsigned int calCnt)
{
	// 维护目标列表的工作状态，并更新每一个目标的威胁优先级
	int i = 0, j = 0, k = 0;

	// 状态维护计算
	for (i = 0; i < *tarNum; i++)
	{
		// 发现状态
		if (tar[i].status == 1)
		{
			// 发现转跟踪
			if ((tar[i].timeLast - tar[i].timeCreate) > listNum + 5)
			{
				tar[i].status = 2;
			}
			// 发现转目标剔除
			if ((calCnt - tar[i].timeLast) > listNum * 3 + 5)
			{
				// 队列前移
				for (j = i; j < *tarNum - 1; j++)
				{
					tar[j] = tar[j + 1];
				}
				*tarNum = *tarNum - 1;
				// 计算位置前移
				i = i - 1;
			}
		}
		// 跟踪状态
		else if (tar[i].status == 2)
		{
			// 跟踪转记忆
			if ((calCnt - tar[i].timeLast) > listNum * 3 + 5)
			{
				tar[i].status = 3;
			}
		}
		// 记忆状态
		else if (tar[i].status == 3)
		{
			// 记忆转跟踪
			if ((calCnt - tar[i].timeLast) < 2)
			{
				tar[i].status = 2;
			}
			// 记忆转目标剔除
			if ((calCnt - tar[i].timeLast) > listNum * 6 + 5)
			{
				// 队列前移
				for (j = i; j < *tarNum - 1; j++)
				{
					tar[j] = tar[j + 1];
				}
				*tarNum = *tarNum - 1;
				// 计算位置前移
				i = i - 1;
			}
		}
	}

	// 优先级维护计算
	for (i = 0; i < *tarNum; i++)
	{
		// 类型、频率、工作状态
		if (tar[i].type == 1)
		{
			if (tar[i].status == 2)
			{
				if ((tar[i].trackFreq > 2400 && tar[i].trackFreq < 2483.5) || (tar[i].trackFreq > 5725 && tar[i].trackFreq < 5850))
				{
					tar[i].trackPriority = 22;// 可定向优先级高
				}
				else
				{
					tar[i].trackPriority = 12;// 不可定向优先级低
				}
			}
			else if (tar[i].status == 1)
			{
				tar[i].trackPriority = 5;// 发现状态-优先级中等
			}
			else
			{
				tar[i].trackPriority = 0;// 记忆状态-优先级低等
			}
		}
		else if (tar[i].type == 2)
		{
			if (tar[i].status == 2)
			{
				if ((tar[i].trackFreq > 2400 && tar[i].trackFreq < 2483.5) || (tar[i].trackFreq > 5725 && tar[i].trackFreq < 5850))
				{
					tar[i].trackPriority = 21;// 可定向优先级高
				}
				else
				{
					tar[i].trackPriority = 11;// 不可定向优先级低
				}
			}
			else if (tar[i].status == 1)
			{
				tar[i].trackPriority = 5;// 发现状态-优先级中等
			}
			else
			{
				tar[i].trackPriority = 0;// 记忆状态-优先级低等
			}
		}
		else if (tar[i].type == 3)
		{
			if (tar[i].status == 2)
			{
				if ((tar[i].trackFreq > 2400 && tar[i].trackFreq < 2483.5) || (tar[i].trackFreq > 5725 && tar[i].trackFreq < 5850))
				{
					tar[i].trackPriority = 13;
				}
				else
				{
					tar[i].trackPriority = 13;
				}
			}
			else if (tar[i].status == 1)
			{
				tar[i].trackPriority = 5;
			}
			else
			{
				tar[i].trackPriority = 0;
			}
		}
	}
}

void sfTargetSort(struct targetInfo *tar, int *tarNum, int *pList)
{
	// 当前目标优先级输出进行排序
	// tar[i].trackPriority 状态：22、21、13、12、11、5、0
	// 状态：22幅度优先 tar[i].ampMean(取大)
	// 状态：21幅度优先 tar[i].ampMean(取大)
	// 状态：13幅度优先 tar[i].ampMean(取大)
	// 状态：12幅度优先 tar[i].ampMean(取大)
	// 状态：11幅度优先 tar[i].ampMean(取大)
	// 状态：5 时间早优先 tar[i].timeCreate(取小)
	// 状态：0 时间近优先 tar[i].timeLast(取大)
	int allnum = *tarNum;

	// 无测量结果
	if (allnum == 0)
	{
		return;
	}

	// 单一测量结果
	if (allnum == 1)
	{
		pList[0] = 0;
	}

	// 多个测量结果
	else
	{
		for (int i = 0; i < allnum; i++)
		{
			pList[i] = i;     // 初始化pList数组为原始结构体数组顺序0~allnum-1
		}
		// 冒泡排序
		for (int i = 0; i < allnum - 1; i++)
		{
			for (int j = 0; j < allnum - i - 1; j++)
			{
				// 根据trackPriority状态值优先比较（较小的放后面）
				if (tar[pList[j]].trackPriority < tar[pList[j + 1]].trackPriority)
				{
					// 交换pList数组中的元素
					int temp = pList[j];
					pList[j] = pList[j + 1];
					pList[j + 1] = temp;
				}

				else if (tar[pList[j]].trackPriority == tar[pList[j + 1]].trackPriority && tar[pList[j]].trackPriority == 5)
				{
					// 如果trackPriority相同且为5，则根据timeCreate进行比较和交换（较大值放后面）
					if (tar[pList[j]].timeCreate > tar[pList[j + 1]].timeCreate)
					{
						// 交换pList数组中的元素
						int temp = pList[j];
						pList[j] = pList[j + 1];
						pList[j + 1] = temp;
					}
				}

				else if (tar[pList[j]].trackPriority == tar[pList[j + 1]].trackPriority && tar[pList[j]].trackPriority == 0)
				{
					// 如果trackPriority相同且为0，则根据timeLast进行比较和交换（较小值放后面）
					if (tar[pList[j]].timeLast < tar[pList[j + 1]].timeLast)
					{
						// 交换pList数组中的元素
						int temp = pList[j];
						pList[j] = pList[j + 1];
						pList[j + 1] = temp;
					}

				}

				else if (tar[pList[j]].trackPriority == tar[pList[j + 1]].trackPriority)
				{
					// 如果trackPriority相同，则根据ampMean进行比较和交换(较小值放后面)
					if (tar[pList[j]].ampMean < tar[pList[j + 1]].ampMean)
					{
						// 交换pList数组中的元素
						int temp = pList[j];
						pList[j] = pList[j + 1];
						pList[j + 1] = temp;
					}

				}
			}
		}
	}
}



// 定向函数部分-------------------------------------------------

// 图传定向幅度计算函数
int sfDxNewProcessing(unsigned short AmPowerQ[CHAN_NUM][LINE_NUM][COLUMN_NUM], unsigned short AmPowerD[CHAN_NUM][LINE_NUM][COLUMN_NUM], int StartLine, int EndLine, int numUav, int calRows, float *listAngle, float *listAmQ, float *listAmD, float *listAmC, int *listFP, int numDx)
{
	/*
	输入描述
	AmPowerQ：全向天线输入时频图
	AmPowerD：定向天线输入时频图
	StartLine：信号起始行号
	EndLine：信号结束行号
	numUav：匹配目标编号
	calRows：计算列数，默认3000（时间列）
	listAngle：每次定向时机对应方位角度索引(实际没用上)
	numDx：定向计数
	输出
	listAmQ：每次定向时机对应全向天线幅度索引
	listAmD：每次定向时机对应定向天线幅度索引
	listAmC：每次定向时机对应定向天线幅度索引
	listFP：每次定向时机对应计算的匹配情况索引
	*/

	// 计算全向天线分选模板
	static int pulseSQ1[sfM2];
	static int pulseWQ1[sfM2];
	int pulseNQ1 = 0;
	static int pulseSQ2[sfM2];
	static int pulseWQ2[sfM2];
	int pulseNQ2 = 0;
	int pulseFQ = 0;
	pulseFQ = sfDxNewGetIndex(AmPowerQ, StartLine, EndLine, droneDigiLIB, numUav, calRows, pulseSQ1, pulseWQ1, &pulseNQ1, pulseSQ2, pulseWQ2, &pulseNQ2);
	// 计算定向天线分选模板
	static int pulseSD1[sfM2];
	static int pulseWD1[sfM2];
	int pulseND1 = 0;
	static int pulseSD2[sfM2];
	static int pulseWD2[sfM2];
	int pulseND2 = 0;
	int pulseFD = 0;
	pulseFD = sfDxNewGetIndex(AmPowerD, StartLine, EndLine, droneDigiLIB, numUav, calRows, pulseSD1, pulseWD1, &pulseND1, pulseSD2, pulseWD2, &pulseND2);

	// 分选失败则赋0值，返回0值
	float tmpAm1 = 0;
	float tmpAm2 = 0;
	if (pulseFQ == 0 && pulseFD == 0)
	{
		listAmQ[numDx] = 0;
		listAmD[numDx] = 0;
		listAmC[numDx] = 0;
		listFP[numDx] = -1;
		return 0;
	}
	else if (pulseFQ == 2)
	{
		// 计算全向定向天线幅度
		tmpAm1 = sfDxNewCalculateAm(AmPowerQ, StartLine, EndLine, calRows, pulseSQ2, pulseWQ2, pulseNQ2);
		tmpAm2 = sfDxNewCalculateAm(AmPowerD, StartLine, EndLine, calRows, pulseSQ2, pulseWQ2, pulseNQ2);
		listAmC[numDx] = 1;
	}
	else if (pulseFD == 2)
	{
		// 计算全向定向天线幅度
		tmpAm1 = sfDxNewCalculateAm(AmPowerQ, StartLine, EndLine, calRows, pulseSD2, pulseWD2, pulseND2);
		tmpAm2 = sfDxNewCalculateAm(AmPowerD, StartLine, EndLine, calRows, pulseSD2, pulseWD2, pulseND2);
		listAmC[numDx] = 2;
	}
	else if (pulseFQ == 1)
	{
		// 计算全向定向天线幅度
		tmpAm1 = sfDxNewCalculateAm(AmPowerQ, StartLine, EndLine, calRows, pulseSQ1, pulseWQ1, pulseNQ1);
		tmpAm2 = sfDxNewCalculateAm(AmPowerD, StartLine, EndLine, calRows, pulseSQ1, pulseWQ1, pulseNQ1);
		listAmC[numDx] = 3;
	}
	else if (pulseFD == 1)
	{
		// 计算全向定向天线幅度
		tmpAm1 = sfDxNewCalculateAm(AmPowerQ, StartLine, EndLine, calRows, pulseSD1, pulseWD1, pulseND1);
		tmpAm2 = sfDxNewCalculateAm(AmPowerD, StartLine, EndLine, calRows, pulseSD1, pulseWD1, pulseND1);
		listAmC[numDx] = 4;
	}
	else
	{
		listAmQ[numDx] = 0;
		listAmD[numDx] = 0;
		listAmC[numDx] = 5;
		listFP[numDx] = -1;
		return 0;
	}
	// 更新定向阶段信息数据流
	listAmQ[numDx] = tmpAm1;
	listAmD[numDx] = tmpAm2;
	float tmpAmC = 0;
	if (listAmQ[numDx] > 10000 && listAmD[numDx] > 10000)
	{
		tmpAmC = tmpAm2 - tmpAm1 - corQD;// 要做匹配修正
	}
	else
	{
		tmpAmC = 0;
	}
	// 根据信息流队列计算目标匹配结果
	if (tmpAmC > 750)
	{
		listFP[numDx] = 2;
	}
	else if (tmpAmC > 550)
	{
		listFP[numDx] = 1;
	}
	else
	{
		listFP[numDx] = 0;
	}

	return 0;
}
// 计算全向幅度匹配模板
int sfDxNewGetIndex(unsigned short AmPower[CHAN_NUM][LINE_NUM][COLUMN_NUM], int StartLine, int EndLine, struct DroneLib *UAVtypes, int numUav, int calRows, int *ps1, int *pw1, int *pn1, int *ps2, int *pw2, int *pn2)
{
	// 求解多行自动门限-求取均值
	int nRows = calRows;
	static float colVecFilted[NRow / M];
	static int q1[NRow / M];
	static int w[NRow / M];
	static float colMeanAmp[NRow / M];
	static float colSnr[NRow / M];
	static int  Indexq[NRow / M];
	int status = 0;

	for (int j = 0; j < NRow / M; j++)
	{
		colVecFilted[j] = 0;
		q1[j] = 0;
		w[j] = 0;
		colMeanAmp[j] = 0;
		Indexq[j] = 0;
	}

	for (int j = 0; j < nRows; j++)
	{
		// 逐行计算
		for (int k = StartLine; k < EndLine; k++)
		{
			colVecFilted[j] += AmPower[0][j][k];
		}
		colVecFilted[j] = colVecFilted[j] / (EndLine - StartLine);
	}

	float thresh = 0;
	static float DT = 0.1;
	static int   MaxIter = 6;

	thresh = sfAutoThreshOnLineF(colVecFilted, nRows, DT, MaxIter);

	int nw = 0;
	static int   Delt = 1;
	float fs = 51.2f;
	float dF = fs / NFFT * N;
	float dT = M * NFFT / fs / 1000.0f;
	int   MinW = 0.7f / dT;
	int   q1IndexErr = 0.5f / dT;


	// 分离脉冲及对应的平均功率
	nw = sfDxThresholdWithAm(colVecFilted, nRows, thresh, Delt, MinW, q1, w, colMeanAmp);
	// 判定脉冲脉宽是否符合要求
	int   mPulseW[maxPulseWNumInLib];
	for (int j = 0; j < maxPulseWNumInLib; j++)
		mPulseW[j] = 0;

	int   nIndex = 0;
	for (int j = 0; j < nw; j++)
	{
		for (int k = 0; k < UAVtypes[numUav].nPulseW; k++)
		{
			if (fabs(UAVtypes[numUav].pulseW[k] - w[j] * dT) < UAVtypes[numUav].pulseWErr[k])
			{
				Indexq[nIndex] = j;
				nIndex++;
				mPulseW[k]++;
				break;
			}
		}
	}
	// 脉冲个数判定-匹配通过原则
	int isUAV = 1;

	for (int j = 0; j < UAVtypes[numUav].nPulseW; j++)
	{
		if (UAVtypes[numUav].meetPulseW[j] > 0 && mPulseW[j] < (UAVtypes[numUav].meetPulseW[j]))
		{
			isUAV = 0;
			break;
		}
		if (UAVtypes[numUav].meetPulseW[j] < 0 && mPulseW[j] >= -(UAVtypes[numUav].meetPulseW[j]))
		{
			isUAV = 0;
			break;
		}
	}

	// 如果不启用周期分选，只用脉宽分选 20230629
	if (isUAV)
	{
		for (int j = 0; j < nIndex; j++)
		{
			ps1[j] = q1[Indexq[j]];
			pw1[j] = w[Indexq[j]];
		}
		*pn1 = nIndex;
		status = 1;
	}
	else
	{
		*pn1 = 0;
		status = 0;
		return status;
	}

	// 周期满足判定
	float *t2 = (float *)malloc(nw * sizeof(float));
	for (int j = 0; j < nw; j++)
		t2[j] = (q1[j] + w[j]) * dT;

	int **match = (int**)malloc(MaxVidPulse * sizeof(int*));
	for (int j = 0; j < MaxVidPulse; j++)
		match[j] = (int*)malloc(MaxNumVidT * sizeof(int));

	int meetPulseTCount = 0;
	int i, j, k = 0;


	if (isUAV && UAVtypes[numUav].nPulseT > 0 && UAVtypes[numUav].pulseT[0] > 0)
	{
		isUAV = 0;
		sfMatchPulseT(match, t2, Indexq, nIndex, UAVtypes[numUav].pulseT, UAVtypes[numUav].nPulseT, UAVtypes[numUav].pulseTErr);
		/*
		// 测试代码
		int csmatch[MaxVidPulse][MaxVidPulse];
		for (i = 0; i < MaxVidPulse; i++)
			for (j = 0; j < MaxNumVidT; j++)
				csmatch[i][j] = match[i][j];
		*/
		for (j = 0; j < nIndex; j++)
		{
			if (j > MaxNumVidT - 1)
				break;
			meetPulseTCount = 0;
			for (k = 0; k < MaxNumVidT; k++)
			{
				if (match[j][k] > 0)
				{
					meetPulseTCount++;
				}
			}
			if (meetPulseTCount > 0 && meetPulseTCount >= UAVtypes[numUav].hoppCnt)
			{
				isUAV = 1;
				break;
			}
		}
		if (isUAV)
		{
			nIndex = 0;
			for (k = 0; k < MaxNumVidT; k++)
			{
				if (match[j][k] >= 0)
				{
					Indexq[nIndex] = match[j][k];
					nIndex++;
				}
			}
		}
	}

	// 模板转换输出
	if (isUAV)
	{
		for (j = 0; j < nIndex; j++)
		{
			ps2[j] = q1[Indexq[j]];
			pw2[j] = w[Indexq[j]];
		}
		*pn2 = nIndex;
		status = 2;

		free(t2);
		for (i = 0; i < MaxVidPulse; i++)
			free(match[i]);
		free(match);

		return status;
	}
	else
	{
		free(t2);
		for (i = 0; i < MaxVidPulse; i++)
			free(match[i]);
		free(match);

		return status;
	}

}
// 计算模板下匹配幅度
float sfDxNewCalculateAm(unsigned short AmPower[CHAN_NUM][LINE_NUM][COLUMN_NUM], int StartLine, int EndLine, int calRows, int *ps, int *pw, int pnum)
{
	// 行均值获取
	int nRows = calRows;
	static float colVecFilted[NRow / M];

	for (int j = 0; j < NRow / M; j++)
	{
		colVecFilted[j] = 0;
	}

	for (int j = 0; j < nRows; j++)
	{
		// 逐行计算
		for (int k = StartLine; k < EndLine; k++)
		{
			colVecFilted[j] += AmPower[0][j][k];
		}
		colVecFilted[j] = colVecFilted[j] / (EndLine - StartLine);
	}

	// 根据点号计算求和
	float finalAm = 0;
	float sumAm = 0;
	int sumP = 0;
	for (int j = 0; j < pnum; j++)
	{
		for (int k = 0; k < pw[j]; k++)
		{
			sumAm = sumAm + colVecFilted[ps[j] + k];
			sumP++;
		}
	}

	finalAm = sumAm / sumP;
	return finalAm;
}
// 分离出脉冲及幅度
int sfDxThresholdWithAm(float *s, int n, float thresh, int delt, int minw, int *q1, int *w, float *colAm)
{
	int i, j;
	char  flag = 1;
	int   nFall = 0;
	int   nw = 0;
	float signalAmp = 0.0f, noiseAmp = 0.0f;

	q1[0] = -1;
	for (i = 0; i < n; i++)
	{
		if (s[i] > thresh)
		{
			if (!flag)
			{
				q1[nw] = i;
				flag = 1;
			}
			nFall = 0;
		}
		else
		{
			if (flag)
			{
				nFall++;
				if (nFall >= delt)
				{
					if (q1[0] > 0)
					{
						// dingyu, 20230202
						w[nw] = i - delt - q1[nw];
						if (w[nw] > minw)
						{
							nw++;
						}
					}
					flag = 0;
				}
			}
		}
	}

	if (nw > 0)
	{
		for (i = 0; i < nw; i++)
		{
			signalAmp = 0.0f;
			for (j = q1[i]; j <= q1[i] + w[i]; j++)
			{
				signalAmp += s[j];
			}
			signalAmp /= w[i] + 1;
			colAm[i] = signalAmp;
		}
	}

	return nw;
}

// Wifi定向幅度计算函数
int sfDxWifiProcessing(unsigned short wifiPowerQ, unsigned short wifiPowerD, float *listAngle, float *listAmQ, float *listAmD, float *listAmC, int *listFP, int numDx)
{
	/*
	输入描述
	wifiPowerQ：全向天线120msWifi输入平均功率-120ms内的平均值请在函数输入之前算好
	wifiPowerD：定向天线120msWifi输入平均功率-120ms内的平均值请在函数输入之前算好
	listAngle：每次定向时机对应方位角度索引(实际没用上)
	numDx：定向计数
	输出
	listAmQ：每次定向时机对应全向天线幅度索引
	listAmD：每次定向时机对应定向天线幅度索引
	listAmC：每次定向时机对应定向天线幅度索引
	listFP：每次定向时机对应计算的匹配情况索引
	*/


	// 分选失败则赋0值，返回0值
	float tmpAm1 = wifiPowerQ;
	float tmpAm2 = wifiPowerD;

	if (tmpAm1 == 0 && tmpAm2 == 0)
	{
		listAmQ[numDx] = 0;
		listAmD[numDx] = 0;
		listAmC[numDx] = 0;
		listFP[numDx] = -1;
		return 0;
	}
	else
	{
		listAmC[numDx] = 1;
	}

	// 更新定向阶段信息数据流
	listAmQ[numDx] = tmpAm1;
	listAmD[numDx] = tmpAm2;
	float tmpAmC = 0;
	if (listAmQ[numDx] != 0 && listAmD[numDx] != 0)
	{
		tmpAmC = tmpAm2 - tmpAm1 - corQD;// 要做匹配修正
	}
	else
	{
		tmpAmC = 0;
	}
	// 根据信息流队列计算目标匹配结果
	if (tmpAmC > 750)
	{
		listFP[numDx] = 2;
	}
	else if (tmpAmC > 550)
	{
		listFP[numDx] = 1;
	}
	else
	{
		listFP[numDx] = 0;
	}

	return 0;
}


// 定向计算部分

int sfDxIfAzimuthEnough(float *listAngleAz, int numDx, float *oStart, float *oEnd)
{
	if (numDx < 2)
	{
		return(0);
	}
	if (numDx > MAXnAngle)
	{
		return(0);
	}

	float sumAzimuth = 0, diffAzimuth = 0;
	int iAzimuth = 0;
	float startAzimuth = 0, endAzimuth = 0;
	float scanAzimuth = 0;
	startAzimuth = listAngleAz[0];
	endAzimuth = listAngleAz[0] + 1;
	for (iAzimuth = 0; iAzimuth < numDx - 1; iAzimuth++)
	{
		diffAzimuth = listAngleAz[iAzimuth + 1] - listAngleAz[iAzimuth];
		if (diffAzimuth > 180)
		{
			diffAzimuth -= 360;
		}
		if (diffAzimuth < -180)
		{
			diffAzimuth += 360;
		}
		sumAzimuth += diffAzimuth;
		// 实时判断范围
		if (listAngleAz[0] + sumAzimuth > endAzimuth)
		{
			endAzimuth = listAngleAz[0] + sumAzimuth;
		}
		else if (listAngleAz[0] + sumAzimuth < startAzimuth)
		{
			startAzimuth = listAngleAz[0] + sumAzimuth;
		}
	}
	// 扫描范围计算
	scanAzimuth = endAzimuth - startAzimuth;
	if (scanAzimuth >= 360)
	{
		endAzimuth = startAzimuth + 359.9;
	}

	// 值域转换
	if (startAzimuth > 360)
	{
		startAzimuth -= 360;
	}
	if (startAzimuth < 0)
	{
		startAzimuth += 360;
	}
	if (endAzimuth > 360)
	{
		endAzimuth -= 360;
	}
	if (endAzimuth < 0)
	{
		endAzimuth += 360;
	}
	*oStart = startAzimuth;
	*oEnd = endAzimuth;

	if (scanAzimuth > 360)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

int sfDxIfLegal(float *listAngleAz, float *listAngleEl, float *listAmQ, float *listAmD, float *listAmC, int *listFP, int numDx)
{
	// 测量属性计算
	int flagScanB30 = 0;
	int flagScanA360 = 0;
	int flagAmQAD = 0;
	int arryFullM[360 + 20] = { 0 };
	int flagFullM = 0;

	// 0:点数过多或者过少
	// -1：扫描范围不足60度
	// -2：定向最大功率小于全向最大功率
	if (numDx < 2)
	{
		return(0);
	}
	if (numDx > MAXnAngle)
	{
		return(0);
	}

	float sumAzimuth = 0, diffAzimuth = 0;
	int iAzimuth = 0;
	float startAzimuth = 0, endAzimuth = 0;
	float scanAzimuth = 0;
	startAzimuth = listAngleAz[0];
	endAzimuth = listAngleAz[0] + 1;
	for (iAzimuth = 0; iAzimuth < numDx - 1; iAzimuth++)
	{
		diffAzimuth = listAngleAz[iAzimuth + 1] - listAngleAz[iAzimuth];
		if (diffAzimuth > 180)
		{
			diffAzimuth -= 360;
		}
		if (diffAzimuth < -180)
		{
			diffAzimuth += 360;
		}
		sumAzimuth += diffAzimuth;
		// 实时判断范围
		if (listAngleAz[0] + sumAzimuth > endAzimuth)
		{
			endAzimuth = listAngleAz[0] + sumAzimuth;
		}
		else if (listAngleAz[0] + sumAzimuth < startAzimuth)
		{
			startAzimuth = listAngleAz[0] + sumAzimuth;
		}
	}
	// 扫描范围计算
	scanAzimuth = endAzimuth - startAzimuth;
	// 标志计算-扫描范围
	if (scanAzimuth < 30)
	{
		flagScanB30 = 1;
	}
	else if (scanAzimuth >= 350)
	{
		scanAzimuth = 360;
		flagScanA360 = 1;
	}
	// 目标信号条件有效识别
	float maxDX = 0;
	float maxQX = 0;
	int i, j, k = 0;
	float tmp1 = 0;

	for (i = 0; i < numDx; i++)
	{
		// 查找全向定向最大值
		if (listAmQ[i] > maxQX)
		{
			maxQX = listAmQ[i];
		}
		if (listAmD[i] > maxDX)
		{
			maxDX = listAmD[i];
		}
		// 有效测量稀疏性计算
		if (listAmC[i] == 1 || listAmC[i] == 2)
		{
			tmp1 = round(listAngleAz[i]);
			j = (int)(tmp1);
			if (j < 0 || j >= 360)
			{
				j = 0;
			}
			arryFullM[j] = 1;
		}
	}
	// 有效测量稀疏性计算
	for (i = 0; i < 20; i++)
	{
		arryFullM[360 + i] = arryFullM[i];
	}
	// 标志计算-定向最大功率高于全向最大功率
	if (maxQX > maxDX)
	{
		flagAmQAD = 1;
	}
	// 有效测量稀疏性计算
	for (i = 0; i < 360; i++)
	{
		if (arryFullM[i] == 1)
		{
			// 前向链接
			for (j = 20; j > 0; j--)
			{
				if (arryFullM[i + j] == 1)
				{
					for (k = 1; k < j; k++)
					{
						arryFullM[i + k] = 1;
					}
					// 调整循环位置
					i = i + j - 1;
					break;
				}
			}

		}
	}
	for (i = 0; i < 20; i++)
	{
		if (arryFullM[i] == 1 || arryFullM[360 + i] == 1)
		{
			arryFullM[i] = 1;
		}
	}
	tmp1 = 0;
	for (i = 0; i < 360; i++)
	{
		if (arryFullM[i] == 1)
		{
			tmp1 = tmp1 + 1;
		}
	}
	tmp1 = tmp1 / scanAzimuth;
	if (tmp1 < 0.7)
	{
		flagFullM = 1;
	}
	// 计算结果至返回值转换
	if (flagScanB30 == 1)
	{
		return 0;
	}
	if (flagAmQAD == 1)
	{
		return -1;
	}
	// 扫描完备性不采用
	/*
	if (flagFullM == 1 && flagScanA360 == 1)
	{
		return -2;
	}
	*/
	return 1;
}

int sfDxDirCalc(float *ampDat, float *angleDat, int nAngle, float *ampMax, float *angleCen, int typeD)
{
	// typeD = 0代表选择2.4定向，typeD = 1代表选择5.8定向
	// 角度输入值必须在0-360之间，为前置约束
	// 20230704:修改输入数组均改为float类型

	// 预定义方向图，线性功率方向图
	static float pattern24[360] = { 0.9750,0.9638,0.9462,0.9290,0.9078,0.8831,0.8570,0.8293,0.7993,0.7675,0.7340,0.6992,0.6631,0.6260,0.5882,0.5498,0.5112,0.4727,0.4345,0.3969,0.3603,0.3250,0.2913,0.2595,0.2300,0.2031,0.1790,0.1581,0.1405,0.1264,0.1159,0.1091,0.1059,0.1062,0.1099,0.1165,0.1258,0.1373,0.1503,0.1643,0.1786,0.1926,0.2056,0.2169,0.2260,0.2324,0.2357,0.2356,0.2322,0.2254,0.2155,0.2030,0.1884,0.1724,0.1557,0.1392,0.1236,0.1097,0.09820,0.08960,0.08430,0.08250,0.08400,0.08880,0.09630,0.1060,0.1172,0.1292,0.1409,0.1517,0.1609,0.1677,0.1718,0.1727,0.1705,0.1652,0.1570,0.1464,0.1338,0.1198,0.1051,0.09030,0.07590,0.06250,0.05030,0.03980,0.03110,0.02420,0.01920,0.01590,0.01420,0.01390,0.01490,0.01690,0.01980,0.02350,0.02780,0.03270,0.03800,0.04370,0.04970,0.05590,0.06230,0.06870,0.07490,0.08070,0.08610,0.09070,0.09460,0.09740,0.09910,0.09960,0.09890,0.09700,0.09390,0.08970,0.08460,0.07870,0.07220,0.06540,0.05830,0.05130,0.04440,0.03780,0.03180,0.02630,0.02160,0.01750,0.01420,0.01170,0.009900,0.008900,0.008500,0.008800,0.009600,0.01080,0.01250,0.01440,0.01660,0.01900,0.02140,0.02390,0.02630,0.02870,0.03090,0.03300,0.03490,0.03660,0.03800,0.03930,0.04020,0.04100,0.04150,0.04170,0.04180,0.04170,0.04140,0.04090,0.04030,0.03950,0.03870,0.03780,0.03680,0.03580,0.03470,0.03360,0.03250,0.03150,0.03040,0.02940,0.02840,0.02750,0.02660,0.02580,0.02510,0.02440,0.02380,0.02340,0.02290,0.02260,0.02230,0.02220,0.02200,0.02200,0.02190,0.02190,0.02190,0.02190,0.02190,0.02180,0.02170,0.02150,0.02120,0.02080,0.02030,0.01960,0.01880,0.01790,0.01690,0.01580,0.01470,0.01350,0.01230,0.01120,0.01010,0.009200,0.008500,0.008000,0.007800,0.007900,0.008200,0.008900,0.009900,0.01120,0.01270,0.01430,0.01610,0.01780,0.01950,0.02100,0.02230,0.02320,0.02380,0.02390,0.02350,0.02270,0.02150,0.02000,0.01830,0.01650,0.01470,0.01300,0.01160,0.01060,0.01000,0.01000,0.01060,0.01170,0.01320,0.01520,0.01740,0.01970,0.02190,0.02400,0.02580,0.02710,0.02790,0.02830,0.02810,0.02750,0.02660,0.02550,0.02430,0.02320,0.02240,0.02180,0.02170,0.02200,0.02270,0.02380,0.02520,0.02680,0.02860,0.03040,0.03220,0.03400,0.03570,0.03730,0.03900,0.04070,0.04270,0.04480,0.04730,0.05020,0.05340,0.05690,0.06070,0.06450,0.06820,0.07170,0.07480,0.07740,0.07920,0.08030,0.08050,0.07990,0.07870,0.07680,0.07440,0.07170,0.06900,0.06620,0.06360,0.06140,0.05940,0.05790,0.05680,0.05610,0.05570,0.05550,0.05560,0.05580,0.05620,0.05670,0.05740,0.05820,0.05920,0.06060,0.06240,0.06470,0.06760,0.07120,0.07550,0.08070,0.08690,0.09400,0.1021,0.1112,0.1215,0.1328,0.1453,0.1590,0.1740,0.1903,0.2080,0.2271,0.2477,0.2698,0.2934,0.3186,0.3453,0.3735,0.4031,0.4340,0.4660,0.4991,0.5330,0.5673,0.6020,0.6368,0.6714,0.7055,0.7388,0.7711,0.8020,0.8316,0.8590,0.8851,0.9078,0.9290,0.9484,0.9638,0.9772,0.9863,0.9954,1,1,0.9977,0.9931,0.9863 };
	static float pattern58[360] = { 0.9931,0.9977,1,0.9954,0.9908,0.9795,0.9661,0.9462,0.9247,0.8954,0.861,0.8202,0.7736,0.7209,0.6631,0.6009,0.5355,0.4686,0.4019,0.3373,0.2766,0.2218,0.1739,0.1342,0.1029,0.0801,0.0650,0.0565,0.0532,0.0536,0.0562,0.0595,0.0628,0.0653,0.0671,0.0682,0.0690,0.0698,0.0706,0.0715,0.0721,0.0718,0.0704,0.0674,0.0628,0.0570,0.0505,0.0443,0.0394,0.0369,0.0380,0.0433,0.0533,0.0683,0.0876,0.110,0.1339,0.1571,0.177,0.1914,0.1987,0.198,0.1894,0.174,0.1538,0.1308,0.1075,0.0857,0.0672,0.0528,0.0433,0.0386,0.0386,0.0427,0.0504,0.0604,0.0718,0.0832,0.0938,0.1028,0.1101,0.1155,0.1195,0.1221,0.1234,0.1231,0.1206,0.1152,0.1067,0.0950,0.0809,0.0655,0.0505,0.0373,0.0271,0.0205,0.0174,0.0172,0.0193,0.0230,0.0281,0.0350,0.0439,0.0552,0.0683,0.0818,0.0936,0.1012,0.1024,0.0966,0.0843,0.0677,0.0499,0.0339,0.0220,0.0151,0.0127,0.0132,0.0149,0.0164,0.0169,0.0171,0.0182,0.0219,0.0294,0.0411,0.0564,0.0733,0.0891,0.1009,0.1064,0.1043,0.0945,0.0788,0.0599,0.0410,0.0252,0.0150,0.0117,0.0153,0.0250,0.0393,0.0562,0.0740,0.0912,0.1067,0.1195,0.1292,0.1353,0.1375,0.1358,0.1303,0.1213,0.1095,0.0958,0.0812,0.0666,0.0530,0.0410,0.0310,0.0232,0.0175,0.0137,0.0116,0.0110,0.0116,0.0132,0.0158,0.0192,0.0232,0.0278,0.0327,0.0374,0.0419,0.0457,0.0485,0.0501,0.0506,0.0500,0.0486,0.0468,0.0451,0.0440,0.0441,0.0456,0.0488,0.0537,0.0598,0.0667,0.0734,0.0791,0.0830,0.0843,0.0827,0.0782,0.0712,0.0626,0.0535,0.0451,0.0384,0.0342,0.0325,0.0328,0.0342,0.0353,0.0351,0.0326,0.0278,0.0212,0.0139,0.00740,0.00310,0.00170,0.00340,0.00770,0.0135,0.0200,0.0266,0.0335,0.0412,0.0499,0.0593,0.0681,0.0742,0.0753,0.0703,0.0599,0.0473,0.0373,0.0349,0.0433,0.0621,0.0874,0.1122,0.1294,0.1342,0.1255,0.1071,0.0850,0.0657,0.0530,0.0475,0.0467,0.0471,0.0463,0.0437,0.0409,0.0393,0.0394,0.0397,0.0380,0.0330,0.0258,0.0200,0.0203,0.0296,0.0475,0.0689,0.0868,0.0944,0.0890,0.0731,0.0535,0.0384,0.0347,0.0443,0.0645,0.0884,0.1082,0.1178,0.1148,0.1008,0.0809,0.0616,0.0483,0.0436,0.0468,0.0544,0.0619,0.0651,0.0624,0.0547,0.0449,0.0365,0.0319,0.0318,0.0349,0.0388,0.0414,0.0413,0.0387,0.0345,0.0300,0.0261,0.0229,0.0198,0.0165,0.0124,0.00800,0.00420,0.00250,0.00440,0.0111,0.0227,0.0385,0.0562,0.0733,0.0868,0.0947,0.0963,0.0925,0.0854,0.0777,0.0715,0.0680,0.0667,0.0663,0.0649,0.0615,0.0559,0.0495,0.0443,0.0427,0.0460,0.0541,0.0656,0.0777,0.0874,0.0922,0.0913,0.0853,0.0768,0.0695,0.0674,0.0741,0.0918,0.1213,0.1617,0.2105,0.2646,0.3208,0.3762,0.4284,0.4763,0.5198,0.5591,0.5955,0.6304,0.6645,0.6989,0.7338,0.7693,0.8046,0.8395,0.873,0.9016,0.929,0.9506,0.9705,0.9840 };

	// 测量数量跳出判定
	if (nAngle <= 0)
	{
		return(0);
	}
	if (nAngle > MAXnAngle) // MAXnAngle定义为256
	{
		return(0);
	}
	// 单个数值赋值情况
	if (nAngle == 1)
	{
		*ampMax = ampDat[0];
		*angleCen = angleDat[0];
		return(1);
	}
	// 数组初始化
	static float angleDatSort[MAXnAngle];
	static float ampDatSort[MAXnAngle];
	static float angleDatDiff[MAXnAngle];
	static float wCoef[MAXnAngle];
	float ampDatTmp = 0, angleDatTmp = 0, ampDatMin = 0;
	int iAngle = 0, jAngle = 0;
	float maxAngleDiff = 0, iMaxAngleDiff = 0;
	float tmpAngleDiff = 0, sumAngle = 0;
	double sumNorth = 0.0, sumWest = 0.0;

	// 查找最大最小值
	*ampMax = 0;
	for (iAngle = 0; iAngle < nAngle; iAngle++)
	{
		angleDatSort[iAngle] = angleDat[iAngle];
		ampDatSort[iAngle] = ampDat[iAngle];

		//去除野值-20230913
		if (ampDat[iAngle] > 20000)
		{
			ampDatSort[iAngle] = 0;
		}

		// 查找最小幅度
		if (ampDatSort[iAngle] > 0)
		{
			if (ampDatSort[iAngle] < ampDatMin || ampDatMin == 0)
				ampDatMin = ampDatSort[iAngle];
		}

		// 查找最大幅度
		if (ampDatSort[iAngle] > *ampMax)
		{
			*ampMax = ampDatSort[iAngle];
		}
	}
	// 排列输出的角度数组
	jAngle = nAngle - 1;
	while (jAngle > 0)
	{
		for (iAngle = 0; iAngle < jAngle; iAngle++)
		{
			if (angleDatSort[iAngle] > angleDatSort[iAngle + 1])
			{
				angleDatTmp = angleDatSort[iAngle + 1];
				angleDatSort[iAngle + 1] = angleDatSort[iAngle];
				angleDatSort[iAngle] = angleDatTmp;
				ampDatTmp = ampDatSort[iAngle + 1];
				ampDatSort[iAngle + 1] = ampDatSort[iAngle];
				ampDatSort[iAngle] = ampDatTmp;
			}
		}
		jAngle--;
	}

	// 剔除0幅度计算结果
	static float angleEffect[MAXnAngle];
	static float ampEffect[MAXnAngle];
	int kAngle = 0;
	for (iAngle = 0; iAngle < nAngle; iAngle++)
	{
		if (ampDatSort[iAngle] > 0)
		{
			angleEffect[kAngle] = angleDatSort[iAngle]; // 角度直接转小数
			ampEffect[kAngle] = pow(10, ((ampDatSort[iAngle] - *ampMax)*0.1 / 85)); // 归一化转线性幅度值
			kAngle++;
		}
	}
	// 方向图插值-1度间隔
	static float ampConstruct[360];
	int i, j, k;
	float tmp1 = 0;
	float tmp1_bv = -180;
	float tmp1_tv = 180;
	int tmp1_bi = 0;
	int tmp1_ti = 0;

	for (i = 0; i < 360; i++)
	{
		// 初始化
		tmp1 = 0;
		tmp1_bv = -180;
		tmp1_tv = 180;
		tmp1_bi = 0;
		tmp1_ti = 0;
		// 查找上下边界点
		for (j = 0; j < kAngle; j++)
		{
			tmp1 = angleEffect[j] - i;// 角度插值
			// 解跳周
			if (tmp1 > 180)
				tmp1 = tmp1 - 360;
			else if (tmp1 < -180)
				tmp1 = tmp1 + 360;
			// 计算最近点位置
			if (tmp1 <= 0)
			{
				if (tmp1 > tmp1_bv)
				{
					tmp1_bv = tmp1;
					tmp1_bi = j;
				}
			}
			else
			{
				if (tmp1 < tmp1_tv)
				{
					tmp1_tv = tmp1;
					tmp1_ti = j;
				}
			}
		}
		// 计算插值
		float tmp2 = 0;
		// 如果单边未找到15范围内的角度，则当前位置插值直接赋值0-20230704
		if (tmp1_bv <= -15 || tmp1_tv >= 15)
		{
			tmp2 = 0;
		}
		else
		{
			float tmp2_l = tmp1_tv - tmp1_bv;
			float tmp2_w = 0 - tmp1_bv;
			float tmp2_v = ampEffect[tmp1_ti] - ampEffect[tmp1_bi];
			float tmp2_s = ampEffect[tmp1_bi];
			tmp2 = tmp2_s + tmp2_v * (tmp2_w / tmp2_l);
		}
		// 插值点赋值
		ampConstruct[i] = tmp2;
	}
	// 判断全局插值结果，是否全零
	j = 0;
	for (i - 0; i < 360; i++)
	{
		if (ampConstruct[i] == 0)
		{
			j++;
		}
	}
	if (j == 360)
	{
		*angleCen = 0;
		*ampMax = 0;
		return(0);
	}

	// 与方向图卷积
	static float convResult[360];
	float tmp3 = 0;
	int tmp4 = 0;
	float* usePattern;
	float maxValue = 0;
	int maxLocation = 0;

	// 加条件判断，不同频段选择不同的曲线
	if (typeD == 0)
	{
		usePattern = pattern24;
	}
	else
	{
		usePattern = pattern58;
	}
	// 循环计算卷积结果
	for (i = 0; i < 360; i++)
	{
		// i值即为方向图角度旋转偏移量
		tmp3 = 0;
		for (j = 0; j < 360; j++)
		{
			tmp4 = j + i; //卷积位置
			if (tmp4 >= 360)
			{
				tmp4 = tmp4 - 360;
			}
			tmp3 = tmp3 + usePattern[j] * ampConstruct[tmp4];
		}
		if (tmp3 > maxValue)
		{
			maxValue = tmp3;
			maxLocation = i;
		}
	}
	// 首次赋值
	*angleCen = maxLocation; // 浮点数赋值
	// 计算旋转方向
	for (iAngle = 0; iAngle < nAngle - 1; iAngle++)
	{
		tmpAngleDiff = angleDat[iAngle + 1] - angleDat[iAngle];

		if (tmpAngleDiff > 180)
		{
			tmpAngleDiff -= 360;
		}
		if (tmpAngleDiff < -180)
		{
			tmpAngleDiff += 360;
		}
		sumAngle += tmpAngleDiff;
	}
	// 旋转修正
	*angleCen += sumAngle / (float)nAngle / 0.120 * 0.150; // T = 120ms, Delay = 150ms
	// 值域约束
	if (*angleCen < 0)
	{
		*angleCen += 360;
	}
	else if (*angleCen > 360)
	{
		*angleCen -= 360;
	}
	return(1);
}

int sfDxIfAimed(float azimuthNow, float elevationNow, float azimuthCen)
{
	float azimuthValidHalf = 15; // 打击天线半波束宽度
	float diffAzimuth = 0;

	diffAzimuth = azimuthNow - azimuthCen;
	if (diffAzimuth > 180)
	{
		diffAzimuth -= 360;
	}
	if (diffAzimuth < -180)
	{
		diffAzimuth += 360;
	}

	if (abs(diffAzimuth) <= azimuthValidHalf && elevationNow >= 5 && elevationNow <= 15)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}