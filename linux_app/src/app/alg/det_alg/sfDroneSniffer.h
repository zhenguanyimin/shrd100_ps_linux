#ifndef _SFDRONESNIFFER_H__
#define _SFDRONESNIFFER_H__

// �궨��

#define NCh  1		// ͨ������
#define NFFT 512	// Ƶ������
#define NRow 12000	// Ƶ������
#define M 4			// �ۼ�����
#define N 4			// �ۼ�����

#define CHAN_NUM    1
#define LINE_NUM    (NRow / M)  // 3000
#define COLUMN_NUM  (NFFT / N)  // 128

#define sfM1 128	// ��׼��������-Ƶ��
#define sfM2 3000	// ��׼��������-ʱ��
#define sfdF 0.4
#define sfdT 0.04

#define MaxDrone 20
#define MaxPulseInGroup 120
#define MaxObjNum 20
#define MaxHistoryNum 1000
#define MaxPulseTimeNum 16

#define PI 3.1415926535898

#define MaxDroneinLib 30	// ���������Ŀ�����
#define MaxNumVidT 50		// ���������
#define MaxPulseInObj 100	// ���������
#define MaxVidPulse 50		// �����������
#define maxPulseWNumInLib 5	// ����������������������

#define MAXnAngle 256		// ����׶�ִ�в�������
#define TarDataLength 20	// TarInfo�ṹ�������ݴ洢����
#define TarLength 16		// TarInfo�ṹ�����鳤��
// ���ݽṹ����

// ����ͼ�������⣬�����ֶν���ͼ����Ч�������ֶν���ң����Ч
struct DroneLib
{
	char  name[50];					//����
	int   psbID[10];				//���ܵĻ������
	int   nPsbID;					//������ſ���������
	char  downOrUp;					//ͼ����ң�أ�����down��ͼ��������up��ң��
	float freqPoints[60];			//Ƶ�㡣��Ӧͼ�����ŵ���ң�ص���Ƶ�㣬MHz
	int   nfreq;					//Ƶ�����
	char  isFixedFreq;				//freqPoints����Ƶ�η�Χ����Ƶ�㣬����0��ʾ��ӦƵ�������ޣ�����1���ӦƵ��
	int   hoppType;					//��Ƶ���ͣ��ò�������ң����Ч��0�ϸ���Ƶ������Ӧʱ����ӦƵ�㷢���źţ�1����������Ƶ�Ҵ󲿷�Ƶ�ʲ��䣬2ֻ��Ҫ������Ƶ����
	float pulseW[maxPulseWNumInLib];//������������ms��
	int   nPulseW;					//�����������
	char  isFixedPulseW;			//�����Ƿ�̶���0���̶���1�̶�
	float pulseT[5];				//���ڣ�ms
	int   nPulseT;					//��������
	float pulseBW[5];				//�����С��MHz
	int   nPulseBW;					//��������
	float pulseWErr[maxPulseWNumInLib];	//������������ޣ�ms
	int   meetPulseW[maxPulseWNumInLib];//���������ȵ���ʹ���
	float pulseTErr;				//�����ظ�������ms
	int   hoppCnt;					//���������Ƶ����ʹ�����ʵ�����������������
	float freqErr;					//Ƶ���������,MHz
	int   method;					//ʹ�÷�������δ���ã�
	float SNR;						//�����������ޣ���δ���ã�
	float duty;						//�ŵ�ռ�ձȣ���δ���ã�

	// �����ֶ�
	float cPower[5];				//�ο����ʣ�0��SRRC��1��FCC��2��CE��3��Max��4��Min
};

// ģ��ͼ�������⣬�����ֶν���ͼ����Ч�������ֶν���ң����Ч
struct VideoLib
{
	char  name[50];					//����
	int   psbID[10];				//���ܵĻ������
	int   nPsbID;					//������ſ���������

	float freqPoints[10];			//Ƶ�㡣��Ӧͼ�����ŵ���ң�ص���Ƶ�㣬MHz
	int   nfreq;					//Ƶ�����
	float freqErr;					//Ƶ��������ޣ�MHz������Ƶ��ƫ������
	int filedIndex;                 //��Ƶ������Ƶ��Ŀ���ƫ������-20230913-ZSY

	float sigBW;					//�źŴ���
	float detBW;					//������
	float detatt;					//�ߴ�˥��
	float detzkb1;					//ͨ��ռ�ձ�
	float detzkb2;					//�ߴ�ռ�ձ�

	float tbSft;					//ͬ���ź�Ƶ��ƫ��
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

// ���˻������
typedef struct droneResult
{
	char type;				// �ź����ͣ�1����ͼ����2ģ��ͼ����3Wifiͼ��
	char index;				// ���ݿ��б��ı��
	char uni[32];			// Ŀ��Ψһ����-Wifi��дMac��ַ,����������дĬ��ֵ

	char name[50];			// Ŀ��ͼ������
	int ID;					// ID��
	int psbID[10];			// ���ܵĻ������
	int nPsbID;				// ������ſ���������
	char  flag;				// �����ݿ��еı��
	float freq[128];		// �����Ӧ��Ƶ��
	float burstTime[128];	// �����Ӧ����ʼʱ��
	float width[128];		// �����Ӧ������
	float bw;				// ͼ���źŴ���
	float amp;				// ͼ���źŷ���
	float range;			// ͼ���źž��루δ�㣩
}droneResult_t;

// Ŀ�������Ϣ�ṹ��
struct targetInfo
{
	unsigned short num;	// Ŀ����
	char type;			// Ŀ�����ͣ�1����ͼ����2ģ��ͼ����3Wifiͼ��
	char typeIndex;		// ���ݿ�������

	int psbID[10];			// ���ܵĻ������
	int nPsbID;				// ������ſ���������

	char name[50];		// Ŀ�����ݿ�����
	char ID[32];		// Ŀ��ID��Ϣ
	unsigned int cnt;	// Ŀ���ۼƵ���

	char status;		// Ŀ��״̬��1���֡�2���١�3����
	unsigned int timeCreate;	// Ŀ�꽨��ʱ��
	unsigned int timeLast;		// Ŀ���������ʱ��
	unsigned int timeLock;		// Ŀ������ʱ��

	float trackFreq;	// ����Ƶ��
	float trackBw;		// ���ٴ���
	float trackCenterFreq;		// ��������Ƶ��
	int trackBwS;		// ����Ƶ����ʼ����
	int trackBwE;		// ����Ƶ����ֹ����

	float update;		// ���ݸ�����
	float ampMean;		// Ŀ��ƽ������
	float ampQF;		// Ŀ��������
	float freqSMean;	// ��ʼƵ�ʾ�ֵ
	float freqEMean;	// ��ֹƵ�ʾ�ֵ

	float dist;			// Ŀ��������

	unsigned int sqTime[TarDataLength];	// ����ʱ������
	float sqAmp[TarDataLength];			// ��������
	float sqFreq[TarDataLength];			// Ƶ������
	float sqBw[TarDataLength];				// ��������

	char trackPriority;	// �������ȼ�
};




// ��⺯������------------------------------------------

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


// ���ٺ�������------------------------------------------
void sfTargetManageCluster(struct targetInfo *tar, int *tarNum, struct droneResult *drone, int droneNum, unsigned int calCnt, float cenFreq);
void sfTargetManageStatus(struct targetInfo *tar, int *tarNum, unsigned int calCnt);
void sfTargetSort(struct targetInfo *tar, int *tarNum, int *pList);

// ����������------------------------------------------

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