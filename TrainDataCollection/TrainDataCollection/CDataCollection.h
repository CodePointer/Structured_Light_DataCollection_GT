#ifndef _CDATACOLLECTION_H_
#define _CDATACOLLECTION_H_

#include <opencv2/opencv.hpp>
#include "CSensor.h"
#include "CDecodeGray.h"
#include "CDecodePhase.h"
#include "StaticParameters.h"
#include "GlobalFunction.h"
#include "CVisualization.h"
#include "CStorage.h"

// ���ݲɼ��ࡣ������������ݲɼ��������²������
// �ⲿ����ֻ��������Ҫ���ݼ��ɡ�
// ���ã�Init��Collect��Close
class DataCollector
{
private:
	SensorManager * sensor_manager_;

	bool flag_ground_truth_;

	// ���ڴ洢����
	Mat * vgray_mats_;
	Mat * hgray_mats_;
	Mat * vphase_mats_;
	Mat * hphase_mats_;
	Mat * dyna_mats_;
	Mat * flow_mats_;
	Mat * xpro_mats_;
	Mat * ypro_mats_;

	// ͼ��·��������
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
	string flow_name_;
	string flow_suffix_;
	string wait_name_;
	string wait_suffix_;

	// �洢·��������
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

	// ���ڲ���
	bool visualize_flag_;
	bool storage_flag_;
	int max_frame_num_;

	bool StorageData(int groupNum, int frameNum);
	int GetInputSignal(int frameNum);
	bool CollectStaticFrame(int frameNum);
	bool DecodeSingleFrame(int frameNum);
	bool VisualizationForDynamicScene(int total_frame_num);

public:
	DataCollector();
	~DataCollector();
	bool Init();
	bool CollectData();
	bool Close();
};


#endif