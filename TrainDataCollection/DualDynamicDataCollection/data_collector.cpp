#include "data_collector.h"

DataCollector::DataCollector() {
	this->sensor_manager_ = NULL;
  this->cam_mats_ = NULL;
	this->cam_view_ = NULL;
  this->res_view_ = NULL;
  this->visualize_flag_ = false;
  this->storage_flag_ = false;
  this->max_frame_num_ = 0;
	return;
}

DataCollector::~DataCollector() {
	if (this->sensor_manager_ != NULL) {
		delete this->sensor_manager_;
		this->sensor_manager_ = NULL;
	}
  if (this->cam_mats_ != NULL) {
    delete[] this->cam_mats_;
    this->cam_mats_ = NULL;
  }
	if (this->cam_view_ != NULL) {
		delete this->cam_view_;
		this->cam_view_ = NULL;
	}
  if (this->res_view_ != NULL) {
    delete this->res_view_;
    this->res_view_ = NULL;
  }
	return;
}

// For initialization; set parameters and create new object
bool DataCollector::Init() {
	bool status = true;

	// flags
	this->max_frame_num_ = 100;
	this->visualize_flag_ = true;
	this->storage_flag_ = true;

	// pattern paths
	this->pattern_path_ = "Patterns/";
	this->vgray_name_ = "vGray";
	this->hgray_name_ = "hGray";
	this->gray_suffix_ = ".bmp";
	this->vgray_code_name_ = "vGrayCode";
	this->hgray_code_name_ = "hGrayCode";
	this->gray_code_suffix_ = ".txt";
	this->vphase_name_ = "vPhase";
	this->hphase_name_ = "hPhase";
	this->phase_suffix_ = ".bmp";
	// this->dyna_name_ = "part_pattern_2size4color";
	this->dyna_name_ = "pattern_2size4color";
	this->dyna_suffix_ = ".png";
	this->wait_name_ = "pattern_2size4color";
	this->wait_suffix_ = ".png";

	// storage paths
	this->save_data_path_ = "E:/Structured_Light_Data/20171104/";
	this->dyna_frame_path_ = "dyna/";
	this->dyna_frame_name_ = "dyna_mat";
	this->dyna_frame_suffix_ = ".png";
	this->pro_frame_path_ = "pro/";
	this->xpro_frame_name_ = "xpro_mat";
	this->ypro_frame_name_ = "ypro_mat";
	this->pro_frame_suffix_ = ".png";

	// Create folder for storage
	StorageModule storage;
	storage.CreateFolder(this->save_data_path_);
	//storage.CreateFolder(this->save_data_path_ + "1/" + this->dyna_frame_path_);
	//storage.CreateFolder(this->save_data_path_ + "1/" + this->pro_frame_path_);

	// Initialize sensor manager
	if (status)	{
		this->sensor_manager_ = new SensorManager;
		status = this->sensor_manager_->InitSensor();
		if (!status) {
			ErrorHandling("DataCollector::Init(), InitSensor failed.");
		}
	}

	// Allocate space
  this->cam_mats_ = new CamMatSet[kCamDeviceNum];
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    this->cam_mats_[cam_idx].ver_gray = new Mat[kVerGrayNum * 2];
    this->cam_mats_[cam_idx].hor_gray = new Mat[kHorGrayNum * 2];
    this->cam_mats_[cam_idx].ver_phase = new Mat[kPhaseNum];
    this->cam_mats_[cam_idx].hor_phase = new Mat[kPhaseNum];
    this->cam_mats_[cam_idx].dyna = new Mat[this->max_frame_num_];
    this->cam_mats_[cam_idx].x_pro = new Mat[this->max_frame_num_];
    this->cam_mats_[cam_idx].y_pro = new Mat[this->max_frame_num_];
  }
	this->cam_view_ = new VisualModule("Camera");
  this->res_view_ = new VisualModule("Result");

	return status;
}

