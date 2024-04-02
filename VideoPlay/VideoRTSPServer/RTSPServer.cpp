#include "RTSPServer.h"
#include <rpc.h>

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
        RTSPSession session(client);
        m_lstSession.PushBack(session);
        m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::ThreadSession));
    }
    return 0;
}

int RTSPServer::ThreadSession()
{
    //接收数据请求，解析请求，应答请求
    RTSPSession session;   
    if (m_lstSession.PopFront(session)) {
        return session.PickRequestAndReply();
    }
    return -1;
}

RTSPSession::RTSPSession()
{
    UUID uuid;
    UuidCreate(&uuid);
    m_id.resize(8);
    snprintf((char*)m_id.c_str(), m_id.size(), "%08d", uuid.Data1);
}

RTSPSession::RTSPSession(const ESocket& client) : m_client(client)
{
    UUID uuid;
    UuidCreate(&uuid);
    m_id.resize(8);
    snprintf((char*)m_id.c_str(), m_id.size(), "%08d", uuid.Data1);
}

RTSPSession::RTSPSession(const RTSPSession& session)
{
    m_id = session.m_id;
    m_client = session.m_client;
}

RTSPSession& RTSPSession::operator=(const RTSPSession& session)
{
    if (this != &session) {
        m_id = session.m_id;
        m_client = session.m_client;
    }
    return *this;
}

int RTSPSession::PickRequestAndReply()
{
    EBuffer buffer = Pick();
    if (buffer.size() <= 0) {
        return -1;
    }
    RTSPRequest req = AnalyseRequest(buffer);
    if (req.method() < 0) {
        TRACE("buffer[%s]\r\n", (char*)buffer);
        return -2;
    }
    RTSPReply rep = Reply(req);
    int ret = m_client.Send(rep.toBuffer());
    if (ret < 0)
        return ret;

    return 0;
}

EBuffer RTSPSession::PickOneLine(EBuffer& buffer)
{
    if (buffer.size() <= 0) return EBuffer();
    EBuffer result, temp;
    int i = 0;
    for (; i < buffer.size(); i++) {
        result += buffer.at(i);
        if (buffer.at(i) == '\n') 
            break;
    }
    temp = i + (char*)buffer;
    buffer = temp;
    return result;
}

EBuffer RTSPSession::Pick()
{
    EBuffer result;
    int ret = 1;
    EBuffer buf(1);
    while (ret > 0) {
        buf.Zero(); //内存值置0，不会改变大小
        ret = m_client.Recv(buf);
        if (ret > 0) {
            result += buf;
            if (result.size() >= 4) {
                UINT val = *(UINT*)(result.size() - 4 + (char*)result);
                if (val == *(UINT*)"\r\n\r\n") {
                    break;
                }
            }
        }
    }
    return result;
}

RTSPRequest RTSPSession::AnalyseRequest(const EBuffer& buffer)
{
    EBuffer data = buffer;
    RTSPRequest request;
    EBuffer line = PickOneLine(data);
    EBuffer method(32), url(1024), version(16), seq(64);
    if (sscanf(line, "%s %s %s\r\n", (char*)method, (char*)url, (char*)version) < 3) {
        TRACE("Error at: [%s]\r\n", (char*)line);
        return request;
    }
    line = PickOneLine(data);
    if (sscanf(line, "CSeq: %s\r\n", (char*)seq) < 1) {
        TRACE("Error at: [%s]\r\n", (char*)line);
        return request;
    }
    request.SetMethod(method);
    request.SetUrl(url);
    request.SetSequence(seq);
    if ((strcmp(method, "OPTIONS") == 0) || (strcmp(method, "DESCRIBE") == 0)) {
        return request;
    }
    else if (strcmp(method, "SETUP") == 0) {
        line = PickOneLine(data);
        int port[2] = { 0,0 };
        if (sscanf(line, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n", port, port + 1) == 2) {
            request.SetClientPort(port);
            return request;
        }
    }
    else if ((strcmp(method, "PLAY") == 0) || (strcmp(method, "TEARDOWN") == 0)) {
        line = PickOneLine(data);
        EBuffer session(64);
        if (sscanf(line, "Session: %s\r\n", (char*)session) == 1) {
            request.SetSession(session);
            return request;
        }
    }
    return request;
}

RTSPReply RTSPSession::Reply(const RTSPRequest& request)
{
    RTSPReply reply;
    reply.SetSequence(request.sequence());
    if (request.session().size() > 0) {
        reply.SetSession(request.session());
    }
    switch (request.method()) {
    case 0: //OPTIONS
        reply.SetOptions("Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n");
        break;
    case 1: //DESCRIBE
    {
        EBuffer sdp;
        sdp << "v=0\r\n";
        sdp << "o=- " << m_id << " 1 IN IP4 127.0.0.1\r\n";
        sdp << "t=0 0\r\n" << "a=control:*\r\n" << "m=video 0 RTP/AVP 96\r\n";
        sdp << "a=rtpmap:96 H264/90000\r\n" << "a=control:track0\r\n";
        reply.SetSdp(sdp);
    }
        break;
    case 2: //SETUP
        reply.SetClientPort(request.port(0), request.port(1));
        reply.SetServerPort("55000", "55001");
        break;
    case 3: //PLAY
    case 4: //TEARDOWN
        break;
    }
    return reply;
}

RTSPRequest::RTSPRequest()
{
    m_method = -1;
}

RTSPRequest::RTSPRequest(const RTSPRequest& protocol)
{
    m_method = protocol.m_method;
    m_url = protocol.m_url;
    m_session = protocol.m_session;
    m_seq = protocol.m_seq;
    m_client_port[0] = protocol.m_client_port[0];
    m_client_port[1] = protocol.m_client_port[1];
}

RTSPRequest& RTSPRequest::operator=(const RTSPRequest& protocol)
{
    if (this != &protocol) {
        m_method = protocol.m_method;
        m_url = protocol.m_url;
        m_session = protocol.m_session;
        m_seq = protocol.m_seq;
        m_client_port[0] = protocol.m_client_port[0];
        m_client_port[1] = protocol.m_client_port[1];
    }
    return *this;
}

void RTSPRequest::SetMethod(const EBuffer& method)
{
    if (method == "OPTIONS") m_method = 0;
    else if (method == "DESCRIBE") m_method = 1;
    else if (method == "SETUP") m_method = 2;
    else if (method == "PLAY") m_method = 3;
    else if (method == "TEARDOWN") m_method = 4;
}

void RTSPRequest::SetUrl(const EBuffer& url)
{
    m_url = url;
}

void RTSPRequest::SetSequence(const EBuffer& seq)
{
    m_seq = seq;
}

void RTSPRequest::SetClientPort(int ports[])
{
    m_client_port[0] << ports[0];
    m_client_port[1] << ports[1];
}

void RTSPRequest::SetSession(const EBuffer& session)
{
    m_session = session;
}

