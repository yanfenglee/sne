#include "UdpPeer.h"
#include <assert.h>
#include "NetUtil.h"
#include "Packet.h"
#include "PacketFactoryMgr.h"

#define UDP_HBT_TIME 5000

UdpPeer::UdpPeer( SocketHandler* handler)
: Socket(handler)
{
	m_org_socket = CreateSocket(AF_INET, SOCK_DGRAM);
	m_udp_socket = CreateSocket(AF_INET, SOCK_DGRAM);
	m_send_hbt_time = 0;
}

UdpPeer::~UdpPeer()
{
	if (m_udp_socket != -1)
	{
		closesocket(m_udp_socket);
		m_udp_socket = -1;
	}
}

void UdpPeer::_Update()
{
	if (m_udp_socket == -1)
	{
		return;
	}

	unsigned int tnow = getTime();

	// keep udp alive
	if (tnow - m_send_hbt_time > UDP_HBT_TIME)
	{
		TSession::iterator it = m_sessions.begin();
		for (; it != m_sessions.end(); ++it)
		{
			it->second->SendHeartbeat();
		}

		m_send_hbt_time = tnow;
	}


	fd_set readfds;
	timeval timeout;

	timeout.tv_sec	= 0;
	timeout.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(m_udp_socket, &readfds);

	int ret = select(0, &readfds, 0, 0, &timeout);

	if (ret > 0)
	{
		if(FD_ISSET(m_udp_socket, &readfds))
		{
			sockaddr_in addrRemote;
			int nAddrLen = sizeof(addrRemote);

			int nRecv = recvfrom(m_udp_socket, m_tempBuf, SOCK_READ_SIZE, 0, (SOCKADDR*)&addrRemote, &nAddrLen);

			int remote_ip = ip_s2i(inet_ntoa(addrRemote.sin_addr));
			unsigned short remote_port = ntohs(addrRemote.sin_port);

			if(nRecv > 0)                        
			{
				Buffer* buf = new Buffer(nRecv);
				buf->putRaw(m_tempBuf, nRecv);

				int pktlen = buf->getInt();

				if (nRecv != pktlen)
				{
					UTL_LOG_HIGH("sne", "udp receive packet length error");
					delete buf;
					return;
				}

				short id = buf->getShort();
				
				Packet* pkt = PacketFactoryMgr::Get()->CreatePacket(id);
				if (!pkt)
				{
					UTL_LOG_HIGH("sne", "udp receive unknown packet id = [%d]", id);
					delete buf;
					return;
				}

				pkt->Decode(buf);

				Session ssss(-1, -1, this, m_local_ip, m_local_port, remote_ip, remote_port);
				m_handler->OnMessageRecv(&ssss, pkt);

				delete pkt;
				delete buf;
			}
			else
			{
				UTL_LOG_HIGH("sne", "udp receive error, errno=[%d]", WSAGetLastError());

				closesocket(m_udp_socket);
				m_udp_socket = CreateSocket(AF_INET, SOCK_DGRAM);

				Bind(ip_i2s(m_local_ip), m_local_port);
			}
		}
	}
}

void UdpPeer::Bind( const char* ip, short port)
{
	sockaddr_in local;

	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.S_un.S_addr = inet_addr(ip);


	int res = bind(m_udp_socket, (sockaddr*)&local, sizeof(local));

	if (res == SOCKET_ERROR)
	{
		UTL_LOG_HIGH("sne", "Bind Failed! error: %d", WSAGetLastError());
	}

	assert(res != SOCKET_ERROR);

	m_local_ip = ip_s2i(ip);
	m_local_port = port;

}

int UdpPeer::SendTo(const char* ip, unsigned short port, Packet* pkt, bool broadcast)
{
	sockaddr_in remote;
	char data[MAX_PACKET_LENGTH];
	int len = pkt->GetDataSize()+4+2;
	short id = pkt->GetID();

	Buffer* buf = new Buffer(len);

	buf->putInt(len);
	buf->putShort(id);
	pkt->Encode(buf);

	buf->getRaw(data, len);

	remote.sin_family = AF_INET;
	remote.sin_port =htons(port);

	int on = 1;

	if (!broadcast)
	{
		on = 0;
		remote.sin_addr.S_un.S_addr = inet_addr(ip);
	}
	else
	{
		on = 1;
		remote.sin_addr.S_un.S_addr = inet_addr("255.255.255.255");
	}

	if((setsockopt(m_udp_socket,SOL_SOCKET,SO_BROADCAST,
		(const char*)&on,sizeof(on))) == -1)
	{
		UTL_LOG_HIGH("sne", "udp peer setsockopt - SO_SOCKET - error: %d, socket=[%d]", WSAGetLastError(), m_udp_socket);
	}

	int inRet = sendto(m_udp_socket, data, len, 0,(const sockaddr *)&remote, sizeof(remote));//
	if (inRet != len)
	{
		UTL_LOG_HIGH("sne", "udp send error, send len=[%d], ret len=[%d]", len, inRet);
	}
	return inRet;
}

void UdpPeer::Connect( const char* ip, unsigned short port )
{
	m_remotePort	= port;

	if (!isIp(ip))
	{
		ResolveAndConnect(ip);
		return;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = inet_addr(ip);

	m_remoteIP		= ip_s2i(ip);

	if (m_org_socket == -1)
	{
		m_org_socket = CreateSocket(AF_INET, SOCK_DGRAM);
	}

	int res = connect(m_org_socket, (sockaddr*)&sin, sizeof(sin));
	if (res < 0)
	{
		UTL_LOG_HIGH("sne", "socket = [%d], connect error: %d", m_org_socket, WSAGetLastError());
	}
	else
	{
		if (!IsConnected())
		{
			Session* pSession = new Session(NewSessionID(), m_org_socket, this, 0, 0, m_remoteIP, m_remotePort);
			pSession->SetEnableHbt(false);
			AddSession(pSession);
		}
	}
}

bool UdpPeer::Send( Packet* pkt )
{
	if (!IsConnected())
	{
		//UTL_LOG_HIGH("sne", "udp haven't connected!!!");
		return false;
	}

	TSession::iterator it = m_sessions.begin();
	it->second->Write(pkt);

	return true;
}

bool UdpPeer::IsConnected()
{
	return (!m_sessions.empty());
}