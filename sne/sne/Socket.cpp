#include "NetConst.h"
#include "Socket.h"
#include "NetUtil.h"
#include <Winsock2.h>
#include <assert.h>
#include "NetConst.h"
#include "PacketFactoryMgr.h"
#include <vector>
#include "adns.h"

#define MULTI_THREAD_ENABLE

#ifdef MULTI_THREAD_ENABLE //启用多线程
#include "ThreadMgr.h"

#define ADD_SESSION_TASK(ss) { ThreadMgr::Get()->AddTask(new SessionTask(ss)); }
#define THREAD_RELEASE(p) { if(p) (p)->Release(); (p) = 0; }

#else //禁用多线程

#define ADD_SESSION_TASK(ss)
#define THREAD_RELEASE(p) { if (p) delete (p); (p) = 0; }

#endif

enum
{
	DNS_LOOKUP_INIT,
	DNS_LOOKUP_RUNNING,
	DNS_LOOKUP_FINI,
};


Socket::Socket( SocketHandler* handler)
: m_handler(handler)
, m_org_socket(-1)
, m_needUpdateSessionState(false)
, m_session_count(0)
, m_dnsLookupState(DNS_LOOKUP_INIT)
, m_remoteIP(0)
, m_remotePort(0)
, m_dns(NULL)
{	
	//初始化winsock库
	WSADATA wsadata;
	WORD version = MAKEWORD(2,2);

	int ret = WSAStartup(version, &wsadata);
	if(ret != 0)
	{
		UTL_LOG_HIGH("sne", "Winsock Initialize Failed\n");
	}

	assert(ret == 0);

	m_dns = Adns::New();
}

Socket::~Socket()
{
	m_handler = NULL;

	Adns::Delelte(m_dns);
	m_dns = NULL;

	m_session_count = 0;

	Clear();

	if (m_org_socket != -1)
	{
		closesocket(m_org_socket);
	}

	WSACleanup();
}

int Socket::CreateSocket( int af, int type )
{
	int s = socket(af, type, 0);
	if (s == INVALID_SOCKET)
	{
		UTL_LOG_HIGH("sne", "create socket error");
		return -1;
	}

	int on = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0)
	{
		UTL_LOG_HIGH("sne", "setsockopt(SO_REUSEADDR) failed");
	}

	if (type == SOCK_STREAM)
	{
		//禁用nagle算法
		const char chOpt=1;   
		int   nErr=setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));   
		if(nErr==-1)   
		{   
			UTL_LOG_HIGH("sne", "禁用nalge算法失败"); 
		} 
	}


	int sock_opt = 1;
	// This doubles the max throughput rate
	sock_opt=1024*256;
	setsockopt(s, SOL_SOCKET, SO_RCVBUF, ( char * ) & sock_opt, sizeof ( sock_opt ) );

	// Immediate hard close. Don't linger the socket, or recreating the socket quickly on Vista fails.
	// Fail with voice and xbox

	sock_opt=0;
	setsockopt(s, SOL_SOCKET, SO_LINGER, ( char * ) & sock_opt, sizeof ( sock_opt ) );


	// This doesn't make much difference: 10% maybe
	// Not supported on console 2
	sock_opt=1024*16;
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, ( char * ) & sock_opt, sizeof ( sock_opt ) );


	//set socket Non-block
	unsigned long cmd;
	int nStatus;

	nStatus = ioctlsocket(s, FIONBIO, &cmd);
	if (nStatus == SOCKET_ERROR)
	{
		UTL_LOG_HIGH("sne", "Error: set Non-block socket Failed.\n");
	}

	assert(nStatus == 0);

	UTL_LOG_NORMAL("sne", "create socket=[%d]", s);

	return s;
}

void Socket::_Update()
{

}

Session* Socket::GetSession( int id )
{
	TSession::iterator it = m_sessions.find(id);
	if (it != m_sessions.end())
	{
		return it->second;
	}
	else
	{
		UTL_LOG_HIGH("sne", "cann't find session: %d", id);
		return 0;
	}
}

void Socket::AddSession( Session* ss )
{
	TSession::iterator it = m_sessions.find(ss->GetID());
	if (it != m_sessions.end())
	{
		UTL_LOG_HIGH("sne", "already have session: %d, will replaced", ss->GetID());
		RemoveSession(ss->GetID());
	}

	ss->m_last_recv_time = getTime();
	ss->m_heartbeat_send_time = getTime();

	m_sessions[ss->GetID()] = ss;

	UTL_LOG_NORMAL("sne", "session opened: id: %d, fd: %d, local ip: %s, local port: %d, remote ip: %s, remote port: %d", 
		ss->GetID(), ss->m_socket, ip_i2s(ss->m_local_ip), ss->m_local_port, ip_i2s(ss->m_remote_ip), ss->m_remote_port);

	if (m_handler != NULL)
	{
		m_handler->OnSessionOpened(ss);
	}


	ADD_SESSION_TASK(ss);

}

