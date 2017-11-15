#ifndef _SENSORMANAGER_H_
#define _SENSORMANAGER_H_

#include <string>
#include <opencv2/opencv.hpp>
#include <strstream>
#include "cam_manager.h"
#include "pro_manager.h"
#include "static_para.h"
#include "global_fun.h"

using namespace std;
using namespace cv;

// Sensor module
// usage: init, load/unload, close
class SensorManager {
private:
	// 待投影图案存储：
	int pattern_num_;
	int now_num_;
	string file_path_;
	string file_name_;
	string file_suffix_;
	Mat * pattern_mats_;

	// 设备管理
	CamManager * cam_device_;
	ProManager * pro_device_;

public:
	SensorManager();
	~SensorManager();

	// Initialization
	bool InitSensor();

	// Close sensor.
	bool CloseSensor();

	// Read patterns
	bool LoadPatterns(int pattern_num, string file_path, 
                    string file_name, string file_suffix);

	// Release patterns
	bool UnloadPatterns();

	// Set project patterns, count from 0
	bool SetProPicture(int nowNum = 0);
	
	// Get camera picture according to index
	Mat GetCamPicture(int idx);

	// Get now projected pattern
	Mat GetProPicture();
	
};

#endif