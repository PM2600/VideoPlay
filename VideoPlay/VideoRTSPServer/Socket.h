#pragma once
#include <WinSock2.h>
#include <share.h>
#pragma warning(disable:6031)

class Socket
{
public:
	Socket(bool bIsTcp = true) { //0ÊÇTCP£¬1ÊÇUDP
		m_sock = INVALID_SOCKET;
		if (bIsTcp) {
			m_sock = socket(PF_INET, SOCK_STREAM, 0);
		}
		else {
			m_sock = socket(PF_INET, SOCK_DGRAM, 0);
		}
	}
	operator SOCKET() {
		return m_sock;
	}
	Socket(SOCKET s) {
		m_sock = s;
	}
	void Close() {
		if (m_sock != INVALID_SOCKET) {
			SOCKET temp = m_sock;
			m_sock = INVALID_SOCKET;
			closesocket(temp);
		}
	}
	~Socket() {
		Close();
	}
private:
	SOCKET m_sock;
};

class ESocket {
public:
	ESocket(bool isTcp = true):m_socket(new Socket(isTcp)) {}
	ESocket(const ESocket& sock) :m_socket(sock.m_socket){}
	ESocket& operator=(const ESocket& sock) {
		if (this != &sock) {
			m_socket = sock.m_socket;
		}
		return *this;
	}
	~ESocket() {
		m_socket.reset();
	}
	operator SOCKET() {
		return *m_socket;
	}
private:
	std::shared_ptr<Socket>m_socket;
};

class SocketIniter {
public:
	SocketIniter() {
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	}
	~SocketIniter() {
		WSACleanup();
	}
};
