#include "CSensor.h"


// ������ģ�顣
// ����ģ�⴫�����������ݡ�
// ���ڱ궨����֮ǰ�Ѿ����߲ɼ��ã���������ֻ��Ҫ���ļ��ж����ļ����ɡ�

SensorManager::SensorManager()
{
	this->m_patterns = NULL;
	this->m_patternNum = 0;
	this->m_nowNum = 0;
	this->m_filePath = "";
	this->m_fileName = "";
	this->m_fileSuffix = "";

	this->m_camera = NULL;
	this->m_projector = NULL;
}

SensorManager::~SensorManager()
{
	if (this->m_patterns != NULL)
	{
		delete[]this->m_patterns;
		this->m_patterns = NULL;
	}
	if (this->m_camera != NULL)
	{
		delete[]this->m_camera;
		this->m_camera = NULL;
	}
	if (this->m_projector != NULL)
	{
		delete[]this->m_projector;
		this->m_projector = NULL;
	}
}

// ��ʼ��������
// ����Camera��Projector�����г�ʼ��
bool SensorManager::InitSensor()
{
	bool status;

	this->m_camera = new CamManager();
	status = this->m_camera->InitCamera();

	this->m_projector = new ProManager();
	status = this->m_projector->InitProjector();

	return status;
}

// �رմ�����
bool SensorManager::CloseSensor()
{
	bool status = true;

	if (this->m_camera != NULL)
	{
		this->m_camera->CloseCamera();
		delete[]this->m_camera;
		this->m_camera = NULL;
	}
	if (this->m_projector != NULL)
	{
		delete[]this->m_projector;
		this->m_projector = NULL;
	}
	this->UnloadPatterns();

	return status;
}

// ��ȡͼ��
bool SensorManager::LoadPatterns(int patternNum, string filePath, string fileName, string fileSuffix)
{
	// ���״̬�Ƿ�Ϸ�
	if (this->m_patterns != NULL)
	{
		this->UnloadPatterns();
	}

	// ���ò���������ռ�
	this->m_patternNum = patternNum;
	this->m_nowNum = 0;
	this->m_filePath = filePath;
	this->m_fileName = fileName;
	this->m_fileSuffix = fileSuffix;
	this->m_patterns = new Mat[this->m_patternNum];

	// ��ȡ
	for (int i = 0; i < patternNum; i++)
	{
		Mat tempMat;
		string idx2Str;
		strstream ss;
		ss << i;
		ss >> idx2Str;
		tempMat = imread(this->m_filePath
			+ this->m_fileName
			+ idx2Str
			+ this->m_fileSuffix, CV_LOAD_IMAGE_GRAYSCALE);
		tempMat.copyTo(this->m_patterns[i]);

		if (tempMat.empty())
		{
			string file_name = this->m_filePath
				+ this->m_fileName
				+ idx2Str
				+ this->m_fileSuffix;
			ErrorHandling("SensorManager::LoadPatterns::<Read>, imread error, idx=" + idx2Str + " file_path:" + file_name);
		}
	}

	return true;
}

// �ͷ��Ѷ�ȡͼ��
bool SensorManager::UnloadPatterns()
{
	if (this->m_patterns != NULL)
	{
		delete[]this->m_patterns;
		this->m_patterns = NULL;
	}
	this->m_patternNum = 0;
	this->m_nowNum = 0;
	this->m_filePath = "";
	this->m_fileName = "";
	this->m_fileSuffix = "";

	return true;
}

// ����ͶӰ��ͶӰ��ͼ�񣬴�0����
bool SensorManager::SetProPicture(int nowNum)
{
	bool status = true;

	// �������Ƿ�Ϸ�
	if (nowNum >= this->m_patternNum)
	{
		status = false;
		return status;
	}

	this->m_nowNum = nowNum;

	status = this->m_projector->presentPicture(
		this->m_patterns[this->m_nowNum], 150);

	return status;
}

// ��ȡ���ͼ��
Mat SensorManager::GetCamPicture()
{
	bool status = true;

	Mat tempMat;
	status = this->m_camera->getPicture(tempMat);

	return tempMat;
}

// ��ȡͶӰ��ͶӰ��ͼ��
Mat SensorManager::GetProPicture()
{
	return this->m_patterns[this->m_nowNum];
}