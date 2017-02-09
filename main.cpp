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

	//// ��������ͷ
	//CCamera Camera;
	//CProjector Projector;
	//CPattern Pattern;
	//CStorage Storage;
	//Mat CamMat;
	//Mat ProMat;
	//Mat CamRec;
	//namedWindow("Camera", CV_WINDOW_NORMAL);


	//// ��ʼ������
	//if (status)	// ���
	//{
	//	status = Camera.InitCamera();
	//	int stableNum = 5;
	//	for (int i = 0; i < stableNum; i++)
	//	{
	//		status = Camera.getPicture(CamMat);
	//	}
	//}
	//if (status) // ͶӰ��
	//{
	//	status = Projector.InitProjector();
	//}
	//if (status) // ͼ������
	//{
	//	Pattern.SetMatFileName(MATFILE_PATH + PATTERNFILE_NAME, MATFILE_END);
	//	status = Pattern.LoadPattern(1);
	//}
	//CamRec = imread(MATFILE_PATH + "Rec.png", CV_LOAD_IMAGE_GRAYSCALE);

	//// ѭ���ɼ�������
	//int numGetPicture = 4;
	//int nowNum = 1;
	//while (nowNum <= numGetPicture)
	//{
	//	// �趨�洢·��
	//	std::string subPath;
	//	std::strstream ss;
	//	ss << nowNum << "/";
	//	ss >> subPath;
	//	std::string storePath = STORAGE_PATH + subPath;
	//	printf("StoragePath:%s\n", storePath.c_str());
	//	Storage.SetMatFilePath(storePath);
	//	nowNum++;

	//	// ׼��ͶӰ��
	//	if (status)
	//	{
	//		status = Pattern.GetPattern(0, ProMat);
	//	}
	//	if (status)
	//	{
	//		status = Projector.presentPicture(ProMat, 20);
	//	}

	//	// ¼��
	//	bool recording = false;
	//	int kNum = 0;
	//	Mat Show;
	//	while (true)
	//	{
	//		// ��ȡͼ��
	//		Camera.getPicture(CamMat);
	//		CamMat.copyTo(Show);
	//		
	//		// ���ӻ�
	//		if (recording)
	//		{
	//			Show = CamMat + CamRec;
	//		}
	//		imshow("Camera", Show);

	//		// �����Ƿ�洢
	//		int button = waitKey(20);
	//		if ((button == 'y') && !recording)	// ��ʼ¼��
	//		{
	//			printf("Begin Record.\n");
	//			recording = true;
	//			continue;
	//		}
	//		if ((button == 'n') && recording)	// ֹͣ¼��
	//		{
	//			printf("End Record.\n");
	//			recording = false;
	//			break;
	//		}
	//		if (recording)		// �洢
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

	//// �ر�������
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