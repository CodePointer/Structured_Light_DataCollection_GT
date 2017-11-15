// ����ͷ�������
#include <Windows.h>
#include <HVDAILT.h>
#include <HVDef.h>
#include <HVExtend.h>
#include <Raw2Rgb.h>

// OpenCV
#include <opencv2/opencv.hpp>
#include <vector>

#define MAT_GRAYNUM_V 6
#define MAT_GRAYNUM_H 5
#define MAT_PHASENUM 4

using namespace std;
using namespace cv;

// һЩ����
static int PROJECTOR_RESLINE = 1280;
static int PROJECTOR_RESROW = 800;
static int CAMERA_RESLINE = 640;
static int CAMERA_RESROW = 512;
static int PC_BIASLINE = 1366;
static int PC_BIASROW = 0;
static std::string MATFILE_PATH = "Pattern/";
static std::string GRAYFILE_NAME = "GrayMat";
static std::string PHASEFILE_NAME = "PhaseMat";
static std::string PATTERNFILE_NAME = "DeBruijn";
static std::string MATFILE_END = ".jpg";
static std::string STORAGE_PATH = "Data/20160510_Rotation/";
static std::string STORAGE_END = ".bmp";

// ���ݴ洢ģ�顣�洢�м��������浵
// ���ã�SetMatFilePath��SetMatFileName��Store
class StorageModule
{
private:
	std::string m_matFileNamePath;
	std::string m_matFileName;
	std::string m_matFileNameEnd;
public:
	StorageModule();
	~StorageModule();

	bool Store(cv::Mat *pictures, int num);
	bool SetMatFilePath(std::string matFileNamePath);
	bool SetMatFileName(std::string matFileName, std::string matFileNameEnd);
};

// ����ͷ�����ࡣ��ȡһ֡����ͷͼƬ����ʵ�ֶ�����ͷ�Ĺ����򿪡��رյȵȣ�
// Ĭ����ֻ��һ������ͷ��
// ���ã�InitCamera��getPicture��CloseCamera
class CamManager
{
private:
	int sumDeviceNum;			// �ܹ�������ͷ��Ŀ
	int nowDeviceNum;			// ��ǰʹ�õ�����ͷ��š�������ͷ��Ϊ0��
	HHV DeviceHandle;			// ����ͷ�豸���
	BYTE **ppBuffer;			// ͼ�񻺳���
	int BufferNum;				// ͼ�񻺳�����С����ͼƬ����
	int used;					// ����������ͼƬ����
	int resRow;					// ͼƬ�ֱ��ʣ���
	int resLine;				// ͼƬ�ֱ��ʣ���


public:
	CamManager();
	~CamManager();
	int getSumDeviceNum();		// ��ȡ�ܹ�����ͷ��Ŀ
	int getNowDeviceNum();		// ��ǰ����ͷ��
	bool InitCamera();			// ��ʼ���豸�Ա��ɼ�
	bool SnapShot();
	bool getPicture(cv::Mat & pic);	// ��ȡһ֡ͼ�񣬲�����pic�С�
	bool CloseCamera();			// �ر�����ͷ����ֹ�ɼ�
};

// ͶӰ�ǿ����ࡣ���ڿ���ͶӰ��Ͷ��Ҫ���ͼ����
// ���ã�InitProjector��presentPicture��CloseProjector
class ProManager
{
private:
	int m_resRow;			// ͶӰ�ǵ��зֱ���
	int m_resLine;			// ͶӰ�ǵ��зֱ���
	int m_biasRow;			// ���ڵ���ƫ�Ʒֱ���
	int m_biasLine;			// ���ڵ���ƫ�Ʒֱ���
	std::string m_winName;	// ��������

public:
	ProManager();			// ���캯��
	~ProManager();			// ��������
	bool InitProjector();							// ��ʼ���豸
	bool presentPicture(cv::Mat pic, int time);		// ʹͶӰ�Ƿų�pic����ʱtime
	bool presentPicture(uchar x, int time);			// ʹͶӰ�Ƿų�ȫΪx��ͼƬ����ʱtime
	bool CloseProjector();							// �ر��豸
};

// ͼ�������ࡣ����ͶӰ��Щͼ����
// ������ʲôͼ����ֻ��һ�����ͶӰ������
// ���ã�SetMatFileName��LoadPattern��GetPattern��UnloadPattern
class CPattern
{
private:
	int m_resRow;			// ͼ����зֱ���
	int m_resLine;			// ͼ����зֱ���
	std::string m_matFileNamePre;	// �洢ͼ����ļ���
	std::string m_matFileNameEnd;	// �洢ͼ��ĺ�׺��
	int m_presentNum;				// ���������Mat
	int m_totalNum;					// Mat����
	cv::Mat * m_Pictures;			// �洢���е�ͼ��

public:
	CPattern();
	~CPattern();
	bool LoadPattern(int totalNum);					// ��ȡ�洢��Pattern��Ϣ
	bool UnloadPattern();							// ����洢��Pattern��Ϣ
	int GetPreviousNum();							// ��ȡ��ǰͼ�����
	int GetTotalNum();								// ��ȡ��ͼ�����
	bool GetPattern(int matNum, cv::Mat & pic);		// ���ݱ�Ż�ȡͼ��

	bool SetMatFileName(std::string pre, std::string end);			// ���ô洢ͼ����ļ���
};

// ���ݲɼ��ࡣ������������ݲɼ��������²������
// �ⲿ����ֻ��������Ҫ���ݼ��ɡ�
// ���ã�Init��Collect��Close
// ���ã�Init��GetStatFrame��GetDynaFrame��Close
class DataCollector
{
private:
	CamManager * m_camera;
	ProManager * m_projector;
	CPattern m_vGray;
	Mat m_CameraMat;
	Mat m_vGrayMat[MAT_GRAYNUM_V * 2];
	/*CPattern m_hGray;
	Mat m_hGrayMat[MAT_GRAYNUM_H * 2];*/
	CPattern m_vPhase;
	Mat m_vPhaseMat[MAT_PHASENUM];
	/*CPattern m_hPhase;
	Mat m_hPhaseMat[MAT_PHASENUM];*/
	CPattern m_dynaPattern;
	Mat * m_dynaMat;
	bool m_visualize;
	bool m_storage;
	int m_maxFrame;

	bool Storage();

public:
	DataCollector();
	~DataCollector();
	bool Init();
	bool Collect();
	bool Close();
};