#include "phase_decoder.h"

PhaseDecoder::PhaseDecoder() {
	this->image_num_ = 0;
	this->pix_period_ = 16;
	this->img_height_ = kCamHeight;
	this->img_width_ = kCamWidth;
	this->grey_mats_ = NULL;
	this->vis_mod_ = NULL;
}

PhaseDecoder::~PhaseDecoder() {
	this->DeleteSpace();
}

// Allocate space for mats
bool PhaseDecoder::AllocateSpace() {
	if (this->image_num_ <= 0)
		return false;
	this->grey_mats_ = new Mat[this->image_num_];
	return true;
}

// Release space
bool PhaseDecoder::DeleteSpace() {
	if (this->grey_mats_ != NULL)	{
		delete[](this->grey_mats_);
		this->grey_mats_ = NULL;
	}
	if (this->vis_mod_ != NULL)	{
		delete(this->vis_mod_);
		this->vis_mod_ = NULL;
	}
	return true;
}

// Calculate result according to grey mats
bool PhaseDecoder::CountResult()
{
	this->res_mat_.create(this->img_height_, this->img_width_, CV_64FC1);
	Mat SinValue0;
	Mat SinValue1;
	for (int h = 0; h < this->img_height_; h++)	{
		for (int w = 0; w < this->img_width_; w++) {
			// acquaire 4 grey value at this point:
      // greyValue0 = (sin(x) + 1) * 127;
      // greyValue1 = (sin(x + CV_PI / 2) + 1) * 127
      // greyValue2 = (sin(x + CV_PI) + 1) * 127
      // greyValue3 = (sin(x + 3 * CV_PI / 2) + 1) * 127
			float greyValue0 = this->grey_mats_[0].at<uchar>(h, w);
			float greyValue1 = this->grey_mats_[1].at<uchar>(h, w);
			float greyValue2 = this->grey_mats_[2].at<uchar>(h, w);
			float greyValue3 = this->grey_mats_[3].at<uchar>(h, w);
			float sinValue = (greyValue0 - greyValue2) / 2;
			float cosValue = (greyValue1 - greyValue3) / 2;
			float x = cvFastArctan(sinValue, cosValue);
			float pix = (x) / (360)*(float)(this->pix_period_);
			pix += 0.5;
			if (pix > this->pix_period_)
				pix -= this->pix_period_;
			this->res_mat_.at<double>(h, w) = (double)pix;
		}
	}
	return true;
}

// decode the result
bool PhaseDecoder::Decode()
{
	if (!this->CountResult())	{
		ErrorHandling("PhaseDecoder.Decode()->CountResult fault");
		return false;
	}
	this->Visualize();
	return true;
}

// get result when decode finished.
Mat PhaseDecoder::GetResult()
{
	Mat result;
	this->res_mat_.copyTo(result);
	return result;
}

// Input grey images
bool PhaseDecoder::SetMat(int num, cv::Mat pic)
{
	if (this->grey_mats_ == NULL)	{
		ErrorHandling("PhaseDecoder.SetMat->grePicture Space is not allocated.");
		return false;
	}
	pic.copyTo(this->grey_mats_[num]);
	return true;
}

// Set num digit information
bool PhaseDecoder::SetNumDigit(int num_digit, int pixperiod)
{
	if ((num_digit <= 0))
		return false;
	this->image_num_ = num_digit;
	this->pix_period_ = pixperiod;

	if ((this->grey_mats_ != NULL))
		this->DeleteSpace();
	this->AllocateSpace();
	return true;
}

// visualization middle result
bool PhaseDecoder::Visualize()
{
	if (kVisualFlagForDebug) {
		if (this->vis_mod_ == NULL)
			this->vis_mod_ = new VisualModule("PhaseDecoder");
		for (int i = 0; i < this->image_num_; i++) {
			this->vis_mod_->Show(this->grey_mats_[i], 300);
		}
		this->vis_mod_->Show(this->res_mat_, 1000, true, 0.5);
	}
	return true;
}