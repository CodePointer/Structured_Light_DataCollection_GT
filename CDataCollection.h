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
class CDataCollection
{
private:
	CSensor * sensor_manager_;

	// ���ڴ洢����
	Mat * gray_mats_;
	Mat * phase_mats_;
	Mat * dyna_mats_;
	Mat * ipro_mats_;

	// ͼ��·��������
	string pattern_path_;
	string gray_name_;
	string gray_suffix_;
	string gray_code_name_;
	string gray_code_suffix_;
	string phase_name_;
	string phase_suffix_;
	string wait_name_;
	string wait_suffix_;

	// �洢·��������
	string save_data_path_;
	string dyna_frame_path_;
	string dyna_frame_name_;
	string dyna_frame_suffix_;
	string ipro_frame_path_;
	string ipro_frame_name_;
	string ipro_frame_suffix_;


	// ���ڲ���
	bool visualize_flag_;
	bool storage_flag_;
	int max_frame_num_;

	bool StorageData(int groupNum, int frameNum);
	bool CollectSingleFrame(int frameNum);

public:
	CDataCollection();
	~CDataCollection();
	bool Init();
	bool CollectData();
	bool Close();
};


#endif