#ifndef _DATACOLLECTOR_H_
#define _DATACOLLECTOR_H_

#include <opencv2/opencv.hpp>
#include <string>
#include <sstream>
#include "sensor_manager.h"
#include "gray_decoder.h"
#include "phase_decoder.h"
#include "static_para.h"
#include "global_fun.h"
#include "visual_mod.h"
#include "storage_mod.h"

using namespace std;
using namespace cv;

// Data collector for dynamic data collection.
// Usage: Init£¬Collect£¬Close
class DataCollector {
private:
  // Sensor part
	SensorManager * sensor_manager_;
  // pattern paths
  string pattern_path_;
  string vgray_name_;
  string hgray_name_;
  string gray_suffix_;
  string vgray_code_name_;
  string hgray_code_name_;
  string gray_code_suffix_;
  string vphase_name_;
  string hphase_name_;
  string phase_suffix_;
  string dyna_name_;
  string dyna_suffix_;
  string wait_name_;
  string wait_suffix_;
  // storage data
  CamMatSet * cam_mats_;
  Mat cam_mask_;
	// storage paths
	string save_data_path_;
	string dyna_frame_path_;
	string dyna_frame_name_;
	string dyna_frame_suffix_;
	string pro_frame_path_;
	string xpro_frame_name_;
	string ypro_frame_name_;
	string pro_frame_suffix_;
	// visualization
	VisualModule * cam_view_;
  VisualModule * res_view_;
	// flag parameters
	bool visualize_flag_;
	bool storage_flag_;
	int max_frame_num_;

	bool StorageDataByGroup(int group_num);
  bool StorageDataByFrame(int group_num, int frame_idx);
	int GetInputSignal();
	bool CollectStaticFrame(int frameNum);
	bool CollectDynamicFrame();
	bool DecodeSingleFrame(int frameNum);
	bool VisualizationForDynamicScene(int total_frame_num);
  bool VisualizationForStaticScene(int frame_idx);

public:
	DataCollector();
	~DataCollector();
	bool Init();
	bool CollectDynaData();
  bool CollectStatData();
	bool Close();
};

#endif