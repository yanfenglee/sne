#pragma once

#include "Socket.h"

class UdpPeer : public Socket
{
public:
	UdpPeer(SocketHandler* handler);
	~UdpPeer();

	void _Update();

	void Bind(const char* ip, short port);
	void Connect(const char* ip, unsigned short port);
	bool Send(Packet* pkt);

	int SendTo(const char* ip, unsigned short port, Packet* pkt, bool broadcast = false);

	bool IsConnected();

private:

	int m_udp_socket;
	int m_local_ip;
	short m_local_port;

	char m_tempBuf[SOCK_READ_SIZE];

	unsigned int m_send_hbt_time;

};