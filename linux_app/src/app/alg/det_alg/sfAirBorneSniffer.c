#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sfAirBorneSniffer.h"

// 数字图传特征库定义
static struct arbnDigiLib droneDigiLIB[30] =
{
	{"DJI OcuSync2         ",{0,1,2,3,5,6},6,0,{2400,2483.5f},2,0,0,{2.1f,3.1f,4.1f,5.1f},4,0,{4,6},2,{10,18},2,{0.2f,0.2f,0.2f,0.2f},{1,3,0,3},0.2f,8,0,1,0,0.758f,{20,33,20,33,20}},
	{"DJI OcuSync2         ",{0,1,2,3,5,6},6,0,{5725,5850},2,0,0,{2.1f,3.1f,4.1f,5.1f},4,0,{4,6},2,{10,18},2,{0.2f,0.2f,0.2f,0.2f},{1,3,0,3},0.2f,8,0,1,0,0.758f,{30,33,14,33,14}},

	{"DJI OcuSync3         ",{1,2,5,6},4,0,{2400,2483.5f},2,0,0,{1.1f,2.1f},2,0,{2,4,4},3,{10,18},2,{0.2f,0.2f},{14,4},0.2f,30,0,1,0,0.41f,{20,33,20,33,20}},
	{"DJI OcuSync3         ",{1,2,5,6},4,0,{5725,5850},2,0,0,{1.1f,2.1f},2,0,{2,4,4},3,{10,18},2,{0.2f,0.2f},{14,4},0.2f,20,0,1,0,0.41f,{30,33,14,33,14}},

	{"Autel Lt/V3/M4T      ",{18,18,20},1,0,{2400,2483.5},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{20,32,20,32,20}},// 20230624更改数据库参数
	{"Autel Lt/V3/M4T      ",{18,18,20},1,0,{5725,5850},2,0,0,{0.9},1,1,{1,1,1,2},4,{9.4},1,{0.15},{40},0.15,25,0,1,0,0.5,{33,33,14,33,14}},// 20230624更改数据库参数
};
// 特征库内图传种类
static int nArbnDigiLib = 6;


// 数字遥控特征库定义
static struct arbnYkLib droneYkLIB[30] =
{
	{"DJI-Marvic3          ",14,7,-7,6,-3,3,{100,150},2,12,5,50},
	{"DJI-Marvic2          ",14,4,-7,6,-2,1,{100,150},2,12,3,30},

	{"Autel-All            ",23,4,-11,11,-2,1,{125},1,21,3,50},
};
// 特征库内图传种类
static int nArbnYkLib = 3;

// 频点相关计算信息-待更新
static float listArbnFrequence[5] = { 2420, 2460, 5745, 5785, 5825 };
static float listArbnNoice[5] = { 13000, 13000, 11700, 11700, 11700 };
static int   listArbnNum = 5;


// 总输入输出管理
void sfArbnProcessing(struct CompensateData bcData, unsigned short am1Mat[1][3000][128], unsigned short am2Mat[1][3000][128], unsigned short ph1Mat[1][3000][128], unsigned short ph2Mat[1][3000][128], float cenFreq, int antStatus, unsigned int mCnt, struct ioController inC, struct ioFlyStatus inF, struct ioMeasureInfo *outInfo, short *buffer)
{
	// buffer总长度为15*1024 
	// buffer[0] 用于存储帧号
	// buffer[1] - buffer[1] 之前用于存储原始数据

	int numDet = 0;
	static struct ykResult msYK[ARBN_PulseNumInFrame];
	static struct arbnTargetInfo tarYK[ARBN_TargetLength];
	static int arbnTargetPX[ARBN_TargetLength];
	static int tarNum = 0;

	if (inC.workMode == 0)
	{
		// 关闭侦测模式下处理 
		tarNum = 0;
		// 输出信息转译
		sfArbnInfoTransfer(tarYK, &tarNum, arbnTargetPX, inC, outInfo);
	}
	else if (inC.workMode == 1 || inC.workMode == 2)
	{
		// 打开遥控侦测/定向模式下处理 

		// 信号侦测
		buffer[0] = (short)mCnt;
		numDet = sfArbnSniffer(bcData, am1Mat, ph1Mat, ph2Mat, cenFreq, antStatus, msYK, buffer);

		// 识别为干扰处理
		if (numDet == -1)
		{
			numDet = sfArbnJammerFind(bcData, am1Mat, ph1Mat, ph2Mat, cenFreq, antStatus, msYK, buffer);
		}
		// 信号入队列-无论检测结果如何
		sfArbnTargetManageCluster(tarYK, &tarNum, msYK, numDet, mCnt, inF);
		sfArbnTargetManageStatus(tarYK, &tarNum, mCnt);
		// 信号威胁排序
		sfArbnTargetSort(tarYK, &tarNum, arbnTargetPX);
		// 输出信息转译
		sfArbnInfoTransfer(tarYK, &tarNum, arbnTargetPX, inC, outInfo);
	}
	else if (inC.workMode == 11 || inC.workMode == 12)
	{
		// 打开图传侦测/定向模式下处理 

		// 信号侦测
		buffer[0] = (short)mCnt;
		numDet = sfArbnToSkySniffer(bcData, am1Mat, ph1Mat, ph2Mat, cenFreq, antStatus, msYK, buffer);

		// 信号入队列-无论检测结果如何
		sfArbnTargetManageCluster(tarYK, &tarNum, msYK, numDet, mCnt, inF);
		sfArbnTargetManageStatus(tarYK, &tarNum, mCnt);
		// 信号威胁排序
		sfArbnTargetSort(tarYK, &tarNum, arbnTargetPX);
		// 输出信息转译
		sfArbnInfoTransfer(tarYK, &tarNum, arbnTargetPX, inC, outInfo);
	}
	else if (inC.workMode == 101 || inC.workMode == 102)
	{
		// 测试模式下处理-用于现场观测数据-遥控
		buffer[0] = (short)mCnt;
		numDet = sfArbnSniffer(bcData, am1Mat, ph1Mat, ph2Mat, cenFreq, antStatus, msYK, buffer);
		// 测试转译
		sfArbnInfoTestOut(msYK, numDet, inC.workMode, mCnt, outInfo);
	}
	else if (inC.workMode == 111 || inC.workMode == 112)
	{
		// 测试模式下处理-用于现场观测数据-图传
		buffer[0] = (short)mCnt;
		numDet = sfArbnToSkySniffer(bcData, am1Mat, ph1Mat, ph2Mat, cenFreq, antStatus, msYK, buffer);
		// 测试转译
		sfArbnInfoTestOut(msYK, numDet, inC.workMode, mCnt, outInfo);
	}
	else if (inC.workMode == 8001 || inC.workMode == 8002)
	{
		// 测试模式下处理-用于现场观测数据
		buffer[0] = (short)mCnt;
		numDet = sfArbnSniffer(bcData, am1Mat, ph1Mat, ph2Mat, cenFreq, antStatus, msYK, buffer);
		// 测试转译
		sfArbnInfoTestOut(msYK, numDet, inC.workMode, mCnt, outInfo);
	}
}

// 侦测测向函数部分-------------------------------------------------

