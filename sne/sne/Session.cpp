#include "Session.h"
#include "NetUtil.h"
#include "Packet.h"
#include "Socket.h"

#define HEARTBEAT_TIMEOUT (10 * 1000)
#define HEARTBEAT_SEND_INTERVAL (15 * 1000)


void Session::Write( Packet* pkt )
{
	int psz = pkt->GetDataSize() + 4 + 2;
	short id = pkt->GetID();

	Buffer* buf = new Buffer(psz);
	buf->putInt(psz);
	buf->putShort(id);

	pkt->Encode(buf);

	m_sendque.enqueue(buf);

	Update();
}

void Session::SendHeartbeat()
{
	Buffer* buf = new Buffer(6);
	buf->putInt(6);
	buf->putShort(HEARTBEAT_ID);

	m_sendqueHighPriority.enqueue(buf);
}

void Session::SendHeartbeatReply()
{
	Buffer* buf = new Buffer(6);
	buf->putInt(6);
	buf->putShort(HEARTBEAT_REPLY_ID);

	m_sendqueHighPriority.enqueue(buf);
}

void Session::OnRecv( const char* data, int len )
{
	if (GetState() == CLOSED)
	{
		UTL_LOG_HIGH("sne", "receive message when close");
		return;
	}

	int need_write = len;
	int offset = 0;

	while (need_write > 0)
	{
		//header receive
		if (m_recv_packet_fragment_offset < 4)
		{
			int nwn = 4 - m_recv_packet_fragment_offset;
			int wn = (need_write >= nwn ? nwn : need_write);

			memcpy(m_recv_packet_fragment_buf + m_recv_packet_fragment_offset, data+offset, wn);
			m_recv_packet_fragment_offset += wn;
			need_write -= wn;
			offset += wn;
		}

		//header receive haven't finished yet
		if (m_recv_packet_fragment_offset < 4)
		{
			return;
		}

		//content receive
		int pkt_len = ntohl(*(int*)m_recv_packet_fragment_buf);
		if (pkt_len > PACKET_BUFFER_SIZE || pkt_len < 6)
		{
			UTL_LOG_HIGH("sne", "error packet length: %d", pkt_len);

			Close();

			return;
		}

		if (m_recv_packet_fragment_offset + need_write >= pkt_len)
		{
			//one packet receive finished
			int writesize = pkt_len - m_recv_packet_fragment_offset;

			memcpy(m_recv_packet_fragment_buf + m_recv_packet_fragment_offset, data+offset, writesize);
			m_recv_packet_fragment_offset = 0;
			need_write -= writesize;
			offset += writesize;

			//check if there is heartbeat reply
			short packetID = ntohs(*((short*)(m_recv_packet_fragment_buf+4)));

			if (packetID == HEARTBEAT_ID)
			{
				SendHeartbeatReply();
			}
			else if (packetID == HEARTBEAT_REPLY_ID)
			{
				//clock time;
				m_last_recv_time = getTime();

				m_last_ping_time = m_last_recv_time - m_heartbeat_send_time;

				UTL_LOG_NORMAL("sne", "ping return, sid=[%d], cost time: %d", GetID(), m_last_ping_time);
			}
			else
			{
				Buffer* newbuf = new Buffer(pkt_len-4);
				newbuf->putRaw(m_recv_packet_fragment_buf+4, pkt_len-4);
				m_recvque.enqueue(newbuf);
			}

		}
		else
		{
			memcpy(m_recv_packet_fragment_buf + m_recv_packet_fragment_offset, data+offset, need_write);
			m_recv_packet_fragment_offset += need_write;
			need_write = 0;
			offset += need_write;
		}
	}

}

int Session::SendFromBuf()
{
	if (m_send_packet_size == 0)
	{
		Buffer* buf = NULL;
		if (m_sendqueHighPriority.dequeue(buf))
		{

		}
		else if (m_sendque.dequeue(buf))
		{

		}

		if (buf != NULL)
		{
			m_send_packet_size = buf->getSize();

			buf->getRaw(m_send_packet_fragment_buf, m_send_packet_size);
			m_send_packet_fragment_offset = 0;

			delete buf;
		}
	}

	if (m_send_packet_size == 0)
	{
		//send finished
		return 0;
	}
	else
	{
		int sn = send(m_socket, 
			m_send_packet_fragment_buf + m_send_packet_fragment_offset,
			m_send_packet_size, 0);

		if (sn > 0)
		{
			m_send_packet_size -= sn;
			m_send_packet_fragment_offset += sn;
		}

		return sn;

	}

}

