#include "calibrator.h"

//CVisualization myDebug("Debug");
VisualModule cam_visual("CameraShow");
VisualModule myProjector("ProjectorShow");

// Set NULL to pointers
Calibrator::Calibrator() {
  this->sensor_ = NULL;
  this->cam_mats_ = NULL;
  this->cam_points_ = NULL;
  this->tmp_cam_points_ = NULL;
  this->cam_matrix_ = NULL;
  this->cam_distor_ = NULL;
  this->stereo_set_ = NULL;
}

// Release
Calibrator::~Calibrator() {
  this->ReleaseSpace();
}

// Release the space allocated by pointers
bool Calibrator::ReleaseSpace() {
  if (this->sensor_ != NULL) {
    delete(this->sensor_);
    this->sensor_ = NULL;
  }
  if (this->cam_mats_ != NULL) {
    delete[](this->cam_mats_);
    this->cam_mats_ = NULL;
  }
  if (this->cam_points_ != NULL) {
    delete[](this->cam_points_);
    this->cam_points_ = NULL;
  }
  if (this->tmp_cam_points_ != NULL) {
    delete[](this->tmp_cam_points_);
    this->tmp_cam_points_ = NULL;
  }
  if (this->cam_matrix_ != NULL) {
    delete[](this->cam_matrix_);
    this->cam_matrix_ = NULL;
  }
  if (this->cam_distor_ != NULL) {
    delete[](this->cam_distor_);
    this->cam_distor_ = NULL;
  }
  if (this->stereo_set_ != NULL) {
    delete[](this->stereo_set_);
    this->stereo_set_ = NULL;
  }
  return true;
}

// Initialize
bool Calibrator::Init() {
  this->ReleaseSpace();
  // Sensor
  this->sensor_ = new SensorManager;
  this->sensor_->InitSensor();
  this->pattern_path_ = "Patterns/";
  this->cam_mats_ = new CamMatSet[kCamDeviceNum];
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    this->cam_mats_[cam_idx].ver_gray = new Mat[kVerGrayNum * 2];
    this->cam_mats_[cam_idx].hor_gray = new Mat[kHorGrayNum * 2];
    this->cam_mats_[cam_idx].ver_phase = new Mat[kPhaseNum];
    this->cam_mats_[cam_idx].hor_phase = new Mat[kPhaseNum];
    this->cam_mats_[cam_idx].x_pro = new Mat[kChessFrameNumber];
    this->cam_mats_[cam_idx].y_pro = new Mat[kChessFrameNumber];
  }
  // Chess points reco
  this->cam_points_ = new vector<vector<Point2f>>[kCamDeviceNum];
  this->tmp_cam_points_ = new vector<vector<Point2f>>[kCamDeviceNum];
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    vector<vector<cv::Point2f>>().swap(this->cam_points_[cam_idx]);
  }
  vector<vector<cv::Point2f>>().swap(this->pro_points_);
  vector<vector<cv::Point3f>>().swap(this->obj_points_);
  // Result
  this->cam_matrix_ = new Mat[kCamDeviceNum];
  this->cam_distor_ = new Mat[kCamDeviceNum];
  this->stereo_set_ = new StereoCalibSet[kStereoSize];
  return true;
}

