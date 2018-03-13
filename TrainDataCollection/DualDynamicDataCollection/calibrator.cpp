#include "calibrator.h"

//CVisualization myDebug("Debug");

// Set NULL to pointers
Calibrator::Calibrator() {
  this->sensor_ = NULL;
  this->cam_mats_ = NULL;
  this->cam_points_ = NULL;
  this->tmp_cam_points_ = NULL;
  this->cam_matrix_ = NULL;
  this->cam_distor_ = NULL;
  this->stereo_set_ = NULL;
  this->cam_visual_ = nullptr;
  this->pro_visual_ = nullptr;
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
  if (this->cam_visual_ != NULL) {
    delete(this->cam_visual_);
    this->cam_visual_ = NULL;
  }
  if (this->pro_visual_ != NULL) {
    delete(this->pro_visual_);
    this->pro_visual_ = NULL;
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
  this->tmp_cam_points_ = new vector<Point2f>[kCamDeviceNum];
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    vector<vector<cv::Point2f>>().swap(this->cam_points_[cam_idx]);
  }
  vector<vector<cv::Point2f>>().swap(this->pro_points_);
  vector<vector<cv::Point3f>>().swap(this->obj_points_);
  // Result
  this->cam_matrix_ = new Mat[kCamDeviceNum];
  this->cam_distor_ = new Mat[kCamDeviceNum];
  this->stereo_set_ = new StereoCalibSet[kStereoSize];
  this->cam_visual_ = new VisualModule("CameraShow");
  this->pro_visual_ = new VisualModule("ProjectorShow");
  return true;
}

// main function for calibration
bool Calibrator::Calibrate() {
  bool status = true;
  Mat * cam_tmp;
  cam_tmp = new Mat[kCamDeviceNum];
  Mat cam_mask(kCamHeight, kCamWidth, CV_8UC1);
  cam_mask.setTo(0);
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
      key = this->cam_visual_->Show(cam_tmp[0], 100, false, 0.5);
      //key = this->cam_visual_->CombineShow(cam_tmp, 2, 100, cam_mask, 0.5);
      if (key == 'y') {
        break;
      }
    }
    this->sensor_->UnloadPatterns();
    this->sensor_->LoadPatterns(1, this->pattern_path_, "empty", ".png");
    this->sensor_->SetProPicture(0);
    this->sensor_->UnloadPatterns();

    // Fill ObjPoint£»
    status = this->RecoChessPointObj(frm_idx);
    printf("For %dth picture: ObjPoint finished.\n", frm_idx + 1);

    // Fill CamPoint£»
    status = this->RecoChessPointCam(frm_idx);
    printf("For %dth picture: CamPoint finished.\n", frm_idx + 1);

    // Fill ProPoint£»
    status = this->RecoChessPointPro(frm_idx);
    // Make sure the recognition result is correct
    // save the corner information and correspondence information
    if (status) {
      printf("For %dth picture: ProPoint finished.\n", frm_idx + 1);
      this->PushChessPoint(frm_idx);
    } else {
      printf("Invalid pro_point. Discard.\n");
      frm_idx = frm_idx - 1;
    }
  }

  // Calibration
  printf("Begin Calibrating.\n");
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    calibrateCamera(this->obj_points_,
      this->cam_points_[cam_idx],
      Size(kCamWidth, kCamHeight),
      this->cam_matrix_[cam_idx],
      this->cam_distor_[cam_idx],
      noArray(),
      noArray(),
      CALIB_FIX_K3,
      TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30, 1e-16));
    printf("Finish cam[%d].\n", cam_idx);
  }
  calibrateCamera(this->obj_points_,
    this->pro_points_,
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

// Output and save
bool Calibrator::Result() {
  // Output individuals
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    stringstream ss;
    ss << cam_idx;
    string idx2str;
    ss >> idx2str;
    FileStorage fs_cam("cam" + idx2str + ".xml", FileStorage::WRITE);
    fs_cam << "cam_matrix" << this->cam_matrix_[cam_idx];
    fs_cam << "cam_distor" << this->cam_distor_[cam_idx];
    fs_cam.release();
    fstream file_cam;
    file_cam.open("cam" + idx2str + ".txt", ios::out);
    file_cam << "cam_matrix" << endl;
    file_cam << this->cam_matrix_[cam_idx] << endl;
    file_cam << "cam_distor" << endl;
    file_cam << this->cam_distor_[cam_idx] << endl;
    file_cam.close();
  }
  FileStorage fs_pro("pro.xml", FileStorage::WRITE);
  fs_pro << "pro_matrix" << this->pro_matrix_;
  fs_pro << "pro_distor" << this->pro_distor_;
  fs_pro.release();
  fstream file_pro;
  file_pro.open("pro.txt", ios::out);
  file_pro << "pro_matrix" << endl;
  file_pro << this->pro_matrix_ << endl;
  file_pro << "pro_distor" << endl;
  file_pro << this->pro_distor_ << endl;
  file_pro.close();
  // Stereo parameters
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    stringstream ss;
    ss << cam_idx;
    string idx2str;
    ss >> idx2str;
    FileStorage fs_cam_pro("cam" + idx2str + "_pro.xml", FileStorage::WRITE);
    fs_cam_pro << "rot" << this->stereo_set_[cam_idx].R;
    fs_cam_pro << "trans" << this->stereo_set_[cam_idx].T;
    fs_cam_pro.release();
    fstream file_cam_pro;
    file_cam_pro.open("cam" + idx2str + "_pro.txt", ios::out);
    file_cam_pro << "rot" << endl;
    file_cam_pro << this->stereo_set_[cam_idx].R << endl;
    file_cam_pro << "trans" << endl;
    file_cam_pro << this->stereo_set_[cam_idx].T << endl;
    file_cam_pro.close();
  }
  int ste_idx = kCamDeviceNum;
  for (int c_1 = 0; c_1 < kCamDeviceNum; c_1++) {
    stringstream ss_1;
    ss_1 << c_1;
    string idx2str_1;
    ss_1 >> idx2str_1;
    for (int c_2 = 1; c_2 < kCamDeviceNum; c_2++) {
      if (c_1 == c_2) {
        continue;
      }
      stringstream ss_2;
      ss_2 << c_2;
      string idx2str_2;
      ss_2 >> idx2str_2;
      FileStorage fs_cam_cam("cam" + idx2str_1 + "_cam" + idx2str_2 + ".xml",
                             FileStorage::WRITE);
      fs_cam_cam << "rot" << this->stereo_set_[ste_idx].R;
      fs_cam_cam << "trans" << this->stereo_set_[ste_idx].T;
      fs_cam_cam.release();
      fstream file_cam_cam;
      file_cam_cam.open("cam" + idx2str_1 + "_cam" + idx2str_2 + ".txt", 
                        ios::out);
      file_cam_cam << "rot" << endl;
      file_cam_cam << this->stereo_set_[ste_idx].R << endl;
      file_cam_cam << "trans" << endl;
      file_cam_cam << this->stereo_set_[ste_idx].T << endl;
      file_cam_cam.close();
      ste_idx++;
    }
  }
  printf("Calibration Finished.\n");
  return true;
}

