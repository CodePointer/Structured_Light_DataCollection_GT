#ifndef _PROMANAGER_H_
#define _PROMANAGER_H_

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "static_para.h"

using namespace std;
using namespace cv;

// ͶӰ�ǿ����ࡣ���ڿ���ͶӰ��Ͷ��Ҫ���ͼ����
// ���ã�InitProjector��presentPicture��CloseProjector
class ProManager
{
private:
	int pattern_height_;			  // ͶӰ�ǵ��зֱ���
	int pattern_width_;			    // ͶӰ�ǵ��зֱ���
	int bias_height_;			      // ���ڵ���ƫ�Ʒֱ���
	int bias_width_;			      // ���ڵ���ƫ�Ʒֱ���
	string win_name_;		        // ��������

public:
	ProManager();			  // ���캯��
	~ProManager();			// ��������
	bool InitProjector();							// ��ʼ���豸
	bool PresentPicture(Mat pic, int time);			// ʹͶӰ�Ƿų�pic����ʱtime
	bool PresentPicture(uchar x, int time);			// ʹͶӰ�Ƿų�ȫΪx��ͼƬ����ʱtime
	bool CloseProjector();							// �ر��豸
};

#endif