bool DataCollector::CollectDynaData() {
	bool status = true;

	int now_group_idx = 0;
	int max_group_num = 5;
  printf("Begin collection.\n");

	while (now_group_idx++ <= max_group_num) {
    printf("Now group: %d\n", now_group_idx);
    // First create folder for now group
    string idx2str;
    stringstream ss;
    ss << now_group_idx;
    ss >> idx2str;
    StorageModule storage;
    // Wait for signal
    if (status) {
      int info = this->GetInputSignal();
      if (info == 2) { // End this program
        break;
      }
      else if (info == 1) { // End this group
        continue;
      }
    }
		// --------------------------------------------
		// First, collect frame_0 picture		
		// frame_0 should have the depth value	
		// Thus use gray_code and phase shifting
		// ------------------------------------------
		if (status) {
			status = this->CollectStaticFrame(0);
		}
		if (status) {
			status = this->DecodeSingleFrame(0);
		}
		// --------------------------------------------
		// Then collect dynamic frame data
		// These are combined into function
		// --------------------------------------------
		if (status) {
			status = this->CollectDynamicFrame();
		}
		// --------------------------------------------
		// Show result to judge the data value
		// Check the pro and dyna mats
		// And save results
		// --------------------------------------------
		if (status) {
			status = this->VisualizationForDynamicScene(this->max_frame_num_);
		}
		if (status) {
			status = this->StorageDataByGroup(now_group_idx);
		}
	}

	return status;
}

bool DataCollector::CollectStatData() {
  bool status = true;
  int now_group_idx = 0;
  int max_group_num = 5;

  printf("Begin collection.\n");
  while (now_group_idx++ <= max_group_num) {
    printf("Now group: %d\n", now_group_idx);
    // First create folder for now group
    string idx2str;
    stringstream ss;
    ss << now_group_idx;
    ss >> idx2str;
    StorageModule storage;
    
    // Collect each frames
    bool exit_flag = false;
    for (int frame_idx = 0; frame_idx < this->max_frame_num_; frame_idx++) {
      // Wait for signal
      printf("frm[%d]:\n", frame_idx);
      if (status) {
        int info = this->GetInputSignal();
        if (info == 2) { // End this program
          exit_flag = true;
          break;
        }
        else if (info == 1) { // End this group
          break;
        }
      }
      // Collect static frame and decode
      if (status) {
        status = this->CollectStaticFrame(frame_idx);
      }
      if (status) {
        status = this->sensor_manager_->LoadPatterns(1,
			    this->pattern_path_,
			    this->dyna_name_,
			    this->dyna_suffix_);
	    }
	    if (status) {
		    status = this->sensor_manager_->SetProPicture(0);
	    }
      if (status) {
        for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
          Mat Cam_mat = this->sensor_manager_->GetCamPicture(cam_idx);
          Cam_mat.copyTo(this->cam_mats_[cam_idx].dyna[frame_idx]);
        }
        status = this->sensor_manager_->UnloadPatterns();
      }
      if (status) {
        status = this->DecodeSingleFrame(frame_idx);
      }
      // Show and Check: storage or not
      if (status) {
        printf("\t<Y>/<N>\n");
        status = this->VisualizationForStaticScene(frame_idx);
      }
      if (status) {
        this->StorageDataByFrame(now_group_idx, frame_idx);
      } else {
        frame_idx--;
        status = true;
      }
    }
    if (exit_flag)
      break;
  }
  return status;
}