// Fill the obj_points
bool Calibrator::RecoChessPointObj(int frameIdx) {
  vector<Point3f>().swap(this->tmp_obj_points_);
  for (int x = 0; x < kChessWidth; x++) {
    for (int y = 0; y < kChessHeight; y++) {
      this->tmp_obj_points_.push_back(Point3f(x, y, 0));
    }
  }
  return true;
}

// Reco cam_points and fill in
bool Calibrator::RecoChessPointCam(int frameIdx) {
  bool status = true;
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    vector<Point2f>().swap(this->tmp_cam_points_[cam_idx]);
  }
  Mat * cam_tmp;
  cam_tmp = new Mat[kCamDeviceNum];
  Mat cam_mask(kCamHeight, kCamWidth, CV_8UC1);
  cam_mask.setTo(0);
  // Set picture
  this->sensor_->LoadPatterns(1, this->pattern_path_, "empty", ".png");
  this->sensor_->SetProPicture(0);
  this->sensor_->UnloadPatterns();
  while (true) {
    bool reco_flag = true;
    for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
      cam_tmp[cam_idx] = this->sensor_->GetCamPicture(cam_idx);
    }
    //this->cam_visual_->CombineShow(cam_tmp, kCamDeviceNum, 100, cam_mask, 0.5);
    this->cam_visual_->Show(cam_tmp[0], 100, false, 0.5);
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
        this->pro_visual_->Show(cam_tmp[cam_idx], 100, false, 0.5);
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
        Mat draw_mat;
        cam_tmp[cam_idx].copyTo(draw_mat);
        drawChessboardCorners(
            draw_mat, Size(kChessHeight, kChessWidth),
            this->tmp_cam_points_[cam_idx], found);
        this->pro_visual_->Show(draw_mat, 100, false, 0.5);
      }
    }
    if (reco_flag) {
      //this->cam_visual_->CombineShow(cam_tmp, 2, 100, cam_mask, 0.5);
      this->cam_visual_->Show(cam_tmp[0], 100, false, 0.5);
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
  temp_total_mat.create(kCamHeight, kCamWidth, CV_64FC1);
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
      tmp_mul_collect.convertTo(tmp_mat, CV_64FC1);
      temp_total_mat += tmp_mat / kMultiCollectNum;
    }
    temp_total_mat.convertTo(this->cam_mats_[0].hor_phase[i], CV_8UC1);
  }
  this->sensor_->UnloadPatterns();

  // Decode
  GrayDecoder vgray_decoder;
  vgray_decoder.SetNumDigit(kVerGrayNum, true);
  vgray_decoder.SetCodeFileName(this->pattern_path_, "vGrayCode.txt");
  for (int i = 0; i < kVerGrayNum * 2; i++) {
    vgray_decoder.SetMat(i, this->cam_mats_[0].ver_gray[i]);
  }
  vgray_decoder.Decode();
  ver_gray_mat = vgray_decoder.GetResult();
  GrayDecoder hgray_decoder;
  hgray_decoder.SetNumDigit(kHorGrayNum, false);
  hgray_decoder.SetCodeFileName(this->pattern_path_, "hGrayCode.txt");
  for (int i = 0; i < kHorGrayNum * 2; i++) {
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
  //pro_visual.Show(ver_gray_mat, 0, true, 0.5);
  //pro_visual.Show(hor_gray_mat, 0, true, 0.5);
  //pro_visual.Show(ver_phase_mat, 0, true, 0.5);
  //pro_visual.Show(hor_phase_mat, 0, true, 0.5);
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
  this->cam_mats_[0].y_pro[0] = hor_gray_mat + hor_phase_mat;
  for (int h = 1; h < kCamHeight - 1; h++) {
    for (int w = 1; w < kCamWidth - 1; w++) {
      double x_pro_val = this->cam_mats_[0].x_pro[0].at<double>(h, w);
      if (x_pro_val < 0) {
        double x_up_val = this->cam_mats_[0].x_pro[0].at<double>(h - 1, w);
        double x_dn_val = this->cam_mats_[0].x_pro[0].at<double>(h + 1, w);
        if ((x_up_val > 0) && (x_dn_val > 0)) {
          this->cam_mats_[0].x_pro[0].at<double>(h, w) = (x_up_val + x_dn_val) / 2;
        }
      }
      double y_pro_val = this->cam_mats_[0].y_pro[0].at<double>(h, w);
      if (y_pro_val < 0) {
        double y_lf_val = this->cam_mats_[0].y_pro[0].at<double>(h, w - 1);
        double y_rt_val = this->cam_mats_[0].y_pro[0].at<double>(h, w + 1);
        if ((y_lf_val > 0) && (y_rt_val > 0)) {
          this->cam_mats_[0].y_pro[0].at<double>(h, w) = (y_lf_val + y_rt_val) / 2;
        }
      }
    }
  }
  //pro_visual.Show(this->cam_mats_[0].x_pro[0], 0, true, 0.5);
  //pro_visual.Show(this->cam_mats_[0].y_pro[0], 0, true, 0.5);

  // projector coord
  vector<Point2f>::iterator i;
  for (i = this->tmp_cam_points_[0].begin(); i != this->tmp_cam_points_[0].end(); ++i) {
    Point2f cam = *i;
    Point2f pro;
    int X = cam.x;
    int Y = cam.y;
    pro.x = this->cam_mats_[0].x_pro[0].at<double>(Y, X);
    if ((pro.x < 0) && (X > 0) && (X < kCamWidth - 1)) {
      float lf_val = this->cam_mats_[0].x_pro[0].at<double>(Y, X - 1);
      float rt_val = this->cam_mats_[0].x_pro[0].at<double>(Y, X + 1);
      if ((lf_val > 0) && (rt_val > 0)) {
        pro.x = (lf_val + rt_val) / 2;
      }
    }
    pro.y = this->cam_mats_[0].y_pro[0].at<double>(Y, X);
    if ((pro.y < 0) && (Y > 0) && (Y < kCamHeight - 1)) {
      float up_val = this->cam_mats_[0].y_pro[0].at<double>(Y - 1, X);
      float dn_val = this->cam_mats_[0].y_pro[0].at<double>(Y + 1, X);
      if ((up_val > 0) && (dn_val > 0)) {
        pro.y = (up_val + dn_val) / 2;
      }
    }
    this->tmp_pro_points_.push_back(pro);
  }
  // Draw it
  Mat draw_mat;
  draw_mat.create(800, 1280, CV_8UC1);
  draw_mat.setTo(0);
  drawChessboardCorners(
      draw_mat, Size(kChessHeight, kChessWidth), this->tmp_pro_points_, true);
  int key = this->pro_visual_->Show(draw_mat, 0, false, 0.5);
  if (key == 'y') {
    status = true;
  } else {
    status = false;
  }

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
  return status;
}