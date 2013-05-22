#include "../sne/sne/NetUtil.h"
//#include "../sne/sne/UdpPeer.h"
#include "../sne/sne/SocketHandler.h"
#include "../sne/sne/Packet.h"
#include "../sne/sne/PacketFactoryMgr.h"
#include "../sne/sne/TcpConnector.h"
#include "../packets.h"
#include "../file/File.h"
#include "../sne/sne/UdpPeer.h"

#pragma comment(lib, "winmm.lib")

class KTimer
{
public:
	KTimer()
	{
		QueryPerformanceFrequency(&tc);
		reset();
	}

	double getEclipse()
	{
		QueryPerformanceCounter(&t2);
		double dTotalTime = (double)(t2.QuadPart-t1.QuadPart) / (double)tc.QuadPart;
		return dTotalTime;
	}

	void reset()
	{
		QueryPerformanceCounter(&t1);
	}

private:
	LARGE_INTEGER t1;
	LARGE_INTEGER t2;
	LARGE_INTEGER tc;
};

class ClientHandler : public SocketHandler
{
	enum
	{
		INIT,
		TRANSFER,
		PING,
	};

public:
	void transferfile(Session* ss)
	{
		char* fn = "testsnefile";

		File file(fn, "rb");

		filePacket fpacket;

		strcpy(fpacket.filename, fn);

		printf("---------start transfer %s-------------", fn);

		int block_size = sizeof(fpacket.buf);

		int rsz = block_size;
		int bidx = 0;
		while (rsz == block_size)
		{
			rsz = file.fread(fpacket.buf, 1, block_size);

			fpacket.block_index = bidx;
			++bidx;

			fpacket.len = rsz;
			ss->Write(&fpacket);

			printf("send bytes : %d, block: %d", rsz, fpacket.block_index);
			Sleep(0);

		}

		printf("---------end  transfer %s-------------", fn);
	}


	void OnSessionOpened(Session* ss)
	{

		//UTL_LOG_HIGH("cdn_test", "start ping, ping index=[%d]", ping_index);

		//ping();

		state = TRANSFER;

		//transferfile(ss);
	}

	void OnSessionClosed(Session* ss)
	{
	}

	void OnMessageRecv( Session* ss, Packet* pkt )
	{
		if (pkt->GetID() == ID_HELLO)
		{
			helloPacket* hpkt = (helloPacket*)pkt;

			if (ping_index == hpkt->index)
			{				
				double dt = kt.getEclipse();
				total_count++;
				time_sum += dt;

				//UTL_LOG_HIGH("cdn_test", "receive : %s, index=[%d], time cost=[%d], precise time=[%f]", hpkt->hello, hpkt->index, timeGetTime() - ping_t0, dt);
				char temp[128];
				sprintf(temp, "statics, %f, ", dt);

				UTL_LOG_HIGH(temp, "receive : %s, index=[%d], time cost=[%d], precise time=[%f]", hpkt->hello, hpkt->index, timeGetTime() - ping_t0, dt);

				Sleep(1000);

				ping();
			}
			else
			{
				UTL_LOG_HIGH("cdn_test", "receive timeout ping: %s, index=[%d]", hpkt->hello, hpkt->index);
			}
		}

	}

	void ping()
	{
		helloPacket hello;
		hello.index = ping_count;
		ping_count++;
		
		UTL_LOG_HIGH("cdn_test", "send ping, ping index=[%d]", hello.index);

		ping_index = hello.index;

		strcpy(hello.hello, "ping");

		client->Write(&hello);

		ping_t0 = timeGetTime();

		kt.reset();
		
	}

	void run(char* hostname)
	{			
		client = new TcpConnector(this);

		REGISTER_PACKET(filePacket);

		state = INIT;

		unsigned short port = 5566;

		char* ip = resolveIp(hostname);


		UTL_LOG_HIGH("cdn_test", "begin connect hostname=[%s], resolve ip=[%s], port=[%d]", hostname, ip, port);

		client->Connect(ip, port);

		ping_count = 0;
		ping_index = 0;
		total_count = 0;
		time_sum = 0;

		const DWORD PING_TIMEOUT = 5000;
		const DWORD TEST_TIME = 60000*5;
		

			
			
		char* fn = "testsnefile";

			File file(fn, "rb");

			filePacket fpacket;

			strcpy(fpacket.filename, fn);

			printf("---------start transfer %s-------------", fn);

			int block_size = sizeof(fpacket.buf);

			int rsz = block_size;
			int bidx = 0;

			while (rsz == block_size)
			{
				client->UpdateAll();

				if (state == TRANSFER)
				{
					rsz = file.fread(fpacket.buf, 1, block_size);

					fpacket.block_index = bidx;
					++bidx;

					fpacket.len = rsz;
					client->Write(&fpacket);

					printf("send bytes : %d, block: %d\n", rsz, fpacket.block_index);
				}

				Sleep(0);

			}

			Sleep(1000000000);


		
	}

	int state;
	DWORD ping_t0;
	int ping_index;
	int ping_count;
	TcpConnector* client;

	KTimer kt;
	double time_sum;

	unsigned int total_count;
};

#if 1 //tcp
int main(int argc, char** argv)
{

	char* server_ip = "127.0.0.1";
	char temp[128];

	printf("ÇëÊäÈëserver ip:\n");
	gets(temp);
	if (temp[0])
	{
		server_ip = temp;
	}


	ClientHandler* clientHandler = new ClientHandler;

	clientHandler->run(server_ip);

	return 0;

}

#else
//udp
int main()
{
	log("======= udp client ========\n");

	SocketHandler* clientHandler = new ClientHandler;
	PacketFactory* clientFactory = new packetsFactory;
	UdpPeer* client = new UdpPeer(clientHandler,clientFactory);

	//client->Connect();
	client->Bind("0.0.0.0", 5588);

	char ip[64];

	printf("please input ip: \n");
	gets(ip);


	//log("begin connect %s", ip);

	//client->Connect(ip, 5566);

	helloPacket hh;
	strcpy(hh.hello, "hello, udp");

	client->SendTo(ip, 5566, &hh);

	while(1)
	{
		client->UpdateAll();
		//Sleep(100);
	}
}
#endif