void Socket::RemoveSession( int id )
{
	Session* ss = GetSession(id);
	if (ss)
	{
		m_sessions.erase(id);

		if (ss->m_socket == m_org_socket)
		{
			m_org_socket = -1;
		}

		UTL_LOG_NORMAL("sne", "session closed: id: %d, fd: %d, local ip: %s, local port: %d, remote ip: %s, remote port: %d", 
			ss->GetID(), ss->m_socket, ip_i2s(ss->m_local_ip), ss->m_local_port, ip_i2s(ss->m_remote_ip), ss->m_remote_port);

		if (m_handler != NULL)
		{
			m_handler->OnSessionClosed(ss);
		}

		THREAD_RELEASE(ss);
	}
}

void Socket::UpdateAll()
{
	UpdateDNS();

	// remove closed session
	_UpdateSessionState();
	
	// detect new connections
	_Update();

//#ifndef MULTI_THREAD_ENABLE	
	// process read & write
	_ProcessReadWrite();
//#endif


	// dispatch messages
	_Dispatch();
}

void Socket::_Dispatch()
{
	TSession::iterator it = m_sessions.begin();
	for (; it != m_sessions.end(); ++it)
	{
		Session* ss = it->second;
		Buffer* buf;
		while (ss->m_recvque.dequeue(buf))
		{
			short id = buf->getShort();

			Packet* pkt = PacketFactoryMgr::Get()->CreatePacket(id);

			if (pkt)
			{
				pkt->Decode(buf);

				if (m_handler != NULL)
				{
					m_handler->OnMessageRecv(ss, pkt);
				}

				PacketFactoryMgr::Get()->DestroyPacket(pkt);  
			}
			else
			{
				UTL_LOG_HIGH("sne", "unknown packet, id=[%d]", id);
			}

			delete buf;
		}
	}

	if (m_needUpdateSessionState)
	{
		_UpdateSessionState();
		m_needUpdateSessionState = false;
	}
}

void Socket::_UpdateSessionState()
{
	std::vector<int> removedSessions;
	removedSessions.clear();

	unsigned int tnow = getTime();

	TSession::iterator it = m_sessions.begin();
	for (; it != m_sessions.end(); ++it)
	{
		Session* ss = it->second;

		if (ss->IsClosed())
		{
			removedSessions.push_back(ss->GetID());
		}
	}

	for (size_t i = 0; i < removedSessions.size(); ++i)
	{
		RemoveSession(removedSessions[i]);
	}

}

void Socket::_ProcessReadWrite()
{
	TSession::iterator it = m_sessions.begin();
	for (; it != m_sessions.end(); ++it)
	{
		Session* ss = it->second;
		ss->Update();
	}
}

int Socket::GetSessionNum()
{
	return m_sessions.size();
}

void Socket::Clear()
{
	TSession::iterator it = m_sessions.begin();
	for (; it != m_sessions.end(); ++it)
	{
		Session* ss = it->second;
		ss->Close();
	}

	_UpdateSessionState();
}

void Socket::Reset(bool force)
{
	TSession::iterator it = m_sessions.begin();
	for (; it != m_sessions.end(); ++it)
	{
		Session* ss = it->second;
		ss->Close();
	}

	if (force)
	{
		_UpdateSessionState();
	}
	else
	{
		m_needUpdateSessionState = true;
	}
}

int Socket::NewSessionID()
{
	++m_session_count;

	return m_session_count;
}

void Socket::CloseSessions( int* sessions, int num, bool keep /*= false*/ )
{
	if (keep)
	{
		for (int i = 0; i < num; ++i)
		{
			if (!GetSession(sessions[i]))
			{
				UTL_LOG_HIGH("sne", "this session is not exist: %d", sessions[i]);
			}
		}

		TSession::iterator it = m_sessions.begin();
		for (; it != m_sessions.end(); ++it)
		{
			Session* ss = it->second;
			
			bool closeit = true;
			for (int i = 0; i < num; ++i)
			{
				if (ss->GetID() == sessions[i])
				{
					closeit = false;
				}
			}

			if (closeit)
			{
				ss->Close();
			}
		}
	}
	else
	{
		for (int i = 0; i < num; ++i)
		{
			Session* ss = GetSession(sessions[i]);
			if (ss)
			{
				ss->Close();
			}
		}
	}

	_UpdateSessionState();
}

void Socket::Connect(const char* ip, unsigned short port)
{
	UTL_ASSERT(false, "Connect not implement yet!");
}

void Socket::ResolveAndConnect(const char* hostname)
{
	//m_dnsLookupState = DNS_LOOKUP_RUNNING;

	m_dns->StartLookup(hostname, ConnectCb, this);
}

void Socket::UpdateDNS()
{
	m_dns->Update();
}

void Socket::ConnectCb( const char* ip, void* arg )
{
	Socket* skt = (Socket*)arg;
	skt->Connect(ip, skt->m_remotePort);
}