#include "head.h"
#include <strstream>
using namespace cv;

Mat Result;

bool IsValid(Mat pic)
{
	//return true;
	bool valid;
	vector<Point2f> camPoint;
	Mat temp;
	pic.copyTo(temp);
	pic.copyTo(Result);
	resize(pic, temp, Size(640, 512));
	valid = findChessboardCorners(temp, Size(6, 9), camPoint, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE);
	drawChessboardCorners(temp, Size(6, 9), camPoint, valid);
	imshow("test", temp);
	waitKey(1000);
	return valid;
}

int main()
{
	bool status = true;
	
	CDataCollection dc;

	if (status)
	{
		status = dc.Init();
	}
	if (status)
	{
		status = dc.Collect();
	}
	if (status)
	{
		status = dc.Close();
	}

	//// 创建摄像头
	//CCamera Camera;
	//CProjector Projector;
	//CPattern Pattern;
	//CStorage Storage;
	//Mat CamMat;
	//Mat ProMat;
	//Mat CamRec;
	//namedWindow("Camera", CV_WINDOW_NORMAL);


	//// 初始化各类
	//if (status)	// 相机
	//{
	//	status = Camera.InitCamera();
	//	int stableNum = 5;
	//	for (int i = 0; i < stableNum; i++)
	//	{
	//		status = Camera.getPicture(CamMat);
	//	}
	//}
	//if (status) // 投影仪
	//{
	//	status = Projector.InitProjector();
	//}
	//if (status) // 图案管理
	//{
	//	Pattern.SetMatFileName(MATFILE_PATH + PATTERNFILE_NAME, MATFILE_END);
	//	status = Pattern.LoadPattern(1);
	//}
	//CamRec = imread(MATFILE_PATH + "Rec.png", CV_LOAD_IMAGE_GRAYSCALE);

	//// 循环采集数据组
	//int numGetPicture = 4;
	//int nowNum = 1;
	//while (nowNum <= numGetPicture)
	//{
	//	// 设定存储路径
	//	std::string subPath;
	//	std::strstream ss;
	//	ss << nowNum << "/";
	//	ss >> subPath;
	//	std::string storePath = STORAGE_PATH + subPath;
	//	printf("StoragePath:%s\n", storePath.c_str());
	//	Storage.SetMatFilePath(storePath);
	//	nowNum++;

	//	// 准备投影仪
	//	if (status)
	//	{
	//		status = Pattern.GetPattern(0, ProMat);
	//	}
	//	if (status)
	//	{
	//		status = Projector.presentPicture(ProMat, 20);
	//	}

	//	// 录制
	//	bool recording = false;
	//	int kNum = 0;
	//	Mat Show;
	//	while (true)
	//	{
	//		// 获取图像
	//		Camera.getPicture(CamMat);
	//		CamMat.copyTo(Show);
	//		
	//		// 可视化
	//		if (recording)
	//		{
	//			Show = CamMat + CamRec;
	//		}
	//		imshow("Camera", Show);

	//		// 决定是否存储
	//		int button = waitKey(20);
	//		if ((button == 'y') && !recording)	// 开始录制
	//		{
	//			printf("Begin Record.\n");
	//			recording = true;
	//			continue;
	//		}
	//		if ((button == 'n') && recording)	// 停止录制
	//		{
	//			printf("End Record.\n");
	//			recording = false;
	//			break;
	//		}
	//		if (recording)		// 存储
	//		{
	//			ss.clear();
	//			string fileName;
	//			ss << "Frame" << kNum;
	//			ss >> fileName;
	//			kNum++;
	//			
	//			Storage.SetMatFileName(fileName, STORAGE_END);
	//			status = Storage.Store(&CamMat, 1);
	//		}
	//	}
	//}

	//// 关闭与析构
	//if (status)
	//{
	//	status = Camera.CloseCamera();
	//	destroyWindow("Camera");
	//}
	//if (status)
	//{
	//	status = Projector.CloseProjector();
	//}
	//if (status)
	//{
	//	status = Pattern.UnloadPattern();
	//}

	return 0;
}