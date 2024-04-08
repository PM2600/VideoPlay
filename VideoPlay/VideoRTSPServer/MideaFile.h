#pragma once
#include "base.h"

class MideaFile
{
public:
	MideaFile();
	~MideaFile();
	int Open(const EBuffer& path, int nType = 96);
	EBuffer ReadOneFrame(); //如果buffer的size为0，表示没有帧了
	void Close();
	void Reset(); //重置后，ReadOneFrame又会有值返回
private:
	long FindH264Head(int& headsize);
	EBuffer ReadH264Frame();
private:
	long m_size;
	FILE* m_file;
	EBuffer m_filepath;
	//96 H264
	int m_type; 
};

