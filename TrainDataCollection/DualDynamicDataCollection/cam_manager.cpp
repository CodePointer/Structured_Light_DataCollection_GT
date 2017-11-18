#include "cam_manager.h"

// Constructor function:
// Set initial parameter values
CamManager::CamManager() {
	this->sum_device_num_ = 0;
  this->device_handles_ = NULL;
  this->device_valid_ = NULL;
	this->pp_buffer_ = NULL;
	this->kBufferNum = 1;
	this->buffer_used_ = 1;
	this->img_height_ = 0;
	this->img_width_ = 0;
}

// Destructor function:
// Release spaces, release cameras
CamManager::~CamManager() {
  if (this->device_handles_ != NULL) {
    delete[](this->device_handles_);
    this->device_handles_ = NULL;
  }
  if (this->device_valid_ != NULL) {
    delete[](this->device_valid_);
    this->device_valid_ = NULL;
  }
	if (this->pp_buffer_ != NULL) {
		for (int i = 1; i < this->kBufferNum; i++) {
			if (this->pp_buffer_[i] != NULL) {
				delete[](this->pp_buffer_[i]);
				this->pp_buffer_[i] = NULL;
			}
		}
		delete[]this->pp_buffer_;
		this->pp_buffer_ = NULL;
	}
}

// get total camera number
int CamManager::get_sum_device_num() {
	return this->sum_device_num_;
}

// if camera is valid
bool CamManager::get_device_valid(int idx) {
  if ((idx >= this->sum_device_num_) || (idx < 0)) {
    return false;
  }
  else {
    return this->device_valid_[idx];
  }
}

// Initial devices.
// Open the camera and set some parameters.
bool CamManager::InitCamera() {
	// Set initial parameters of resolution
  HVSTATUS status;
	this->img_height_ = kCamHeight;
  this->img_width_ = kCamWidth;
  
	// Get total device number, make true have 2 cameras
	int sum_num;
	status = HVGetDeviceTotal(&sum_num);
	HV_VERIFY(status);
	this->sum_device_num_ = sum_num;
  if (this->sum_device_num_ != kCamDeviceNum) {
    return false;
  }
  this->device_handles_ = new HHV[this->sum_device_num_];
  this->device_valid_ = new bool[this->sum_device_num_];

  // Set buffer space
  this->pp_buffer_ = new BYTE*[this->kBufferNum];
  for (int i = 0; i < this->kBufferNum; i++) {
    this->pp_buffer_[i] = new BYTE[this->img_height_*this->img_width_];
  }
  this->buffer_used_ = this->kBufferNum;
  
  // Set cameras
  for (int i = 0; i < this->sum_device_num_; i++) {
    // Open cameras
    status = BeginHVDevice(i+1, &(this->device_handles_[i]));
    HV_VERIFY(status);
    this->device_valid_[i] = true;
    // Set capture speed
    status = HVSetSnapSpeed(this->device_handles_[i], HIGH_SPEED);
    HV_VERIFY(status);
    // Set capture resolution
    status = HVSetResolution(this->device_handles_[i], RES_MODE0);
    HV_VERIFY(status);
    HVSetOutputWindow(this->device_handles_[i], 0, 0,
                      this->img_width_, this->img_height_);
    // Capture some images to make camera begin to work
    int stable_num = 5;
    Mat tmp_mat;
    while (stable_num-- != 0) {
      this->GetPicture(i, tmp_mat);
    }
  }

  return true;
}

// Acquiare image from camera
bool CamManager::SnapShot(int idx)
{
	HVSTATUS status;
	int max_attempt_num = 30;
	int now_attempt = 0;

	// get image
	status = HVSnapShot(this->device_handles_[idx], this->pp_buffer_, this->kBufferNum);
	while ((status != STATUS_OK) && (now_attempt < max_attempt_num)) {
    now_attempt++;
		status = HVSnapShot(this->device_handles_[idx], this->pp_buffer_, this->kBufferNum);
	}
	if (now_attempt >= max_attempt_num)
	{
		cout << HVGetErrorString(status) << std::endl;
		return false;
	}
	this->buffer_used_ = 0;
	return true;
}

// Out put image as mat
bool CamManager::GetPicture(int idx, Mat & pic)
{
	// if buffer is empty
	if (this->buffer_used_ >= this->kBufferNum)
	{
		if (!this->SnapShot(idx))
			return false;
	}
	// Create mat
	pic = Mat(Size(this->img_width_, this->img_height_), CV_8UC1, 
            this->pp_buffer_[this->buffer_used_]);
	this->buffer_used_++;
	return true;
}

// 关闭摄像头终止采集。
bool CamManager::CloseCamera()
{
	HVSTATUS status;

	// close camera
  for (int i = 0; i < this->sum_device_num_; i++) {
    status = EndHVDevice(this->device_handles_[i]);
    HV_VERIFY(status);
  }

	// delete buffer
  if (this->device_handles_ != NULL) {
    delete[](this->device_handles_);
    this->device_handles_ = NULL;
  }
  if (this->device_valid_ != NULL) {
    delete[](this->device_valid_);
    this->device_valid_ = NULL;
  }
  if (this->pp_buffer_ != NULL) {
    for (int i = 1; i < this->kBufferNum; i++) {
      if (this->pp_buffer_[i] != NULL) {
        delete[](this->pp_buffer_[i]);
        this->pp_buffer_[i] = NULL;
      }
    }
    delete[]this->pp_buffer_;
    this->pp_buffer_ = NULL;
  }

	return true;
}