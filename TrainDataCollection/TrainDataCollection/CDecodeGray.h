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

// ������������������Ѿ���д�õĸ����롣
// ����Ϊһ��Ҷ�ͼ�����Ϊһ�ŻҶ�ͼ��ÿһ��洢����projector�е����ꡣ
// ʹ��ǰ��Ҫ�ֱ����4��Set�������Ρ�
class GrayDecoder
{
private:
	int num_digit_;			// λ��
	int code_size_;		// �ܹ��ĸ�������Ŀ
	short * gray2bin_;		// �����뵽���������ת��
	string code_file_path_;	// �洢��������ļ�·��
	string code_file_name_;	// �洢��������ļ���
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
	bool SetMatFileName(string codeFilePath,
		string codeFileName);			// ���ô洢��������ļ���
};

#endif