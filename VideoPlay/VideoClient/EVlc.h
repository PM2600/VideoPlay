#pragma once
#include "vlc.h"
#include <string>

class VlcSize {
public:
	int nWidth;
	int nHeight;
	VlcSize(int width = 0, int height = 0) {
		nWidth = width;
		nHeight = height;
	}
	~VlcSize();
	VlcSize(const VlcSize& size) {
		nWidth = size.nWidth;
		nHeight = size.nHeight;
	}
	VlcSize& operator=(const VlcSize& size) {
		if (this != &size) {
			nWidth = size.nWidth;
			nHeight = size.nHeight;
		}
		return *this;
	}
};

class EVlc
{
public:
	EVlc();
	~EVlc();
	//strUrl如果包含中文，请传入utf-8的编码格式字符串
	int SetMedia(const std::string& strUrl);
	int SetHwnd(HWND hwnd);
	int Play();
	int Pause();
	int Stop();
	float GetPosition();
	void SetPosition(float pos);
	int GetVolume();
	int SetVolume(int volume);
	VlcSize GetMediaInfo();
protected:
	libvlc_instance_t* m_instance;
	libvlc_media_t* m_media;
	libvlc_media_player_t* m_player;
};