// main function for calibration
bool Calibrator::Calibrate() {
  bool status = true;
  Mat * cam_tmp;
  cam_tmp = new Mat[kCamDeviceNum];
  // Chess Reco
  for (int frm_idx = 0; frm_idx < kChessFrameNumber; frm_idx++) {
    // set projected pattern as empty
    this->sensor_->LoadPatterns(1, this->pattern_path_, "wait", ".bmp");
    this->sensor_->SetProPicture(0);
    printf("Ready for collection. Press 'y' to continue\n");
    while (true) {
      for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
        cam_tmp[cam_idx] = this->sensor_->GetCamPicture(cam_idx);
      }
      int key;
      key = cam_visual.CombineShow(cam_tmp, 2, 100, 0.5);
      if (key == 'y') {
        break;
      }
    }
    this->sensor_->UnloadPatterns();
    this->sensor_->LoadPatterns(1, this->pattern_path_, "empty", ".bmp");
    this->sensor_->SetProPicture(0);
    this->sensor_->UnloadPatterns();

    // Fill ObjPoint；
    status = this->RecoChessPointObj(frm_idx);
    printf("For %dth picture: ObjPoint finished.\n", frm_idx + 1);

    // Fill CamPoint；
    status = this->RecoChessPointCam(frm_idx);
    printf("For %dth picture: CamPoint finished.\n", frm_idx + 1);

    // Fill ProPoint；
    status = this->RecoChessPointPro(frm_idx);
    printf("For %dth picture: ProPoint finished.\n", frm_idx + 1);

    // Make sure the recognition result is correct
    // save the corner information and correspondence information
    int key = 0;
    key = cam_visual.Show(this->m_chessMatDraw, 100, false, 0.5);
    key = myProjector.Show(this->m_proMatDraw, 0, false, 0.5);
    if (key == 'y') {
      status = this->PushChessPoint(frm_idx);
      printf("Finish %dth picture.\n", frm_idx + 1);
    } else {
      printf("Invalid %dth picture. Data discarded.\n", frm_idx + 1);
      frm_idx = frm_idx - 1;
      continue;
    }
  }

  // Calibration
  printf("Begin Calibrating.\n");
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    calibrateCamera(this->obj_points_,
      this->cam_points_,
      Size(kCamWidth, kCamHeight),
      this->cam_matrix_[cam_idx],
      this->cam_distor_[cam_idx],
      noArray(),
      noArray(),
      CALIB_FIX_K3 + CALIB_FIX_PRINCIPAL_POINT,
      TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 300, 1e-16));
    printf("Finish cam[%d].\n", cam_idx);
  }
  calibrateCamera(this->obj_points_,
    this->cam_points_,
    Size(kProWidth, kProHeight),
    this->pro_matrix_,
    this->pro_distor_,
    noArray(),
    noArray(),
    CALIB_FIX_K3,
    TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 300, 1e-16));
  printf("Finish pro.\n");
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    stereoCalibrate(this->obj_points_,
      this->cam_points_[cam_idx],
      this->pro_points_,
      this->cam_matrix_[cam_idx],
      this->cam_distor_[cam_idx],
      this->pro_matrix_,
      this->pro_distor_,
      Size(kCamWidth, kCamHeight),
      this->stereo_set_[cam_idx].R,
      this->stereo_set_[cam_idx].T,
      this->stereo_set_[cam_idx].E,
      this->stereo_set_[cam_idx].F,
      256,
      TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 300, 1e-16));
    printf("Finish cam[%d] & pro.\n", cam_idx);
  }
  int ste_idx = kCamDeviceNum;
  for (int c1 = 0; c1 < kCamDeviceNum; c1++) {
    for (int c2 = 1; c2 < kCamDeviceNum; c2++) {
      if (c1 != c2) {
        stereoCalibrate(this->obj_points_,
          this->cam_points_[c1],
          this->cam_points_[c2],
          this->cam_matrix_[c1],
          this->cam_distor_[c1],
          this->cam_matrix_[c2],
          this->cam_distor_[c2],
          Size(kCamWidth, kCamHeight),
          this->stereo_set_[ste_idx].R,
          this->stereo_set_[ste_idx].T,
          this->stereo_set_[ste_idx].E,
          this->stereo_set_[ste_idx].F,
          256,
          TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 300, 1e-16));
        printf("Finish cam[%d] & cam[%d].\n", c1, c2);
        ste_idx++;
      }
    }
  }
  delete[]cam_tmp;
  return true;
}

// 输出保存数据 TODO
bool Calibrator::Result()
{
  FileStorage fs("CalibrationResult.xml", FileStorage::WRITE);
  cout << "CamMat" << endl;
  fs << "CamMat" << this->m_camMatrix;
  cout << this->m_camMatrix << endl;

  cout << "ProMat" << endl;
  fs << "ProMat" << this->m_proMatrix;
  cout << this->m_proMatrix << endl;

  cout << "R,T" << endl;
  fs << "R" << this->m_R;
  fs << "T" << this->m_T;
  cout << this->m_R << endl;
  cout << this->m_T << endl;
  fs.release();

  cout << "Calibration Finished. Data was stored at <CalibrationResult.xml>." << endl;

  return true;
}

// Fill the obj_points
bool Calibrator::RecoChessPointObj(int frameIdx) {
  vector<Point3f>().swap(this->tmp_obj_points_);
  for (int x = 0; x < kChessWidth; x++) {
    for (int y = 0; y < kChessHeight; y++) {
      this->m_objPointTmp.push_back(Point3f(x, y, 0));
    }
  }
  return true;
}

