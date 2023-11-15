#ifndef _SFAIRBORNESNIFFER_H__
#define _SFAIRBORNESNIFFER_H__

// 宏定义

#define ARBN_NCh  1		// 通道数量
#define ARBN_NFFT 512	// 频谱列数
#define ARBN_NRow 12000	// 频谱行数

#define ARBN_M1 128	// 标准数据行数-频率
#define ARBN_M2 3000	// 标准数据列数-时间
#define ARBN_DF 0.4
#define ARBN_DT 0.04

#define ARBN_MaxBlockGroupNum 20
#define ARBN_MaxPulseInGroup 120

#define ARBN_MaxDroneInLib 30	// 特征库最大目标个数
#define ARBN_MaxNumVidT 50		// 最大周期数
#define ARBN_MaxPulseInObj 100	// 最大脉冲数
#define ARBN_MaxVidPulseNum 50		// 最大检测脉冲数
#define ARBN_MaxPWNumInLib 5	// 特征库内脉宽类型最大个数

#define ARBN_PulseNumInFrame 40 // 单个时频图内允许的最大有效遥控脉冲数量

#define ARBN_TarDataLength 40	// TarInfo结构体内数据存储长度
#define ARBN_TargetLength 8		// TarInfo结构体数组长度


// 数据结构定义

// 数字图传特征库，部分字段仅对图传有效，部分字段仅对遥控有效
struct arbnDigiLib
{
	char  name[50];					//名称
	int   psbID[10];				//可能的机型序号
	int   nPsbID;					//机型序号可能性数量
	char  downOrUp;					//图传或遥控，下行down是图传，上行up是遥控
	float freqPoints[60];			//频点。对应图传的信道，遥控的跳频点，MHz
	int   nfreq;					//频点个数
	char  isFixedFreq;				//freqPoints中是频段范围还是频点，若是0表示对应频段下上限，若是1则对应频点
	int   hoppType;					//跳频类型，该参数仅对遥控有效。0严格按照频表在相应时间相应频点发出信号，1按照周期跳频且大部分频率不变，2只需要周期跳频即可
	float pulseW[ARBN_MaxPWNumInLib];//脉冲宽度特征，ms。
	int   nPulseW;					//宽度种类数量
	char  isFixedPulseW;			//脉宽是否固定，0不固定，1固定
	float pulseT[5];				//周期，ms
	int   nPulseT;					//周期种类
	float pulseBW[5];				//带宽大小，MHz
	int   nPulseBW;					//带宽种类
	float pulseWErr[ARBN_MaxPWNumInLib];	//脉冲宽度误差容限，ms
	int   meetPulseW[ARBN_MaxPWNumInLib];//满足脉冲宽度的最低次数
	float pulseTErr;				//脉冲重复周期误差，ms
	int   hoppCnt;					//满足规律跳频的最低次数，实际用作周期满足个数
	float freqErr;					//频率误差容限,MHz
	int   method;					//使用方法（暂未启用）
	float SNR;						//最低信噪比门限（暂未启用）
	float duty;						//信道占空比（暂未启用）

	// 新增字段
	float cPower[5];				//参考功率，0：SRRC，1：FCC，2：CE，3：Max，4：Min
};

// 数字遥控特征库
struct arbnYkLib
{
	char  name[50];					//名称
	int	  pulseWN;					//脉冲时间宽度点数
	int   pulseFN;					//脉冲频率宽度点数

	int	  TscaS;
	int	  TscaE;
	int	  FscaS;
	int	  FscaE;
	
	int   pulseTN[5];				//重复周期点数
	int   nPulseTN;					//周期种类

	int   Tmust;
	int   Fmust;
	int   Nmust;
};


struct arbnBlockRow
{
	int   id;
	int   uavIndex;
	int   antIndex;
	char  inCh[ARBN_NCh];
	int   colIndex;
	int   allColIndex[ARBN_M1];
	int   nCol;
	int   q1[ARBN_MaxPulseInGroup];
	int   w[ARBN_MaxPulseInGroup];
	float meanColAmp;
	float snr;
	int   nw;
};

struct arbnblockGroup
{
	int   id;
	int   index;
	float range;
	char  inCh[ARBN_NCh];
	float freq[ARBN_MaxPulseInGroup];
	float bw;
	float burstTime[ARBN_MaxPulseInGroup];
	float width[ARBN_MaxPulseInGroup];
	float amp[ARBN_NCh][ARBN_MaxPulseInGroup];
	float dist[ARBN_MaxPulseInGroup];
	int   n;
};

// 无人机侦测结果
typedef struct arbnDroneResult
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
}arbnDroneResult_t;


// 遥控侦测结果
struct ykResult
{
	char  sigType;			// 类型
	char  sigSubType;		// 子类型
	float sigBw;			// 带宽
	float sigPw;			// 脉宽
	float sigHopType;		// 跳频类型
	float sigHopStep;		// 跳频步进
	float sigPRT;			// 重复周期

