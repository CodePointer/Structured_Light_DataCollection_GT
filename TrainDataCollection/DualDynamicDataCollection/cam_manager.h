#ifndef _CAMMANAGER_H_
#define _CAMMANAGER_H_

#include <opencv2/opencv.hpp>
#include <Windows.h>	// ����ͷ������Ҫ
#include <HVDAILT.h>
#include <HVDef.h>
#include <HVExtend.h>
#include <Raw2Rgb.h>	// ����ͷ������Ҫ
#include "static_para.h"

using namespace std;
using namespace cv;

// ����ͷ�����ࡣ��ȡһ֡����ͷͼƬ����ʵ�ֶ�����ͷ�Ĺ����򿪡��رյȵȣ�
// 2 camera is registered
// Usage: InitCamera��GetPicture��CloseCamera
class CamManager {
private:
	int sum_device_num_;			// camera numbers in total
	HHV * device_handles_;		  // camera handles
	bool * device_valid_;				// camera valid
	BYTE **pp_buffer_;				// image buffer
	int kBufferNum;					// size of buffer. image numbers
	int buffer_used_;				// used image numbers of buffer
	int img_height_;				// resolution of image: height
	int img_width_;					// resolution of image: width

public:
	CamManager();
	~CamManager();
	int get_sum_device_num();		// get total camera number
	bool get_device_valid(int idx);		// if left camera is valid
	bool InitCamera();			// ��ʼ���豸�Ա��ɼ�
	bool SnapShot(int idx);
	bool GetPicture(int idx, Mat & pic);	// Get a picture from camera
	bool CloseCamera();			// �ر�����ͷ����ֹ�ɼ�
};

#endif