void Session::Close()
{
	SetState(CLOSED);
}

int Session::GetID()
{
	return m_id;
}

bool Session::IsClosed()
{
	return (m_state == CLOSED);
}

Session::Session(int id, int sock, Socket* pSocket, int local_ip, unsigned short local_port, int remote_ip, unsigned short remote_port):
m_id(id),
m_socket(sock),
m_local_ip(local_ip),
m_local_port(local_port),
m_remote_ip(remote_ip),
m_remote_port(remote_port),
m_state(CONNECTED), 
m_recv_packet_fragment_offset(0),
m_send_packet_fragment_offset(0),
m_send_packet_size(0),
m_release(false),
m_enbalehbt(true)
{
	m_pSocket = pSocket;
	m_last_ping_time = -1;
}

Session::~Session()
{
	if (m_socket != -1)
	{
		closesocket(m_socket);
		UTL_LOG_NORMAL("sne", "close socket=[%d]", m_socket);
	}
}

int Session::GetPing()
{
	return m_last_ping_time;
}

bool Session::Select()
{
	timeval timeout;

	timeout.tv_sec	= 0;
	timeout.tv_usec = 0;

	fd_set writefds;
	fd_set readfds;
	fd_set exceptfds;

	FD_ZERO(&writefds);
	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);
	
	FD_SET(m_socket, &writefds);
	FD_SET(m_socket, &readfds);
	FD_SET(m_socket, &exceptfds);


	int ret = select(0, &readfds, &writefds, &exceptfds, &timeout);
	if (ret > 0)
	{
		//check read
		if(FD_ISSET(m_socket, &readfds))
		{
			int nRecv = recv(m_socket, m_tempBuf, SOCK_READ_SIZE, 0);
			if(nRecv > 0)                        
			{
				OnRecv(m_tempBuf, nRecv);
				return true;
			}
			else if (nRecv == 0)
			{
				UTL_LOG_HIGH("sne", "recv return 0, session will close");
				Close();
				return false;
			}
			else if (nRecv == WSAEWOULDBLOCK)
			{
				UTL_LOG_HIGH("sne", "recv WSAEWOULDBLOCK");
				return false;
			}
			else
			{
				Close();
				UTL_LOG_HIGH("sne", "recv error: %d, session closed", WSAGetLastError());
				return false;
			}
		}

		//check write
		if(FD_ISSET(m_socket, &writefds))
		{
			bool havadata = !(m_sendque.empty() && m_sendqueHighPriority.empty() && m_send_packet_size == 0);
			if (havadata)
			{
				int sn = SendFromBuf();
				if (sn > 0)
				{
					return true;
				}
				else if (sn == 0)
				{
					return false;
				}
				else 
				{
					Close();
					UTL_LOG_HIGH("sne", "send data error: %d", WSAGetLastError());
					return false;
				}
			}
			
			return false;
		}

		//check error
		if(FD_ISSET(m_socket, &exceptfds))
		{
			UTL_LOG_HIGH("sne", "except error: %d", WSAGetLastError());
			return false;
		}

		return true;
	}
	else
	{
		if (ret == SOCKET_ERROR)
		{
			UTL_LOG_HIGH("sne", "select return error: %d", WSAGetLastError());
		}

		return false;
	}
}

void Session::Update()
{	
	m_mutex.Lock();

	if (!IsClosed())
	{
		if (m_enbalehbt)
		{
			//check heartbeat status, send heartbeat
			unsigned int tnow = getTime();

			if ((m_heartbeat_send_time > m_last_recv_time) && 
				(tnow - m_heartbeat_send_time > HEARTBEAT_TIMEOUT))
			{
				UTL_LOG_HIGH("sne", "heartbeat ³¬Ê±¶ÏÏß, session id=[%d]", m_id);
				Close();
			}

			if (tnow - m_heartbeat_send_time > HEARTBEAT_SEND_INTERVAL)
			{
				SendHeartbeat();

				m_heartbeat_send_time = tnow;
			}
		}
		

		while (m_state == CONNECTED)
		{
			bool havadata = !(m_sendque.empty() && m_sendqueHighPriority.empty() && m_send_packet_size == 0);
			bool ret = Select();

			if (!ret || !havadata)
			{
				break;
			}
		}
	}

	m_mutex.Unlock();
}

//===================================================================================================
bool SessionTask::DoTask()
{	
	m_session->Update();

	return m_session->IsRelease();
}

SessionTask::SessionTask( Session* ss )
: m_session(ss)
{

}

SessionTask::~SessionTask()
{
	delete m_session;
}
