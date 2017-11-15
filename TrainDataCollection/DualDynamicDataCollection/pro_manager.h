#ifndef _PROMANAGER_H_
#define _PROMANAGER_H_

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "static_para.h"

using namespace std;
using namespace cv;

// 投影仪控制类。用于控制投影仪投射要求的图案。
// 调用：InitProjector，presentPicture，CloseProjector
class ProManager
{
private:
	int pattern_height_;			  // 投影仪的行分辨率
	int pattern_width_;			    // 投影仪的列分辨率
	int bias_height_;			      // 窗口的行偏移分辨率
	int bias_width_;			      // 窗口的列偏移分辨率
	string win_name_;		        // 窗口名称

public:
	ProManager();			  // 构造函数
	~ProManager();			// 析构函数
	bool InitProjector();							// 初始化设备
	bool PresentPicture(Mat pic, int time);			// 使投影仪放出pic，延时time
	bool PresentPicture(uchar x, int time);			// 使投影仪放出全为x的图片，延时time
	bool CloseProjector();							// 关闭设备
};

#endif