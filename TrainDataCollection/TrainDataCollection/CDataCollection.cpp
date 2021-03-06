#include "CDataCollection.h"

DataCollector::DataCollector()
{
	this->sensor_manager_ = NULL;
	this->vgray_mats_ = NULL;
	this->vphase_mats_ = NULL;
	this->dyna_mats_ = NULL;
	this->flow_mats_ = NULL;
	this->xpro_mats_ = NULL;
	this->cam_view_ = NULL;

	return;
}


DataCollector::~DataCollector()
{
	if (this->sensor_manager_ != NULL)
	{
		delete this->sensor_manager_;
		this->sensor_manager_ = NULL;
	}
	if (this->vgray_mats_ != NULL)
	{
		delete[]this->vgray_mats_;
		this->vgray_mats_ = NULL;
	}
	if (this->hgray_mats_ != NULL)
	{
		delete[]this->hgray_mats_;
		this->hgray_mats_ = NULL;
	}
	if (this->vphase_mats_ != NULL)
	{
		delete[]this->vphase_mats_;
		this->vphase_mats_ = NULL;
	}
	if (this->hphase_mats_ != NULL)
	{
		delete[]this->hphase_mats_;
		this->hphase_mats_ = NULL;
	}
	if (this->dyna_mats_ != NULL)
	{
		delete[]this->dyna_mats_;
		this->dyna_mats_ = NULL;
	}
	if (this->flow_mats_ != NULL)
	{
		delete[]this->flow_mats_;
		this->flow_mats_ = NULL;
	}
	if (this->xpro_mats_ != NULL)
	{
		delete[]this->xpro_mats_;
		this->xpro_mats_ = NULL;
	}
	if (this->ypro_mats_ != NULL)
	{
		delete[]this->ypro_mats_;
		this->ypro_mats_ = NULL;
	}
	if (this->cam_view_ != NULL)
	{
		delete this->cam_view_;
		this->cam_view_ = NULL;
	}

	return;
}


bool DataCollector::Init()
{
	bool status = true;

	// 手动设置
	this->flag_ground_truth_ = true;
	this->max_frame_num_ = 100;

	// 类内参数设定
	this->visualize_flag_ = true;
	this->storage_flag_ = true;

	// 图案路径与名称
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
	this->flow_name_ = "pattern_optflow";
	this->flow_suffix_ = ".png";
	this->wait_name_ = "part_pattern_2size4color";
	this->wait_suffix_ = ".png";

	// 存储路径与名称
	this->save_data_path_ = "E:/Structured_Light_Data/20171104/";
	this->dyna_frame_path_ = "dyna/";
	this->dyna_frame_name_ = "dyna_mat";
	this->dyna_frame_suffix_ = ".png";
	this->pro_frame_path_ = "pro/";
	this->xpro_frame_name_ = "xpro_mat";
	this->ypro_frame_name_ = "ypro_mat";
	this->pro_frame_suffix_ = ".png";

	// Create folder
	StorageModule storage;
	storage.CreateFolder(this->save_data_path_);
	storage.CreateFolder(this->save_data_path_ + "1/" + this->dyna_frame_path_);
	storage.CreateFolder(this->save_data_path_ + "1/" + this->pro_frame_path_);

	// 初始化传感器
	if (status)
	{
		this->sensor_manager_ = new SensorManager;
		status = this->sensor_manager_->InitSensor();
		if (!status)
		{
			ErrorHandling("DataCollector::Init(), InitSensor failed.");
		}
	}

	// 初始化存储空间
	this->vgray_mats_ = new Mat[GRAY_V_NUMDIGIT * 2];
	this->hgray_mats_ = new Mat[GRAY_H_NUMDIGIT * 2];
	this->vphase_mats_ = new Mat[PHASE_NUMDIGIT];
	this->hphase_mats_ = new Mat[PHASE_NUMDIGIT];
	this->dyna_mats_ = new Mat[this->max_frame_num_];
	this->flow_mats_ = new Mat[this->max_frame_num_];
	this->xpro_mats_ = new Mat[this->max_frame_num_];
	this->ypro_mats_ = new Mat[this->max_frame_num_];
	this->cam_view_ = new VisualModule("Debug");

	return status;
}


