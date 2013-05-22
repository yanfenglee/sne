
#include "TcpConnector.h"
#include <assert.h>

TcpConnector::TcpConnector( SocketHandler* handler)
: Socket(handler)
{
	Reset();
}


TcpConnector::~TcpConnector()
{

}

void TcpConnector::_Update()
{
	if (m_org_socket == -1)
	{
		return;
	}

	timeval timeout;

	timeout.tv_sec	= 0;
	timeout.tv_usec = 0;

	fd_set writefds;

	FD_ZERO(&writefds);

	FD_SET(m_org_socket, &writefds);

	select(0, 0, &writefds, 0, &timeout);

	if(FD_ISSET(m_org_socket, &writefds))
	{
		if (!IsConnected())
		{
			int error;
			int len = sizeof(int);

			getsockopt(m_org_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len); 
			if (error == 0)
			{
				Session* pSession = new Session(NewSessionID(), m_org_socket, this, 0, 0, m_remoteIP, m_remotePort);
				AddSession(pSession);
				m_org_socket = -1;
			}
		}
	}
}

void TcpConnector::Connect( const char* ip, unsigned short port )
{
	m_remotePort	= port;

	if (!isIp(ip))
	{
		ResolveAndConnect(ip);
		return;
	}
	else
	{
		Reset();
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = inet_addr(ip);

	m_remoteIP		= ip_s2i(ip);

	int res = connect(m_org_socket, (sockaddr*)&sin, sizeof(sin));
	if (res < 0)
	{
		if (WSAGetLastError() !=  WSAEWOULDBLOCK)
		{
			UTL_LOG_HIGH("sne", "socket = [%d], connect error: %d", m_org_socket, WSAGetLastError());
		}
	}
}

bool TcpConnector::IsConnected()
{
	return !m_sessions.empty();
}

void TcpConnector::Reset(bool force)
{
	Socket::Reset(force);

	if (m_org_socket != -1)
	{
		closesocket(m_org_socket);
	}

	m_org_socket = CreateSocket(AF_INET, SOCK_STREAM);
}

bool TcpConnector::Write( Packet* pkt )
{
	if (!IsConnected())
	{
		UTL_LOG_HIGH("sne", "haven't connected!!!");
		return false;
	}

	TSession::iterator it = m_sessions.begin();
	it->second->Write(pkt);

	return true;
}

int TcpConnector::GetPing()
{
	if (m_sessions.empty())
	{
		return -1;
	}

	TSession::iterator it = m_sessions.begin();

	return it->second->GetPing();
}