// Reco cam_points and fill in
bool Calibrator::RecoChessPointCam(int frameIdx) {
  bool status = true;
  vector<Point2f>().swap(this->m_camPointTmp);
  Mat * cam_tmp;
  cam_tmp = new Mat[kCamDeviceNum];
  while (true) {
    bool reco_flag = true;
    for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
      cam_tmp[cam_idx] = this->sensor_->GetCamPicture(cam_idx);
    }
    cam_visual.CombineShow(cam_tmp, kCamDeviceNum, 100, 0.5);
    for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
      int max_attempt_times = 1;
      int k = 0;
      int found = 0;
      while (k++ < max_attempt_times) {
        found = findChessboardCorners(
            cam_tmp[cam_idx], Size(kChessHeight, kChessWidth), 
            this->tmp_cam_points_[cam_idx], 
            CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE);
        /*drawChessboardCorners(
            cam_tmp[cam_idx], Size(kChessHeight, kChessWidth), 
            this->tmp_cam_points_[cam_idx], found);*/
        cam_visual.Show(cam_tmp[cam_idx], 100, false, 0.5);
        if (found) {
          printf("Success for cam[%d].\n", cam_idx);
          break;
        } else {
          printf("Failed for cam[%d](%d/%d).\n", cam_idx, k, max_attempt_times);
          reco_flag = false;
        }
      }
      if (!reco_flag) {
        break;
      } else {
        cornerSubPix(
            cam_tmp[cam_idx], this->tmp_cam_points_[cam_idx], Size(5,5), Size(-1,-1), 
            TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 30, 0.1));
      }
    }
    if (reco_flag) {
      break;
    }
  }
  delete[]cam_tmp;
  return status;
}

