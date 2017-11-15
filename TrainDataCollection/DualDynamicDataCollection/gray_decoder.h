#ifndef _GRAYDECODER_H_
#define _GRAYDECODER_H_

#include <string>
#include <strstream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "visual_mod.h"
#include "static_para.h"
#include "global_fun.h"

using namespace std;
using namespace cv;

// ������������������Ѿ���д�õĸ����롣
// ����Ϊһ��Ҷ�ͼ�����Ϊһ�ŻҶ�ͼ��ÿһ��洢����projector�е����ꡣ
// ʹ��ǰ��Ҫ�ֱ����4��Set�������Ρ�
class GrayDecoder
{
private:
	int num_digit_;			          // the digit number of gray code
	int code_size_;		            // gray code size
	short * gray2bin_;		        // from gray to binary
	string code_file_path_;	
	string code_file_name_;	
  string code_file_suffix_;
	int img_height_;				// ͼ����зֱ���
	int img_width_;			// ͼ����зֱ���
	bool is_vertical_;		// �趨�����뷽��

	Mat * grey_mats_;	// ����ĻҶ�ͼ
	Mat * bin_mats_;	// �ӹ���Ķ�ֵͼ
	Mat res_mat_;		// ���

	VisualModule * vis_mod_;	// ������ʾ�м���

	bool AllocateSpace();		// Ϊ����ľ���short������������ռ�
	bool ReleaseSpace();		// ɾ�����пռ�
	bool Grey2Bin();			// ���Ҷ�ͼ�ӹ�Ϊ��ֵͼ���Ա��һ������
	bool CountResult();			// ���ݶ�ֵͼͳ�ƽ��
	bool Visualize();			// ��ʾ�м���

public:
	GrayDecoder();
	~GrayDecoder();

	bool Decode();
	Mat GetResult();

	bool SetMat(int num, Mat pic);						// ������Ӧ�Ҷ�ͼ
	bool SetNumDigit(int numDigit, bool ver);				// ���ø�����λ��
	bool SetCodeFileName(string code_file_path, string code_file_name);
};

#endif