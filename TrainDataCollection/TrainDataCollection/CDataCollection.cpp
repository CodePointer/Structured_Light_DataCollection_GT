#include "CDataCollection.h"

CDataCollection::CDataCollection()
{
	this->sensor_manager_ = NULL;
	this->gray_mats_ = NULL;
	this->phase_mats_ = NULL;
	this->dyna_mats_ = NULL;
	this->ipro_mats_ = NULL;

	return;
}


CDataCollection::~CDataCollection()
{
	if (this->sensor_manager_ != NULL)
	{
		delete this->sensor_manager_;
		this->sensor_manager_ = NULL;
	}
	if (this->gray_mats_ != NULL)
	{
		delete this->gray_mats_;
		this->gray_mats_ = NULL;
	}
	if (this->phase_mats_ != NULL)
	{
		delete this->phase_mats_;
		this->phase_mats_ = NULL;
	}
	if (this->dyna_mats_ != NULL)
	{
		delete this->dyna_mats_;
		this->dyna_mats_ = NULL;
	}
	if (this->ipro_mats_ != NULL)
	{
		delete this->ipro_mats_;
		this->ipro_mats_ = NULL;
	}

	return;
}


bool CDataCollection::Init()
{
	bool status = true;

	// 类内参数设定
	this->visualize_flag_ = true;
	this->storage_flag_ = true;
	this->max_frame_num_ = COLLECTION_MAX_FRAME;

	// 图案路径与名称
	this->pattern_path_ = "Patterns/";
	this->gray_name_ = "vGray";
	this->gray_suffix_ = ".bmp";
	this->gray_code_name_ = "vGrayCode";
	this->gray_code_suffix_ = ".txt";
	this->phase_name_ = "vPhase";
	this->phase_suffix_ = ".bmp";
	this->dyna_name_ = "randDot";
	this->dyna_suffix_ = ".bmp";
	this->wait_name_ = "wait";
	this->wait_suffix_ = ".bmp";

	// 存储路径与名称
	this->save_data_path_ = "E:/Structured_Light_Data/20170410/";
	this->dyna_frame_path_ = "dyna/";
	this->dyna_frame_name_ = "dyna_mat";
	this->dyna_frame_suffix_ = ".png";
	this->ipro_frame_path_ = "ipro/";
	this->ipro_frame_name_ = "ipro_mat";
	this->ipro_frame_suffix_ = ".png";

	// 初始化传感器
	if (status)
	{
		this->sensor_manager_ = new CSensor;
		status = this->sensor_manager_->InitSensor();
		if (!status)
		{
			ErrorHandling("CDataCollection::Init(), InitSensor failed.");
		}
	}

	// 初始化存储空间
	this->gray_mats_ = new Mat[GRAY_V_NUMDIGIT * 2];
	this->phase_mats_ = new Mat[PHASE_NUMDIGIT];
	this->dyna_mats_ = new Mat[this->max_frame_num_];
	this->ipro_mats_ = new Mat[this->max_frame_num_];

	return status;
}


bool CDataCollection::CollectData()
{
	bool status = true;
	CVisualization myCamera("DebugCamera");

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
		for (int frameIdx = 0; frameIdx < this->max_frame_num_; frameIdx++)
		{
			// 等待采集命令
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
				printf("\tf[%d], Input(<y>, <e>:", frameIdx);
				bool exit_flag = false;
				while (true)
				{
					CamMat = this->sensor_manager_->GetCamPicture();
					int key = myCamera.Show(CamMat, 100, false, 0.5);
					if (key == 'y')
					{
						printf("y\n");
						exit_flag = false;
						break;
					}
					else if (key == 'e')
					{
						printf("e\n");
						exit_flag = true;
						break;
					}
				}
				if (exit_flag)
				{
					break;
				}
			}
			if (status)
			{
				this->sensor_manager_->UnloadPatterns();
			}

			// 采集并解码，保存
			bool flag = this->CollectSingleFrame(frameIdx);
			if (!flag)
			{
				frameIdx--;
			}
			else
			{
				this->StorageData(nowGroupIdx, frameIdx);
			}
		}
	}

	return true;
}