// 遥控处理
int sfArbnSniffer(struct CompensateData bcData, unsigned short amMat[1][3000][128], unsigned short ph1Mat[1][3000][128], unsigned short ph2Mat[1][3000][128], float cenFreq, int antStatus, struct ykResult *outYK, short *buffer)
{
	// 1.单目标假设，目前不具备处理多目标能力
	// 2.图形学保留跳频图案
	// 3.跳频图案大小（6,13）(8,13),检测(10,15)
	// 4.定义角度信息

	// FPGA相位比例尺定义
	static float phaseLG = 1.0 / 32;

	static float tmpFw = 0;
	static float tmpFy = 0;

	static unsigned short ltMat[1][3000][128];
	static unsigned short anaTime[3000];
	static unsigned short anaFreq[3000];
	static int aTlist[50];
	static int aFlist[50];
	static int aYlist[50];
	static int oklist[50];
	static float phaseFwList[300];
	static float phaseFyList[300];
	static int phaseFwNum = 0;
	static int phaseFyNum = 0;

	static int angleNum = 0;

	static int grCntT[3000];
	static int grCntF[128];

	static float outTracerFw = 0;
	static float outTracerFy = 0;

	static unsigned short useAm[1][3000][128];

	// useAm赋值
	for (int i = 0; i < 3000; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			useAm[0][i][j] = amMat[0][i][j];
		}
	}

	// 数据单元定义
	int nRows = 3000;
	int nCols = 128;
	float dF = 0.4f;	// MHz
	float dT = 0.04f;	// ms

	// 常用变量定义
	int i = 0, j = 0, k = 0, tmp = 0;

	// [0] 缓存信息初始化
	int bufLength = 0;
	buffer[1] = 0;// 计算阶段标记
	buffer[2] = (short)cenFreq;

	// [0] 干扰情况识别
	float ltT0 = 0;
	sfArbnThresholdCompare(&ltT0, cenFreq, 6, 6 + 6);
	int statisF = 0;
	int statisT = 0;
	// 干扰时间统计
	for (i = 0; i < 3000; i++)
	{
		grCntT[i] = 0;
		for (j = 0; j < 128; j++)
		{
			if (useAm[0][i][j] > ltT0)
			{
				grCntT[i] = grCntT[i] + 1;
			}
		}
		if (grCntT[i] > 120)
		{
			statisT = statisT + 1;
		}
	}
	// 干扰频率统计
	for (i = 0; i < 128; i++)
	{
		grCntF[i] = 0;
		for (j = 0; j < 3000; j++)
		{
			if (useAm[0][j][i] > ltT0)
			{
				grCntF[i] = grCntF[i] + 1;
			}
		}
		if (grCntF[i] > 2990)
		{
			statisF = statisF + 1;
		}
	}
	// 干扰识别-是干扰直接退出
	if (statisT > 2990 && statisF > 120)
	{
		return -1;
	}
	// [1] 首先侦测当前频谱中是否有无人机信号，如果有则剔除相应的时间索引
	struct arbnBlockRow aBlkRow;
	int det = 0;
	det = sfArbnDigiSigFind(amMat, cenFreq, nRows, dT, dF, &aBlkRow);
	buffer[1] = 1;// 计算阶段标记
	buffer[3] = det;
	// [2] 基于det结果对幅度平面进行剔除
	if (det == 1)
	{
		for (i = 0; i < aBlkRow.nw; i++)
		{
			for (j = aBlkRow.q1[i] - 1; j <= aBlkRow.q1[i] + aBlkRow.w[i] + 1; j++)
			{
				// 索引越界保护
				if (j < 0)
				{
					tmp = 0;
				}
				else if (j >= 3000)
				{
					tmp = 2999;
				}
				else
				{
					tmp = j;
				}
				for (k = 0; k < 128; k++)
				{
					useAm[0][tmp][k] = 0;
				}
			}
		}
	}
	buffer[1] = 2;// 计算阶段标记
	if (buffer[3] == 1)
	{
		buffer[4] = aBlkRow.nw;
	}
	else
	{
		buffer[4] = 0;
	}
	// [3] 连通域计算门限滤除
	float ltT1 = 0;
	sfArbnThresholdCompare(&ltT1, cenFreq, 6, 6 + 6);
	for (i = 0; i < 128; i++)
	{

		if (i < 14 || i > 114)
		{
			// 正负20M范围外强制不检测
			for (j = 0; j < 3000; j++)
			{
				useAm[0][j][i] = 0;
			}
		}
		else
		{
			// 正负20M范围内双值化
			for (j = 0; j < 3000; j++)
			{
				if (useAm[0][j][i] < ltT1)
				{
					useAm[0][j][i] = 0;
				}
				else
				{
					useAm[0][j][i] = 1;
				}
			}
		}
	}
	buffer[1] = 3;// 计算阶段标记
	buffer[5] = (short)ltT1;
	// [4] 获取连通域模板-连通域内部最多返回40个以供后级筛选
	// 跳频图案大小（9,14)-检测窗口（11，16）

	int numLT = 0;
	numLT = sfArbnConnectAreaCalculate(useAm, ltMat, amMat, aTlist, aFlist, aYlist, ltT1);
	// 退出点
	if (numLT == 0)
	{
		return 0;
	}
	buffer[1] = 4;// 计算阶段标记
	buffer[6] = numLT;
	// [4] 连通域信号时域筛选
	sfArbnConnectAreaSelect(aTlist, aFlist, aYlist, numLT, oklist);
	// [5] 相位链提取-角度运算
	float phaSum = 0;
	float phaFwMean = 0;
	float phaFyMean = 0;
	float ampSum = 0;
	int   ampNum = 0;
	int corP = 0;
	// 相位补偿索引获取
	int bcShiftFw;
	int bcShiftFy;
	if (cenFreq == 2420)
	{
		bcShiftFw = 256 * 0 + 0;
		bcShiftFy = 256 * 0 + 128;
	}
	else if (cenFreq == 2460)
	{
		bcShiftFw = 256 * 1 + 0;
		bcShiftFy = 256 * 1 + 128;
	}
	else if (cenFreq == 5745)
	{
		bcShiftFw = 256 * 2 + 0;
		bcShiftFy = 256 * 2 + 128;
	}
	else if (cenFreq == 5785)
	{
		bcShiftFw = 256 * 3+ 0;
		bcShiftFy = 256 * 3 + 128;
	}
	else if (cenFreq == 5825)
	{
		bcShiftFw = 256 * 4 + 0;
		bcShiftFy = 256 * 4 + 128;
	}
	else
	{
		bcShiftFw = 256 * 0 + 0;
		bcShiftFy = 256 * 0 + 128;
	}

	// 两维相位获取及幅度获取-筛选到20个目标遥控则跳出处理
	angleNum = 0;
	int nummod = 0;
	for (i = 0; i < numLT; i++)
	{
		// 跳出保护
		if (angleNum >= ARBN_PulseNumInFrame)
		{
			break;
		}
		// 有效性判断
		if (oklist[i] == -1)
		{
			continue;
		}
		// 面向图块进行相位管理
		phaseFwNum = 0;
		phaseFyNum = 0;
		ampSum = 0;
		ampNum = 0;
		tmpFw = 0;
		tmpFy = 0;
		//printf("Fy--------\n");
		// 20231016更新
		for (j = aTlist[i] + droneYkLIB[aYlist[i]].TscaS; j <= aTlist[i] + droneYkLIB[aYlist[i]].TscaE; j++)
		{
			// 取余取整-计算相位计算规则
			nummod = j % 8;
			for (k = aFlist[i] + droneYkLIB[aYlist[i]].FscaS; k <= aFlist[i] + droneYkLIB[aYlist[i]].FscaE; k++)
			{
				// 0-3 俯仰 4-7 方位
				if (ltMat[0][j][k] > 0 && phaseFwNum < 300 && nummod >= 5 && nummod <= 7)// 根据联通域模板-相位索引判定
				{
					phaseFwList[phaseFwNum] = (ph2Mat[0][j][k] - ph1Mat[0][j][k]) * phaseLG - bcData.compensate_data[bcShiftFw + k] * 0.1f;
					buffer[10 + angleNum * 620 + phaseFwNum] = (short)(phaseFwList[phaseFwNum] * 10);//相位记录
					phaseFwNum = phaseFwNum + 1;
					ampSum = ampSum + amMat[0][j][k];
					ampNum = ampNum + 1;

				}
				else if (ltMat[0][j][k] > 0 && phaseFyNum < 300 && nummod >= 1 && nummod <= 3)// 根据联通域模板-相位索引判定
				{
					phaseFyList[phaseFyNum] = (ph2Mat[0][j][k] - ph1Mat[0][j][k]) * phaseLG - bcData.compensate_data[bcShiftFy + k] * 0.1f;
					buffer[10 + angleNum * 620 + phaseFyNum + 300] = (short)(phaseFyList[phaseFyNum] * 10);//相位记录
					phaseFyNum = phaseFyNum + 1;
					ampSum = ampSum + amMat[0][j][k];
					ampNum = ampNum + 1;	
				}
			}
		}
		buffer[10 + angleNum * 620 + 600] = phaseFwNum;//记录相位点数
		buffer[10 + angleNum * 620 + 601] = phaseFyNum;//记录相位点数
		// 点数合法性判定-每个维度至少有一个完成时隙
		if (aYlist[i] == 0 && (phaseFwNum < 12 || phaseFyNum < 12))
		{
			continue;
		}
		if (aYlist[i] == 1 && (phaseFwNum < 9 || phaseFyNum < 9))
		{
			continue;
		}
		if (aYlist[i] == 2 && (phaseFwNum < 12 || phaseFyNum < 12))
		{
			continue;
		}
		// 相位解跳轴运算-以0点为解缠绕基准-相位值域0-360
		for (j = 0; j < phaseFwNum; j++)
		{
			for (k = 0; k < 5; k++)
			{
				if (phaseFwList[j] >= 360)
				{
					phaseFwList[j] = phaseFwList[j] - 360;
				}
				else if (phaseFwList[j] < 0)
				{
					phaseFwList[j] = phaseFwList[j] + 360;
				}
				else
				{
					break;
				}
			}
		}
		for (j = 0; j < phaseFyNum; j++)
		{
			for (k = 0; k < 5; k++)
			{
				if (phaseFyList[j] >= 360)
				{
					phaseFyList[j] = phaseFyList[j] - 360;
				}
				else if (phaseFyList[j] < 0)
				{
					phaseFyList[j] = phaseFyList[j] + 360;
				}
				else
				{
					break;
				}
			}
		}
		// 方位处理
		int tmpCntFw = 0;
		tmpCntFw = sfArbnPhaseGet(phaseFwList, phaseFwNum, &phaFwMean);
		// 俯仰处理
		int tmpCntFy = 0;
		tmpCntFy = sfArbnPhaseGet(phaseFyList, phaseFyNum, &phaFyMean);

		buffer[10 + angleNum * 620 + 602] = (short)(tmpCntFw);//记录方位相位统计有效点
		buffer[10 + angleNum * 620 + 603] = (short)(tmpCntFy);//记录俯仰相位统计有效点
		buffer[10 + angleNum * 620 + 604] = (short)(phaFwMean * 10);//记录归一方位相位
		buffer[10 + angleNum * 620 + 605] = (short)(phaFyMean * 10);//记录归一俯仰相位
		// 点数合法性判定-每个维度至少有一个完成时隙
		if (aYlist[i] == 1 && (tmpCntFw < 6 || tmpCntFy < 6))
		{
			continue;
		}
		if (aYlist[i] == 2 && (tmpCntFw < 8 || tmpCntFy < 8))
		{
			continue;
		}
		// 根据脉冲计算目标角度
		if (cenFreq < 3000)
		{
			// 根据实测交换方位-俯仰定义
			tmpFw = -(float)asin(phaFwMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / 0.088);
			tmpFy = -(float)asin(phaFyMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / 0.088);
		}
		else
		{
			tmpFw = -(float)asin(phaFwMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / (0.044 * 1));
			tmpFy = -(float)asin(phaFyMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / (0.044 * 1));
		}
		outTracerFy = tmpFy * 180 / 3.1415926f;
		outTracerFw = (float)asin(sin(tmpFw) / cos(tmpFy)) * 180 / 3.1415926f;
		// 封装单次输出信息
		outYK[angleNum].sigType = 1;
		outYK[angleNum].sigSubType = aYlist[i];			// 遥控类型

		outYK[angleNum].sigBw = 2.4f;			// 带宽
		outYK[angleNum].sigPw = 0.5f;			// 脉宽
		outYK[angleNum].sigHopType = 1;		// 跳频类型
		outYK[angleNum].sigHopStep = 8;		// 跳频步进
		outYK[angleNum].sigPRT = 5;			// 重复周期
		if (cenFreq < 3000)
		{
			outYK[angleNum].freqBand = 2400;	// 频段
		}
		else
		{
			outYK[angleNum].freqBand = 5800;	// 频段
		}
		outYK[angleNum].freqCenter = cenFreq;
		outYK[angleNum].freqMeasure = cenFreq + (aFlist[i] - 64)*0.4f;	// 中心频率
		outYK[angleNum].measureNum = ampNum;
		outYK[angleNum].fw = outTracerFw;					// 方位
		outYK[angleNum].fy = outTracerFy;					// 俯仰
		outYK[angleNum].amp = ampSum / ampNum;				// 图传信号幅度

		outYK[angleNum].orgTPoint = aTlist[i];
		outYK[angleNum].orgFwPhase = phaFwMean;
		outYK[angleNum].orgFyPhase = phaFyMean;

		buffer[10 + angleNum * 620 + 606] = (short)(cenFreq + (aFlist[i] - 64)*0.4);	//记录载频
		buffer[10 + angleNum * 620 + 607] = ampNum;									//记录点数
		buffer[10 + angleNum * 620 + 608] = (short)(outTracerFw * 10);				//记录方位
		buffer[10 + angleNum * 620 + 609] = (short)(outTracerFy * 10);				//记录俯仰
		buffer[10 + angleNum * 620 + 610] = (short)(ampSum / ampNum);				//记录幅度

		// PRTF
		// printf("[YK]%d-%d-%d\t:Fw%f\tFy%f\tAm%f\tPfw%f\tPfy%f\t\n", (int)outYK[angleNum].sigSubType, outYK[angleNum].orgTPoint, aFlist[i], outYK[angleNum].fw, outYK[angleNum].fy, outYK[angleNum].amp, phaFwMean, phaFyMean);

		// printf("%f\t%d\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t\n",cenFreq,(int)outYK[angleNum].sigSubType, outYK[angleNum].orgTPoint, aFlist[i], outYK[angleNum].fw, outYK[angleNum].fy, outYK[angleNum].amp, phaFwMean, phaFyMean);

		angleNum = angleNum + 1;	
	}
	return angleNum;
}

