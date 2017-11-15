#include "CDecodeGray.h"

// ���캯�����趨һЩĬ�����á�
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

// ����������ȷ���ͷſռ䡣
GrayDecoder::~GrayDecoder()
{
	this->ReleaseSpace();
}

// ������Ӧ�Ҷ�ͼ
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

// ���ø�����λ��
bool GrayDecoder::SetNumDigit(int numDigit, bool ver)
{
	// �����жϲ����Ƿ�Ϸ�
	if ((numDigit <= 0) || (numDigit > 16))
		return false;
	
	// ����
	this->num_digit_ = numDigit;
	this->code_size_ = 1 << (this->num_digit_);
	this->is_vertical_ = ver;

	// ������Ӧ�ռ�
	if ((this->gray2bin_ != NULL) || (this->grey_mats_ != NULL) || (this->bin_mats_ != NULL))
		this->ReleaseSpace();
	this->AllocateSpace();

	return true;
}

// ���ô洢��������ļ���
bool GrayDecoder::SetMatFileName(std::string codeFilePath,
	std::string codeFileName)
{
	this->code_file_path_ = codeFilePath;
	this->code_file_name_ = codeFileName;
	return true;
}

// Ϊ����ľ���short������������ռ�
bool GrayDecoder::AllocateSpace()
{
	using namespace cv;

	// �жϲ����Ƿ�Ϸ�
	if ((this->num_digit_ <= 0) || (this->num_digit_ > 16))
		return false;

	// ����ռ䲢��ʼ��
	this->gray2bin_ = new short[this->code_size_];
	this->grey_mats_ = new Mat[this->num_digit_ * 2];
	this->bin_mats_ = new Mat[this->num_digit_];

	return true;
}

// ɾ�����пռ�
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

// ���롣
bool GrayDecoder::Decode()
{
	using namespace cv;

	// ��������ļ�
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

	// �ֱ������еĻҶ�ͼ������ת��Ϊ��ֵ����
	if (!this->Grey2Bin())
		return false;

	// ���ݶ�ֵͼ�ָ�����
	if (!this->CountResult())
		return false;

	// ���ӻ�����
	this->Visualize();

	return true;
}

// ��ȡ������
cv::Mat GrayDecoder::GetResult()
{
	Mat result;
	this->res_mat_.copyTo(result);
	return result;
}

// ���Ҷ�ͼ�ӹ�Ϊ��ֵͼ���Ա��һ������
bool GrayDecoder::Grey2Bin()
{
	using namespace cv;

	// ���ÿһ��binPicture
	for (int binIdx = 0; binIdx < this->num_digit_; binIdx++)
	{
		// �趨��ֵ
		Mat tempMat1, tempMat2;
		tempMat1 = this->grey_mats_[binIdx * 2];
		tempMat2 = this->grey_mats_[binIdx * 2 + 1];
		uchar threshold = 5;

		// ��ȡ����ͼƬ�Ĳ�
		Mat tempMat;
		tempMat = this->grey_mats_[binIdx * 2] - this->grey_mats_[binIdx * 2 + 1];
		
		// �������ֵ���ж�ֵ��
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

// ���ݶ�ֵͼͳ�ƽ��
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
	}// ���ÿ�������ֵ���������/���ϲ��ֵ
	return true;
}

// �ڼ��������ʾͼƬ
bool GrayDecoder::Visualize()
{
	if (VISUAL_DEBUG)
	{
		// ����಻���ڣ��򴴽�һ��
		if (this->vis_mod_ == NULL)
		{
			this->vis_mod_ = new VisualModule("GrayDecoder");
		}

		// ��ʾ�м�Ķ�ֵ��ͼ��
		for (int i = 0; i < this->num_digit_; i++)
		{
			this->vis_mod_->Show(this->bin_mats_[i], 300);
		}

		// ��ʾ���ս���������һ����
		this->vis_mod_->Show(this->res_mat_, 1000, true);
	}
	return true;
}