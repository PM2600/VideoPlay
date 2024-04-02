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
	void SetMethod(const EBuffer& method);
	void SetUrl(const EBuffer& url);
	void SetSequence(const EBuffer& seq);
	void SetClientPort(int ports[]);
	void SetSession(const EBuffer& session);
	int method() const;
	EBuffer& url() const;
	EBuffer& session() const;
	EBuffer& sequence() const;
	EBuffer& port(int index = 0) const;
private:
	int m_method;
	EBuffer m_url;
	EBuffer m_session;
	EBuffer m_seq;
	EBuffer m_client_port[2];
};

class RTSPReply {
public:
	RTSPReply();
	RTSPReply(const RTSPReply& protocol);
	RTSPReply operator=(const RTSPReply& protocol);
	~RTSPReply();
	EBuffer toBuffer();
	void SetOptions(const EBuffer& options);
	void SetSequence(const EBuffer& seq);
	void SetSdp(const EBuffer& sdp);
	void SetClientPort(const EBuffer& port0, const EBuffer& port1);
	void SetServerPort(const EBuffer& port0, const EBuffer& port1);
	void SetSession(const EBuffer& session);
private:
	int m_method;
	short m_client_port[2];
	short m_server_port[2];
	std::string m_sdp;
	EBuffer m_options;
	EBuffer m_session;
};

class RTSPSession {
public:
	RTSPSession();
	RTSPSession(const ESocket& client);
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession() {}
	int PickRequestAndReply();
private:
	EBuffer PickOneLine(EBuffer& buffer);
	EBuffer Pick();
	RTSPRequest AnalyseRequest(const EBuffer& buffer);
	RTSPReply Reply(const RTSPRequest& request);
private:
	std::string m_id;
	ESocket m_client;
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
	int ThreadSession();
private:
	static SocketIniter m_initer;
	ESocket m_socket;
	EAddress m_addr;
	int m_status;// 0未初始化 1初始化完成 2正在运行 3关闭
	EdyThread m_threadMain;
	EdyThreadPool m_pool;
	CQueue<RTSPSession> m_lstSession;
};