#pragma once
#include "base.h"
#include "Socket.h"
class RTPHeader
{
public:
	unsigned short csrccount : 4;
	unsigned short extension : 1;
	unsigned short padding : 1;
	unsigned short version : 2;
	unsigned short pytype : 7;
	unsigned short mark : 1;
	unsigned short serial;
	unsigned timestamp;
	unsigned ssrc;
	unsigned csrc[15];

public:
	RTPHeader();
	RTPHeader(const RTPHeader& header);
	RTPHeader& operator=(const RTPHeader& header);
	operator EBuffer();
};

class RTPFrame
{
public:
	RTPHeader m_head;
	EBuffer m_pyload;
	operator EBuffer();
};

class RTPHelper
{
public:
	RTPHelper() : timestamp(0), m_udp(false){
		m_udp.Bind(EAddress("0.0.0.0", (short)55000));
	}
	~RTPHelper(){}
	int SendMediaFrame(RTPFrame& rtpframe, EBuffer& frame, const EAddress& client);
private:
	int GetFrameSepSize(EBuffer& frame);
	int SendFrame(const EBuffer& frame, const EAddress& client);
	DWORD timestamp;
	ESocket m_udp;
};