	float freqBand;			// 频段
	float freqCenter;		// 中心频率
	float freqMeasure;		// 测量频率
	unsigned int measureNum;// 捕获采样点数
	float fw;				// 方位
	float fy;				// 俯仰
	float amp;				// 图传信号幅度

	int orgTPoint;		// 原始时间序列
	float orgFwPhase;		// 方位原始相位
	float orgFyPhase;		// 俯仰原始相位

};

// 目标跟踪信息结构体
struct arbnTargetInfo
{
	unsigned short num;	// 目标编号
	char type;			// 目标类型：1数字图传、2模拟图传、3Wifi图传
	char typeIndex;		// 数据库索引号
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
	float trackBand;	// 跟踪波段
	float trackNum;		// 跟踪中心频点对应点数
	float trackFW1;		// 测量系方位
	float trackFY1;		// 测量系俯仰
	float trackFW2;		// 固定系方位
	float trackFY2;		// 固定系俯仰
	int trackBwS;		// 跟踪频率起始索引
	int trackBwE;		// 跟踪频率终止索引		

	float update;		// 数据更新率
	float ampMean;		// 目标平均幅度
	float ampQF;		// 目标幅度起伏
	float freqSMean;	// 起始频率均值	
	float freqEMean;	// 终止频率均值

	float dist;			// 目标距离估计

	unsigned int sqTime[ARBN_TarDataLength];	// 到达时间序列
	float sqAmp[ARBN_TarDataLength];			// 幅度序列
	float sqFreq[ARBN_TarDataLength];			// 频率序列
	float sqFw[ARBN_TarDataLength];				// 带宽序列
	float sqFy[ARBN_TarDataLength];				// 带宽序列

	char trackPriority;	// 跟踪优先级
};

// 无人机侦测结果
typedef struct arbnTargetResult
{
	unsigned short tarNum;

	unsigned short t1Num;
	unsigned short t1Freq;


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
} arbnTargetResult_t;


// 输入定义-补偿数据
typedef struct CompensateData
{
	char sn[64];
	short compensate_data[2560];
	short extra_compensate_data[128];
} CompensateData_t;

// 输入定义-控制指令
typedef struct ioController
{
	unsigned short workMode;	// 工作状态控制
	unsigned short dxNum;		// 定向目标编号
	unsigned short dxFreq;		// 定向频率
} ioController_t;

// 输入定义-姿态信息
typedef struct ioFlyStatus
{
	short yaw;		// 飞机偏航角,0.1°
	short pitch;	// 飞机俯仰角,0.1°
	short roll;		// 飞机滚转角,0.1°
	short wY;		// 飞机偏航角速度,0.01°/ s
	short wP;		// 飞机俯仰角速度,0.01°/ s
	short wR;		// 飞机滚转角速度,0.01°/ s
	int longitude;  // 飞机经度,0.0000001°
	int latitude;	// 飞机纬度,0.0000001°
	int altitude;	// 飞机高度,0.01m
	short vX;		// 飞机速度X分量,0.01m / s
	short vY;		// 飞机速度X分量,0.01m / s
	short vZ;		// 飞机速度X分量,0.01m / s
} ioFlyStatus_t;

// 输出定义-测量信息
typedef struct ioMeasureInfo
{
	unsigned short workStatus;	// 系统工作状态
	unsigned short tarNum;		// 目标有效个数

	unsigned short t1Num;	// 目标1编号
	unsigned short t1Freq;	// 目标1频率
	unsigned short t1Amp;	// 目标1幅度
	short t1mFw;			// 目标1测量系方位角,0.1°
	short t1mFy;			// 目标1测量系方位角,0.1°
	short t1fFw;			// 目标1测量系方位角,0.1°
	short t1fFy;			// 目标1测量系方位角,0.1°

	unsigned short t2Num;	// 目标2编号
	unsigned short t2Freq;	// 目标2频率
	unsigned short t2Amp;	// 目标2幅度
	short t2mFw;			// 目标2测量系方位角,0.1°
	short t2mFy;			// 目标2测量系方位角,0.1°
	short t2fFw;			// 目标2测量系方位角,0.1°
	short t2fFy;			// 目标2测量系方位角,0.1°

	unsigned short t3Num;	// 目标3编号
	unsigned short t3Freq;	// 目标3频率
	unsigned short t3Amp;	// 目标3幅度
	short t3mFw;			// 目标3测量系方位角,0.1°
	short t3mFy;			// 目标3测量系方位角,0.1°
	short t3fFw;			// 目标3测量系方位角,0.1°
	short t3fFy;			// 目标3测量系方位角,0.1°

	unsigned short t4Num;	// 目标4编号
	unsigned short t4Freq;	// 目标4频率
	unsigned short t4Amp;	// 目标4幅度
	short t4mFw;			// 目标4测量系方位角,0.1°
	short t4mFy;			// 目标4测量系方位角,0.1°
	short t4fFw;			// 目标4测量系方位角,0.1°
	short t4fFy;			// 目标4测量系方位角,0.1°

}ioMeasureInfo_t;


