#include "pro_manager.h"

// Constructor function
ProManager::ProManager() {
	this->pattern_height_ = kProHeight;
	this->pattern_width_ = kProWidth;
	this->bias_height_ = kScreenBiasHeight;
	this->bias_width_ = kScreenBiasWidth;
	this->win_name_ = "ProWindow";
}

// Destruction funtion
ProManager::~ProManager() {

}

// Intital projector. Create an opencv window at position.
bool ProManager::InitProjector() {
	// Create window and set position
	namedWindow(this->win_name_, WINDOW_NORMAL);
	setWindowProperty(this->win_name_, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	resizeWindow(this->win_name_, this->pattern_width_, this->pattern_height_);
	moveWindow(this->win_name_, this->bias_width_, this->bias_height_);
	return true;
}

// Destory window.
bool ProManager::CloseProjector() {
	destroyWindow(this->win_name_);
	return true;
}

// make projector to project <pic> for <time>ms.
bool ProManager::PresentPicture(cv::Mat pic, int time) {
	imshow(this->win_name_, pic);
	waitKey(time);
	return true;
}

bool ProManager::PresentPicture(uchar x, int time)
{
	Mat pic;
	pic.create(this->pattern_height_, this->pattern_width_, CV_8UC1);
	pic.setTo(x);
	imshow(this->win_name_, pic);
	waitKey(time);
	return true;
}