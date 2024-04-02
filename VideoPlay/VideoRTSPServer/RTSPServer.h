#pragma once
#include "CQueue.h"
#include "Socket.h"
#include <string>
#include "EdyThread.h"
#include <map>

class RTSPRequest {
public:
	RTSPRequest();
	RTSPRequest(const RTSPRequest& protocol);
	RTSPRequest operator=(const RTSPRequest& protocol);
	~RTSPRequest();
private:
	int m_method;
};

class RTSPReply {
public:
	RTSPReply();
	RTSPReply(const RTSPReply& protocol);
	RTSPReply operator=(const RTSPReply& protocol);
	~RTSPReply();
	EBuffer toBuffer();
private:
	int m_method;
};

class RTSPSession {
public:
	RTSPSession();
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession();
};



class RTSPServer : public ThreadFuncBase
{
public:
	RTSPServer() : m_socket(true), m_status(0), m_pool(10){
		m_threadMain.UpdateWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::threadWorker));
	}

	int Init(const std::string& strIP = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
	~RTSPServer();
protected:
	int threadWorker(); //返回0继续，返回负数终止，返回其他警告
	RTSPRequest AnalyseRequest(const std::string& data);
	RTSPReply MakeReply(const RTSPRequest& request);
	int ThreadSession();
private:
	ESocket m_socket;
	EAddress m_addr;
	int m_status;// 0未初始化 1初始化完成 2正在运行 3关闭
	EdyThread m_threadMain;
	EdyThreadPool m_pool;
	std::map<std::string, RTSPSession> m_mapSessions;
	static SocketIniter m_initer;
	CQueue<ESocket> m_clients;
};