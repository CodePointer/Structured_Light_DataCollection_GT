#include "data_collector.h"

int main()
{
	bool status = true;
	DataCollector dc;
	if (status)	{
		status = dc.Init();
	}
	if (status)	{
		status = dc.CollectDynaData();
	}
	if (status)	{
		status = dc.Close();
	}
	return 0;
}

//#include "calibrator.h"
//
//int main()
//{
//  bool status = true;
//  Calibrator calib;
//  if (status) {
//    status = calib.Init();
//  }
//  if (status) {
//    status = calib.Calibrate();
//  }
//  if (status) {
//    status = calib.Result();
//  }
//  return 0;
//}