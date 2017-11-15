#ifndef _CDECODEPHASE_H_
#define _CDECODEPHASE_H_

#include <opencv2/opencv.hpp>
#include "CVisualization.h"
#include "GlobalFunction.h"
#include "StaticParameters.h"
using namespace cv;

// ���ƽ������������Ѿ���д�õ������롣
// ����Ϊһ��Ҷ�ͼ�����Ϊһ�ŻҶ�ͼ��ÿһ��洢����projector�е�ƫ�����ꡣ
class PhaseDecoder
{
private:
	int image_num_;		// ͼƬ��Ŀ��Ĭ��Ϊ2��
	int pix_period_;	// �������ڡ�
	int img_height_;				// ͼ����зֱ���
	int img_width_;			// ͼ����зֱ���

	Mat * grey_mats_;	// ����ĻҶ�ͼ
	Mat res_mat_;		// ���

	VisualModule * vis_mod_;	// ������ʾ�м���

	bool AllocateSpace();		// Ϊ����ľ�������ռ�
	bool DeleteSpace();			// ɾ�����пռ�
	bool CountResult();			// ���ݹ�һ���Ҷ�ͼͳ�ƽ��
	bool Visualize();			// ��ʾ�м���

public:
	PhaseDecoder();
	~PhaseDecoder();

	bool Decode();
	Mat GetResult();

	bool SetMat(int num, Mat pic);						// ������Ӧ�Ҷ�ͼ
	bool SetNumDigit(int numDigit, int pixperiod);			// ���ò���
};

#endif