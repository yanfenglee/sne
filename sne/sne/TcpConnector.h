#pragma once

#include "Socket.h"
#include "NetUtil.h"

class TcpConnector : public Socket
{
public:
	TcpConnector(SocketHandler* handler);
	~TcpConnector();

	void _Update();
	void Connect(const char* ip, unsigned short port);

	void Reset(bool force = false);

	bool IsConnected();

	bool Write(Packet* pkt);

	int GetPing();

private:
};