void sfArbnProcessing(struct CompensateData bcData, unsigned short am1Mat[1][3000][128], unsigned short am2Mat[1][3000][128], unsigned short ph1Mat[1][3000][128], unsigned short ph2Mat[1][3000][128], float cenFreq, int antStatus, unsigned int mCnt, struct ioController inC, struct ioFlyStatus inF, struct ioMeasureInfo *outInfo, short *buffer);

// 侦测函数声明------------------------------------------

int sfArbnSniffer(struct CompensateData bcData, unsigned short amMat[1][3000][128], unsigned short ph1Mat[1][3000][128], unsigned short ph2Mat[1][3000][128], float cenFreq, int antStatus, struct ykResult *outYK, short *buffer);

int sfArbnJammerFind(struct CompensateData bcData, unsigned short amMat[1][3000][128], unsigned short ph1Mat[1][3000][128], unsigned short ph2Mat[1][3000][128], float cenFreq, int antStatus, struct ykResult *outYK, short *buffer);

int sfArbnDigiSigFind(unsigned short amMat[1][3000][128], float cenFreq, int nRows, float dT, float dF, struct arbnBlockRow *aBlkRow);
float sfArbnAutoThreshOnMap(unsigned short downMat[1][3000][128], int chnum, int d128s, int d128e, int d3000, float dT, int maxIter, float baseT);
void  sfArbnFrequenceHist(unsigned short downMat[1][3000][128], int chnum, int* hist, int d128s, int d128e, int d3000, float threshold);
float sfArbnAutoThreshOnLineI(int *line, int d128s, int d128e, float dT, int maxIter);
int   sfArbnCalculateFrequenceSE(int *line, float dT, int lineWidthT, int *lineS, int *lineE);
int sfArbnThresholdToPulse(float *s, int n, float thresh, int delt, int minw, int *q1, int *w, float *colSnr, float *colAm);
int sfArbnVidBlocksDetect(struct arbnBlockRow *aBlkRow, float *s, int n, int *q1, int *w, int nw, float *colSnr, int antIndex, int colIndexS, int colIndexE, float cenFreq, float dT, float dF, struct arbnDigiLib *UAVtypes, int nUAV);
void sfArbnMatchPulseT(int **match, float *t2, int *index, int n, float *pulseT, int nPulseT, float pulseTErr);
void sfArbnPulseToTarget(struct arbnblockGroup *droneBlock, int nBlock, struct arbnBlockRow aBlkRow, int fstart, int fend, float cenFreq, float dT, float dF, struct arbnDigiLib *UAVtypes);
float sfArbnAutoThreshOnLineF(float *line, int d128, float dT, int maxIter);
void  sfArbnThresholdCompare(float* thresh, float cenFreq, int tdB, int mdB);
int sfArbnConnectAreaCalculate(unsigned short amMatIn[1][3000][128], unsigned short amMatOut[1][3000][128], unsigned short amMatOrg[1][3000][128], int *numTList, int *numFList, int *typeList, float inThd);
void sfArbnConnectAreaSelect(int *numTList, int *numFList, int *typeList, int num, int *okList);

int sfArbnPhaseGet(float *phaseList, int phaseNum, float *outPhase);

int sfArbnToSkySniffer(struct CompensateData bcData, unsigned short amMat[1][3000][128], unsigned short ph1Mat[1][3000][128], unsigned short ph2Mat[1][3000][128], float cenFreq, int antStatus, struct ykResult *outYK, short *buffer);



void sfArbnTargetManageCluster(struct arbnTargetInfo *tar, int *tarNum, struct ykResult *yk, int ykNum, unsigned int calCnt, struct ioFlyStatus inF);
void sfArbnTargetManageStatus(struct arbnTargetInfo *tar, int *tarNum, unsigned int calCnt);
void sfArbnAngleTransform(float inFw, float inFy, float installFy, struct ioFlyStatus inF, float *outFw, float *outFy);
void sfArbnTargetSort(struct arbnTargetInfo *tar, int *tarNum, int *pList);



// 信息转译部分
void sfArbnInfoTransfer(struct arbnTargetInfo *tar, int *tarNum, int *pList, struct ioController inC, struct ioMeasureInfo *outInfo);
void sfArbnInfoInitial(struct ioMeasureInfo *outInfo);

void sfArbnInfoTestOut(struct ykResult *yk, int ykNum, unsigned short mode, unsigned int cnt, struct ioMeasureInfo *outInfo);
#endif
