#include "CDecodePhase.h"

PhaseDecoder::PhaseDecoder()
{
	this->image_num_ = 0;
	this->pix_period_ = 16;
	this->img_height_ = CAMERA_RESROW;
	this->img_width_ = CAMERA_RESLINE;
	this->grey_mats_ = NULL;
	this->vis_mod_ = NULL;
}

PhaseDecoder::~PhaseDecoder()
{
	this->DeleteSpace();
}

// 为输入的矩阵申请空间
bool PhaseDecoder::AllocateSpace()
{
	// 判断参数是否合法
	if (this->image_num_ <= 0)
		return false;

	// 申请空间并初始化
	this->grey_mats_ = new Mat[this->image_num_];

	return true;
}

// 删除所有空间
bool PhaseDecoder::DeleteSpace()
{
	if (this->grey_mats_ != NULL)
	{
		delete[](this->grey_mats_);
		this->grey_mats_ = NULL;
	}
	if (this->vis_mod_ != NULL)
	{
		delete(this->vis_mod_);
		this->vis_mod_ = NULL;
	}
	return true;
}

// 根据灰度图统计结果
bool PhaseDecoder::CountResult()
{
	this->res_mat_.create(this->img_height_, this->img_width_, CV_64FC1);
	Mat SinValue0;
	Mat SinValue1;

	for (int i = 0; i < this->img_height_; i++)
	{
		for (int j = 0; j < this->img_width_; j++)
		{
			// 先获取该点灰度值
			float greyValue0 = this->grey_mats_[0].at<uchar>(i, j);	// greyValue0 = (sin(x) + 1) * 127;
			float greyValue1 = this->grey_mats_[1].at<uchar>(i, j);	// greyValue1 = (sin(x + CV_PI / 2) + 1) * 127
			float greyValue2 = this->grey_mats_[2].at<uchar>(i, j);	// greyValue2 = (sin(x + CV_PI) + 1) * 127
			float greyValue3 = this->grey_mats_[3].at<uchar>(i, j);	// greyValue3 = (sin(x + 3 * CV_PI / 2) + 1) * 127
			// 将灰度值转变为sin值
			float sinValue = (greyValue0 - greyValue2) / 2;
			float cosValue = (greyValue1 - greyValue3) / 2;
			// 计算x
			float x = cvFastArctan(sinValue, cosValue);			
			// 计算偏移量
			float pix = (x) / (360)*(double)(this->pix_period_);
			pix += 0.5;
			if (pix > this->pix_period_)
			{
				pix -= this->pix_period_;
			}
			this->res_mat_.at<double>(i, j) = (double)pix;
		}
	}

	return true;
}

// 解码
bool PhaseDecoder::Decode()
{
	// 根据灰度图恢复坐标
	if (!this->CountResult())
	{
		ErrorHandling("PhaseDecoder.Decode()->CountResult fault");
		return false;
	}

	// 可视化
	this->Visualize();

	return true;
}

// 获取解码结果
Mat PhaseDecoder::GetResult()
{
	Mat result;
	this->res_mat_.copyTo(result);
	return result;
}

// 输入相应灰度图
bool PhaseDecoder::SetMat(int num, cv::Mat pic)
{
	if (this->grey_mats_ == NULL)
	{
		ErrorHandling("PhaseDecoder.SetMat->grePicture Space is not allocated.");
		return false;
	}
	pic.copyTo(this->grey_mats_[num]);
	return true;
}

// 设置参数
bool PhaseDecoder::SetNumDigit(int numMat, int pixperiod)
{
	// 判断参数是否合法
	if ((numMat <= 0))
		return false;

	// 传参
	this->image_num_ = numMat;
	this->pix_period_ = pixperiod;

	// 分配空间
	if ((this->grey_mats_ != NULL))
	{
		this->DeleteSpace();
	}
	this->AllocateSpace();

	return true;
}

// 在计算机上显示图片
bool PhaseDecoder::Visualize()
{
	if (VISUAL_DEBUG)
	{
		// 如果类不存在，则创建一个
		if (this->vis_mod_ == NULL)
		{
			this->vis_mod_ = new VisualModule("PhaseDecoder");
		}

		// 显示灰度图像
		for (int i = 0; i < this->image_num_; i++)
		{
			this->vis_mod_->Show(this->grey_mats_[i], 300);
		}

		// 显示最终解码结果（归一化）
		this->vis_mod_->Show(this->res_mat_, 1000, true, 0.5);
	}
	return true;
}