// Reco pro_points according to cam[0]
bool Calibrator::RecoChessPointPro(int frameIdx) {
  bool status = true;
  vector<Point2f>().swap(this->tmp_pro_points_);
  Mat ver_gray_mat, hor_gray_mat, ver_phase_mat, hor_phase_mat;
  Mat ver_pro_mat, hor_pro_mat;
  int kMultiCollectNum = 5;
  Mat tmp_mul_collect;
  Mat tmp_mat;
  Mat temp_total_mat;
  temp_total_mat.create(CAMERA_RESROW, CAMERA_RESLINE, CV_64FC1);
  int ver_period, hor_period;
  int ver_gray_num, hor_gray_num;
  int ver_gray_period, hor_gray_period;

  // Collect
  this->sensor_->LoadPatterns(kVerGrayNum * 2, this->pattern_path_, "vGray", ".bmp");
  for (int i = 0; i < kVerGrayNum * 2; i++) {
    this->sensor_->SetProPicture(i);
    temp_total_mat.setTo(0);
    for (int k = 0; k < kMultiCollectNum; k++) {
      tmp_mul_collect = this->sensor_->GetCamPicture(0);
      tmp_mul_collect.convertTo(tmp_mat, CV_64FC1);
      temp_total_mat += tmp_mat / kMultiCollectNum;
    }
    temp_total_mat.convertTo(this->cam_mats_[0].ver_gray[i], CV_8UC1);
  }
  this->sensor_->UnloadPatterns();
  this->sensor_->LoadPatterns(kHorGrayNum * 2, this->pattern_path_, "hGray", ".bmp");
  for (int i = 0; i < kHorGrayNum * 2; i++)	{
    this->sensor_->SetProPicture(i);
    temp_total_mat.setTo(0);
    for (int k = 0; k < kMultiCollectNum; k++) {
      tmp_mul_collect = this->sensor_->GetCamPicture(0);
      tmp_mul_collect.convertTo(tmp_mat, CV_64FC1);
      temp_total_mat += tmp_mat / kMultiCollectNum;
    }
    temp_total_mat.convertTo(this->cam_mats_[0].hor_gray[i], CV_8UC1);
  }
  this->sensor_->UnloadPatterns();
  this->sensor_->LoadPatterns(kPhaseNum, this->pattern_path_, "vPhase", ".bmp");
  for (int i = 0; i < kPhaseNum; i++) {
    this->sensor_->SetProPicture(i);
    temp_total_mat.setTo(0);
    for (int k = 0; k < kMultiCollectNum; k++) {
      tmp_mul_collect = this->sensor_->GetCamPicture(0);
      tmp_mul_collect.convertTo(tmp_mat, CV_64FC1);
      temp_total_mat += tmp_mat / kMultiCollectNum;
    }
    temp_total_mat.convertTo(this->cam_mats_[0].ver_phase[i], CV_8UC1);
  }
  this->sensor_->UnloadPatterns();
  this->sensor_->LoadPatterns(kPhaseNum, this->pattern_path_, "hPhase", ".bmp");
  for (int i = 0; i < kPhaseNum; i++) {
    this->sensor_->SetProPicture(i);
    temp_total_mat.setTo(0);
    for (int k = 0; k < kMultiCollectNum; k++) {
      tmp_mul_collect = this->sensor_->GetCamPicture(0);
      tmp_mul_collect.convertTo(tempMat, CV_64FC1);
      temp_total_mat += tempMat / kMultiCollectNum;
    }
    temp_total_mat.convertTo(this->cam_mats_[0].hor_phase[i], CV_8UC1);
  }
  this->sensor_->UnloadPatterns();

  // Decode
  GrayDecoder vgray_decoder;
  vgray_decoder.SetNumDigit(kVerGrayNum, true);
  vgray_decoder.SetCodeFileName(this->pattern_path_, "vGray.txt");
  for (int i = 0; i < kVerGrayNum; i++) {
    vgray_decoder.SetMat(i, this->cam_mats_[0].ver_gray[i]);
  }
  vgray_decoder.Decode();
  ver_gray_mat = vgray_decoder.GetResult();
  GrayDecoder hgray_decoder;
  hgray_decoder.SetNumDigit(kHorGrayNum, false);
  hgray_decoder.SetCodeFileName(this->pattern_path_, "hGray.txt");
  for (int i = 0; i < kHorGrayNum; i++) {
    hgray_decoder.SetMat(i, this->cam_mats_[0].hor_gray[i]);
  }
  hgray_decoder.Decode();
  hor_gray_mat = hgray_decoder.GetResult();
  PhaseDecoder vphase_decoder;
  ver_period = kProWidth / (1 << kVerGrayNum - 1);
  vphase_decoder.SetNumDigit(kPhaseNum, ver_period);
  for (int i = 0; i < kPhaseNum; i++) {
    vphase_decoder.SetMat(i, this->cam_mats_[0].ver_phase[i]);
  }
  vphase_decoder.Decode();
  ver_phase_mat = vphase_decoder.GetResult();
  PhaseDecoder hphase_decoder;
  hor_period = kProHeight / (1 << kHorGrayNum - 1);
  hphase_decoder.SetNumDigit(kPhaseNum, hor_period);
  for (int i = 0; i < kPhaseNum; i++) {
    hphase_decoder.SetMat(i, this->cam_mats_[0].hor_phase[i]);
  }
  hphase_decoder.Decode();
  hor_phase_mat = hphase_decoder.GetResult();

  // Combine
  /*myProjector.Show(vGrayMat, 0, true, 0.5);
  myProjector.Show(hGrayMat, 0, true, 0.5);
  myProjector.Show(vPhaseMat, 0, true, 0.5);
  myProjector.Show(hPhaseMat, 0, true, 0.5);*/
  ver_gray_num = 1 << kVerGrayNum;
  ver_gray_period = kProWidth / ver_gray_num;
  for (int h = 0; h < kCamHeight; h++) {
    for (int w = 0; w < kCamWidth; w++) {
      double gray_val = ver_gray_mat.at<double>(h, w);
      double phase_val = ver_phase_mat.at<double>(h, w);
      if (gray_val < 0) {
        ver_phase_mat.at<double>(h, w) = 0;
        continue;
      }
      if ((int)(gray_val / ver_gray_period) % 2 == 0) {
        if (phase_val >(double)ver_period * 0.75) {
          ver_phase_mat.at<double>(h, w) = phase_val - ver_period;
        }
      }
      else {
        if (phase_val < (double)ver_period * 0.25) {
          ver_phase_mat.at<double>(h, w) = phase_val + ver_period;
        }
        ver_phase_mat.at<double>(h, w) =
          ver_phase_mat.at<double>(h, w) - 0.5 * ver_period;
      }
    }
  }
  this->cam_mats_[0].x_pro[0] = ver_gray_mat + ver_phase_mat;
  hor_gray_num = 1 << kHorGrayNum;
  hor_gray_period = kProHeight / hor_gray_num;
  //this->my_debug_->Show(tmp_gray_mat, 0, true, 0.5);
  //this->my_debug_->Show(tmp_phase_mat, 0, true, 0.5);
  for (int h = 0; h < kCamHeight; h++) {
    for (int w = 0; w < kCamWidth; w++) {
      double gray_val = hor_gray_mat.at<double>(h, w);
      double phase_val = hor_phase_mat.at<double>(h, w);
      if (gray_val < 0) {
        hor_phase_mat.at<double>(h, w) = 0;
        continue;
      }
      if ((int)(gray_val / hor_gray_period) % 2 == 0) {
        if (phase_val >(double)hor_period * 0.75) {
          hor_phase_mat.at<double>(h, w) = phase_val - hor_period;
        }
      }
      else {
        if (phase_val < (double)hor_period * 0.25) {
          hor_phase_mat.at<double>(h, w) = phase_val + hor_period;
        }
        hor_phase_mat.at<double>(h, w) =
          hor_phase_mat.at<double>(h, w) - 0.5 * hor_period;
      }
    }
  }
  /*this->my_debug_->Show(tmp_gray_mat, 0, true, 0.5);
  this->my_debug_->Show(tmp_phase_mat, 0, true, 0.5);*/
  this->cam_mats_[0].y_pro[0] = hor_gray_mat + hor_phase_mat;

  // projector coord
  vector<Point2f>::iterator i;
  for (i = this->tmp_cam_points_[0].begin(); i != this->tmp_cam_points_[0].end(); ++i) {
    Point2f cam = *i;
    Point2f pro;
    int X = cam.x;
    int Y = cam.y;
    pro.x = this->cam_mats_[0].x_pro[0].at<double>(Y, X);
    pro.y = this->cam_mats_[0].y_pro[0].at<double>(Y, X);
    this->tmp_pro_points_.push_back(pro);
  }
  // Draw it
  Mat draw_mat;
  draw_mat.create(800, 1280, CV_8UC1);
  draw_mat.setTo(0);
  drawChessboardCorners(
      draw_mat, Size(kChessHeight, kChessWidth), this->tmp_pro_points_, true);
  myProjector.Show(draw_mat, 500, false, 0.5);

  return status;
}

