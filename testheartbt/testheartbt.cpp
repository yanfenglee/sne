// performance_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Packet.h>
#include <TcpConnector.h>
#include <TcpAcceptor.h>
#include <Buffer.h>
#include <PacketFactoryMgr.h>

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


enum
{
	ID_PERF_PKT = 1
};

class PerfPacket : public Packet
{
	enum
	{
		CONTENT_LEN = 32
	};
public:
	PerfPacket()
	{
		memset(m_content, 'p', CONTENT_LEN);
		m_content[CONTENT_LEN-1] = 0;
		index = 0;
	}

	virtual short GetID() { return ID_PERF_PKT; }
	virtual int GetDataSize() { return CONTENT_LEN+4; }

	virtual void Encode(Buffer* out)
	{
		out->putInt(index);
		out->putRaw(m_content, CONTENT_LEN);
	}
	virtual void Decode(Buffer* in)
	{
		index = in->getInt();
		in->getRaw(m_content, CONTENT_LEN);
	}

	int index;
	char m_content[CONTENT_LEN];
};



class PerformanceTest : public SocketHandler
{
public:
	PerformanceTest(bool isServer, const char* server_ip = "127.0.0.1"):m_acceptor(0), m_connector(0), m_start(false)
	{
		if (isServer)
		{
			m_acceptor = new TcpAcceptor(this);
		}
		else
		{
			strcpy(m_server_ip, server_ip);
			m_connector = new TcpConnector(this);
		}
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
		if (pkt->GetID() == ID_PERF_PKT)
		{
			PerfPacket* hh = (PerfPacket*)pkt;

			if (m_connector)
			{
				int cost = int(m_timer.getEclipse()*1000);

				printf("收到回复 id = [%d], time cost=[%d]\n", hh->index, cost);
				hh->index++;

				if (cost > 10000)
				{
					printf("cost time > 10, will close\n");
					ss->Close();
				}

				Sleep(1000);
				m_timer.reset();
				//printf("\n");
				//printf("send ping id    = [%d], time now=[%f]\n", hh->index, m_timer.getEclipse());
			}
			else
			{
				printf("receive ping id = [%d], time now=[%f]\n", hh->index, m_timer.getEclipse());
			}


			ss->Write(pkt);

		}
	}

	void run()
	{
		if (m_connector)
		{
			printf("start connect %s ...", m_server_ip);
			m_connector->Connect(m_server_ip, 8102);
		}
		else if (m_acceptor)
		{
			m_acceptor->Bind("0.0.0.0", 7788);
		}

		REGISTER_PACKET(PerfPacket);

		const double timeout = 60000.0;

		while (true)
		{
			if (m_connector)
			{
				m_connector->UpdateAll();

				if (m_start)
				{
					if (m_timer2.getEclipse() > timeout)
					{
						break;
					}
				}
			}
			else if (m_acceptor)
			{
				m_acceptor->UpdateAll();
			}

			Sleep(0);
			//SwitchToThread();

		}


		Sleep(10000000);
	}

private:

	KTimer m_timer;
	KTimer m_timer2;
	TcpAcceptor* m_acceptor;
	TcpConnector* m_connector;
	char m_server_ip[16];

	bool m_start;
};


int _tmain(int argc, _TCHAR* argv[])
{
	printf("是否做server? 是输入y，否输入n\n");
	char str[8];
	char* server_ip = "127.0.0.1";
	char temp[128];
	gets(str);

	bool isServer = (str[0]=='y');
	if (!isServer)
	{
		printf("请输入server ip:\n");
		gets(temp);
		if (temp[0])
		{
			server_ip = temp;
		}
	}

	PerformanceTest* perftest = new PerformanceTest(isServer, server_ip);
	perftest->run();

	system("pause");

	return 0;
}

