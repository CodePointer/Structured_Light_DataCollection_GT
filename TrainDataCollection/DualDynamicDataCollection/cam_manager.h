#ifndef _CAMMANAGER_H_
#define _CAMMANAGER_H_

#include <opencv2/opencv.hpp>
#include <Windows.h>	// 摄像头驱动需要
#include <HVDAILT.h>
#include <HVDef.h>
#include <HVExtend.h>
#include <Raw2Rgb.h>	// 摄像头驱动需要
#include "static_para.h"

using namespace std;
using namespace cv;

// 摄像头控制类。获取一帧摄像头图片，并实现对摄像头的管理（打开、关闭等等）
// 2 camera is registered
// Usage: InitCamera，GetPicture，CloseCamera
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
	bool InitCamera();			// 初始化设备以备采集
	bool SnapShot(int idx);
	bool GetPicture(int idx, Mat & pic);	// Get a picture from camera
	bool CloseCamera();			// 关闭摄像头，终止采集
};

#endif