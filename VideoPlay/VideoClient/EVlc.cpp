#include "pch.h"
#include "EVlc.h"

EVlc::EVlc()
{
	m_instance = libvlc_new(0, NULL);
	m_media = NULL;
	m_player = NULL;
	m_hwnd = NULL;
}

EVlc::~EVlc()
{
	if (m_player != NULL) {
		libvlc_media_player_t* temp = m_player;
		m_player = NULL;
		libvlc_media_player_release(temp);
	}
	if (m_media != NULL) {
		libvlc_media_t* temp = m_media;
		m_media = NULL;
		libvlc_media_release(temp);
	}
	if (m_instance != NULL) {
		libvlc_instance_t* temp = m_instance;
		m_instance = NULL;
		libvlc_release(temp);
	}
}

int EVlc::SetMedia(const std::string& strUrl)
{
	if (m_instance == NULL || m_hwnd == NULL)
		return -1;
	if (strUrl == m_url) 
		return 0;
	m_url = strUrl;
	if (m_media != NULL) {
		libvlc_media_release(m_media);
		m_media = NULL;
	}
	m_media = libvlc_media_new_location(m_instance, strUrl.c_str());
	if (!m_media)
		return -2;
	if (m_player != NULL) {
		libvlc_media_player_release(m_player);
		m_player = NULL; 
	}
	m_player = libvlc_media_player_new_from_media(m_media);
	if (!m_player)
		return -3;
	libvlc_media_player_set_hwnd(m_player, m_hwnd);
	return 0;
}

#ifdef WIN32
int EVlc::SetHwnd(HWND hWnd)
{
	m_hwnd = hWnd;
	return 0;
}
#endif

int EVlc::Play()
{
	if (!m_player || !m_instance || !m_media)
		return -1;
	return libvlc_media_player_play(m_player);
}

int EVlc::Pause()
{
	if (!m_player || !m_instance || !m_media)
		return -1;
	libvlc_media_player_pause(m_player);
	return 0;
}

int EVlc::Stop()
{
	if (!m_player || !m_instance || !m_media)
		return -1;
	libvlc_media_player_stop(m_player);
	return 0;
}

float EVlc::GetPosition()
{
	if (!m_player || !m_instance || !m_media)
		return -1.0;
	return libvlc_media_player_get_position(m_player);
}

void EVlc::SetPosition(float pos)
{
	if (!m_player || !m_instance || !m_media)
		return;
	libvlc_media_player_set_position(m_player, pos);
}

int EVlc::GetVolume()
{
	if (!m_player || !m_instance || !m_media)
		return -1;
	return libvlc_audio_get_volume(m_player);
}

int EVlc::SetVolume(int volume)
{
	if (!m_player || !m_instance || !m_media)
		return -1;
	return libvlc_audio_set_volume(m_player, volume);
}

VlcSize EVlc::GetMediaInfo()
{
	if (!m_player || !m_instance || !m_media)
		return VlcSize(-1, -1);

	return VlcSize(libvlc_video_get_width(m_player), libvlc_video_get_height(m_player));
}

std::string EVlc::Unicode2Utf8(const std::wstring& strIn)
{
	std::string str;
	int length = ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), NULL, 0, NULL, NULL);
	str.resize(length + 1);
	::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), (LPSTR)str.c_str(), length, NULL, NULL);
	return str;
}
