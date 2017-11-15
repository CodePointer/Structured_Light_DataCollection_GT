// 摄像头驱动相关
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

// 一些常量
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

// 数据存储模块。存储中间数据做存档
// 调用：SetMatFilePath、SetMatFileName、Store
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

// 摄像头控制类。获取一帧摄像头图片，并实现对摄像头的管理（打开、关闭等等）
// 默认是只有一个摄像头。
// 调用：InitCamera，getPicture，CloseCamera
class CamManager
{
private:
	int sumDeviceNum;			// 总共的摄像头数目
	int nowDeviceNum;			// 当前使用的摄像头序号。无摄像头则为0。
	HHV DeviceHandle;			// 摄像头设备句柄
	BYTE **ppBuffer;			// 图像缓冲区
	int BufferNum;				// 图像缓冲区大小，即图片张数
	int used;					// 缓冲区已用图片张数
	int resRow;					// 图片分辨率：行
	int resLine;				// 图片分辨率：列


public:
	CamManager();
	~CamManager();
	int getSumDeviceNum();		// 获取总共摄像头数目
	int getNowDeviceNum();		// 当前摄像头。
	bool InitCamera();			// 初始化设备以备采集
	bool SnapShot();
	bool getPicture(cv::Mat & pic);	// 获取一帧图像，并存在pic中。
	bool CloseCamera();			// 关闭摄像头，终止采集
};

// 投影仪控制类。用于控制投影仪投射要求的图案。
// 调用：InitProjector，presentPicture，CloseProjector
class ProManager
{
private:
	int m_resRow;			// 投影仪的行分辨率
	int m_resLine;			// 投影仪的列分辨率
	int m_biasRow;			// 窗口的行偏移分辨率
	int m_biasLine;			// 窗口的列偏移分辨率
	std::string m_winName;	// 窗口名称

public:
	ProManager();			// 构造函数
	~ProManager();			// 析构函数
	bool InitProjector();							// 初始化设备
	bool presentPicture(cv::Mat pic, int time);		// 使投影仪放出pic，延时time
	bool presentPicture(uchar x, int time);			// 使投影仪放出全为x的图片，延时time
	bool CloseProjector();							// 关闭设备
};

// 图案管理类。管理投影哪些图案。
// 不管是什么图案，只是一组组的投影出来。
// 调用：SetMatFileName，LoadPattern，GetPattern，UnloadPattern
class CPattern
{
private:
	int m_resRow;			// 图像的行分辨率
	int m_resLine;			// 图像的列分辨率
	std::string m_matFileNamePre;	// 存储图像的文件名
	std::string m_matFileNameEnd;	// 存储图像的后缀名
	int m_presentNum;				// 现在输出的Mat
	int m_totalNum;					// Mat总数
	cv::Mat * m_Pictures;			// 存储所有的图案

public:
	CPattern();
	~CPattern();
	bool LoadPattern(int totalNum);					// 读取存储的Pattern信息
	bool UnloadPattern();							// 清除存储的Pattern信息
	int GetPreviousNum();							// 获取当前图案编号
	int GetTotalNum();								// 获取总图案编号
	bool GetPattern(int matNum, cv::Mat & pic);		// 根据编号获取图案

	bool SetMatFileName(std::string pre, std::string end);			// 设置存储图像的文件名
};

// 数据采集类。控制整体的数据采集，屏蔽下层的内容
// 外部调用只需向其索要数据即可。
// 调用：Init，Collect，Close
// 调用：Init，GetStatFrame，GetDynaFrame，Close
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