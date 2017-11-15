#include "visual_mod.h"

VisualModule::VisualModule(string winName) {
	this->win_name_ = winName;
	namedWindow(this->win_name_);
}

VisualModule::~VisualModule() {
	destroyWindow(this->win_name_);
}

int VisualModule::Show(Mat pic, int time, bool norm, double zoom) {
	Mat show_mat;
	Size show_size = Size(pic.size().width*zoom, pic.size().height*zoom);
	resize(pic, show_mat, show_size, 0.0, 0.0, INTER_NEAREST);
	
	// if need to norm
	if (norm)	{
    // Set range
		int range = 0;		
		if (show.depth() == CV_8U) {
			range = 0xff;
		}	else if (show.depth() == CV_16U) {
			range = 0xffff;
		}	else if (show.depth() == CV_64F) {
			range = 0xffff;
			Mat tmp;
			tmp.create(show.size(), CV_16UC1);
			for (int h = 0; h < show_mat.size().height; h++) {
				for (int w = 0; w < show_mat.size().width; w++)	{
					double value;
					value = show_mat.at<double>(h, w);
					if (value < 0)
						value = 0;
					tmp.at<ushort>(h, w) = (ushort)value;
				}
			}
			tmp.copyTo(show_mat);
		}
		// find min, max of initial mat
		int min, max;
		min = range;
		max = 0;
		for (int i = 0; i < show_mat.size().height; i++) {
			for (int j = 0; j < show_mat.size().width; j++)	{
				int value = 0;
				if (show_mat.depth() == CV_8U) {
					value = show_mat.at<uchar>(i, j);
				}	else if (show_mat.depth() == CV_16U) {
					value = show_mat.at<ushort>(i, j);
				}
				if (value < min)
					min = value;
				if (value > max)
					max = value;
			}
		}

		// initialization
    show_mat = (show_mat - min) / (max - min) * range;
	}

	imshow(this->win_name_, show_mat);
	return waitKey(time);
}