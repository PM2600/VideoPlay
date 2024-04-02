#include "RTSPServer.h"

int RTSPServer::Init(const std::string& strIP, short port)
{
    m_addr.Update(strIP, port);
    m_socket.Bind(m_addr);
    m_socket.Listen();
    return 0;
}

int RTSPServer::Invoke()
{
    m_threadMain.Start();
    
    return 0;
}

void RTSPServer::Stop()
{
    m_socket.Close();
    m_threadMain.Stop();
    m_pool.Stop();
}

RTSPServer::~RTSPServer()
{
    Stop();
}

int RTSPServer::threadWorker()
{
    EAddress clientaddr;
    ESocket client = m_socket.Accept(clientaddr);
    if (client != INVALID_SOCKET) {
        m_clients.PushBack(client);
        m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::ThreadSession));
    }
    return 0;
}

int RTSPServer::ThreadSession()
{
    //接收数据请求，解析请求，应答请求
    ESocket client;
    EBuffer buffer(1024 * 16);
    int len = client.Recv(buffer);
    if (len <= 0) {
        //清理client
        return -1;
    }
    buffer.resize(len);
    RTSPRequest req = AnalyseRequest(buffer);
    RTSPReply ack = MakeReply(req);
    client.Send(ack.toBuffer());
    return 0;
}
