
#include <assert.h>
#include "TcpAcceptor.h"
#include "SocketHandler.h"

#define MAX_CONNECT_LEN 1204

TcpAcceptor::TcpAcceptor( SocketHandler* handler)
: Socket(handler)
, m_enableAccept(true)
, m_bindIP(-1)
, m_bindPort(0)
{
	m_org_socket = CreateSocket(AF_INET, SOCK_STREAM);
}

TcpAcceptor::~TcpAcceptor()
{
}

void TcpAcceptor::_Update()
{
	if (m_org_socket == -1)
	{
		return;
	}

	timeval timeout;

	timeout.tv_sec	= 0;
	timeout.tv_usec = 0;

	fd_set readfds;

	FD_ZERO(&readfds);

	FD_SET(m_org_socket, &readfds);
	
	select(0, &readfds, 0, 0, &timeout);

	if(m_enableAccept && FD_ISSET(m_org_socket, &readfds))
	{
		sockaddr_in addrRemote;
		int nAddrLen = sizeof(addrRemote);
		SOCKET sNew = accept(m_org_socket, (SOCKADDR*)&addrRemote, &nAddrLen);

		Session* pSession = new Session(NewSessionID(), sNew, this, m_bindIP, m_bindPort, ip_s2i(inet_ntoa(addrRemote.sin_addr)), ntohs(addrRemote.sin_port));

		AddSession(pSession);
	}
}

void TcpAcceptor::Bind( const char* ip, unsigned short port )
{ 
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = inet_addr(ip);

	int res = bind(m_org_socket, (sockaddr*)&sin, sizeof(sin));
	if (res == SOCKET_ERROR)
	{
		UTL_LOG_HIGH("sne", "Bind Failed! errno = %d\n", WSAGetLastError());
	}

	assert(res != SOCKET_ERROR);

	res = listen(m_org_socket, MAX_CONNECT_LEN);

	if (res == SOCKET_ERROR)
	{
		UTL_LOG_HIGH("sne", "Listen Failed!\n");
	}

	assert(res == 0);	

	m_bindIP		= ip_s2i(ip);
	m_bindPort    = port;

}

void TcpAcceptor::EnableAccept( bool enable )
{
	if (m_enableAccept == enable)
	{
		return;
	}

	m_enableAccept = enable;

	if (enable)
	{
		if (m_org_socket == -1)
		{
			m_org_socket = CreateSocket(AF_INET, SOCK_STREAM);
			if (m_bindIP != -1 && m_bindPort != 0)
			{
				Bind(ip_i2s(m_bindIP), m_bindPort);
			}

			UTL_LOG_HIGH("sne", "enable accept, recreate socket, new socket=[%d]", m_org_socket);
		}
	}
	else
	{
		if (m_org_socket >= 0)
		{
			UTL_LOG_HIGH("sne", "disable accept, close socket[%d]", m_org_socket);

			closesocket(m_org_socket);
			m_org_socket = -1;
		}
	}
}