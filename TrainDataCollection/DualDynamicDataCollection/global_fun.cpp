#include "global_fun.h"

int ErrorHandling(string message)
{
	cout << "An Error Occurs:" << message << endl;
	system("PAUSE");
	return 0;
}

void Double2Uchar(Mat src_mat, Mat & dst_mat) {
  dst_mat.create(src_mat.size(), CV_8UC1);
  for (int h = 0; h < src_mat.size().height; h++) {
    for (int w = 0; w < src_mat.size().width; w++) {
      dst_mat.at<uchar>(h, w) = (uchar)src_mat.at<double>(h, w);
    }
  }
  return;
}