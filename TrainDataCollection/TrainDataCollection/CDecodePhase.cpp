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

// Ϊ����ľ�������ռ�
bool PhaseDecoder::AllocateSpace()
{
	// �жϲ����Ƿ�Ϸ�
	if (this->image_num_ <= 0)
		return false;

	// ����ռ䲢��ʼ��
	this->grey_mats_ = new Mat[this->image_num_];

	return true;
}

// ɾ�����пռ�
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

// ���ݻҶ�ͼͳ�ƽ��
bool PhaseDecoder::CountResult()
{
	this->res_mat_.create(this->img_height_, this->img_width_, CV_64FC1);
	Mat SinValue0;
	Mat SinValue1;

	for (int i = 0; i < this->img_height_; i++)
	{
		for (int j = 0; j < this->img_width_; j++)
		{
			// �Ȼ�ȡ�õ�Ҷ�ֵ
			float greyValue0 = this->grey_mats_[0].at<uchar>(i, j);	// greyValue0 = (sin(x) + 1) * 127;
			float greyValue1 = this->grey_mats_[1].at<uchar>(i, j);	// greyValue1 = (sin(x + CV_PI / 2) + 1) * 127
			float greyValue2 = this->grey_mats_[2].at<uchar>(i, j);	// greyValue2 = (sin(x + CV_PI) + 1) * 127
			float greyValue3 = this->grey_mats_[3].at<uchar>(i, j);	// greyValue3 = (sin(x + 3 * CV_PI / 2) + 1) * 127
			// ���Ҷ�ֵת��Ϊsinֵ
			float sinValue = (greyValue0 - greyValue2) / 2;
			float cosValue = (greyValue1 - greyValue3) / 2;
			// ����x
			float x = cvFastArctan(sinValue, cosValue);			
			// ����ƫ����
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

// ����
bool PhaseDecoder::Decode()
{
	// ���ݻҶ�ͼ�ָ�����
	if (!this->CountResult())
	{
		ErrorHandling("PhaseDecoder.Decode()->CountResult fault");
		return false;
	}

	// ���ӻ�
	this->Visualize();

	return true;
}

// ��ȡ������
Mat PhaseDecoder::GetResult()
{
	Mat result;
	this->res_mat_.copyTo(result);
	return result;
}

// ������Ӧ�Ҷ�ͼ
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

// ���ò���
bool PhaseDecoder::SetNumDigit(int numMat, int pixperiod)
{
	// �жϲ����Ƿ�Ϸ�
	if ((numMat <= 0))
		return false;

	// ����
	this->image_num_ = numMat;
	this->pix_period_ = pixperiod;

	// ����ռ�
	if ((this->grey_mats_ != NULL))
	{
		this->DeleteSpace();
	}
	this->AllocateSpace();

	return true;
}

// �ڼ��������ʾͼƬ
bool PhaseDecoder::Visualize()
{
	if (VISUAL_DEBUG)
	{
		// ����಻���ڣ��򴴽�һ��
		if (this->vis_mod_ == NULL)
		{
			this->vis_mod_ = new VisualModule("PhaseDecoder");
		}

		// ��ʾ�Ҷ�ͼ��
		for (int i = 0; i < this->image_num_; i++)
		{
			this->vis_mod_->Show(this->grey_mats_[i], 300);
		}

		// ��ʾ���ս���������һ����
		this->vis_mod_->Show(this->res_mat_, 1000, true, 0.5);
	}
	return true;
}