// return:
//     0: Continue
//     1: End collection of this group
//     2: End collection of program
//     3: Error
int DataCollector::GetInputSignal() {
  int info = 0;
  // Output information
  printf("\t<Y_es>/<N_extgroup>/<E_scape>\n");
  Mat cam_mat, little_mat;
  Mat * cam_tmp = new Mat[kCamDeviceNum];
  Mat * ltt_tmp = new Mat[kCamDeviceNum];
  // Set projector pattern as wait
  bool status = true;
  if (status) {
    status = this->sensor_manager_->LoadPatterns(1,
      this->pattern_path_,
      this->wait_name_,
      this->wait_suffix_);
  }
  if (status) {
    status = this->sensor_manager_->SetProPicture(0);
  }
  // Show camera image on visualization module
  if (status) {
    while (true) {
		  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
        cam_tmp[cam_idx] = this->sensor_manager_->GetCamPicture(cam_idx);
        ltt_tmp[cam_idx] = cam_tmp[cam_idx](Range(502, 523), Range(630, 651));
		  }
      int key1 = this->res_view_->CombineShow(cam_tmp, kCamDeviceNum, 100, 0.5);
      int key2 = this->cam_view_->CombineShow(ltt_tmp, kCamDeviceNum, 100, 20);
      if ((key1 == 'y') || (key2 == 'y')) {
        info = 0;
        break;
      } else if ((key1 == 'n') || (key2 == 'n')) {
        info = 1;
        break;
      }
      else if ((key1 == 'e') || (key2 == 'e')) {
        info = 2;
        break;
      }
    }
  }
  if (status) {
    this->sensor_manager_->UnloadPatterns();
  }
  delete[]cam_tmp;
  delete[]ltt_tmp;
  return info;
}

// Collect first frame
bool DataCollector::CollectStaticFrame(int frameNum) {
	bool status = true;
	int kMultiCollectNum = 5;
	Mat tmp_mul_collect;
	Mat temp_total_mat;
	Mat temp_mat;
  Mat ver_gray_mat, ver_phase_mat, hor_gray_mat, hor_phase_mat;
	temp_total_mat.create(kCamHeight, kCamWidth, CV_64FC1);

	// ver_gray:
	if (status) {
		status = this->sensor_manager_->LoadPatterns(
        kVerGrayNum * 2, this->pattern_path_, 
        this->vgray_name_, this->gray_suffix_);
	}
	if (status) {
		for (int i = 0; (i < kVerGrayNum * 2) && status; i++) {
			status = this->sensor_manager_->SetProPicture(i);
      for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
        if (status) {
          temp_total_mat.setTo(0);
          for (int k = 0; k < kMultiCollectNum; k++) {
            tmp_mul_collect = this->sensor_manager_->GetCamPicture(cam_idx);
            tmp_mul_collect.convertTo(temp_mat, CV_64FC1);
            temp_total_mat += temp_mat / kMultiCollectNum;
          }
          //this->cam_mats_[cam_idx].ver_gray[i].create(temp_total_mat.size(), CV_8UC1);
          temp_total_mat.convertTo(this->cam_mats_[cam_idx].ver_gray[i], CV_8UC1);
        }
      }
			
		}
	}
	if (status) {
		status = this->sensor_manager_->UnloadPatterns();
	}

	// ver_phase:
	if (status) {
		status = this->sensor_manager_->LoadPatterns(
        kPhaseNum, this->pattern_path_, 
        this->vphase_name_, this->phase_suffix_);
	}
	if (status) {
		for (int i = 0; (i < kPhaseNum) && status; i++) {
			status = this->sensor_manager_->SetProPicture(i);
			if (status) {
        for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
          temp_total_mat.setTo(0);
          for (int k = 0; k < kMultiCollectNum; k++) {
            tmp_mul_collect = this->sensor_manager_->GetCamPicture(cam_idx);
            tmp_mul_collect.convertTo(temp_mat, CV_64FC1);
            temp_total_mat += temp_mat / kMultiCollectNum;
          }
          temp_total_mat.convertTo(
              this->cam_mats_[cam_idx].ver_phase[i], CV_8UC1);
        }
			}
		}
	}
	if (status) {
		status = this->sensor_manager_->UnloadPatterns();
	}

	// hor_gray:
	if (status) {
		status = this->sensor_manager_->LoadPatterns(
        kHorGrayNum * 2, this->pattern_path_, 
        this->hgray_name_, this->gray_suffix_);
	}
	if (status) {
		for (int i = 0; (i < kHorGrayNum * 2) && status; i++) {
			status = this->sensor_manager_->SetProPicture(i);
      for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
        if (status) {
          temp_total_mat.setTo(0);
          for (int k = 0; k < kMultiCollectNum; k++) {
            tmp_mul_collect = this->sensor_manager_->GetCamPicture(cam_idx);
            tmp_mul_collect.convertTo(temp_mat, CV_64FC1);
            temp_total_mat += temp_mat / kMultiCollectNum;
          }
          temp_total_mat.convertTo(
              this->cam_mats_[cam_idx].hor_gray[i], CV_8UC1);
        }
      }
		}
	}
	if (status) {
		status = this->sensor_manager_->UnloadPatterns();
	}

	// hor_phase
	if (status) {
		status = this->sensor_manager_->LoadPatterns(
      kPhaseNum, this->pattern_path_,
			this->hphase_name_, this->phase_suffix_);
	}
	if (status) {
		for (int i = 0; (i < kPhaseNum) && status; i++) {
			status = this->sensor_manager_->SetProPicture(i);
      for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
        if (status) {
          temp_total_mat.setTo(0);
          for (int k = 0; k < kMultiCollectNum; k++) {
            tmp_mul_collect = this->sensor_manager_->GetCamPicture(cam_idx);
            tmp_mul_collect.convertTo(temp_mat, CV_64FC1);
            temp_total_mat += temp_mat / kMultiCollectNum;
          }
          temp_total_mat.convertTo(
              this->cam_mats_[cam_idx].hor_phase[i], CV_8UC1);
        }
      }
		}
	}
	if (status) {
		status = this->sensor_manager_->UnloadPatterns();
	}

	return status;
}

