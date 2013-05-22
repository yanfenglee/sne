#pragma once

#include "NetUtil.h"
#include "Socket.h"

class TcpAcceptor : public Socket
{
public:
	TcpAcceptor(SocketHandler* handler);
	~TcpAcceptor();

	void _Update();
	void Bind(const char* ip, unsigned short port);

	void EnableAccept(bool enable);

private:
	int				m_bindIP;
	unsigned short	m_bindPort;

	bool			m_enableAccept;

};