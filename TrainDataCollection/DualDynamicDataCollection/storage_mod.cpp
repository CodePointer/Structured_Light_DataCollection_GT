#include "storage_mod.h"

StorageModule::StorageModule() {
	this->file_path_ = "";
	this->file_name_ = "";
	this->file_suffix_ = "";
	this->final_path_ = "";
}

StorageModule::~StorageModule() {
}

bool StorageModule::Store(Mat * pictures, int num) {

  if (num <= 0) {
    return false;
  }
	bool status = true;

	for (int i = 0; i < num; i++)	{
		string idx2str;
		if (num == 1) {
      idx2str = "";
		}	else {
			std::strstream ss;
			ss << i;
			ss >> idx2str;
		}

    this->final_path_ = this->file_path_ + this->file_name_
        + idx2str + this->file_suffix_;
		status = imwrite(this->final_path_, pictures[i]);
		if (!status)
		{
			// Create folder
      this->CreateFolder(this->file_path_);
      status = imwrite(this->final_path_, pictures[i]);
		}
	}
	if (!status) {
		ErrorHandling("StorageModule.Store->imwrite Error.");
	}

	return status;
}

bool StorageModule::SetMatFileName(string file_path, string file_name, string file_suffix) {

	// 变更参数
	this->file_path_ = file_path;
	this->file_name_ = file_name;
	this->file_suffix_ = file_suffix;
	// Check folder exist and create if not
	fstream file;
	file.open(this->file_path_ + "test.txt", ios::out);
	if (!file) {
    this->CreateFolder(this->file_path_);
	}
	file.close();
	return true;
}

bool StorageModule::CreateFolder(string FilePath)
{
	bool status = true;
	// Create a dir
	string temp = FilePath;
	for (int i = 0; i < temp.length(); i++) {
		if (temp[i] == '/') {
			temp[i] = '\\';
		}
	}
	system((string("mkdir ") + temp).c_str());
	return status;
}