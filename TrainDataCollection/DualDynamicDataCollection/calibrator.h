#ifndef _CALIBRATOR_H_
#define _CALIBRATOR_H_

#include <opencv2/opencv.hpp>
#include <vector>
#include <sstream>
#include "sensor_manager.h"
#include "gray_decoder.h"
#include "phase_decoder.h"
#include "visual_mod.h"
#include "global_fun.h"
#include "static_para.h"
using namespace cv;
using namespace std;

// Used for calibration for system.
class Calibrator {
private:
  // Sensor
  SensorManager * sensor_;
  string pattern_path_;
  CamMatSet * cam_mats_;
  // Chess points reco
  vector<vector<Point2f>> * cam_points_;
  vector<Point2f> * tmp_cam_points_;
  vector<vector<Point2f>> pro_points_;
  vector<Point2f> tmp_pro_points_;
  vector<vector<Point3f>> obj_points_;
  vector<Point3f> tmp_obj_points_;
  // Result
  Mat * cam_matrix_;
  Mat * cam_distor_;
  Mat pro_matrix_;
  Mat pro_distor_;
  int stereo_size_;
  StereoCalibSet * stereo_set_;

  bool ReleaseSpace();			// 释放空间
  bool RecoChessPointObj(int frameIdx);		// 填充m_objPoint
  bool RecoChessPointCam(int frameIdx);		// 填充m_camPoint
  bool RecoChessPointPro(int frameIdx);		// 填充m_proPoint
  bool PushChessPoint(int frameIdx);			// 保存棋盘格，存储到本地
public:
  Calibrator();
  ~Calibrator();
  bool Init();				// 初始化
  bool Calibrate();			// 标定主函数。
  bool Result();
};

#endif