bool CDataCollection::CollectSingleFrame(int frameNum)
{
	bool status = true;

	// 填充gray_mats_
	if (status)
	{
		status = this->sensor_manager_->LoadPatterns(GRAY_V_NUMDIGIT * 2,
			this->pattern_path_,
			this->gray_name_,
			this->gray_suffix_);
	}
	if (status)
	{
		for (int i = 0; i < GRAY_V_NUMDIGIT * 2; i++)
		{
			if (status)
			{
				status = this->sensor_manager_->SetProPicture(i);
			}
			if (status)
			{
				Mat CamMat = this->sensor_manager_->GetCamPicture();
				CamMat.copyTo(this->gray_mats_[i]);
			}
		}
	}
	if (status)
	{
		status = this->sensor_manager_->UnloadPatterns();
	}

	// 填充phase_mats_
	if (status)
	{
		status = this->sensor_manager_->LoadPatterns(PHASE_NUMDIGIT,
			this->pattern_path_,
			this->phase_name_,
			this->phase_suffix_);
	}
	if (status)
	{
		for (int i = 0; i < PHASE_NUMDIGIT; i++)
		{
			if (status)
			{
				status = this->sensor_manager_->SetProPicture(i);
			}
			if (status)
			{
				Mat CamMat = this->sensor_manager_->GetCamPicture();
				CamMat.copyTo(this->phase_mats_[i]);
			}
		}
	}
	if (status)
	{
		status = this->sensor_manager_->UnloadPatterns();
	}

	// 填充dyna_mats_
	if (status)
	{
		status = this->sensor_manager_->LoadPatterns(1,
			this->pattern_path_,
			this->dyna_name_,
			this->dyna_suffix_);
	}
	if (status)
	{
		status = this->sensor_manager_->SetProPicture(0);
	}
	if (status)
	{
		Mat CamMat = this->sensor_manager_->GetCamPicture();
		CamMat.copyTo(this->dyna_mats_[frameNum]);
	}
	if (status)
	{
		status = this->sensor_manager_->UnloadPatterns();
	}

	// gray解码
	CDecodeGray gray_decoder;
	Mat tmp_gray_mat;
	if (status)
	{
		status = gray_decoder.SetNumDigit(GRAY_V_NUMDIGIT, true);
	}
	if (status)
	{
		status = gray_decoder.SetMatFileName(this->pattern_path_,
			this->gray_code_name_ + this->gray_code_suffix_);
	}
	if (status)
	{
		for (int i = 0; i < GRAY_V_NUMDIGIT * 2; i++)
		{
			if (status)
			{
				status = gray_decoder.SetMat(i, this->gray_mats_[i]);
			}
		}
	}
	if (status)
	{
		status = gray_decoder.Decode();
	}
	if (status)
	{
		tmp_gray_mat = gray_decoder.GetResult();
	}

	// phase解码
	CDecodePhase phase_decoder;
	Mat tmp_phase_mat;
	if (status)
	{
		int v_pixPeriod = PROJECTOR_RESLINE / (1 << GRAY_V_NUMDIGIT - 1);
		status = phase_decoder.SetNumMat(PHASE_NUMDIGIT, v_pixPeriod);
	}
	if (status)
	{
		for (int i = 0; i < PHASE_NUMDIGIT; i++)
		{
			if (status)
			{
				status = phase_decoder.SetMat(i, this->phase_mats_[i]);
			}
		}
	}
	if (status)
	{
		status = phase_decoder.Decode();
	}
	if (status)
	{
		tmp_phase_mat = phase_decoder.GetResult();
	}

	// 合并结果
	if (status)
	{
		int vGrayNum = 1 << GRAY_V_NUMDIGIT;
		int v_pixPeriod = PROJECTOR_RESLINE / (1 << GRAY_V_NUMDIGIT - 1);
		int vGrayPeriod = PROJECTOR_RESLINE / vGrayNum;
		for (int h = 0; h < CAMERA_RESROW; h++)
		{
			for (int w = 0; w < CAMERA_RESLINE; w++)
			{
				double grayVal = tmp_gray_mat.at<double>(h, w);
				double phaseVal = tmp_phase_mat.at<double>(h, w);
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
		this->ipro_mats_[frameNum] = tmp_gray_mat + tmp_phase_mat;
	}

	// 确认是否正确采集
	if (status)
	{
		CVisualization myCamera("DebugCamera");
		int key;
		while (true)
		{
			key = myCamera.Show(this->ipro_mats_[frameNum], 500, true, 0.5);
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
			key = myCamera.Show(this->dyna_mats_[frameNum], 500, false, 0.5);
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


bool CDataCollection::Close()
{
	bool status = true;

	if (status)
	{
		status = this->sensor_manager_->CloseSensor();
	}

	return status;
}


bool CDataCollection::StorageData(int groupNum, int frameNum)
{
	if (!this->storage_flag_)
		return true;

	CStorage store;

	// 设定文件夹名
	stringstream ss;
	ss << groupNum;
	string folderName;
	ss >> folderName;
	ss.clear();
	ss << frameNum;
	string frameName;
	ss >> frameName;

	// 保存动态帧信息
	store.SetMatFileName(this->save_data_path_
		+ folderName
		+ "/"
		+ this->dyna_frame_path_,
		this->dyna_frame_name_ + frameName,
		this->dyna_frame_suffix_);
	store.Store(&this->dyna_mats_[frameNum], 1);
	
	// 保存对应的真值信息
	/*store.SetMatFileName(this->save_data_path_
		+ folderName
		+ "/"
		+ this->ipro_frame_path_,
		this->ipro_frame_name_ + frameName,
		this->ipro_frame_suffix_);
	store.Store(&this->ipro_mats_[frameNum], 1);*/
	FileStorage fs(this->save_data_path_
		+ folderName
		+ "/"
		+ this->ipro_frame_path_
		+ this->ipro_frame_name_
		+ frameName
		+ ".xml", FileStorage::WRITE);
	fs << "ipro_mat" << this->ipro_mats_[frameNum];
	fs.release();

	return true;
}