// 干扰处理
int sfArbnJammerFind(struct CompensateData bcData, unsigned short amMat[1][3000][128], unsigned short ph1Mat[1][3000][128], unsigned short ph2Mat[1][3000][128], float cenFreq, int antStatus, struct ykResult *outYK, short *buffer)
{
	// FPGA相位比例尺定义
	static float phaseLG = 1.0 / 32;

	static float tmpFw = 0;
	static float tmpFy = 0;

	static int aTlist[50];
	static int aFlist[50];
	static int aYlist[50];
	static float phaseFwList[300];
	static float phaseFyList[300];
	static int phaseFwNum = 0;
	static int phaseFyNum = 0;

	static float outTracerFw = 0;
	static float outTracerFy = 0;



	// 数据单元定义
	int nRows = 3000;
	int nCols = 128;

	// 常用变量定义
	int i = 0, j = 0, k = 0, tmp = 0;

	// [0] 缓存信息初始化
	int bufLength = 0;
	buffer[1] = 0;// 计算阶段标记
	buffer[2] = (short)cenFreq;

	
	// [1] 计算44-84行，1000-2000列平均功率
	unsigned int sumPow = 0;
	float meanPow = 0;
	for (i = 44; i < 84; i++)
	{
		for (j = 1000; j < 2000; j++)
		{
			sumPow = sumPow + amMat[0][j][i];
		}
	}
	meanPow = ((float)sumPow) / 40 / 1000;
	buffer[1] = -1;// 计算阶段标记
	buffer[3] = (short)meanPow;

	// [2] 相位链提取-角度运算
	float phaSum = 0;
	float phaFwMean = 0;
	float phaFyMean = 0;
	float ampSum = 0;
	int   ampNum = 0;
	int corP = 0;
	// 相位补偿索引获取
	int bcShiftFw;
	int bcShiftFy;
	if (cenFreq == 2420)
	{
		bcShiftFw = 256 * 0 + 0;
		bcShiftFy = 256 * 0 + 128;
	}
	else if (cenFreq == 2460)
	{
		bcShiftFw = 256 * 1 + 0;
		bcShiftFy = 256 * 1 + 128;
	}
	else if (cenFreq == 5745)
	{
		bcShiftFw = 256 * 2 + 0;
		bcShiftFy = 256 * 2 + 128;
	}
	else if (cenFreq == 5785)
	{
		bcShiftFw = 256 * 3 + 0;
		bcShiftFy = 256 * 3 + 128;
	}
	else if (cenFreq == 5825)
	{
		bcShiftFw = 256 * 4 + 0;
		bcShiftFy = 256 * 4 + 128;
	}
	else
	{
		bcShiftFw = 256 * 0 + 0;
		bcShiftFy = 256 * 0 + 128;
	}

	// 面向图块进行相位管理
	phaseFwNum = 0;
	phaseFyNum = 0;
	ampSum = 0;
	ampNum = 0;
	tmpFw = 0;
	tmpFy = 0;
	int nummod = 0;

	for (j = 1000; j < 2000; j++)
	{
		// 取余取整-计算相位计算规则
		nummod = j % 8;
		for (k = 44; k < 84; k++)
		{
			// 0-3 俯仰 4-7 方位
			if (amMat[0][j][k] > meanPow && phaseFwNum < 300 && nummod >= 5 && nummod <= 7)// 根据联通域模板-相位索引判定
			{
				phaseFwList[phaseFwNum] = (ph2Mat[0][j][k] - ph1Mat[0][j][k]) * phaseLG - bcData.compensate_data[bcShiftFw + k] * 0.1f;
				buffer[10 + phaseFwNum] = (short)(phaseFwList[phaseFwNum] * 10);//相位记录
				phaseFwNum = phaseFwNum + 1;
				ampSum = ampSum + amMat[0][j][k];
				ampNum = ampNum + 1;
			}
			else if (amMat[0][j][k] > meanPow && phaseFyNum < 300 && nummod >= 1 && nummod <= 3)// 根据联通域模板-相位索引判定
			{
				phaseFyList[phaseFyNum] = (ph2Mat[0][j][k] - ph1Mat[0][j][k]) * phaseLG - bcData.compensate_data[bcShiftFy + k] * 0.1f;
				buffer[10 + phaseFyNum + 300] = (short)(phaseFyList[phaseFyNum] * 10);//相位记录
				phaseFyNum = phaseFyNum + 1;
				ampSum = ampSum + amMat[0][j][k];
				ampNum = ampNum + 1;
			}
		}
	}
	buffer[10 + 600] = phaseFwNum;//记录相位点数
	buffer[10 + 601] = phaseFyNum;//记录相位点数

	// 点数合法性判定-每个维度至少有一个完成时隙
	if (phaseFwNum < 290 || phaseFyNum < 290)
	{
		return 0;
	}
	// 相位解跳轴运算-以0点为解缠绕基准-相位值域0-360
	for (j = 0; j < phaseFwNum; j++)
	{
		for (k = 0; k < 5; k++)
		{
			if (phaseFwList[j] >= 360)
			{
				phaseFwList[j] = phaseFwList[j] - 360;
			}
			else if (phaseFwList[j] < 0)
			{
				phaseFwList[j] = phaseFwList[j] + 360;
			}
			else
			{
				break;
			}
		}
	}
	for (j = 0; j < phaseFyNum; j++)
	{
		for (k = 0; k < 5; k++)
		{
			if (phaseFyList[j] >= 360)
			{
				phaseFyList[j] = phaseFyList[j] - 360;
			}
			else if (phaseFyList[j] < 0)
			{
				phaseFyList[j] = phaseFyList[j] + 360;
			}
			else
			{
				break;
			}
		}
	}
	// 方位处理
	int tmpCntFw = 0;
	tmpCntFw = sfArbnPhaseGet(phaseFwList, phaseFwNum, &phaFwMean);
	// 俯仰处理
	int tmpCntFy = 0;
	tmpCntFy = sfArbnPhaseGet(phaseFyList, phaseFyNum, &phaFyMean);

	buffer[10 + 602] = (short)(tmpCntFw);//记录方位相位统计有效点
	buffer[10 + 603] = (short)(tmpCntFy);//记录俯仰相位统计有效点
	buffer[10 + 604] = (short)(phaFwMean * 10);//记录归一方位相位
	buffer[10 + 605] = (short)(phaFyMean * 10);//记录归一俯仰相位

	// 点数合法性判定-每个维度至少有一个完成时隙

	if (tmpCntFw < 150 || tmpCntFy < 150)
	{
		return 0;
	}
	if (tmpCntFw < 150 || tmpCntFy < 150)
	{
		return 0;
	}
	// 根据脉冲计算目标角度
	if (cenFreq < 3000)
	{
		// 根据实测交换方位-俯仰定义
		tmpFw = -(float)asin(phaFwMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / 0.088);
		tmpFy = -(float)asin(phaFyMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / 0.088);
	}
	else
	{
		tmpFw = -(float)asin(phaFwMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / (0.044 * 1));
		tmpFy = -(float)asin(phaFyMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / (0.044 * 1));
	}
	outTracerFy = tmpFy * 180 / 3.1415926f;
	outTracerFw = (float)asin(sin(tmpFw) / cos(tmpFy)) * 180 / 3.1415926f;
	// 封装单次输出信息
	outYK[0].sigType = 3;			// 干扰类型
	outYK[0].sigSubType = 0;
	outYK[0].sigBw = 51.2f;			// 带宽
	outYK[0].sigPw = 3000.0f;			// 脉宽
	outYK[0].sigHopType = 0;		// 跳频类型
	outYK[0].sigHopStep = 0;		// 跳频步进
	outYK[0].sigPRT = 0;			// 重复周期

	if (cenFreq < 3000)
	{
		outYK[0].freqBand = 2400;	// 频段
	}
	else
	{
		outYK[0].freqBand = 5800;	// 频段
	}
	outYK[0].freqCenter = cenFreq;
	outYK[0].freqMeasure = cenFreq + (aFlist[i] - 64)*0.4f;	// 中心频率
	outYK[0].measureNum = ampNum;
	outYK[0].fw = outTracerFw;					// 方位
	outYK[0].fy = outTracerFy;					// 俯仰
	outYK[0].amp = ampSum / ampNum;				// 图传信号幅度

	outYK[0].orgTPoint = aTlist[i];
	outYK[0].orgFwPhase = phaFwMean;
	outYK[0].orgFyPhase = phaFyMean;

	buffer[10 + 606] = (short)(cenFreq + (aFlist[i] - 64)*0.4);	//记录载频
	buffer[10 + 607] = ampNum;									//记录点数
	buffer[10 + 608] = (short)(outTracerFw * 10);				//记录方位
	buffer[10 + 609] = (short)(outTracerFy * 10);				//记录俯仰
	buffer[10 + 610] = (short)(ampSum / ampNum);				//记录幅度
	
	return 1;
}

