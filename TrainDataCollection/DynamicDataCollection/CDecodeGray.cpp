#include "CDecodeGray.h"

// 构造函数。设定一些默认设置。
GrayDecoder::GrayDecoder()
{
	this->num_digit_ = 0;
	this->code_size_ = 0;
	this->gray2bin_ = NULL;
	this->code_file_name_ = "";
	this->img_height_ = CAMERA_RESROW;
	this->img_width_ = CAMERA_RESLINE;
	this->grey_mats_ = NULL;
	this->bin_mats_ = NULL;
	this->vis_mod_ = NULL;
}

// 析构函数。确保释放空间。
GrayDecoder::~GrayDecoder()
{
	this->ReleaseSpace();
}

// 输入相应灰度图
bool GrayDecoder::SetMat(int num, Mat pic)
{
	if (this->grey_mats_ == NULL)
	{
		ErrorHandling("GrayDecoder.SetMat->grePicture Space is not allocated.");
		return false;
	}
	pic.copyTo(this->grey_mats_[num]);
	return true;
}

// 设置格雷码位数
bool GrayDecoder::SetNumDigit(int numDigit, bool ver)
{
	// 首先判断参数是否合法
	if ((numDigit <= 0) || (numDigit > 16))
		return false;
	
	// 传参
	this->num_digit_ = numDigit;
	this->code_size_ = 1 << (this->num_digit_);
	this->is_vertical_ = ver;

	// 分配相应空间
	if ((this->gray2bin_ != NULL) || (this->grey_mats_ != NULL) || (this->bin_mats_ != NULL))
		this->ReleaseSpace();
	this->AllocateSpace();

	return true;
}

// 设置存储格雷码的文件名
bool GrayDecoder::SetMatFileName(std::string codeFilePath,
	std::string codeFileName)
{
	this->code_file_path_ = codeFilePath;
	this->code_file_name_ = codeFileName;
	return true;
}

// 为输入的矩阵、short类型内容申请空间
bool GrayDecoder::AllocateSpace()
{
	using namespace cv;

	// 判断参数是否合法
	if ((this->num_digit_ <= 0) || (this->num_digit_ > 16))
		return false;

	// 申请空间并初始化
	this->gray2bin_ = new short[this->code_size_];
	this->grey_mats_ = new Mat[this->num_digit_ * 2];
	this->bin_mats_ = new Mat[this->num_digit_];

	return true;
}

// 删除所有空间
bool GrayDecoder::ReleaseSpace()
{
	if (this->gray2bin_ != NULL)
	{
		delete[](this->gray2bin_);
		this->gray2bin_ = NULL;
	}
	if (this->grey_mats_ != NULL)
	{
		delete[](this->grey_mats_);
		this->grey_mats_ = NULL;
	}
	if (this->bin_mats_ != NULL)
	{
		delete[](this->bin_mats_);
		this->bin_mats_ = NULL;
	}
	if (this->vis_mod_ != NULL)
	{
		delete(this->vis_mod_);
		this->vis_mod_ = NULL;
	}
	return true;
}

// 解码。
bool GrayDecoder::Decode()
{
	using namespace cv;

	// 读入解码文件
	std::ifstream codeFile;
	codeFile.open(this->code_file_path_ + this->code_file_name_, std::ios::in);
	if (!codeFile)
	{
		ErrorHandling("Gray Decode->Open file error.");
		return false;
	}
	for (int i = 0; i < this->code_size_; i++)
	{
		int binCode, grayCode;
		codeFile >> binCode >> grayCode;
		this->gray2bin_[grayCode] = binCode;
	}

	// 分别处理所有的灰度图，将其转换为二值内容
	if (!this->Grey2Bin())
		return false;

	// 根据二值图恢复坐标
	if (!this->CountResult())
		return false;

	// 可视化部分
	this->Visualize();

	return true;
}

// 获取解码结果
cv::Mat GrayDecoder::GetResult()
{
	Mat result;
	this->res_mat_.copyTo(result);
	return result;
}

// 将灰度图加工为二值图，以便进一步处理
bool GrayDecoder::Grey2Bin()
{
	using namespace cv;

	// 填充每一张binPicture
	for (int binIdx = 0; binIdx < this->num_digit_; binIdx++)
	{
		// 设定阈值
		Mat tempMat1, tempMat2;
		tempMat1 = this->grey_mats_[binIdx * 2];
		tempMat2 = this->grey_mats_[binIdx * 2 + 1];
		uchar threshold = 5;

		// 获取两张图片的差
		Mat tempMat;
		tempMat = this->grey_mats_[binIdx * 2] - this->grey_mats_[binIdx * 2 + 1];
		
		// 根据相减值进行二值化
		this->bin_mats_[binIdx].create(Size(this->img_width_, this->img_height_), CV_8UC1);
		for (int i = 0; i < this->img_height_; i++)
		{
			for (int j = 0; j < this->img_width_; j++)
			{
				uchar value1 = tempMat1.at<uchar>(i, j);
				uchar value2 = tempMat2.at<uchar>(i, j);
				if ((double)value1 - (double)value2 >(double)threshold)
				{
					this->bin_mats_[binIdx].at<uchar>(i, j) = 255;
				}
				else if ((double)value2 - (double)value1 >(double)threshold)
				{
					this->bin_mats_[binIdx].at<uchar>(i, j) = 0;
				}
				else
				{
					this->bin_mats_[binIdx].at<uchar>(i, j) = 128;
				}
			}
		}
	}
	return true;
}

// 根据二值图统计结果
bool GrayDecoder::CountResult()
{
	double pixPeriod = 0;
	if (this->is_vertical_)
		pixPeriod = PROJECTOR_RESLINE / this->code_size_;
	else
		pixPeriod = PROJECTOR_RESROW / this->code_size_;

	res_mat_.create(img_height_, img_width_, CV_64FC1);
	for (int i = 0; i < this->img_height_; i++)
	{
		for (int j = 0; j < this->img_width_; j++)
		{
			short grayCode = 0;
			bool flag = true;
			for (int binIdx = 0; binIdx < this->num_digit_; binIdx++)
			{
				if (this->bin_mats_[binIdx].at<uchar>(i, j) == 128)
				{
					flag = false;
					break;
				}
				if (this->bin_mats_[binIdx].at<uchar>(i, j) == 255)
				{
					grayCode += 1 << binIdx;
				}
			}
			if (flag)
			{
				this->res_mat_.at<double>(i, j) = (double)this->gray2bin_[grayCode] * pixPeriod;
			}
			else
			{
				this->res_mat_.at<double>(i, j) = -100.0;
			}
		}
	}// 最后，每个区间的值都是最左侧/最上侧的值
	return true;
}

// 在计算机上显示图片
bool GrayDecoder::Visualize()
{
	if (VISUAL_DEBUG)
	{
		// 如果类不存在，则创建一个
		if (this->vis_mod_ == NULL)
		{
			this->vis_mod_ = new VisualModule("GrayDecoder");
		}

		// 显示中间的二值化图像
		for (int i = 0; i < this->num_digit_; i++)
		{
			this->vis_mod_->Show(this->bin_mats_[i], 300);
		}

		// 显示最终解码结果（归一化）
		this->vis_mod_->Show(this->res_mat_, 1000, true);
	}
	return true;
}