#ifndef _SFDRONESNIFFER_H__
#define _SFDRONESNIFFER_H__

// 宏定义

#define NCh  1		// 通道数量
#define NFFT 512	// 频谱列数
#define NRow 12000	// 频谱行数
#define M 4			// 累加行数
#define N 4			// 累加列数

#define CHAN_NUM    1
#define LINE_NUM    (NRow / M)  // 3000
#define COLUMN_NUM  (NFFT / N)  // 128

#define sfM1 128	// 标准数据行数-频率
#define sfM2 3000	// 标准数据列数-时间
#define sfdF 0.4
#define sfdT 0.04

#define MaxDrone 20
#define MaxPulseInGroup 120
#define MaxObjNum 20
#define MaxHistoryNum 1000
#define MaxPulseTimeNum 16

#define PI 3.1415926535898

#define MaxDroneinLib 30	// 特征库最大目标个数
#define MaxNumVidT 50		// 最大周期数
#define MaxPulseInObj 100	// 最大脉冲数
#define MaxVidPulse 50		// 最大检测脉冲数
#define maxPulseWNumInLib 5	// 特征库内脉宽类型最大个数

#define MAXnAngle 256		// 定向阶段执行测量次数
#define TarDataLength 20	// TarInfo结构体内数据存储长度
#define TarLength 16		// TarInfo结构体数组长度
// 数据结构定义

// 数字图传特征库，部分字段仅对图传有效，部分字段仅对遥控有效
struct DroneLib
{
	char  name[50];					//名称
	int   psbID[10];				//可能的机型序号
	int   nPsbID;					//机型序号可能性数量
	char  downOrUp;					//图传或遥控，下行down是图传，上行up是遥控
	float freqPoints[60];			//频点。对应图传的信道，遥控的跳频点，MHz
	int   nfreq;					//频点个数
	char  isFixedFreq;				//freqPoints中是频段范围还是频点，若是0表示对应频段下上限，若是1则对应频点
	int   hoppType;					//跳频类型，该参数仅对遥控有效。0严格按照频表在相应时间相应频点发出信号，1按照周期跳频且大部分频率不变，2只需要周期跳频即可
	float pulseW[maxPulseWNumInLib];//脉冲宽度特征，ms。
	int   nPulseW;					//宽度种类数量
	char  isFixedPulseW;			//脉宽是否固定，0不固定，1固定
	float pulseT[5];				//周期，ms
	int   nPulseT;					//周期种类
	float pulseBW[5];				//带宽大小，MHz
	int   nPulseBW;					//带宽种类
	float pulseWErr[maxPulseWNumInLib];	//脉冲宽度误差容限，ms
	int   meetPulseW[maxPulseWNumInLib];//满足脉冲宽度的最低次数
	float pulseTErr;				//脉冲重复周期误差，ms
	int   hoppCnt;					//满足规律跳频的最低次数，实际用作周期满足个数
	float freqErr;					//频率误差容限,MHz
	int   method;					//使用方法（暂未启用）
	float SNR;						//最低信噪比门限（暂未启用）
	float duty;						//信道占空比（暂未启用）

	// 新增字段
	float cPower[5];				//参考功率，0：SRRC，1：FCC，2：CE，3：Max，4：Min
};

// 模拟图传特征库，部分字段仅对图传有效，部分字段仅对遥控有效
struct VideoLib
{
	char  name[50];					//名称
	int   psbID[10];				//可能的机型序号
	int   nPsbID;					//机型序号可能性数量

	float freqPoints[10];			//频点。对应图传的信道，遥控的跳频点，MHz
	int   nfreq;					//频点个数
	float freqErr;					//频率误差容限，MHz，中心频率偏移修正
	int filedIndex;                 //场频与中心频点的可疑偏移行数-20230913-ZSY

	float sigBW;					//信号带宽
	float detBW;					//检测带宽
	float detatt;					//边带衰减
	float detzkb1;					//通带占空比
	float detzkb2;					//边带占空比

	float tbSft;					//同步信号频率偏移
};

struct BlockRow
{
	int   id;
	int   uavIndex;
	int   antIndex;
	char  inCh[NCh];
	int   colIndex;
	int   allColIndex[NFFT / N];
	int   nCol;
	int   q1[MaxPulseInGroup];
	int   w[MaxPulseInGroup];
	float meanColAmp;
	float snr;
	int   nw;
};