// Save chess points
bool Calibrator::PushChessPoint(int frameIdx) {
  bool status = true;
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    this->cam_points_[cam_idx].push_back(this->tmp_cam_points_[cam_idx]);
  }
  this->obj_points_.push_back(this->tmp_obj_points_);
  this->pro_points_.push_back(this->tmp_pro_points_);

  //// 存储本地图像
  //strstream ss;
  //string IdxtoStr;
  //ss << frameIdx + 1;
  //ss >> IdxtoStr;

  //// 相机图像
  //CStorage camChessMat;
  //camChessMat.SetMatFileName("RecoChessPoint/", "cam_mat" + IdxtoStr, ".png");
  //camChessMat.Store(&(this->m_chessMat), 1);
  //camChessMat.SetMatFileName("RecoChessPoint/", "corner_res" + IdxtoStr, ".png");
  //camChessMat.Store(&(this->m_chessMatDraw), 1);

  //// 投影仪坐标
  //FileStorage fs;
  //fs.open("RecoChessPoint/xpro_mat" + IdxtoStr + ".xml", FileStorage::WRITE);
  //fs << "xpro_mat" << this->xpro_mats_[frameIdx];
  //fs.release();
  //fs.open("RecoChessPoint/ypro_mat" + IdxtoStr + ".xml", FileStorage::WRITE);
  //fs << "ypro_mat" << this->ypro_mats_[frameIdx];
  //fs.release();
  /*CStorage proChessMat;
  proChessMat.SetMatFileName("RecoChessPoint/" + IdxtoStr + "/", "vGray", ".bmp");
  proChessMat.Store(this->m_grayV, GRAY_V_NUMDIGIT * 2);
  proChessMat.SetMatFileName("RecoChessPoint/" + IdxtoStr + "/", "hGray", ".bmp");
  proChessMat.Store(this->m_grayH, GRAY_H_NUMDIGIT * 2);
  proChessMat.SetMatFileName("RecoChessPoint/" + IdxtoStr + "/", "vPhase", ".bmp");
  proChessMat.Store(this->m_phaseV, PHASE_NUMDIGIT);
  proChessMat.SetMatFileName("RecoChessPoint/" + IdxtoStr + "/", "hPhase", ".bmp");
  proChessMat.Store(this->m_phaseH, PHASE_NUMDIGIT);
  proChessMat.SetMatFileName("RecoChessPoint/" + IdxtoStr + "/", "ProMatDraw", ".bmp");
  proChessMat.Store(&(this->m_proMatDraw), 1);*/

  return status;
}