#ifndef _CVISUALIZATION_H_
#define _CVISUALIZATION_H_

#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// ���ӻ�ģ�飬����debug���Զ��������ٴ��ڡ�
class VisualModule
{
private:
	string m_winName;		// ��������
public:
	VisualModule(string winName);
	~VisualModule();
	int Show(Mat pic, int time, bool norm = false, double zoom = 1.0);
};

#endif