// 图传信号行索引查找
int sfArbnDigiSigFind(unsigned short amMat[1][3000][128], float cenFreq, int nRows, float dT, float dF, struct arbnBlockRow *aBlkRow)
{
	int i, j, k = 0;
	int det = 0;
	// 获取计算基准频率范围
	int sCol = 64 - 50;
	int eCol = 64 + 50;
	// 计算全图门限
	float maxT = 0;
	sfArbnThresholdCompare(&maxT, cenFreq, 18, 18 + 6);
	float threshold1 = sfArbnAutoThreshOnMap(amMat, 0, sCol, eCol, nRows, 0.1f, 6, maxT);
	sfArbnThresholdCompare(&threshold1, cenFreq, 0, 3 + 6);
	// 计算频率维度过门限的点
	static int mPass1[ARBN_M1];
	for (i = 0; i < ARBN_M1; i++)
	{
		mPass1[i] = 0;
	}
	sfArbnFrequenceHist(amMat, 0, mPass1, sCol, eCol, nRows, threshold1);
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
	float threshold2 = sfArbnAutoThreshOnLineI(mPass1, sCol, eCol, 0.1f, 6);
	// 如果点数超过1500，占空比过半，则进行检测
	if (threshold2 > 1500)
	{
		threshold2 = 1500;
	}
	// 获取可能的频率
	static int fStart[ARBN_M1];
	static int fEnd[ARBN_M1];
	int fnum = 0;
	fnum = sfArbnCalculateFrequenceSE(mPass1, threshold2, 6, fStart, fEnd);
	// 循环计算脉冲
	static float mPass2[ARBN_M2];
	static int q1[ARBN_M2];
	static int w[ARBN_M2];
	static float colMeanAmp[ARBN_M2];
	static float colSnr[ARBN_M2];
	// 目标查找
	if (fnum > 0)
	{
		for (i = 0; i < fnum; i++)
		{
			// 生成合并脉冲列
			for (j = 0; j < ARBN_M2; j++)
			{
				mPass2[j] = 0;
				for (k = fStart[i]; k <= fEnd[i]; k++)
				{
					mPass2[j] = mPass2[j] + amMat[0][j][k];
				}
				mPass2[j] = mPass2[j] / (fEnd[i] - fStart[i] + 1);
			}
			// 计算幅度门限
			float threshold3 = sfArbnAutoThreshOnLineF(mPass2, nRows, 0.1f, 6);
			// 门限增加固定值保护
			sfArbnThresholdCompare(&threshold3, cenFreq, 0, 3 + 6);
			// 获取脉冲描述信息
			int	nw;
			int Delt = 1;
			int MinW = (int)(0.5f / dT); // 20230626,窄脉冲识别门限由0.7下降至0.5
			nw = sfArbnThresholdToPulse(mPass2, nRows, threshold3, Delt, MinW, q1, w, colSnr, colMeanAmp);
			// 进行目标数据库匹配
			det = sfArbnVidBlocksDetect(aBlkRow, mPass2, nRows, q1, w, nw, colSnr, 0, fStart[i], fEnd[i], cenFreq, dT, dF, droneDigiLIB, nArbnDigiLib);
			// 最后检测判定
			if (det == 1)
			{
				// 最后进行检测幅度校验
				float threshold4 = 0;
				sfArbnThresholdCompare(&threshold4, cenFreq, -3, 3 + 6);
				if (aBlkRow[0].meanColAmp < threshold4)
				{
					continue;
				}
				else
				{
					break;
				}
			}
			else
			{
				continue;
			}
		}
	}
	return det;
}
// 获取区分信号和噪声的阈值（二维）
float sfArbnAutoThreshOnMap(unsigned short downMat[1][3000][128], int chnum, int d128s, int d128e, int d3000, float dT, int maxIter, float baseT)
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
void  sfArbnFrequenceHist(unsigned short downMat[1][3000][128], int chnum, int* hist, int d128s, int d128e, int d3000, float threshold)
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
float sfArbnAutoThreshOnLineI(int *line, int d128s, int d128e, float dT, int maxIter)
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
int   sfArbnCalculateFrequenceSE(int *line, float dT, int lineWidthT, int *lineS, int *lineE)
{
	static int calM1[ARBN_M1];
	static int calM2[ARBN_M1];
	static int calM3[ARBN_M1];
	static int calM4[ARBN_M1];
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
int sfArbnThresholdToPulse(float *s, int n, float thresh, int delt, int minw, int *q1, int *w, float *colSnr, float *colAm)
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

// 图传图块检测
int sfArbnVidBlocksDetect(struct arbnBlockRow *aBlkRow, float *s, int n, int *q1, int *w, int nw, float *colSnr, int antIndex, int colIndexS, int colIndexE, float cenFreq, float dT, float dF, struct arbnDigiLib *UAVtypes, int nUAV)
{
	int   i, j, k;
	char  isUAV;
	char  isInFreqSpan;
	int   mPulseW[ARBN_MaxPWNumInLib];
	int * Indexq = (int *)malloc(n * sizeof(int));
	int   nIndex;
	float freqL, freqR;
	int   freqIndexL, freqIndexR;
	int   count = 0;

	int **match = (int**)malloc(ARBN_MaxVidPulseNum * sizeof(int*));
	for (i = 0; i < ARBN_MaxVidPulseNum; i++)
		match[i] = (int*)malloc(ARBN_MaxNumVidT * sizeof(int));

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
				freqIndexL = (int)((freqL - cenFreq) / dF + ARBN_M1 / 2);
				freqIndexR = (int)((freqR - cenFreq) / dF + ARBN_M1 / 2);
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
			freqIndexL = (int)((freqL - cenFreq) / dF + ARBN_M1 / 2);
			freqIndexR = (int)((freqR - cenFreq) / dF + ARBN_M1 / 2);

			//上下界有交叠则认为该目标可参与计算-此处后续可考虑做更精细的处理
			if (colIndexE <= freqIndexL || colIndexS > freqIndexR)
				continue;
		}

		for (j = 0; j < ARBN_MaxPWNumInLib; j++)
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

			sfArbnMatchPulseT(match, t2, Indexq, nIndex, UAVtypes[i].pulseT, UAVtypes[i].nPulseT, UAVtypes[i].pulseTErr);

			for (j = 0; j < nIndex; j++)
			{
				if (j > ARBN_MaxNumVidT - 1)
					break;
				meetPulseTCount = 0;
				for (k = 0; k < ARBN_MaxNumVidT; k++)
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
						int tmpn = (int)((120 / sumPulseT)*UAVtypes[i].nPulseT);
						if (tmpn<50 && maxpn>tmpn*1.75)
						{
							continue;// 脉冲过密则认为不判断
						}
					}

					// 20230624误报处理：针对EVOII误报做一个频率分选比较统计
					// 风险点为如果有干扰，可能就无法分选出来了

					// 20230522误报处理：针对8-16 Autel Lt/V3/M4T 误报处理
					// P1：检测120ms是否存在一次及以上的7ms中断
					// P2：检测[1 1 1 2]序列是否存在一次及以上
					// P3：[1 1 1 2]序列中，2ms周期中间应不存在其他脉冲
					// P4：7ms中断不足一次，严格的[1 1 1 2]逻辑不足3个，则认为是虚警
					if (i >= 4 && i <= 5) // 索引号严格核对库信息
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
				for (k = 0; k < ARBN_MaxNumVidT; k++)
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
				if (colSnr[Indexq[j]] < 0) //colSnr[Indexq[j]] < uavTipicalSNR
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

			for (i = 0; i < ARBN_MaxVidPulseNum; i++)
				free(match[i]);
			free(match);

			return 1;
		}
	}
	free(Indexq);
	free(t2);

	for (i = 0; i < ARBN_MaxVidPulseNum; i++)
		free(match[i]);
	free(match);

	return 0;
}
// 周期匹配
void sfArbnMatchPulseT(int **match, float *t2, int *index, int n, float *pulseT, int nPulseT, float pulseTErr)
{
	float diffTime, resTime, TErr;
	float sumPulseT = 0.0f;
	int   nt = 0, nT = 0, nBigT = 0;
	int   count = 0, maxCount = 0, maxIndex = 0;
	int i, j, k, p, m;
	static int   oneMatch[5][ARBN_MaxNumVidT];
	static float oneMatchTimeErr[5][ARBN_MaxNumVidT];

	for (i = 0; i < nPulseT; i++)
		sumPulseT += pulseT[i];

	for (i = 0; i < n; i++)
	{
		if (i > ARBN_MaxNumVidT - 1)
			break;
		for (k = 0; k < nPulseT; k++)
		{
			oneMatch[k][0] = index[i];
			for (m = 1; m < ARBN_MaxNumVidT; m++)
				oneMatch[k][m] = -1;

			for (j = 0; j < n; j++)
			{
				if (j > ARBN_MaxNumVidT - 1)
					break;
				if (i == j)
					continue;

				diffTime = t2[index[j]] - t2[index[i]];
				if (diffTime < 0)
					continue;

				nBigT = (int)(diffTime / sumPulseT);
				resTime = diffTime - nBigT * sumPulseT;

				nt = 0;
				while (1)
				{
					TErr = (float)fabs(resTime);
					if (TErr < pulseTErr)
					{
						nT = nPulseT * nBigT + nt;
						if (nT > 0 && nT < ARBN_MaxNumVidT)
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
			for (p = 0; p < ARBN_MaxNumVidT; p++)
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

		for (k = 0; k < ARBN_MaxNumVidT; k++)
		{
			match[i][k] = oneMatch[maxIndex][k];
		}
	}
}
// 脉冲参数转目标参数
void sfArbnPulseToTarget(struct arbnblockGroup *droneBlock, int nBlock, struct arbnBlockRow aBlkRow, int fstart, int fend, float cenFreq, float dT, float dF, struct arbnDigiLib *UAVtypes)
{
	int i = 0, j = 0, k = 0;
	if (nBlock >= ARBN_MaxBlockGroupNum)
		return;

	droneBlock[nBlock].id = nBlock;
	droneBlock[nBlock].index = aBlkRow.uavIndex;
	droneBlock[nBlock].range = 0;
	droneBlock[nBlock].bw = (fend - fstart)*dF;

	droneBlock[nBlock].n = aBlkRow.nw;
	for (j = 0; j < ARBN_NCh; j++)
	{
		droneBlock[nBlock].inCh[j] = aBlkRow.inCh[j];
	}
	for (j = 0; j < aBlkRow.nw; j++)
	{
		droneBlock[nBlock].freq[j] = cenFreq + ((float)(fend + fstart) / 2 - (ARBN_M1 / 2))*dF;
		droneBlock[nBlock].burstTime[j] = aBlkRow.q1[j] * dT;
		droneBlock[nBlock].width[j] = aBlkRow.w[j] * dT;
		droneBlock[nBlock].dist[j] = 0;
		for (k = 0; k < ARBN_NCh; k++)
		{
			droneBlock[nBlock].amp[k][j] = aBlkRow.meanColAmp;
		}
	}
	nBlock++;
}
// 求幅度门限
float sfArbnAutoThreshOnLineF(float *line, int d128, float dT, int maxIter)
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
void  sfArbnThresholdCompare(float* thresh, float cenFreq, int tdB, int mdB)
{
	// 根据频点判定固定门限保护值
	for (int i = 0; i < listArbnNum; i++)
	{
		if (listArbnFrequence[i] == cenFreq)
		{
			if (*thresh < (listArbnNoice[i] + tdB * 85))
			{
				*thresh = listArbnNoice[i] + tdB * 85;
			}
			else if (*thresh > (listArbnNoice[i] + mdB * 85))
			{
				*thresh = listArbnNoice[i] + mdB * 85;
			}
			break;
		}
	}
}

// 联通域算法计算
int sfArbnConnectAreaCalculate(unsigned short amMatIn[1][3000][128], unsigned short amMatOut[1][3000][128], unsigned short amMatOrg[1][3000][128], int *numTList, int *numFList, int *typeList, float inThd)
{
	//输入：amMatIn,计算输入矩阵，取值0或1
	//输出：amMatOut,计算输出矩阵，每个连通域赋对应值1-50
	//输出：numList,每个连通域对应的点数
	//返回值：连通域个数

	// 定义角度信息
	int i = 0, j = 0, k = 0, m = 0;
	int f = 0;
	
	int stacnt1 = 0;
	int stacnt2 = 0;

	int tmpcnt1 = 0;
	int tmpcnt2 = 0;
	int tmpW1 = 0;
	int tmpW2 = 0;
	int tmpH1 = 0;
	int tmpH2 = 0;
	unsigned short tmpMaxAmp = 0;
	//  临时门限
	unsigned short tmpThd = 0;
	int tmpFindType = 0;

	// 输出初始化
	int ltnum = 0;
	for (i = 0; i < 3000; i++)
	{
		for (j = 0; j < 128; j++)
		{
			amMatOut[0][i][j] = 0;
		}
	}
	// 连通域查找-只找模板符合的目标
	for (i = 0 + 15; i < 3000 - 15; i++)
	{
		for (j = 0 + 11; j < 128 - 11; j++)
		{
			// 【1】为0不计算、队列满不计算-40个满
			if (amMatIn[0][i][j] == 0 || ltnum >= 40)
			{
				continue;
			}
			stacnt1 = stacnt1 + 1;
			// 【2】框边沿不等于0退出,框内沿不等于1退出
			tmpFindType = -1;
			if (amMatOrg[0][i][j] - 16 * 85 > inThd)
			{
				tmpThd = amMatOrg[0][i][j] - 16 * 85;
			}
			else
			{
				tmpThd = (unsigned short)inThd;
			}
			//if (i == 131 && j == 109)
				//printf("y");

			// 逐一目标遥控查找
			for (f = 0; f < nArbnYkLib; f++)
			{
				// 阻带检查
				tmpcnt1 = 0;
				// 时域阻带检查
				for (k = i + droneYkLIB[f].TscaS - 1; k <= i + droneYkLIB[f].TscaE + 1; k = k + droneYkLIB[f].pulseWN + 1)// 时间窗口
				{
					if (tmpcnt1 > 0)
					{
						break;
					}
					for (m = j + droneYkLIB[f].FscaS - 1; m <= j + droneYkLIB[f].FscaE + 1; m = m + 1)// 频率窗口-逐一检测
					{
						// 20dB抑制门限检测
						if (amMatOrg[0][k][m] > tmpThd)
						{
							tmpcnt1 = tmpcnt1 + 1;
							break;
						}
					}
				}
				// 频域阻带检查
				for (k = i + droneYkLIB[f].TscaS - 1; k <= i + droneYkLIB[f].TscaE + 1; k = k + 1)// 时间窗口-逐一检测
				{
					if (tmpcnt1 > 0)
					{
						break;
					}
					for (m = j + droneYkLIB[f].FscaS - 1; m <= j + droneYkLIB[f].FscaE + 1; m = m + droneYkLIB[f].pulseFN + 1)// 频率窗口
					{
						if (amMatOrg[0][k][m] > tmpThd)
						{
							tmpcnt1 = tmpcnt1 + 1;
							break;
						}
					}
				}
				// 阻带判决
				if (tmpcnt1 > 0)
				{
					continue;
				}
				// 通带检查
				tmpcnt1 = 0;
				// 时域通带检查
				for (k = i + droneYkLIB[f].TscaS + 1; k <= i + droneYkLIB[f].TscaE - 1; k = k + droneYkLIB[f].pulseWN - 1)// 时间窗口
				{
					if (tmpcnt1 > 0)
					{
						break;
					}
					for (m = j + droneYkLIB[f].FscaS + 1; m <= j + droneYkLIB[f].FscaE - 1; m = m + 1)// 频率窗口-逐一检测
					{
						// 过门限检测-未过+1
						if (amMatIn[0][k][m] == 0)
						{
							tmpcnt1 = tmpcnt1 + 1;
							break;
						}
					}
				}
				// 频域通带检查
				for (k = i + droneYkLIB[f].TscaS + 1; k <= i + droneYkLIB[f].TscaE - 1; k = k + 1)// 时间窗口-逐一检测
				{
					if (tmpcnt1 > 0)
					{
						break;
					}
					for (m = j + droneYkLIB[f].FscaS + 1; m <= j + droneYkLIB[f].FscaE - 1; m = m + droneYkLIB[f].pulseFN - 1)// 频率窗口
					{
						// 过门限检测-未过+1
						if (amMatIn[0][k][m] == 0)
						{
							tmpcnt1 = tmpcnt1 + 1;
							break;
						}
					}
				}
				// 通带判决
				if (tmpcnt1 > 0)
				{
					continue;
				}
				// 初步识别通过
				if (tmpcnt1 == 0)
				{
					tmpFindType = f;
					stacnt2 = stacnt2 + 1;
					break;
				}
			}
			if (tmpFindType == -1)
			{
				continue;
			}
			// 【3】统计内部宽高及最大幅度
			tmpW1 = 3000;
			tmpW2 = 0;
			tmpH1 = 128;
			tmpH2 = 0;
			tmpMaxAmp = 0;
			for (k = i + droneYkLIB[tmpFindType].TscaS; k <= i + droneYkLIB[tmpFindType].TscaE; k = k + 1)
			{
				for (m = j + droneYkLIB[tmpFindType].FscaS; m <= j + droneYkLIB[tmpFindType].FscaE; m = m + 1)
				{
					if (amMatIn[0][k][m] > 0)
					{
						if (tmpW1 > k)
						{
							tmpW1 = k;
						}
						if (tmpW2 < k)
						{
							tmpW2 = k;
						}
						if (tmpH1 > m)
						{
							tmpH1 = m;
						}
						if (tmpH2 < m)
						{
							tmpH2 = m;
						}
					}
					if (amMatOrg[0][k][m] > tmpMaxAmp)
					{
						tmpMaxAmp = amMatOrg[0][k][m];
					}
				}
			}
			// 宽度识别
			if (tmpW2- tmpW1 < droneYkLIB[tmpFindType].Tmust)
			{
				continue;
			}
			// 高度识别
			if (tmpH2 - tmpH1 < droneYkLIB[tmpFindType].Fmust)
			{
				continue;
			}
			// 【3】框内小于必须值退出
			tmpcnt2 = 0;
			for (k = i + droneYkLIB[tmpFindType].TscaS; k <= i + droneYkLIB[tmpFindType].TscaE; k = k + 1)
			{
				for (m = j + droneYkLIB[tmpFindType].FscaS; m <= j + droneYkLIB[tmpFindType].FscaE; m = m + 1)
				{
					tmpcnt2 = tmpcnt2 + amMatIn[0][k][m];
				}
			}
			if (tmpcnt2 < droneYkLIB[tmpFindType].Nmust)
			{
				continue;
			}
			// 【5】连通域填值-6dB门限-域内最高值6dB内
			if (tmpMaxAmp - 6 * 85 > inThd)
			{
				tmpThd = tmpMaxAmp - 6 * 85;
			}
			else
			{
				tmpThd = (unsigned short)inThd;
			}
			for (k = i + droneYkLIB[tmpFindType].TscaS; k <= i + droneYkLIB[tmpFindType].TscaE; k = k + 1)
			{
				for (m = j + droneYkLIB[tmpFindType].FscaS; m <= j + droneYkLIB[tmpFindType].FscaE; m = m + 1)
				{
					if (amMatOrg[0][k][m] > tmpThd)
					{
						amMatOut[0][k][m] = 1;
					}
				}
			}
			// 【6】入队列前检查-去重处理-
			if (ltnum > 0)
			{
				// 时频两维点距比较-25-10
				int ppflag = 0;
				for (k = 0; k < ltnum; k++)
				{
					if (abs(numTList[k] - i) < 25 && abs(numFList[k] - j) < 10)
					{
						ppflag = 1;
						break;
					}
				}
				
				if (ppflag == 1)
				{
					// 匹配则丢弃当前结果
					ltnum = ltnum;
				}
				else
				{
					// 未匹配则更新队列
					numTList[ltnum] = i;
					numFList[ltnum] = j;
					typeList[ltnum] = tmpFindType;
					ltnum = ltnum + 1;
				}
			}
			else
			{
				numTList[ltnum] = i;
				numFList[ltnum] = j;
				typeList[ltnum] = tmpFindType;
				ltnum = ltnum + 1;
			}
		}
	}
	// 返回
	return ltnum;
}
// 连通域时频筛选
void sfArbnConnectAreaSelect(int *numTList, int *numFList, int *typeList, int num, int *okList)
{
	int i, j, k, m, n = 0;
	static int prc1List[50];
	static int prc2List[50];
	static int prc3List[50];
	int nPrc = 0;

	int numPeriod = 0;
	int numMod = 0;

	// 数组初始化
	for (i = 0; i < 50; i++)
	{
		okList[i] = -1;
	}
	// 退出判断
	if (num <= 0 || num > 50)
	{
		return;
	}
	// 逐一筛选脉冲
	for (i = 0; i < nArbnYkLib; i++)
	{
		// 计算总周期
		numPeriod = 0;
		for (j = 0; j < droneYkLIB[i].nPulseTN; j++)
		{
			numPeriod = numPeriod + droneYkLIB[i].pulseTN[j];
		}
		// 分选脉冲流
		nPrc = 0;
		for (j = 0; j < num; j++)
		{
			if (typeList[j] == i)
			{
				prc1List[nPrc] = numTList[j];
				prc2List[nPrc] = j;
				prc3List[nPrc] = 0;
				nPrc++;
			}
		}
		
		// 脉冲流有效判断
		if (nPrc <= 1)
		{
			continue;
		}
		// 脉冲流特征提取
		for (j = 0; j < nPrc-1; j++)
		{
			// 已被分选则退出
			if (prc3List[j] == 1)
			{
				continue;
			}
			// 分选标记
			for (k = j + 1; k < nPrc; k++)
			{
				numMod = (prc1List[k] - prc1List[j]) % numPeriod;
				for (m = 0; m < droneYkLIB[i].nPulseTN; m++)
				{
					if (numMod - droneYkLIB[i].pulseTN[m]<5 || numMod - droneYkLIB[i].pulseTN[m] > -5)
					{
						prc3List[j] = 1;
						prc3List[k] = 1;
					}
				}
			}
		}
		// 原始队列有效情况赋值
		for (j = 0; j < nPrc; j++)
		{
			if (prc3List[j] == 1)
			{
				okList[prc2List[j]] = 1;
			}
		}
	}
}



// 序列相位提取算法
int sfArbnPhaseGet(float *phaseList, int phaseNum, float *outPhase)
{
	int i, j, k = 0;

	int tmpCnt = 0;
	float tmpPhase = 0;
	int maxCnt = 0;
	float maxPhase = 0;

	float phaseWindow = 30.0f / 2;

	float sumPhase = 0;
	int sumCnt = 0;
	float meanPhase = 0;

	// 直方图滑窗统计
	for (i = 0; i < 360; i = i + 5)
	{
		// 计算当前相位窗口内测量点数
		tmpCnt = 0;
		for (j = 0; j < phaseNum; j++)
		{
			tmpPhase = phaseList[j] - (float)i;

			if (tmpPhase > 180)
			{
				tmpPhase = tmpPhase - 360;
			}
			else if (tmpPhase < -180)
			{
				tmpPhase = tmpPhase + 360;
			}

			if (tmpPhase< phaseWindow && tmpPhase > -phaseWindow)
			{
				tmpCnt = tmpCnt + 1;
			}
		}
		// 更新最大值记录状态
		if (tmpCnt >= maxCnt)
		{
			maxCnt = tmpCnt;
			maxPhase = (float)i;
		}
	}
	// 最终相位计算
	sumPhase = 0;
	sumCnt = 0;
	for (i = 0; i < phaseNum; i++)
	{
		tmpPhase = phaseList[i] - maxPhase;
		if (tmpPhase > 180)
		{
			tmpPhase = tmpPhase - 360;
		}
		else if (tmpPhase < -180)
		{
			tmpPhase = tmpPhase + 360;
		}
		if (tmpPhase< phaseWindow && tmpPhase > -phaseWindow)
		{
			sumPhase = sumPhase + tmpPhase;
			sumCnt = sumCnt + 1;
		}
	}
	meanPhase = (sumPhase / sumCnt)+ maxPhase;
	for (i = 0; i < 3; i++)
	{
		if (meanPhase > 180)
		{
			meanPhase = meanPhase - 360;
		}
		else if (meanPhase < -180)
		{
			meanPhase = meanPhase + 360;
		}
		else
		{
			break;
		}
	}
	// 信息输出
	*outPhase = meanPhase;
	return sumCnt;
}



// 处理-对天看无人机
int sfArbnToSkySniffer(struct CompensateData bcData, unsigned short amMat[1][3000][128], unsigned short ph1Mat[1][3000][128], unsigned short ph2Mat[1][3000][128], float cenFreq, int antStatus, struct ykResult *outYK, short *buffer)
{
	// 1.单目标假设，目前不具备处理多目标能力
	// 2.图形学保留跳频图案
	// 3.跳频图案大小（6,13）(8,13),检测(10,15)
	// 4.定义角度信息

	// FPGA相位比例尺定义
	static float phaseLG = 1.0 / 32;

	static float tmpFw = 0;
	static float tmpFy = 0;

	static unsigned short ltMat[1][3000][128];
	static unsigned short anaTime[3000];
	static unsigned short anaFreq[3000];
	static int aTlist[50];
	static int aFlist[50];
	static int aYlist[50];
	static float phaseFwList[300];
	static float phaseFyList[300];
	static int phaseFwNum = 0;
	static int phaseFyNum = 0;

	static int angleNum = 0;

	static int grCntT[3000];
	static int grCntF[128];

	static float outTracerFw = 0;
	static float outTracerFy = 0;

	static unsigned short useAm[1][3000][128];

	// useAm赋值
	for (int i = 0; i < 3000; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			useAm[0][i][j] = amMat[0][i][j];
		}
	}

	// 数据单元定义
	int nRows = 3000;
	int nCols = 128;
	float dF = 0.4f;	// MHz
	float dT = 0.04f;	// ms

	// 常用变量定义
	int i = 0, j = 0, k = 0, tmp = 0;

	// [0] 缓存信息初始化
	int bufLength = 0;
	buffer[1] = 0;// 计算阶段标记
	buffer[2] = (short)cenFreq;

	// [1] 首先侦测当前频谱中是否有无人机信号，如果有则剔除相应的时间索引
	struct arbnBlockRow aBlkRow;
	int det = 0;
	det = sfArbnDigiSigFind(amMat, cenFreq, nRows, dT, dF, &aBlkRow);
	buffer[1] = 1;// 计算阶段标记
	buffer[3] = det;

	// [2] 基于det结果对目标脉冲进行处理
	if (det == 0)
	{
		return 0;
	}
	buffer[1] = 2;// 计算阶段标记
	if (buffer[3] == 1)
	{
		buffer[4] = aBlkRow.nw;
	}
	else
	{
		buffer[4] = 0;
	}

	// [5] 相位链提取-角度运算
	float phaSum = 0;
	float phaFwMean = 0;
	float phaFyMean = 0;
	float ampSum = 0;
	int   ampNum = 0;
	int corP = 0;
	unsigned short maxAm = 0;
	float tmpAm = 0;

	// 相位补偿索引获取
	int bcShiftFw;
	int bcShiftFy;
	if (cenFreq == 2420)
	{
		bcShiftFw = 256 * 0 + 0;
		bcShiftFy = 256 * 0 + 128;
	}
	else if (cenFreq == 2460)
	{
		bcShiftFw = 256 * 1 + 0;
		bcShiftFy = 256 * 1 + 128;
	}
	else if (cenFreq == 5745)
	{
		bcShiftFw = 256 * 2 + 0;
		bcShiftFy = 256 * 2 + 128;
	}
	else if (cenFreq == 5785)
	{
		bcShiftFw = 256 * 3 + 0;
		bcShiftFy = 256 * 3 + 128;
	}
	else if (cenFreq == 5825)
	{
		bcShiftFw = 256 * 4 + 0;
		bcShiftFy = 256 * 4 + 128;
	}
	else
	{
		bcShiftFw = 256 * 0 + 0;
		bcShiftFy = 256 * 0 + 128;
	}

	// 两维相位获取及幅度获取-筛选到20个目标遥控则跳出处理
	angleNum = 0;
	int nummod = 0;
	for (i = 0; i < aBlkRow.nw; i++)
	{
		// 跳出保护-最多分析40个脉冲
		if (angleNum >= ARBN_PulseNumInFrame)
		{
			break;
		}
		// 面向图块进行相位管理
		phaseFwNum = 0;
		phaseFyNum = 0;
		ampSum = 0;
		ampNum = 0;
		tmpFw = 0;
		tmpFy = 0;
		// 查找图传图案内功率最大值
		maxAm = 0;
		for (j = aBlkRow.q1[i] + 2; j <= aBlkRow.q1[i] + aBlkRow.w[i] - 2; j++)
		{
			for (k = aBlkRow.colIndex - (aBlkRow.nCol - 6) / 2; k <= aBlkRow.colIndex + (aBlkRow.nCol - 6) / 2; k++)
			{
				if (amMat[0][j][k] > maxAm)
				{
					maxAm = amMat[0][j][k];
				}
			}
		}
		// 根据功率及位置提取相位
		maxAm = maxAm - 85 * 3;
		tmpAm = 0;
		sfArbnThresholdCompare(&tmpAm, cenFreq, 3, 6);
		if (maxAm < (unsigned short)tmpAm)
		{
			maxAm = (unsigned short)tmpAm;
		}

		for (j = aBlkRow.q1[i] + 2; j <= aBlkRow.q1[i] + aBlkRow.w[i] - 2; j++)
		{
			// 取余取整-计算相位计算规则
			nummod = j % 8;
			for (k = aBlkRow.colIndex - (aBlkRow.nCol - 6) / 2; k <= aBlkRow.colIndex + (aBlkRow.nCol - 6) / 2; k++)
			{
				// 0-3 俯仰 4-7 方位
				if (amMat[0][j][k] > maxAm && phaseFwNum < 300 && nummod >= 5 && nummod <= 7)// 根据联通域模板-相位索引判定
				{
					phaseFwList[phaseFwNum] = (ph2Mat[0][j][k] - ph1Mat[0][j][k]) * phaseLG - bcData.compensate_data[bcShiftFw + k] * 0.1f;
					buffer[10 + angleNum * 620 + phaseFwNum] = (short)(phaseFwList[phaseFwNum] * 10);//相位记录
					phaseFwNum = phaseFwNum + 1;
					ampSum = ampSum + amMat[0][j][k];
					ampNum = ampNum + 1;
				}
				else if (amMat[0][j][k] > maxAm && phaseFyNum < 300 && nummod >= 1 && nummod <= 3)// 根据联通域模板-相位索引判定
				{
					phaseFyList[phaseFyNum] = (ph2Mat[0][j][k] - ph1Mat[0][j][k]) * phaseLG - bcData.compensate_data[bcShiftFy + k] * 0.1f;
					buffer[10 + angleNum * 620 + phaseFyNum + 300] = (short)(phaseFyList[phaseFyNum] * 10);//相位记录
					phaseFyNum = phaseFyNum + 1;
					ampSum = ampSum + amMat[0][j][k];
					ampNum = ampNum + 1;
				}
			}
		}
		buffer[10 + angleNum * 620 + 600] = phaseFwNum;//记录相位点数
		buffer[10 + angleNum * 620 + 601] = phaseFyNum;//记录相位点数

		// 点数合法性判定-每个维度至少有一个完成时隙
		if ((phaseFwNum < 100 || phaseFyNum < 100))
		{
			continue;
		}
		// 相位解跳轴运算-以0点为解缠绕基准-相位值域0-360
		for (j = 0; j < phaseFwNum; j++)
		{
			for (k = 0; k < 5; k++)
			{
				if (phaseFwList[j] >= 360)
				{
					phaseFwList[j] = phaseFwList[j] - 360;
				}
				else if (phaseFwList[j] < 0)
				{
					phaseFwList[j] = phaseFwList[j] + 360;
				}
				else
				{
					break;
				}
			}
		}
		for (j = 0; j < phaseFyNum; j++)
		{
			for (k = 0; k < 5; k++)
			{
				if (phaseFyList[j] >= 360)
				{
					phaseFyList[j] = phaseFyList[j] - 360;
				}
				else if (phaseFyList[j] < 0)
				{
					phaseFyList[j] = phaseFyList[j] + 360;
				}
				else
				{
					break;
				}
			}
		}

		// 方位处理
		int tmpCntFw = 0;
		tmpCntFw = sfArbnPhaseGet(phaseFwList, phaseFwNum, &phaFwMean);
		// 俯仰处理
		int tmpCntFy = 0;
		tmpCntFy = sfArbnPhaseGet(phaseFyList, phaseFyNum, &phaFyMean);

		buffer[10 + angleNum * 620 + 602] = (short)(tmpCntFw);//记录方位相位统计有效点
		buffer[10 + angleNum * 620 + 603] = (short)(tmpCntFy);//记录俯仰相位统计有效点
		buffer[10 + angleNum * 620 + 604] = (short)(phaFwMean * 10);//记录归一方位相位
		buffer[10 + angleNum * 620 + 605] = (short)(phaFyMean * 10);//记录归一俯仰相位
		// 点数合法性判定-每个维度至少有一个完成时隙
		if ((tmpCntFw < 70 || tmpCntFy < 70))
		{
			continue;
		}
		// 根据脉冲计算目标角度
		if (cenFreq < 3000)
		{
			// 根据实测交换方位-俯仰定义
			tmpFw = -(float)asin(phaFwMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / 0.088);
			tmpFy = -(float)asin(phaFyMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / 0.088);
		}
		else
		{
			tmpFw = -(float)asin(phaFwMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / (0.044 * 1));
			tmpFy = -(float)asin(phaFyMean / 360 * (3E8 / ((cenFreq + (aFlist[i] - 64)*0.4)*1E6)) / (0.044 * 1));
		}
		outTracerFy = tmpFy * 180 / 3.1415926f;
		outTracerFw = (float)asin(sin(tmpFw) / cos(tmpFy)) * 180 / 3.1415926f;
		// 封装单次输出信息
		outYK[angleNum].sigType = 2;			// 类型
		outYK[angleNum].sigSubType = aBlkRow.uavIndex;
		outYK[angleNum].sigBw = aBlkRow.nCol*0.4f;// 带宽
		outYK[angleNum].sigPw = aBlkRow.w[i]*0.04f;			// 脉宽
		outYK[angleNum].sigHopType = 0;		// 跳频类型
		outYK[angleNum].sigHopStep = 0;		// 跳频步进
		outYK[angleNum].sigPRT = 0;			// 重复周期
		if (cenFreq < 3000)
		{
			outYK[angleNum].freqBand = 2400;	// 频段
		}
		else
		{
			outYK[angleNum].freqBand = 5800;	// 频段
		}
		outYK[angleNum].freqCenter = cenFreq;
		outYK[angleNum].freqMeasure = cenFreq + (aBlkRow.colIndex - 64)*0.4f;	// 中心频率
		outYK[angleNum].measureNum = ampNum;
		outYK[angleNum].fw = outTracerFw;					// 方位
		outYK[angleNum].fy = outTracerFy;					// 俯仰
		outYK[angleNum].amp = ampSum / ampNum;				// 图传信号幅度

		outYK[angleNum].orgTPoint = aBlkRow.q1[i];
		outYK[angleNum].orgFwPhase = phaFwMean;
		outYK[angleNum].orgFyPhase = phaFyMean;

		buffer[10 + angleNum * 620 + 606] = (short)(cenFreq + (aFlist[i] - 64)*0.4);	//记录载频
		buffer[10 + angleNum * 620 + 607] = ampNum;									//记录点数
		buffer[10 + angleNum * 620 + 608] = (short)(outTracerFw * 10);				//记录方位
		buffer[10 + angleNum * 620 + 609] = (short)(outTracerFy * 10);				//记录俯仰
		buffer[10 + angleNum * 620 + 610] = (short)(ampSum / ampNum);				//记录幅度

		angleNum = angleNum + 1;
	}
	return angleNum;
}



// 目标管理部分-------------------------------------------------

void sfArbnTargetManageCluster(struct arbnTargetInfo *tar, int *tarNum, struct ykResult *yk, int ykNum, unsigned int calCnt, struct ioFlyStatus inF)
{
	// 全局用于新建目标计数，程序全程不清零，下电清零
	static unsigned short g_tarNum = 0;

	int i = 0, j = 0, k = 0;
	float freqMeet = 0;
	float tmp1S = 0, tmp1E = 0;
	float tmp2S = 0, tmp2E = 0;
	float tmp3S = 0, tmp3E = 0;
	float tmp4S = 0, tmp4E = 0;
	float tmp5S = 0, tmp5E = 0;
	int ppFlag = 0;

	// 测量信息分类计算
	for (i = 0; i < ykNum; i++)
	{
		ppFlag = 0;
		// 逐一匹配计算
		for (j = 0; j < *tarNum; j++)
		{
			// 遥控类别匹配-类型+频段
			if (tar[j].type == yk[i].sigType && tar[j].type == 1 && tar[j].typeIndex == yk[i].sigSubType && tar[j].trackBand == yk[i].freqBand)
			{
				// 计算空间角度-两维角度在10度内则认为是同一目标
				tmp1S = tar[j].trackFW1 - yk[i].fw;
				tmp1E = tar[j].trackFY1 - yk[i].fy;

				if (tmp1S > 10 || tmp1S < -10 ||tmp1E > 10 || tmp1E < -10)
				{
					continue;
				}
				// 信息合并-原始数据录入
				for (k = 0; k < ARBN_TarDataLength - 1; k++)
				{
					tar[j].sqTime[ARBN_TarDataLength - 1 - k] = tar[j].sqTime[ARBN_TarDataLength - 1 - k - 1];
					tar[j].sqAmp[ARBN_TarDataLength - 1 - k] = tar[j].sqAmp[ARBN_TarDataLength - 1 - k - 1];
					tar[j].sqFreq[ARBN_TarDataLength - 1 - k] = tar[j].sqFreq[ARBN_TarDataLength - 1 - k - 1];
					tar[j].sqFw[ARBN_TarDataLength - 1 - k] = tar[j].sqFw[ARBN_TarDataLength - 1 - k - 1];
					tar[j].sqFy[ARBN_TarDataLength - 1 - k] = tar[j].sqFy[ARBN_TarDataLength - 1 - k - 1];
				}
				tar[j].sqTime[0] = calCnt;
				tar[j].sqAmp[0] = yk[i].amp;
				tar[j].sqFreq[0] = yk[i].freqMeasure;
				tar[j].sqFw[0] = yk[i].fw;
				tar[j].sqFy[0] = yk[i].fy;

				if (tar[j].cnt >= 4000000000)
				{
					tar[j].cnt = 4000000000;
				}
				else
				{
					tar[j].cnt = tar[j].cnt + 1;
				}
				tar[j].trackBwS = 0;
				tar[j].trackBwE = 0;
				// 计算参数更新
				tmp1S = 0;
				tmp1E = 0;
				tmp2S = 20000;
				tmp2E = 0;
				tmp3S = 20000;
				tmp3E = 0;
				tmp4S = 0;
				tmp4E = 0;
				if (tar[j].cnt >= ARBN_TarDataLength)
				{
					for (k = 0; k < ARBN_TarDataLength; k++)
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
						if (tmp3S > tar[j].sqFreq[k])
						{
							tmp3S = tar[j].sqFreq[k];
						}
						if (tmp3E < tar[j].sqFreq[k])
						{
							tmp3E = tar[j].sqFreq[k];
						}
						tmp4S = tmp4S + tar[j].sqFw[k];
						tmp4E = tmp4E + tar[j].sqFy[k];
					}
					tar[j].update = (float)(tar[j].sqTime[0] - tar[j].sqTime[ARBN_TarDataLength - 1]) / (ARBN_TarDataLength - 1);
					tar[j].ampMean = tmp1S / ARBN_TarDataLength;
					tar[j].ampQF = tmp2E - tmp2S;
					tar[j].freqSMean = tmp3S;
					tar[j].freqEMean = tmp3E;
					tar[j].trackFW1 = tmp4S / ARBN_TarDataLength;
					tar[j].trackFY1 = tmp4E / ARBN_TarDataLength;

				}
				else if (tar[j].cnt < ARBN_TarDataLength && tar[j].cnt > 1)
				{
					for (k = 0; k < (int)tar[j].cnt; k++)
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
						if (tmp3S > tar[j].sqFreq[k])
						{
							tmp3S = tar[j].sqFreq[k];
						}
						if (tmp3E < tar[j].sqFreq[k])
						{
							tmp3E = tar[j].sqFreq[k];
						}
						tmp4S = tmp4S + tar[j].sqFw[k];
						tmp4E = tmp4E + tar[j].sqFy[k];
					}
					tar[j].update = (float)(tar[j].sqTime[0] - tar[j].sqTime[tar[j].cnt - 1]) / (tar[j].cnt - 1);
					tar[j].ampMean = tmp1S / tar[j].cnt;
					tar[j].ampQF = tmp2E - tmp2S;
					tar[j].freqSMean = tmp3S;
					tar[j].freqEMean = tmp3E;
					tar[j].trackFW1 = tmp4S / tar[j].cnt;
					tar[j].trackFY1 = tmp4E / tar[j].cnt;
				}
				else
				{
					tar[j].update = 0;
					tar[j].ampMean = tar[j].sqAmp[0];
					tar[j].ampQF = 0;
					tar[j].freqSMean = tar[j].sqFreq[0];
					tar[j].freqEMean = tar[j].sqFreq[0];
					tar[j].trackFW1 = tar[j].sqFw[0];
					tar[j].trackFY1 = tar[j].sqFy[0];
				}
				// 跟踪参数更新
				tar[j].timeLast = calCnt;
				if (tar[j].timeLock < calCnt - 20)
				{
					tar[j].trackCenterFreq = yk[i].freqCenter;
					tar[j].timeLock = calCnt;
				}
				else
				{
					// 待核对
					tar[j].trackCenterFreq = yk[i].freqCenter;
					tar[j].timeLock = calCnt;
				}
				tar[j].trackFreq = (tar[j].freqEMean + tar[j].freqSMean) / 2;
				tar[j].trackBw = tar[j].freqEMean - tar[j].freqSMean;

				// 结合姿态计算固定系角度
				// 固定系转换
				sfArbnAngleTransform(tar[j].trackFW1, tar[j].trackFY1, -45, inF, &tar[j].trackFW2, &tar[j].trackFY2);
				// 结合姿态计算距离
				tar[j].dist = 0;

				// 匹配后退出循环
				ppFlag = 1;
				break;
			}

			// 图传类别匹配-类型+频段
			if (tar[j].type == yk[i].sigType && tar[j].type == 2 && tar[j].typeIndex == yk[i].sigSubType && tar[j].trackBand == yk[i].freqBand)
			{
				// 计算空间角度-两维角度在10度内则认为是同一目标
				tmp1S = tar[j].trackFW1 - yk[i].fw;
				tmp1E = tar[j].trackFY1 - yk[i].fy;
				// 计算频率交集-交集小于10M则认为是同一目标
				tmp2S = (tar[j].freqEMean - tar[j].freqSMean) - yk[i].freqMeasure;
				if (tmp1S > 10 || tmp1S < -10 || tmp1E > 10 || tmp1E < -10 || tmp2S > 10 || tmp2S < -10)
				{
					continue;
				}
				// 信息合并-原始数据录入
				for (k = 0; k < ARBN_TarDataLength - 1; k++)
				{
					tar[j].sqTime[ARBN_TarDataLength - 1 - k] = tar[j].sqTime[ARBN_TarDataLength - 1 - k - 1];
					tar[j].sqAmp[ARBN_TarDataLength - 1 - k] = tar[j].sqAmp[ARBN_TarDataLength - 1 - k - 1];
					tar[j].sqFreq[ARBN_TarDataLength - 1 - k] = tar[j].sqFreq[ARBN_TarDataLength - 1 - k - 1];
					tar[j].sqFw[ARBN_TarDataLength - 1 - k] = tar[j].sqFw[ARBN_TarDataLength - 1 - k - 1];
					tar[j].sqFy[ARBN_TarDataLength - 1 - k] = tar[j].sqFy[ARBN_TarDataLength - 1 - k - 1];
				}
				tar[j].sqTime[0] = calCnt;
				tar[j].sqAmp[0] = yk[i].amp;
				tar[j].sqFreq[0] = yk[i].freqMeasure;
				tar[j].sqFw[0] = yk[i].fw;
				tar[j].sqFy[0] = yk[i].fy;

				if (tar[j].cnt >= 4000000000)
				{
					tar[j].cnt = 4000000000;
				}
				else
				{
					tar[j].cnt = tar[j].cnt + 1;
				}
				tar[j].trackBwS = 0;
				tar[j].trackBwE = 0;
				// 计算参数更新
				tmp1S = 0;
				tmp1E = 0;
				tmp2S = 20000;
				tmp2E = 0;
				tmp3S = 20000;
				tmp3E = 0;
				tmp4S = 0;
				tmp4E = 0;
				if (tar[j].cnt >= ARBN_TarDataLength)
				{
					for (k = 0; k < ARBN_TarDataLength; k++)
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
						if (tmp3S > tar[j].sqFreq[k])
						{
							tmp3S = tar[j].sqFreq[k];
						}
						if (tmp3E < tar[j].sqFreq[k])
						{
							tmp3E = tar[j].sqFreq[k];
						}
						tmp4S = tmp4S + tar[j].sqFw[k];
						tmp4E = tmp4E + tar[j].sqFy[k];
					}
					tar[j].update = (float)(tar[j].sqTime[0] - tar[j].sqTime[ARBN_TarDataLength - 1]) / (ARBN_TarDataLength - 1);
					tar[j].ampMean = tmp1S / ARBN_TarDataLength;
					tar[j].ampQF = tmp2E - tmp2S;
					tar[j].freqSMean = tmp3S;
					tar[j].freqEMean = tmp3E;
					tar[j].trackFW1 = tmp4S / ARBN_TarDataLength;
					tar[j].trackFY1 = tmp4E / ARBN_TarDataLength;

				}
				else if (tar[j].cnt < ARBN_TarDataLength && tar[j].cnt > 1)
				{
					for (k = 0; k < (int)tar[j].cnt; k++)
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
						if (tmp3S > tar[j].sqFreq[k])
						{
							tmp3S = tar[j].sqFreq[k];
						}
						if (tmp3E < tar[j].sqFreq[k])
						{
							tmp3E = tar[j].sqFreq[k];
						}
						tmp4S = tmp4S + tar[j].sqFw[k];
						tmp4E = tmp4E + tar[j].sqFy[k];
					}
					tar[j].update = (float)(tar[j].sqTime[0] - tar[j].sqTime[tar[j].cnt - 1]) / (tar[j].cnt - 1);
					tar[j].ampMean = tmp1S / tar[j].cnt;
					tar[j].ampQF = tmp2E - tmp2S;
					tar[j].freqSMean = tmp3S;
					tar[j].freqEMean = tmp3E;
					tar[j].trackFW1 = tmp4S / tar[j].cnt;
					tar[j].trackFY1 = tmp4E / tar[j].cnt;
				}
				else
				{
					tar[j].update = 0;
					tar[j].ampMean = tar[j].sqAmp[0];
					tar[j].ampQF = 0;
					tar[j].freqSMean = tar[j].sqFreq[0];
					tar[j].freqEMean = tar[j].sqFreq[0];
					tar[j].trackFW1 = tar[j].sqFw[0];
					tar[j].trackFY1 = tar[j].sqFy[0];
				}
				// 跟踪参数更新
				tar[j].timeLast = calCnt;
				if (tar[j].timeLock < calCnt - 20)
				{
					tar[j].trackCenterFreq = yk[i].freqCenter;
					tar[j].timeLock = calCnt;
				}
				else
				{
					// 待核对
					tar[j].trackCenterFreq = yk[i].freqCenter;
					tar[j].timeLock = calCnt;
				}
				tar[j].trackFreq = (tar[j].freqEMean + tar[j].freqSMean) / 2;
				tar[j].trackBw = tar[j].freqEMean - tar[j].freqSMean;

				// 结合姿态计算固定系角度
				// 固定系转换
				sfArbnAngleTransform(tar[j].trackFW1, tar[j].trackFY1, -45, inF, &tar[j].trackFW2, &tar[j].trackFY2);
				// 结合姿态计算距离
				tar[j].dist = 0;

				// 匹配后退出循环
				ppFlag = 1;
				break;
			}
		}

		// 若未匹配，且当前队列仍有位置，则新建目标
		if (*tarNum < ARBN_TargetLength && ppFlag == 0)
		{
			g_tarNum = g_tarNum + 1;
			// 满10000重新编号
			if (g_tarNum >= 1000)
			{
				g_tarNum = 0;
			}
			tar[*tarNum].num = (g_tarNum+ yk[i].sigType*10000+ yk[i].sigSubType*1000);
			tar[*tarNum].type = yk[i].sigType;
			tar[*tarNum].typeIndex = yk[i].sigSubType;

			if (yk[i].sigType == 1)
			{
				for (k = 0; k < 50; k++)
				{
					tar[*tarNum].name[k] = 'Y';
				}
			}
			else if (yk[i].sigType == 2)
			{
				for (k = 0; k < 50; k++)
				{
					tar[*tarNum].name[k] = 'T';
				}
			}
			else if (yk[i].sigType == 2)
			{
				for (k = 0; k < 50; k++)
				{
					tar[*tarNum].name[k] = 'G';
				}
			}
			else
			{
				for (k = 0; k < 50; k++)
				{
					tar[*tarNum].name[k] = 'E';
				}
			}
			if (tar[*tarNum].type == 1)
			{
				// 缺省填1
				for (k = 0; k < 32; k++)
				{
					tar[*tarNum].ID[k] = 1;
				}
			}

			tar[*tarNum].cnt = 1;
			tar[*tarNum].status = 1;			// 目标状态：1发现、2跟踪、3记忆
			tar[*tarNum].timeCreate = calCnt;	// 目标建立时间
			tar[*tarNum].timeLast = calCnt;		// 目标最近测量时间
			tar[*tarNum].timeLock = calCnt;		// 目标锁定时间

			tar[*tarNum].trackBand = yk[i].freqBand;	// 跟踪频率
			tar[*tarNum].trackFreq = yk[i].freqMeasure;	// 跟踪频率
			tar[*tarNum].trackBw = 0;					// 跟踪带宽
			tar[*tarNum].trackCenterFreq = yk[i].freqCenter;// 跟踪中心频点

			tar[*tarNum].trackFW1 = yk[i].fw;			// 测量系方位
			tar[*tarNum].trackFY1 = yk[i].fy;			// 测量系俯仰
			// 固定系转换
			sfArbnAngleTransform(tar[*tarNum].trackFW1, tar[*tarNum].trackFY1,-45, inF, &tar[*tarNum].trackFW2, &tar[*tarNum].trackFY2);

			tar[*tarNum].trackBwS = 0;		// 跟踪频率起始索引
			tar[*tarNum].trackBwE = 0;		// 跟踪频率终止索引		

			// 结合姿态计算更新
			tar[*tarNum].trackFW2 = 0;					// 固定系方位
			tar[*tarNum].trackFY2 = 0;					// 固定系俯仰
			tar[*tarNum].dist = 0;

			tar[*tarNum].update = 0;					// 数据更新率
			tar[*tarNum].ampMean = yk[i].amp;			// 目标平均幅度
			tar[*tarNum].ampQF = 0;						// 目标幅度起伏
			tar[*tarNum].freqSMean = yk[i].freqMeasure;	// 起始频率均值
			tar[*tarNum].freqEMean = yk[i].freqMeasure;	// 终止频率均值

			tar[*tarNum].sqTime[0] = calCnt;			// 到达时间序列
			tar[*tarNum].sqAmp[0] = yk[i].amp;			// 幅度序列
			tar[*tarNum].sqFreq[0] = yk[i].freqMeasure;	// 频率序列
			tar[*tarNum].sqFw[0] = yk[i].fw;			// 方位序列
			tar[*tarNum].sqFy[0] = yk[i].fy;			// 俯仰序列

			tar[*tarNum].trackPriority = 0;	// 跟踪优先级

			*tarNum = *tarNum + 1;
		}
	}
}