bool DataCollector::CollectData()
{
	bool status = true;

	// 循环采集数据
	int nowGroupIdx = 0;
	int maxGroupNum = 5;
	cout << "Begin Collection." << endl;

	while (nowGroupIdx < maxGroupNum)
	{
		// 记录当前采集的组数
		nowGroupIdx++;
		printf("Now group: %d\n", nowGroupIdx);

		// 采集数据
		int frameIdx = 0;
		for (frameIdx = 0; frameIdx < this->max_frame_num_; frameIdx++)
		{
			// 等待Signal
			if (status)
			{
				int info = this->GetInputSignal(frameIdx);
				if (info == 1)
				{
					continue;
				}
				else if (info == 2)
				{
					break;
				}
			}
			

			// 采集并解码，保存
			if (status)
			{
				status = this->CollectStaticFrame(frameIdx);
			}
			if (status)
			{
				status = this->DecodeSingleFrame(frameIdx);
			}
			if (!status)
			{
				frameIdx--;
				status = true;
			}
			else
			{
				status = this->StorageData(nowGroupIdx, frameIdx);
			}
		}
	}

	return status;
}


int DataCollector::GetInputSignal(int frameNum)
{
	// return:
	//     0: Continue
	//     1: End collection of this group
	//     2: End collection of program
	//     3: Error

	int info = 0;

	// 等待采集命令
	if ((frameNum == 0) || (this->flag_ground_truth_))
	{
		VisualModule myCamera("DebugCamera");
		bool status = true;
		if (status)
		{
			status = this->sensor_manager_->LoadPatterns(1,
				this->pattern_path_,
				this->wait_name_,
				this->wait_suffix_);
		}
		if (status)
		{
			status = this->sensor_manager_->SetProPicture(0);
		}
		if (status)
		{
			Mat CamMat;
			printf("\tf[%d], Input(<y>, <e>:", frameNum);
			while (true)
			{
				CamMat = this->sensor_manager_->GetCamPicture();
				Mat LittleMat = CamMat(Range(502, 523), Range(430, 451));
				int key1 = this->cam_view_->Show(LittleMat, 100, false, 20);
				int key2 = myCamera.Show(CamMat, 100, false, 0.5);
				if ((key1 == 'y') || (key2 == 'y'))
				{
					printf("y\n");
					info = 0;
					break;
				}
				else if ((key1 == 'e') || (key2 == 'e'))
				{
					printf("e\n");
					info = 1;
					break;
				}
			}
		}
		if (status)
		{
			this->sensor_manager_->UnloadPatterns();
		}
	}

	return info;
}


