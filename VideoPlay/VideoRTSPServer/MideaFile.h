#pragma once
#include "base.h"

class MideaFile
{
public:
	MideaFile();
	~MideaFile();
	int Open(const EBuffer& path, int nType = 96);
	EBuffer ReadOneFrame(); //���buffer��sizeΪ0����ʾû��֡��
	void Close();
	void Reset(); //���ú�ReadOneFrame�ֻ���ֵ����
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