void sfArbnAngleTransform(float inFw, float inFy, float installFy, struct ioFlyStatus inF, float *outFw, float *outFy)
{
	// 1.inFw：测量系方位，inFy：测量系俯仰
	// 2.installFy：测量系安装俯仰角
	// 3.fYaw：飞机偏航角, fPitch：飞机俯仰角, fRoll：飞机滚转角
	// 4.outFw：输出固定系方位角, outFy：输出固定系俯仰角
	// 计算思路：获取测量系目标方向矢量，逐级旋转转换至固定系下，再根据转换后的方向矢量求解方位角，俯仰角

	double pi = 3.1415926;
	double x1 = 0, y1 = 0, z1 = 0;
	double atmp = 0;

	float fYaw = 0 ;
	float fPitch = 0;
	float fRoll = 0;
	fYaw = inF.yaw*0.1f;
	fPitch = inF.pitch*0.1f;
	fRoll = inF.roll*0.1f;

	// 求解目标方向矢量
	x1 = cos(inFy*pi / 180)*cos(inFw*pi / 180);
	y1 = cos(inFy*pi / 180)*sin(inFw*pi / 180);
	z1 = -sin(inFy*pi / 180);

	// 1:修正安装俯仰旋转-y轴旋转-installFy度
	double x2 = 0, y2 = 0, z2 = 0;
	atmp = -installFy * pi / 180;
	x2 = x1 * cos(atmp) + y1 * 0 - z1 * sin(atmp);
	y2 = y1;
	z2 = x1 * sin(atmp) + y1 * 0 + z1 * cos(atmp);

	// 2:修正姿态滚转角度-x轴旋转-fRoll度
	double x3 = 0, y3 = 0, z3 = 0;
	atmp = -fRoll * pi / 180;
	x3 = x2;
	y3 = x2 * 0 + y2 * cos(atmp) + z2 * sin(atmp);
	z3 = x2 * 0 - y2 * sin(atmp) + z2 * cos(atmp);
	
	// 3:修正姿态俯仰角度-y轴旋转-fPitch度
	double x4 = 0, y4 = 0, z4 = 0;
	atmp = -fPitch * pi / 180;
	x4 = x3 * cos(atmp) + y3 * 0 - z3 * sin(atmp);
	y4 = y3;
	z4 = x3 * sin(atmp) + y3 * 0 + z3 * cos(atmp);

	// 4:修正姿态偏航角度-y轴旋转-fYaw度
	double x5 = 0, y5 = 0, z5 = 0;
	atmp = -fYaw * pi / 180;
	x5 =  x4 * cos(atmp) + y4 * sin(atmp) + z4 * 0;
	y5 = -x4 * sin(atmp) + y4 * cos(atmp) + z4 * 0;
	z5 =  z4;

	// 5:求解固定系方位与俯仰角度
	float calFw = 0, calFy = 0;
	double tmp;
	if (z5 >= 1)
	{
		calFy = -90;
		calFw = 0;
	}
	else if (z5 <= -1)
	{
		calFy = 90;
		calFw = 0;
	}
	else
	{
		tmp = sqrt(x5 * x5 + y5 * y5);
		if (tmp == 0 && z5 >= 0)
		{
			calFy = -90;
			calFw = 0;
		}
		else if (tmp == 0 && z5 < 0)
		{
			calFy = 90;
			calFw = 0;
		}
		else
		{
			calFy = (float)(-asin(z5) * 180 / pi);
			calFw = (float)(atan2(y5, x5) * 180 / pi);
		}
		
	}
	// 输出

	*outFw = calFw;
	*outFy = calFy;


}

