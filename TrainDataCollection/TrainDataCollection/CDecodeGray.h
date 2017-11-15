#ifndef _CDECODEGRAY_H_
#define _CDECODEGRAY_H_

#include <string>
#include <strstream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "CVisualization.h"
#include "StaticParameters.h"
#include "GlobalFunction.h"

using namespace std;
using namespace cv;

// 格雷码解码器。解码已经编写好的格雷码。
// 输入为一组灰度图，输出为一张灰度图，每一点存储的是projector中的坐标。
// 使用前需要分别调用4个Set函数传参。
class GrayDecoder
{
private:
	int num_digit_;			// 位数
	int code_size_;		// 总共的格雷码数目
	short * gray2bin_;		// 格雷码到二进制码的转换
	string code_file_path_;	// 存储格雷码的文件路径
	string code_file_name_;	// 存储格雷码的文件名
	int img_height_;				// 图像的行分辨率
	int img_width_;			// 图像的列分辨率
	bool is_vertical_;		// 设定格雷码方向

	Mat * grey_mats_;	// 输入的灰度图
	Mat * bin_mats_;	// 加工后的二值图
	Mat res_mat_;		// 结果

	VisualModule * vis_mod_;	// 用于显示中间结果

	bool AllocateSpace();		// 为输入的矩阵、short类型内容申请空间
	bool ReleaseSpace();		// 删除所有空间
	bool Grey2Bin();			// 将灰度图加工为二值图，以便进一步处理
	bool CountResult();			// 根据二值图统计结果
	bool Visualize();			// 显示中间结果

public:
	GrayDecoder();
	~GrayDecoder();

	bool Decode();
	Mat GetResult();

	bool SetMat(int num, Mat pic);						// 输入相应灰度图
	bool SetNumDigit(int numDigit, bool ver);				// 设置格雷码位数
	bool SetMatFileName(string codeFilePath,
		string codeFileName);			// 设置存储格雷码的文件名
};

#endif