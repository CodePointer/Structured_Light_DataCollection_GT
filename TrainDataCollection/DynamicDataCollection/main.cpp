#include "CDataCollection.h"
using namespace cv;

int main()
{
	bool status = true;
	
	CDataCollection dc;

	if (status)
	{
		status = dc.Init();
	}
	if (status)
	{
		status = dc.CollectData();
	}
	if (status)
	{
		status = dc.Close();
	}

	return 0;
}