void sfArbnTargetManageStatus(struct arbnTargetInfo *tar, int *tarNum, unsigned int calCnt)
{
	// 维护目标列表的工作状态，并更新每一个目标的威胁优先级
	int i = 0, j = 0, k = 0;

	// 状态维护计算
	for (i = 0; i < *tarNum; i++)
	{
		// 发现状态
		if (tar[i].status == 1)
		{
			// 发现转跟踪-2.4S
			if ((tar[i].timeLast - tar[i].timeCreate) > 20 && tar[i].cnt > 12)
			{
				tar[i].status = 2;
			}
			// 发现转目标剔除-2.4S
			if ((calCnt - tar[i].timeCreate) > 20 && tar[i].cnt < 12)
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
			// 跟踪转记忆-3.6S
			if ((calCnt - tar[i].timeLast) > 30)
			{
				tar[i].status = 3;
			}
		}
		// 记忆状态
		else if (tar[i].status == 3)
		{
			// 记忆转跟踪-6S
			if ((calCnt - tar[i].timeLast) < 50)
			{
				tar[i].status = 2;
			}
			// 记忆转目标剔除-6S
			if ((calCnt - tar[i].timeLast) >= 50)
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
		// 工作状态第一优先、频率细分优先
		if (tar[i].status == 2)
		{
			// 跟踪状态-优先级高等
			if (tar[i].trackBand == 5800)
			{
				tar[i].trackPriority = 21;// 高频优先级高
			}
			else
			{
				tar[i].trackPriority = 20;// 低频优先级低
			}
		}
		else if (tar[i].status == 3)
		{
			// 记忆状态-优先级中等
			if (tar[i].trackBand == 5800)
			{
				tar[i].trackPriority = 11;// 高频优先级高
			}
			else
			{
				tar[i].trackPriority = 10;// 低频优先级低
			}
		}
		else
		{
			// 发现状态-优先级低等
			if (tar[i].trackBand == 5800)
			{
				tar[i].trackPriority = 1;// 高频优先级高
			}
			else
			{
				tar[i].trackPriority = 0;// 低频优先级低
			}
		}
	}
}

void sfArbnTargetSort(struct arbnTargetInfo *tar, int *tarNum, int *pList)
{
	// 当前目标优先级输出进行排序
	// tar[i].trackPriority 状态：21、20、11、10、1、0
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
				else if (tar[pList[j]].trackPriority == tar[pList[j + 1]].trackPriority)
				{
					// 如果trackPriority相同，则根据cnt进行比较和交换(较小值放后面)
					if (tar[pList[j]].cnt < tar[pList[j + 1]].cnt)
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


// 信息转译部分

void sfArbnInfoTransfer(struct arbnTargetInfo *tar, int *tarNum, int *pList, struct ioController inC, struct ioMeasureInfo *outInfo)
{
	int tmpMode = 0;
	tmpMode = inC.workMode % 10;

	if (tmpMode == 0)
	{
		// 关闭侦测模式返回全零帧
		sfArbnInfoInitial(outInfo);
		*tarNum = 0;
	}
	else if (tmpMode == 1)
	{
		// 打开侦测模式返回计算结果
		sfArbnInfoInitial(outInfo);
		(*outInfo).workStatus = inC.workMode;	// 系统工作状态

		if (*tarNum > 0 && tar[pList[0]].status > 1)
		{
			(*outInfo).tarNum = 1;		// 目标有效个数
			(*outInfo).t1Num = (unsigned short)tar[pList[0]].num;				// 目标1编号
			(*outInfo).t1Freq = (unsigned short)tar[pList[0]].trackCenterFreq;	// 目标1频率
			(*outInfo).t1Amp = (unsigned short)tar[pList[0]].ampMean;			// 目标1幅度
			(*outInfo).t1mFw = (short)(tar[pList[0]].trackFW1 * 10);			// 目标1测量系方位角,0.1°
			(*outInfo).t1mFy = (short)(tar[pList[0]].trackFY1 * 10);			// 目标1测量系俯仰角,0.1°
			(*outInfo).t1fFw = (short)(tar[pList[0]].trackFW2 * 10);			// 目标1固定系方位角,0.1°
			(*outInfo).t1fFy = (short)(tar[pList[0]].trackFY2 * 10);			// 目标1固定系俯仰角,0.1°
		}
		if (*tarNum > 1 && tar[pList[1]].status > 1)
		{
			(*outInfo).tarNum = 2;		// 目标有效个数
			(*outInfo).t2Num = (unsigned short)tar[pList[1]].num;				// 目标2编号
			(*outInfo).t2Freq = (unsigned short)tar[pList[1]].trackCenterFreq;	// 目标2频率
			(*outInfo).t2Amp = (unsigned short)tar[pList[1]].ampMean;			// 目标2幅度
			(*outInfo).t2mFw = (short)(tar[pList[1]].trackFW1 * 10);			// 目标2测量系方位角,0.1°
			(*outInfo).t2mFy = (short)(tar[pList[1]].trackFY1 * 10);			// 目标2测量系俯仰角,0.1°
			(*outInfo).t2fFw = (short)(tar[pList[1]].trackFW2 * 10);			// 目标2固定系方位角,0.1°
			(*outInfo).t2fFy = (short)(tar[pList[1]].trackFY2 * 10);			// 目标2固定系俯仰角,0.1°
		}
		if (*tarNum > 2 && tar[pList[2]].status > 1)
		{
			(*outInfo).tarNum = 3;		// 目标有效个数
			(*outInfo).t3Num = (unsigned short)tar[pList[2]].num;				// 目标3编号
			(*outInfo).t3Freq = (unsigned short)tar[pList[2]].trackCenterFreq;	// 目标3频率
			(*outInfo).t3Amp = (unsigned short)tar[pList[2]].ampMean;			// 目标3幅度
			(*outInfo).t3mFw = (short)(tar[pList[2]].trackFW1 * 10);			// 目标3测量系方位角,0.1°
			(*outInfo).t3mFy = (short)(tar[pList[2]].trackFY1 * 10);			// 目标3测量系俯仰角,0.1°
			(*outInfo).t3fFw = (short)(tar[pList[2]].trackFW2 * 10);			// 目标3固定系方位角,0.1°
			(*outInfo).t3fFy = (short)(tar[pList[2]].trackFY2 * 10);			// 目标3固定系俯仰角,0.1°
		}
		if (*tarNum > 3 && tar[pList[3]].status > 1)
		{
			(*outInfo).tarNum = 4;		// 目标有效个数
			(*outInfo).t4Num = (unsigned short)tar[pList[3]].num;				// 目标4编号
			(*outInfo).t4Freq = (unsigned short)tar[pList[3]].trackCenterFreq;	// 目标4频率
			(*outInfo).t4Amp = (unsigned short)tar[pList[3]].ampMean;			// 目标4幅度
			(*outInfo).t4mFw = (short)(tar[pList[3]].trackFW1 * 10);			// 目标4测量系方位角,0.1°
			(*outInfo).t4mFy = (short)(tar[pList[3]].trackFY1 * 10);			// 目标4测量系俯仰角,0.1°
			(*outInfo).t4fFw = (short)(tar[pList[3]].trackFW2 * 10);			// 目标4固定系方位角,0.1°
			(*outInfo).t4fFy = (short)(tar[pList[3]].trackFY2 * 10);			// 目标4固定系俯仰角,0.1°
		}
	}
	else if (tmpMode == 2)
	{
		// 打开定向模式返回计算结果
		sfArbnInfoInitial(outInfo);
		(*outInfo).workStatus = inC.workMode;	// 系统工作状态

		for (int i = 0; i < *tarNum; i++)
		{
			if (tar[i].num == inC.dxNum)
			{
				(*outInfo).tarNum = 1;
				(*outInfo).t1Num = (unsigned short)tar[i].num;				// 目标1编号
				(*outInfo).t1Freq = (unsigned short)tar[i].trackCenterFreq;	// 目标1频率
				(*outInfo).t1Amp = (unsigned short)tar[i].ampMean;			// 目标1幅度
				(*outInfo).t1mFw = (short)(tar[i].trackFW1 * 10);			// 目标1测量系方位角,0.1°
				(*outInfo).t1mFy = (short)(tar[i].trackFY1 * 10);			// 目标1测量系方位角,0.1°
				(*outInfo).t1fFw = (short)(tar[i].trackFW2 * 10);			// 目标1测量系方位角,0.1°
				(*outInfo).t1fFy = (short)(tar[i].trackFY2 * 10);			// 目标1测量系方位角,0.1°
			}
		}
	}
	else
	{
		// 异常值返回全零帧+404状态
		sfArbnInfoInitial(outInfo);
		*tarNum = 0;
		(*outInfo).workStatus = 404;	// 系统工作状态
	}
}

void sfArbnInfoInitial(struct ioMeasureInfo *outInfo)
{
	(*outInfo).workStatus = 0;	// 系统工作状态
	(*outInfo).tarNum = 0;		// 目标有效个数

	(*outInfo).t1Num = 0;	// 目标1编号
	(*outInfo).t1Freq = 0;	// 目标1频率
	(*outInfo).t1Amp = 0;	// 目标1幅度
	(*outInfo).t1mFw = 0;	// 目标1测量系方位角,0.1°
	(*outInfo).t1mFy = 0;	// 目标1测量系俯仰角,0.1°
	(*outInfo).t1fFw = 0;	// 目标1固定系方位角,0.1°
	(*outInfo).t1fFy = 0;	// 目标1固定系俯仰角,0.1°

	(*outInfo).t2Num = 0;	// 目标2编号
	(*outInfo).t2Freq = 0;	// 目标2频率
	(*outInfo).t2Amp = 0;	// 目标2幅度
	(*outInfo).t2mFw = 0;	// 目标2测量系方位角,0.1°
	(*outInfo).t2mFy = 0;	// 目标2测量系俯仰角,0.1°
	(*outInfo).t2fFw = 0;	// 目标2固定系方位角,0.1°
	(*outInfo).t2fFy = 0;	// 目标2固定系俯仰角,0.1°

	(*outInfo).t3Num = 0;	// 目标3编号
	(*outInfo).t3Freq = 0;	// 目标3频率
	(*outInfo).t3Amp = 0;	// 目标3幅度
	(*outInfo).t3mFw = 0;	// 目标3测量系方位角,0.1°
	(*outInfo).t3mFy = 0;	// 目标3测量系俯仰角,0.1°
	(*outInfo).t3fFw = 0;	// 目标3固定系方位角,0.1°
	(*outInfo).t3fFy = 0;	// 目标3固定系俯仰角,0.1°

	(*outInfo).t4Num = 0;	// 目标4编号
	(*outInfo).t4Freq = 0;	// 目标4频率
	(*outInfo).t4Amp = 0;	// 目标4幅度
	(*outInfo).t4mFw = 0;	// 目标4测量系方位角,0.1°
	(*outInfo).t4mFy = 0;	// 目标4测量系俯仰角,0.1°
	(*outInfo).t4fFw = 0;	// 目标4固定系方位角,0.1°
	(*outInfo).t4fFy = 0;	// 目标4固定系俯仰角,0.1°
}

// 测试信息转译
void sfArbnInfoTestOut(struct ykResult *yk, int ykNum, unsigned short mode, unsigned int cnt, struct ioMeasureInfo *outInfo)
{
	// 打开侦测模式返回计算结果
	sfArbnInfoInitial(outInfo);
	(*outInfo).workStatus = mode;	// 系统工作状态
	(*outInfo).tarNum = 4;			// 固定返回前4个脉冲

	if (ykNum > 0)
	{
		(*outInfo).t1Num = (unsigned short)yk[0].orgTPoint;				
		(*outInfo).t1Freq = (unsigned short)yk[0].freqMeasure;
		(*outInfo).t1Amp = (unsigned short)yk[0].amp;			
		(*outInfo).t1mFw = (short)(yk[0].fw * 10);			
		(*outInfo).t1mFy = (short)(yk[0].fy * 10);			
		(*outInfo).t1fFw = (short)(yk[0].orgFwPhase * 10);			
		(*outInfo).t1fFy = (short)(yk[0].orgFyPhase * 10);			
	}
	if (ykNum > 1)
	{
		(*outInfo).t2Num = (unsigned short)yk[1].orgTPoint;
		(*outInfo).t2Freq = (unsigned short)yk[1].freqMeasure;
		(*outInfo).t2Amp = (unsigned short)yk[1].amp;
		(*outInfo).t2mFw = (short)(yk[1].fw * 10);
		(*outInfo).t2mFy = (short)(yk[1].fy * 10);
		(*outInfo).t2fFw = (short)(yk[1].orgFwPhase * 10);
		(*outInfo).t2fFy = (short)(yk[1].orgFyPhase * 10);
	}
	if (ykNum > 2)
	{
		(*outInfo).t3Num = (unsigned short)yk[2].orgTPoint;
		(*outInfo).t3Freq = (unsigned short)yk[2].freqMeasure;
		(*outInfo).t3Amp = (unsigned short)yk[2].amp;
		(*outInfo).t3mFw = (short)(yk[2].fw * 10);
		(*outInfo).t3mFy = (short)(yk[2].fy * 10);
		(*outInfo).t3fFw = (short)(yk[2].orgFwPhase * 10);
		(*outInfo).t3fFy = (short)(yk[2].orgFyPhase * 10);
	}
	if (ykNum > 3)
	{
		(*outInfo).t4Num = (unsigned short)yk[3].orgTPoint;
		(*outInfo).t4Freq = (unsigned short)yk[3].freqMeasure;
		(*outInfo).t4Amp = (unsigned short)yk[3].amp;
		(*outInfo).t4mFw = (short)(yk[3].fw * 10);
		(*outInfo).t4mFy = (short)(yk[3].fy * 10);
		(*outInfo).t4fFw = (short)(yk[3].orgFwPhase * 10);
		(*outInfo).t4fFy = (short)(yk[3].orgFyPhase * 10);
	}
	// 记录帧号
	(*outInfo).t4Num = (unsigned short)cnt;
}