// ---------------------------------
// Use for collect data continually
// Set projector pattern previously
// Then collect picture with camera
// ----------------------------------
bool DataCollector::CollectDynamicFrame() {
	bool status = true;
	// Set projector pattern
	if (status) {
		status = this->sensor_manager_->LoadPatterns(1,
			this->pattern_path_,
			this->dyna_name_,
			this->dyna_suffix_);
	}
	if (status) {
		status = this->sensor_manager_->SetProPicture(0);
	}
	// collect camera image
	for (int frame_idx = 0; (frame_idx < this->max_frame_num_) && status; frame_idx++) {
    for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
      Mat CamMat = this->sensor_manager_->GetCamPicture(cam_idx);
      CamMat.copyTo(this->cam_mats_[cam_idx].dyna[frame_idx]);
    }
		printf("%d frame.\n", frame_idx);
	}
	// Unload projector pattern
	if (status) {
		this->sensor_manager_->UnloadPatterns();
	}

	return status;
}

bool DataCollector::DecodeSingleFrame(int frameNum) {
	bool status = true;
  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    Mat ver_gray_mat, hor_gray_mat, ver_phase_mat, hor_phase_mat;
    Mat ver_pro_mat, hor_pro_mat;

    // Decode
    GrayDecoder vgray_decoder;
    if (status) {
      status = vgray_decoder.SetNumDigit(kVerGrayNum, true);
    }
    if (status) {
      status = vgray_decoder.SetCodeFileName(
          this->pattern_path_, 
          this->vgray_code_name_ + this->gray_code_suffix_);
    }
    for (int i = 0; (i < kVerGrayNum * 2) && status; i++) {
      status = vgray_decoder.SetMat(i, this->cam_mats_[cam_idx].ver_gray[i]);
    }
    if (status) {
      status = vgray_decoder.Decode();
    }
    if (status) {
      ver_gray_mat = vgray_decoder.GetResult();
    }
    GrayDecoder hgray_decoder;
    if (status) {
      status = hgray_decoder.SetNumDigit(kHorGrayNum, false);
    }
    if (status) {
      status = hgray_decoder.SetCodeFileName(
          this->pattern_path_,
          this->hgray_code_name_ + this->gray_code_suffix_);
    }
    for (int i = 0; (i < kHorGrayNum * 2) && status; i++) {
      status = hgray_decoder.SetMat(i, this->cam_mats_[cam_idx].hor_gray[i]);
    }
    if (status) {
      status = hgray_decoder.Decode();
    }
    if (status) {
      hor_gray_mat = hgray_decoder.GetResult();
    }
    PhaseDecoder vphase_decoder;
    int ver_period = kProWidth / (1 << (kVerGrayNum - 1));
    if (status) {
      status = vphase_decoder.SetNumDigit(kPhaseNum, ver_period);
    }
    for (int i = 0; (i < kPhaseNum) && status; i++) {
      status = vphase_decoder.SetMat(i, this->cam_mats_[cam_idx].ver_phase[i]);
    }
    if (status) {
      status = vphase_decoder.Decode();
    }
    if (status) {
      ver_phase_mat = vphase_decoder.GetResult();
    }
    PhaseDecoder hphase_decoder;
    int hor_period = kProHeight / (1 << (kHorGrayNum - 1));
    if (status) {
      status = hphase_decoder.SetNumDigit(kPhaseNum, hor_period);
    }
    for (int i = 0; i < kPhaseNum; i++) {
      status = hphase_decoder.SetMat(i, this->cam_mats_[cam_idx].hor_phase[i]);
    }
    if (status) {
      status = hphase_decoder.Decode();
    }
    if (status) {
      hor_phase_mat = hphase_decoder.GetResult();
    }

    // Combine
    if (status) {
      int ver_gray_num = 1 << kVerGrayNum;
      int ver_period = kProWidth / (1 << (kVerGrayNum - 1));
      int ver_gray_period = kProWidth / ver_gray_num;
      //this->my_debug_->Show(ver_gray_mat, 0, true, 0.5);
      //this->my_debug_->Show(ver_phase_mat, 0, true, 0.5);
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
      //this->my_debug_->Show(ver_gray_mat, 0, true, 0.5);
      //this->my_debug_->Show(ver_phase_mat, 0, true, 0.5);
      this->cam_mats_[cam_idx].x_pro[frameNum] = ver_gray_mat + ver_phase_mat;
    }
    if (status) {
      int hor_gray_num = 1 << kHorGrayNum;
      int hor_period = kProHeight / (1 << (kHorGrayNum - 1));
      int hor_gray_period = kProHeight / hor_gray_num;
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
      this->cam_mats_[cam_idx].y_pro[frameNum] = hor_gray_mat + hor_phase_mat;
    }
  }
	return status;
}