bool DataCollector::CollectStaticFrame(int frameNum)
{
	bool status = true;

	int kMultiCollectNum = 5;
	Mat tmp_mul_collect;
	Mat temp_total_mat;
	Mat temp_mat;
	temp_total_mat.create(CAMERA_RESROW, CAMERA_RESLINE, CV_64FC1);

	// fill in dyna_mats_
	if (status)	{
		status = this->sensor_manager_->LoadPatterns(1,
			this->pattern_path_,
			this->dyna_name_,
			this->dyna_suffix_);
	}
	if (status)	{
		status = this->sensor_manager_->SetProPicture(0);
	}
	if (status)	{
		Mat CamMat = this->sensor_manager_->GetCamPicture();
		CamMat.copyTo(this->dyna_mats_[frameNum]);
	}
	if (status)	{
		status = this->sensor_manager_->UnloadPatterns();
	}

	// fill in flow_mats_
	if (status) {
		status = this->sensor_manager_->LoadPatterns(1,
			this->pattern_path_,
			this->flow_name_,
			this->flow_suffix_);
	}
	if (status) {
		status = this->sensor_manager_->SetProPicture(0);
	}
	if (status) {
		Mat CamMat = this->sensor_manager_->GetCamPicture();
		CamMat.copyTo(this->flow_mats_[frameNum]);
	}
	if (status) {
		status = this->sensor_manager_->UnloadPatterns();
	}

	// fill in vgray_mats_
	if (status)	{
		status = this->sensor_manager_->LoadPatterns(GRAY_V_NUMDIGIT * 2,
			this->pattern_path_,
			this->vgray_name_,
			this->gray_suffix_);
	}
	if (status)	{
		for (int i = 0; i < GRAY_V_NUMDIGIT * 2; i++) {
			if (status)	{
				status = this->sensor_manager_->SetProPicture(i);
			}
			if (status)	{
				temp_total_mat.setTo(0);
				for (int k = 0; k < kMultiCollectNum; k++) {
					tmp_mul_collect = this->sensor_manager_->GetCamPicture();
					tmp_mul_collect.convertTo(temp_mat, CV_64FC1);
					temp_total_mat += temp_mat / kMultiCollectNum;
				}
				temp_total_mat.convertTo(this->vgray_mats_[i], CV_8UC1);
			}
		}
	}
	if (status)	{
		status = this->sensor_manager_->UnloadPatterns();
	}

	// fill in vphase_mats_
	if (status)	{
		status = this->sensor_manager_->LoadPatterns(PHASE_NUMDIGIT,
			this->pattern_path_,
			this->vphase_name_,
			this->phase_suffix_);
	}
	if (status)	{
		for (int i = 0; i < PHASE_NUMDIGIT; i++) {
			if (status)	{
				status = this->sensor_manager_->SetProPicture(i);
			}
			if (status)	{
				temp_total_mat.setTo(0);
				for (int k = 0; k < kMultiCollectNum; k++) {
					tmp_mul_collect = this->sensor_manager_->GetCamPicture();
					tmp_mul_collect.convertTo(temp_mat, CV_64FC1);
					temp_total_mat += temp_mat / kMultiCollectNum;
				}
				temp_total_mat.convertTo(this->vphase_mats_[i], CV_8UC1);
			}
		}
	}
	if (status)	{
		status = this->sensor_manager_->UnloadPatterns();
	}

	// fill in hgray_mats_
	if (status)	{
		status = this->sensor_manager_->LoadPatterns(GRAY_H_NUMDIGIT * 2,
			this->pattern_path_,
			this->hgray_name_,
			this->gray_suffix_);
	}
	if (status)	{
		for (int i = 0; i < GRAY_H_NUMDIGIT * 2; i++) {
			if (status) {
				status = this->sensor_manager_->SetProPicture(i);
			}
			if (status)	{
				temp_total_mat.setTo(0);
				for (int k = 0; k < kMultiCollectNum; k++) {
					tmp_mul_collect = this->sensor_manager_->GetCamPicture();
					tmp_mul_collect.convertTo(temp_mat, CV_64FC1);
					temp_total_mat += temp_mat / kMultiCollectNum;
				}
				temp_total_mat.convertTo(this->hgray_mats_[i], CV_8UC1);
			}
		}
	}
	if (status)	{
		status = this->sensor_manager_->UnloadPatterns();
	}

	// fill in hphase_mats_
	if (status)	{
		status = this->sensor_manager_->LoadPatterns(PHASE_NUMDIGIT,
			this->pattern_path_,
			this->hphase_name_,
			this->phase_suffix_);
	}
	if (status)	{
		for (int i = 0; i < PHASE_NUMDIGIT; i++) {
			if (status)	{
				status = this->sensor_manager_->SetProPicture(i);
			}
			if (status)	{
				temp_total_mat.setTo(0);
				for (int k = 0; k < kMultiCollectNum; k++) {
					tmp_mul_collect = this->sensor_manager_->GetCamPicture();
					tmp_mul_collect.convertTo(temp_mat, CV_64FC1);
					temp_total_mat += temp_mat / kMultiCollectNum;
				}
				temp_total_mat.convertTo(this->hphase_mats_[i], CV_8UC1);
			}
		}
	}
	if (status)	{
		status = this->sensor_manager_->UnloadPatterns();
	}

	return status;
}


