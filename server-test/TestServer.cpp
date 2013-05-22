#include "../sne/sne/NetUtil.h"
//#include "../sne/sne/UdpPeer.h"
#include "../sne/sne/SocketHandler.h"
#include "../sne/sne/Packet.h"
#include "../sne/sne/PacketFactoryMgr.h"
#include "../sne/sne/TcpAcceptor.h"
#include "../packets.h"
#include "../file/File.h"
#include "../sne/sne/UdpPeer.h"

#include <time.h>


class KTimer
{
public:
	KTimer()
	{
		QueryPerformanceFrequency(&tc);
		reset();
		m_curr = getEclipse();
	}

	double getEclipse()
	{
		QueryPerformanceCounter(&t2);
		double dTotalTime = (double)(t2.QuadPart-t1.QuadPart) / (double)tc.QuadPart;
		return dTotalTime;
	}

	double getDt()
	{
		double dt = getEclipse() - m_curr;
		m_curr = getEclipse();

		return dt;
	}

	void reset()
	{
		QueryPerformanceCounter(&t1);
	}

private:
	LARGE_INTEGER t1;
	LARGE_INTEGER t2;
	LARGE_INTEGER tc;

	double m_curr;
};

class ServerHandler : public SocketHandler
{
public:
	ServerHandler()
	{
		fp = 0;
		tt = 0;
	}

public:
	void OnSessionOpened(Session* ss)
	{

	}

	void OnSessionClosed(Session* ss)
	{

	}

	void OnMessageRecv( Session* ss, Packet* pkt )
	{
		int pid = pkt->GetID();
		if (pid == ID_HELLO)
		{
			helloPacket* hh = (helloPacket*)pkt;

			printf("receive : %s, index = [%d], from %s:%d\n", hh->hello, hh->index, ip_i2s(ss->m_remote_ip), ss->m_remote_port);

			strcpy(hh->hello, "pong");

			ss->Write(pkt);
			
		}
		else if (pid == ID_FILE)
		{
			printf("receive file...");

			filePacket* fpkt = (filePacket*)pkt;
			if (fp == 0)
			{
				char temp[64];
				sprintf(temp, "H:\\%s", fpkt->filename);
				fp = new File(temp, "wb");
				tt = clock();
				printf("-------------------file : %s transfer begin----------------------", fpkt->filename);

				timer.reset();
			}

			fp->fwrite(fpkt->buf, 1, fpkt->len);

			printf("file : %s receive %d bytes, block: %d\n", fpkt->filename, fpkt->len, fpkt->block_index);

			if (fpkt->len < sizeof(fpkt->buf))
			{
				fp->fclose();
				printf("-------------------file : %s transfer finished, cost time: %f-------------------", fpkt->filename, timer.getEclipse());

				delete fp;
				fp = 0;

				helloPacket hh;
				strcpy(hh.hello, "file finish");

				ss->Write(&hh);
			}
		}
	}

private:

	File* fp;
	int tt;
	KTimer timer;
};

#if 1 //tcp
int main()
{
	//log("======= Select Model Server ========\n");
	
	SocketHandler* serverHandler = new ServerHandler;

	TcpAcceptor* Server = new TcpAcceptor(serverHandler);
	Server->Bind("0.0.0.0", 5566);

	REGISTER_PACKET(helloPacket);
	REGISTER_PACKET(filePacket);
	
	while(1)
	{
		Server->UpdateAll();
		Sleep(0);
	}
}

#else
//udp
int main()
{
	//log("======= udp Server ========\n");

	SocketHandler* serverHandler = new ServerHandler;

	PacketFactory* serverFactory = new packetsFactory;

	UdpPeer* Server = new UdpPeer(serverHandler,serverFactory);
	Server->Bind("0.0.0.0", 5566);

	while(1)
	{
		Server->UpdateAll();
		//Sleep(100);
		//Server->SendData("127.0.0.1",5566,"hello ss",9);
	}
}
#endif