struct blockGroup
{
	int   id;
	int   index;
	float range;
	char  inCh[NCh];
	float freq[MaxPulseInGroup];
	float bw;
	float burstTime[MaxPulseInGroup];
	float width[MaxPulseInGroup];
	float amp[NCh][MaxPulseInGroup];
	float dist[MaxPulseInGroup];
	int   n;
};

// 无人机侦测结果
typedef struct droneResult
{
	char type;				// 信号类型：1数字图传，2模拟图传，3Wifi图传
	char index;				// 数据库中报文编号
	char uni[32];			// 目标唯一编码-Wifi填写Mac地址,其他两种填写默认值

	char name[50];			// 目标图传名称
	int ID;					// ID号
	int psbID[10];			// 可能的机型序号
	int nPsbID;				// 机型序号可能性数量
	char  flag;				// 在数据库中的编号
	float freq[128];		// 脉冲对应的频率
	float burstTime[128];	// 脉冲对应的起始时间
	float width[128];		// 脉冲对应的脉宽
	float bw;				// 图传信号带宽
	float amp;				// 图传信号幅度
	float range;			// 图传信号距离（未算）
}droneResult_t;

// 目标跟踪信息结构体
struct targetInfo
{
	unsigned short num;	// 目标编号
	char type;			// 目标类型：1数字图传、2模拟图传、3Wifi图传
	char typeIndex;		// 数据库索引号

	int psbID[10];			// 可能的机型序号
	int nPsbID;				// 机型序号可能性数量

	char name[50];		// 目标数据库名称
	char ID[32];		// 目标ID信息
	unsigned int cnt;	// 目标累计点数

	char status;		// 目标状态：1发现、2跟踪、3记忆
	unsigned int timeCreate;	// 目标建立时间
	unsigned int timeLast;		// 目标最近测量时间
	unsigned int timeLock;		// 目标锁定时间

	float trackFreq;	// 跟踪频率
	float trackBw;		// 跟踪带宽
	float trackCenterFreq;		// 跟踪中心频点
	int trackBwS;		// 跟踪频率起始索引
	int trackBwE;		// 跟踪频率终止索引

	float update;		// 数据更新率
	float ampMean;		// 目标平均幅度
	float ampQF;		// 目标幅度起伏
	float freqSMean;	// 起始频率均值
	float freqEMean;	// 终止频率均值

	float dist;			// 目标距离估计

	unsigned int sqTime[TarDataLength];	// 到达时间序列
	float sqAmp[TarDataLength];			// 幅度序列
	float sqFreq[TarDataLength];			// 频率序列
	float sqBw[TarDataLength];				// 带宽序列

	char trackPriority;	// 跟踪优先级
};




// 侦测函数声明------------------------------------------

void  sfV2DroneSniffer(struct droneResult *droneInfo, int *nDrone, unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], unsigned short upMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], float cenFreq, float fs, float bw, float gain, int calRows, unsigned short *output, unsigned int *flagMark);

void  sfThresholdCompare(float* thresh, float cenFreq, int tdB, int mdB);
float sfAutoThreshOnLineF(float *line, int d3000, float dT, int maxIter);
int   sfThresholdToPulse(float *s, int n, float thresh, int delt, int minw, int *q1, int *w, float *colSnr, float *colAm);
int   sfVidBlocksDetect(struct BlockRow *aBlkRow, float *s, int n, int *q1, int *w, int nw, float *colSnr, int antIndex, int colIndexS, int colIndexE, float cenFreq, float dT, float dF, struct DroneLib *UAVtypes, int nUAV);
void  sfMatchPulseT(int **match, float *t2, int *index, int n, float *pulseT, int nPulseT, float pulseTErr);
void  sfPulseToTarget(struct blockGroup *droneBlock, int nBlock, struct BlockRow aBlkRow, int fstart, int fend, float cenFreq, float dT, float dF, struct DroneLib *UAVtypes);

void sfV2GetSpectrum(unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chnum, unsigned short *pHist1, unsigned short *pHist2, unsigned short *pHist3);