bool DataCollector::DecodeSingleFrame(int frameNum)
{
	bool status = true;

	if ((frameNum > 0) && (!this->flag_ground_truth_)) {
		return status;
	}

	// vgray decode
	GrayDecoder vgray_decoder;
	Mat tmp_gray_mat;
	if (status)
	{
		status = vgray_decoder.SetNumDigit(GRAY_V_NUMDIGIT, true);
	}
	if (status)
	{
		status = vgray_decoder.SetMatFileName(this->pattern_path_,
			this->vgray_code_name_ + this->gray_code_suffix_);
	}
	if (status)
	{
		for (int i = 0; i < GRAY_V_NUMDIGIT * 2; i++)
		{
			if (status)
			{
				status = vgray_decoder.SetMat(i, this->vgray_mats_[i]);
			}
		}
	}
	if (status)
	{
		status = vgray_decoder.Decode();
	}
	if (status)
	{
		tmp_gray_mat = vgray_decoder.GetResult();
	}

	// vphase decode
	PhaseDecoder vphase_decoder;
	Mat tmp_phase_mat;
	if (status)
	{
		int v_pixPeriod = PROJECTOR_RESLINE / (1 << GRAY_V_NUMDIGIT - 1);
		status = vphase_decoder.SetNumDigit(PHASE_NUMDIGIT, v_pixPeriod);
	}
	if (status)
	{
		for (int i = 0; i < PHASE_NUMDIGIT; i++)
		{
			if (status)
			{
				status = vphase_decoder.SetMat(i, this->vphase_mats_[i]);
			}
		}
	}
	if (status)
	{
		status = vphase_decoder.Decode();
	}
	if (status)
	{
		tmp_phase_mat = vphase_decoder.GetResult();
	}

	// 合并结果至ipro
	if (status)
	{
		int vGrayNum = 1 << GRAY_V_NUMDIGIT;
		int v_pixPeriod = PROJECTOR_RESLINE / (1 << GRAY_V_NUMDIGIT - 1);
		int vGrayPeriod = PROJECTOR_RESLINE / vGrayNum;
		//this->my_debug_->Show(tmp_gray_mat, 0, true, 0.5);
		//this->my_debug_->Show(tmp_phase_mat, 0, true, 0.5);
		for (int h = 0; h < CAMERA_RESROW; h++)
		{
			for (int w = 0; w < CAMERA_RESLINE; w++)
			{
				double grayVal = tmp_gray_mat.at<double>(h, w);
				double phaseVal = tmp_phase_mat.at<double>(h, w);
				if (grayVal < 0)
				{
					tmp_phase_mat.at<double>(h, w) = 0;
					continue;
				}
				if ((int)(grayVal / vGrayPeriod) % 2 == 0)
				{
					if (phaseVal >(double)v_pixPeriod * 0.75)
					{
						tmp_phase_mat.at<double>(h, w) = phaseVal - v_pixPeriod;
					}
				}
				else
				{
					if (phaseVal < (double)v_pixPeriod * 0.25)
					{
						tmp_phase_mat.at<double>(h, w) = phaseVal + v_pixPeriod;
					}
					tmp_phase_mat.at<double>(h, w) = tmp_phase_mat.at<double>(h, w) - 0.5 * v_pixPeriod;
				}
			}
		}
		/*this->my_debug_->Show(tmp_gray_mat, 0, true, 0.5);
		this->my_debug_->Show(tmp_phase_mat, 0, true, 0.5);*/
		this->xpro_mats_[frameNum] = tmp_gray_mat + tmp_phase_mat;
	}

	// hgray解码
	GrayDecoder hgray_decoder;
	if (status)
	{
		status = hgray_decoder.SetNumDigit(GRAY_H_NUMDIGIT, false);
	}
	if (status)
	{
		status = hgray_decoder.SetMatFileName(this->pattern_path_,
			this->hgray_code_name_ + this->gray_code_suffix_);
	}
	if (status)
	{
		for (int i = 0; i < GRAY_H_NUMDIGIT * 2; i++)
		{
			if (status)
			{
				status = hgray_decoder.SetMat(i, this->hgray_mats_[i]);
			}
		}
	}
	if (status)
	{
		status = hgray_decoder.Decode();
	}
	if (status)
	{
		tmp_gray_mat = hgray_decoder.GetResult();
	}

	// hphase解码
	PhaseDecoder hphase_decoder;
	if (status)
	{
		int h_pixPeriod = PROJECTOR_RESROW / (1 << GRAY_H_NUMDIGIT - 1);
		status = hphase_decoder.SetNumDigit(PHASE_NUMDIGIT, h_pixPeriod);
	}
	if (status)
	{
		for (int i = 0; i < PHASE_NUMDIGIT; i++)
		{
			if (status)
			{
				status = hphase_decoder.SetMat(i, this->hphase_mats_[i]);
			}
		}
	}
	if (status)
	{
		status = hphase_decoder.Decode();
	}
	if (status)
	{
		tmp_phase_mat = hphase_decoder.GetResult();
	}

	// 合并结果至jpro
	if (status)
	{
		int hGrayNum = 1 << GRAY_H_NUMDIGIT;
		int h_pixPeriod = PROJECTOR_RESROW / (1 << GRAY_H_NUMDIGIT - 1);
		int hGrayPeriod = PROJECTOR_RESROW / hGrayNum;
		/*this->my_debug_->Show(tmp_gray_mat, 0, true, 0.5);
		this->my_debug_->Show(tmp_phase_mat, 0, true, 0.5);*/

		// Save
		/*FileStorage fs("1.xml", FileStorage::WRITE);
		fs << "gray_mat" << tmp_gray_mat;
		fs << "phase_mat" << tmp_phase_mat;
		fs.release();*/

		for (int h = 0; h < CAMERA_RESROW; h++)
		{
			for (int w = 0; w < CAMERA_RESLINE; w++)
			{
				double grayVal = tmp_gray_mat.at<double>(h, w);
				double phaseVal = tmp_phase_mat.at<double>(h, w);
				if (grayVal < 0)
				{
					tmp_phase_mat.at<double>(h, w) = 0;
					continue;
				}
				if ((int)(grayVal / hGrayPeriod) % 2 == 0)
				{
					if (phaseVal >(double)h_pixPeriod * 0.75)
					{
						tmp_phase_mat.at<double>(h, w) = phaseVal - h_pixPeriod;
					}
				}
				else
				{
					if (phaseVal < (double)h_pixPeriod * 0.25)
					{
						tmp_phase_mat.at<double>(h, w) = phaseVal + h_pixPeriod;
					}
					tmp_phase_mat.at<double>(h, w) = tmp_phase_mat.at<double>(h, w) - 0.5 * h_pixPeriod;
				}
			}
		}
		/*this->my_debug_->Show(tmp_gray_mat, 0, true, 0.5);
		this->my_debug_->Show(tmp_phase_mat, 0, true, 0.5);*/
		this->ypro_mats_[frameNum] = tmp_gray_mat + tmp_phase_mat;
		//this->my_debug_->Show(this->jpro_mats_[frameNum], 0, true, 0.5);
	}

	// 确认是否正确采集
	if (status)
	{
		int key;
		while (true)
		{
			key = this->cam_view_->Show(this->xpro_mats_[frameNum], 500, true, 0.5);
			if (key == 'y')
			{
				status = true;
				break;
			}
			else if (key == 'n')
			{
				status = false;
				break;
			}
			key = this->cam_view_->Show(this->ypro_mats_[frameNum], 500, true, 0.5);
			if (key == 'y')
			{
				status = true;
				break;
			}
			else if (key == 'n')
			{
				status = false;
				break;
			}
			key = this->cam_view_->Show(this->dyna_mats_[frameNum], 500, false, 0.5);
			if (key == 'y')
			{
				status = true;
				break;
			}
			else if (key == 'n')
			{
				status = false;
				break;
			}
		}
	}

	return status;
}


