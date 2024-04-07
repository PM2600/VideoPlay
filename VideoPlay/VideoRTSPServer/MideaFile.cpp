#include "MideaFile.h"

MideaFile::MideaFile() : m_file(NULL), m_type(-1)
{}

MideaFile::~MideaFile()
{
	Close();
}

int MideaFile::Open(const EBuffer& path, int nType)
{
	m_file = fopen(path, "rb");
	if (m_file == NULL) {
		return -1;
	}
	m_type = nType;
	fseek(m_file, 0, SEEK_END);
	m_size = ftell(m_file);
	Reset();
	return 0;
}

EBuffer MideaFile::ReadOneFrame()
{
	switch (m_type) {
	case 96:
		return ReadH264Frame();
	}
	return EBuffer();
}

void MideaFile::Close()
{
	m_type = -1;
	if (m_file != NULL) {
		FILE* file = m_file;
		m_file = NULL;
		fclose(file);
	}
}

void MideaFile::Reset()
{
	if (m_file) {
		fseek(m_file, 0, SEEK_SET);
	}
}

long MideaFile::FindH264Head()
{
	while (!feof(m_file)) {
		char c = 0x7F;
		while (!feof(m_file)) {
			c = fgetc(m_file);
			if (c == 0)
				break;
		}
		if (!feof(m_file)) {
			c = fgetc(m_file);
			if (c == 0) {
				c = fgetc(m_file);
				if (c == 1) { //找到一个头 00 00 01
					return ftell(m_file) - 3;
				}
				else if (c == 0) {
					c = fgetc(m_file);
					if (c == 1) { //找到一个头 00 00 00 01
						return ftell(m_file) - 4;
					}
				}
			}
		}
	}
	return -1;
}

EBuffer MideaFile::ReadH264Frame()
{
	if (m_file) {
		long off = FindH264Head();
		if (off == -1) {
			return EBuffer();
		}
		fseek(m_file, off + 3, SEEK_SET);
		long tail = FindH264Head();
		if (tail == -1) tail = m_size;
		long size = tail - off;
		fseek(m_file, off, SEEK_SET);
		EBuffer result(size);
		fread(result, 1, size, m_file);
		return result;
	}
	return EBuffer();
}