bool DataCollector::VisualizationForDynamicScene(int total_frame_num) {
	int key;
	bool status = true;
  while (true) {
    for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
      key = this->cam_view_->Show(this->cam_mats_[cam_idx].x_pro[0],
                                  500, true, 0.5);
      if (key == 'y' || key == 'n')
        break;
      key = this->cam_view_->Show(this->cam_mats_[cam_idx].y_pro[0],
                                  500, true, 0.5);
      if (key == 'y' || key == 'n')
        break;
      for (int frm_idx = 0; frm_idx < total_frame_num; frm_idx++) {
        key = this->cam_view_->Show(this->cam_mats_[cam_idx].dyna[frm_idx],
                                    100, false, 0.5);
        if (key == 'y' || key == 'n')
          break;
      }
      if (key == 'y' || key == 'n')
        break;
    }
    if (key == 'y' || key == 'n')
      break;
  }
  if (key == 'y') {
    status = true;
  } else if (key == 'n') {
    status = false;
  }

	return status;
}

bool DataCollector::VisualizationForStaticScene(int frame_idx) {
  int key;
  bool status = true;
  while (true) {
    for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
      key = this->cam_view_->Show(
          this->cam_mats_[cam_idx].x_pro[frame_idx], 500, true, 0.5);
      if (key == 'y' || key == 'n')
        break;
      key = this->cam_view_->Show(
          this->cam_mats_[cam_idx].y_pro[frame_idx], 500, true, 0.5);
      if (key == 'y' || key == 'n')
        break;
      key = this->cam_view_->Show(
          this->cam_mats_[cam_idx].dyna[frame_idx], 500, false, 0.5);
      if (key == 'y' || key == 'n')
        break;
    }
    if (key == 'y' || key == 'n')
      break;
  }
  if (key == 'y') {
    status = true;
  }
  else if (key == 'n') {
    status = false;
  }
  return status;
}

