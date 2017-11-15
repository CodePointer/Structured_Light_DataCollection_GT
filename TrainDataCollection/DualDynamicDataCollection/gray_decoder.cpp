#include "gray_decoder.h"

// 构造函数。设定一些默认设置。
GrayDecoder::GrayDecoder() {
	this->num_digit_ = 0;
	this->code_size_ = 0;
	this->gray2bin_ = NULL;
  this->code_file_path_ = "";
	this->code_file_name_ = "";
	this->img_height_ = kCamHeight;
	this->img_width_ = kCamWidth;
	this->grey_mats_ = NULL;
	this->bin_mats_ = NULL;
	this->vis_mod_ = NULL;
}

// 析构函数。确保释放空间。
GrayDecoder::~GrayDecoder() {
	this->ReleaseSpace();
}

// Input gray image
bool GrayDecoder::SetMat(int num, Mat pic) {
	if (this->grey_mats_ == NULL) {
		ErrorHandling("GrayDecoder.SetMat()->grePicture Space is not allocated.");
		return false;
	}
	pic.copyTo(this->grey_mats_[num]);
	return true;
}

// Set digit number of graycode
bool GrayDecoder::SetNumDigit(int num_digit, bool is_ver) {
	if ((num_digit <= 0) || (num_digit > 16))
		return false;
	this->num_digit_ = num_digit;
	this->code_size_ = 1 << (this->num_digit_);
	this->is_vertical_ = is_ver;

	if ((this->gray2bin_ != NULL) || (this->grey_mats_ != NULL) || (this->bin_mats_ != NULL))
		this->ReleaseSpace();
	this->AllocateSpace();
	return true;
}

// Set file path for gray code storage
bool GrayDecoder::SetCodeFileName(string code_file_path, string code_file_name) {
	this->code_file_path_ = code_file_path;
	this->code_file_name_ = code_file_name;
	return true;
}

// Allocate space for pointers
bool GrayDecoder::AllocateSpace() {
	if ((this->num_digit_ <= 0) || (this->num_digit_ > 16))
		return false;
	this->gray2bin_ = new short[this->code_size_];
	this->grey_mats_ = new Mat[this->num_digit_ * 2];
	this->bin_mats_ = new Mat[this->num_digit_];
	return true;
}

// Release all pointers' space
bool GrayDecoder::ReleaseSpace()
{
	if (this->gray2bin_ != NULL) {
		delete[](this->gray2bin_);
		this->gray2bin_ = NULL;
	}
	if (this->grey_mats_ != NULL)	{
		delete[](this->grey_mats_);
		this->grey_mats_ = NULL;
	}
	if (this->bin_mats_ != NULL) {
		delete[](this->bin_mats_);
		this->bin_mats_ = NULL;
	}
	if (this->vis_mod_ != NULL)	{
		delete(this->vis_mod_);
		this->vis_mod_ = NULL;
	}
	return true;
}

// Decode the input gray picture.
bool GrayDecoder::Decode()
{
	// Read
	ifstream codeFile;
	codeFile.open(this->code_file_path_ + this->code_file_name_, ios::in);
	if (!codeFile) {
		ErrorHandling("Gray Decode->Open file error.");
		return false;
	}
	for (int i = 0; i < this->code_size_; i++) {
		int binCode, grayCode;
		codeFile >> binCode >> grayCode;
		this->gray2bin_[grayCode] = binCode;
	}

	// Convert to binary image
	if (!this->Grey2Bin())
		return false;
	// Trans to result
	if (!this->CountResult())
		return false;
	// Visualization if needed
	this->Visualize();
	return true;
}

// Get result
Mat GrayDecoder::GetResult()
{
	Mat result;
	this->res_mat_.copyTo(result);
	return result;
}

// Convert grey image to binary image
bool GrayDecoder::Grey2Bin()
{
	for (int binIdx = 0; binIdx < this->num_digit_; binIdx++)
	{
    uchar threshold = 5;
		this->bin_mats_[binIdx].create(Size(this->img_width_, this->img_height_), CV_8UC1);
		
    for (int i = 0; i < this->img_height_; i++) {
			for (int j = 0; j < this->img_width_; j++) {
				uchar value1 = this->grey_mats_[2 * binIdx].at<uchar>(i, j);
				uchar value2 = this->grey_mats_[2 * binIdx + 1].at<uchar>(i, j);
				if ((double)value1 - (double)value2 >(double)threshold) {
					this->bin_mats_[binIdx].at<uchar>(i, j) = 255;
				}	else if ((double)value2 - (double)value1 >(double)threshold) {
					this->bin_mats_[binIdx].at<uchar>(i, j) = 0;
				}	else {
					this->bin_mats_[binIdx].at<uchar>(i, j) = 128;
				}
			}
		}
	}
	return true;
}

// Binary image to final result
// For every pixperiod, value is mark as left/up line's value
bool GrayDecoder::CountResult()
{
	double pix_period = 0;
	if (this->is_vertical_)
		pix_period = kProWidth / this->code_size_;
	else
		pix_period = kProHeight / this->code_size_;

	res_mat_.create(img_height_, img_width_, CV_64FC1);
	for (int i = 0; i < this->img_height_; i++) {
		for (int j = 0; j < this->img_width_; j++) {
			short gray_code = 0;
			bool flag = true;
			for (int binIdx = 0; binIdx < this->num_digit_; binIdx++) {
				if (this->bin_mats_[binIdx].at<uchar>(i, j) == 128)	{
					flag = false;	break;
				} else if (this->bin_mats_[binIdx].at<uchar>(i, j) == 255)	{
					gray_code += 1 << binIdx;
				}
			}
			if (flag)	{
				this->res_mat_.at<double>(i, j) = (double)this->gray2bin_[gray_code] * pix_period;
			}	else {
				this->res_mat_.at<double>(i, j) = -100.0;
			}
		}
	}
	return true;
}

// Visualization part
bool GrayDecoder::Visualize() {
	if (kVisualFlagForDebug)	{
		if (this->vis_mod_ == NULL)	{
			this->vis_mod_ = new VisualModule("GrayDecoder");
		}
		for (int i = 0; i < this->num_digit_; i++) {
			this->vis_mod_->Show(this->bin_mats_[i], 300);
		}
		this->vis_mod_->Show(this->res_mat_, 1000, true);
	}
	return true;
}