bool DataCollector::VisualizationForDynamicScene(int total_frame_num)
{
	int key;
	bool status = true;
	while (true) {
		key = this->cam_view_->Show(this->xpro_mats_[0], 500, true, 0.5);
		if (key == 'y')	{
			status = true;
			break;
		}
		else if (key == 'n') {
			status = false;
			break;
		}
		key = this->cam_view_->Show(this->ypro_mats_[0], 500, true, 0.5);
		if (key == 'y') {
			status = true;
			break;
		}
		else if (key == 'n') {
			status = false;
			break;
		}
		for (int frame_idx = 0; frame_idx < total_frame_num; frame_idx++) {
			key = this->cam_view_->Show(this->dyna_mats_[frame_idx], 100, false, 0.5);
			if (key == 'y') {
				status = true;
				break;
			}
			else if (key == 'n') {
				status = false;
				break;
			}
		}
	}

	return status;
}


bool DataCollector::Close()
{
	bool status = true;

	if (status)
	{
		status = this->sensor_manager_->CloseSensor();
	}

	return status;
}


bool DataCollector::StorageData(int groupNum, int frameNum)
{
	if (!this->storage_flag_)
		return true;

	StorageModule store;

	// 设定文件夹名
	stringstream ss;
	ss << groupNum;
	string folderName;
	ss >> folderName;
	ss.clear();
	ss << frameNum;
	string frameName;
	ss >> frameName;

	// save dyna_mat
	store.SetMatFileName(this->save_data_path_
		+ folderName
		+ "/"
		+ this->dyna_frame_path_,
		this->dyna_name_ + frameName,
		this->dyna_frame_suffix_);
	store.StoreAsImage(&this->dyna_mats_[frameNum], 1);

	// save flow_mat
	store.SetMatFileName(this->save_data_path_
		+ folderName
		+ "/"
		+ this->pro_frame_path_,
		this->flow_name_ + frameName,
		this->dyna_frame_suffix_);
	store.StoreAsImage(&this->flow_mats_[frameNum], 1);
	
	// 保存对应的真值信息
	/*store.SetMatFileName(this->save_data_path_
		+ folderName
		+ "/"
		+ this->ipro_frame_path_,
		this->ipro_frame_name_ + frameName,
		this->ipro_frame_suffix_);
	store.Store(&this->ipro_mats_[frameNum], 1);*/
	if (frameNum >= 0)
	{
		/*store.SetMatFileName(this->save_data_path_
			+ folderName
			+ "/"
			+ this->pro_frame_path_,
			this->ipro_frame_name_ + frameName,
			this->ipro_frame_suffix_);*/
		FileStorage fs_i(this->save_data_path_
			+ folderName
			+ "/"
			+ this->pro_frame_path_
			+ this->xpro_frame_name_
			+ frameName
			+ ".xml", FileStorage::WRITE);
		fs_i << "xpro_mat" << this->xpro_mats_[frameNum];
		fs_i.release();
		FileStorage fs_j(this->save_data_path_
			+ folderName
			+ "/"
			+ this->pro_frame_path_
			+ this->ypro_frame_name_
			+ frameName
			+ ".xml", FileStorage::WRITE);
		fs_j << "ypro_mat" << this->ypro_mats_[frameNum];
		fs_j.release();
	}

	return true;
}


