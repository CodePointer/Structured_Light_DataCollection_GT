#include "data_collector.h"
using namespace cv;

int main()
{
	bool status = true;
	DataCollector dc;
	if (status)	{
		status = dc.Init();
	}
	if (status)	{
		status = dc.CollectData();
	}
	if (status)	{
		status = dc.Close();
	}
	return 0;
}