void  sfV2S1DownLoadSniffer(struct blockGroup *droneBlock, int *nBlock, unsigned short ***downMat, int chn, int nRows, int nCols, float cenFreq, float gain, float dT, float dF, struct DroneLib *UAVtypes, int nUav, unsigned short worknum, unsigned short *workStatus, unsigned short *workPowerN, unsigned short *workPowerS);
void  sfV2S1FrequenceHist(unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chnum, int* hist, float cenFreq);
int   sfV2S1CalculateFrequenceSE(int *line, float dT, int *lineS, int *lineE);

void  sfV2S2DownLoadSniffer(struct blockGroup *droneBlock, int *nBlock, unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chn, int nRows, int nCols, float cenFreq, float gain, float dT, float dF, struct DroneLib *UAVtypes, int nUav, unsigned short worknum, unsigned short *workStatus, unsigned short *workPowerN, unsigned short *workPowerS, unsigned int *flagMark);
float sfV2S2AutoThreshOnMap(unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chnum, int d128s, int d128e, int d3000, float dT, int maxIter, float baseT);
void  sfV2S2FrequenceHist(unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chnum, int* hist, int d128s, int d128e, int d3000, float threshold);
float sfV2S2AutoThreshOnLineI(int *line, int d128s, int d128e, float dT, int maxIter);
int   sfV2S2CalculateFrequenceSE(int *line, float dT, int lineWidthT, int *lineS, int *lineE);
void sfSetDigiResult(struct droneResult *droneInfo, int *nDrone, struct DroneLib *droneDigiLIB, struct blockGroup *uavPulse, int nUavPulse, char type);



void sfMoniSniffer(struct blockGroup *droneBlock, int *nBlock, unsigned short downMat[CHAN_NUM][LINE_NUM][COLUMN_NUM], int chn, int nRows, int nCols, float cenFreq);
void sfMoniToTarget(struct blockGroup *droneBlock, int nBlock, float cenFreq, float tarFreq, float tarBW, float tarAm, float T);
void sfSetMoniResult(struct droneResult *droneInfo, int *nDrone, struct blockGroup *uavPulse, int nUavPulse, char type);


// 跟踪函数声明------------------------------------------
void sfTargetManageCluster(struct targetInfo *tar, int *tarNum, struct droneResult *drone, int droneNum, unsigned int calCnt, float cenFreq);
void sfTargetManageStatus(struct targetInfo *tar, int *tarNum, unsigned int calCnt);
void sfTargetSort(struct targetInfo *tar, int *tarNum, int *pList);

// 定向函数声明------------------------------------------

int   sfDxNewProcessing(unsigned short AmPowerQ[CHAN_NUM][LINE_NUM][COLUMN_NUM], unsigned short AmPowerD[CHAN_NUM][LINE_NUM][COLUMN_NUM], int StartLine, int EndLine, int numUav, int calRows, float *listAngle, float *listAmQ, float *listAmD, float *listAmC, int *listFP, int numDx);
int   sfDxNewGetIndex(unsigned short AmPower[CHAN_NUM][LINE_NUM][COLUMN_NUM], int StartLine, int EndLine, struct DroneLib *UAVtypes, int numUav, int calRows, int *ps1, int *pw1, int *pn1, int *ps2, int *pw2, int *pn2);
float sfDxNewCalculateAm(unsigned short AmPower[CHAN_NUM][LINE_NUM][COLUMN_NUM], int StartLine, int EndLine, int calRows, int *ps, int *pw, int pnum);
int   sfDxThresholdWithAm(float *s, int n, float thresh, int delt, int minw, int *q1, int *w, float *colAm);

int sfDxWifiProcessing(unsigned short wifiPowerQ, unsigned short wifiPowerD, float *listAngle, float *listAmQ, float *listAmD, float *listAmC, int *listFP, int numDx);

int sfDxIfAzimuthEnough(float *listAngleAz, int numDx, float *oStart, float *oEnd);
int sfDxDirCalc(float *ampDat, float *angleDat, int nAngle, float *ampMax, float *angleCen, int typeD);
int sfDxIfAimed(float azimuthNow, float elevationNow, float azimuthCen);
int sfDxIfLegal(float *listAngleAz, float *listAngleEl, float *listAmQ, float *listAmD, float *listAmC, int *listFP, int numDx);

#endif