bool DataCollector::Close() {
	bool status = true;
	if (status)	{
		status = this->sensor_manager_->CloseSensor();
	}
	return status;
}

bool DataCollector::StorageDataByGroup(int group_num) {
	if (!this->storage_flag_)
		return true;
	// Set Folder
	stringstream ss;
	ss << group_num << "/";
	string group_folder_path;
	ss >> group_folder_path;
  StorageModule store;

  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    // CamFolderInfo
    stringstream ss_cam;
    ss_cam << "cam_" << cam_idx << "/";
    string cam_folder_path;
    ss_cam >> cam_folder_path;

    // Save dyna mats
    store.SetMatFileName(
        this->save_data_path_ + group_folder_path + cam_folder_path
        + this->dyna_frame_path_, this->dyna_frame_name_, 
        this->dyna_frame_suffix_);
    store.StoreAsImage(this->cam_mats_[cam_idx].dyna, this->max_frame_num_);
    // Save x_pro
    store.SetMatFileName(
        this->save_data_path_ + group_folder_path + cam_folder_path
        + this->pro_frame_path_, this->xpro_frame_name_, 
        this->pro_frame_suffix_);
    store.StoreAsXml(this->cam_mats_[cam_idx].x_pro, 1);
    store.StoreAsText(this->cam_mats_[cam_idx].x_pro, 1);
    // Save y_pro
    store.SetMatFileName(
        this->save_data_path_ + group_folder_path + cam_folder_path
        + this->pro_frame_path_, this->ypro_frame_name_,
        this->pro_frame_suffix_);
    store.StoreAsXml(this->cam_mats_[cam_idx].y_pro, 1);
    store.StoreAsText(this->cam_mats_[cam_idx].y_pro, 1);
  }
	return true;
}

bool DataCollector::StorageDataByFrame(int group_num, int frame_idx) {
  if (!this->storage_flag_)
    return true;
  // Set Folder
  stringstream ss;
  ss << group_num << "/";
  string group_folder_path;
  ss >> group_folder_path;
  StorageModule store;

  for (int cam_idx = 0; cam_idx < kCamDeviceNum; cam_idx++) {
    // CamFolderInfo
    stringstream ss_cam;
    ss_cam << "cam_" << cam_idx << "/";
    string cam_folder_path;
    ss_cam >> cam_folder_path;

    // Save dyna mats
    store.SetMatFileName(
        this->save_data_path_ + group_folder_path + cam_folder_path
        + this->dyna_frame_path_, this->dyna_frame_name_,
        this->dyna_frame_suffix_);
    store.StoreAsImage(&this->cam_mats_[cam_idx].dyna[frame_idx], 1);
    // Save x_pro
    store.SetMatFileName(
        this->save_data_path_ + group_folder_path + cam_folder_path
        + this->pro_frame_path_, this->xpro_frame_name_,
        this->pro_frame_suffix_);
    store.StoreAsXml(&this->cam_mats_[cam_idx].x_pro[frame_idx], 1);
    store.StoreAsText(&this->cam_mats_[cam_idx].x_pro[frame_idx], 1);
    // Save y_pro
    store.SetMatFileName(
        this->save_data_path_ + group_folder_path + cam_folder_path
        + this->pro_frame_path_, this->ypro_frame_name_,
        this->pro_frame_suffix_);
    store.StoreAsXml(&this->cam_mats_[cam_idx].y_pro[frame_idx], 1);
    store.StoreAsText(&this->cam_mats_[cam_idx].y_pro[frame_idx], 1);
  }
  return true;
}
