// ***************************************************************
//  Session.h
//  -------------------------------------------------------------
//  @Author: GLB
//	@Date:   2012/3/12
//  -------------------------------------------------------------
//  Copyright (C) 2012 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#pragma once
#include "NetConst.h"
#include "coqueue.h"
#include "Buffer.h"
#include "ThreadTask.h"
#include "Mutex.h"

class Socket;
class Packet;

enum
{
	CLOSED,
	CLOSING,
	CONNECTED
};

class Session
{

public:
	Session(int id, int sock, Socket* pSocket, int local_ip, unsigned short local_port, int remote_ip, unsigned short remote_port);

	~Session();

	void Write(Packet* pkt);


	void Close();

	int GetID();

	int GetPing();
	
	void Update();

	bool IsClosed();

	void Release() { m_release = true; }
	bool IsRelease() { return m_release; }

	void SetEnableHbt(bool enable) { m_enbalehbt = enable; }

	void SendHeartbeat();
	void SendHeartbeatReply();

private:
	//�ڲ�����
	int GetState() { return m_state; }
	void SetState(int state) { m_state = state; }
	void OnRecv(const char* data, int len);
	int SendFromBuf();

	
	bool Select();

public:
	int m_id;								//session id
	int m_socket;							//�׽���
	int m_local_ip;							//����IP
	unsigned short m_local_port;			//���ض˿�
	int m_remote_ip;						//Զ��IP
	unsigned short m_remote_port;			//Զ�̶˿�

	unsigned int m_last_recv_time;
	unsigned int m_heartbeat_send_time;

	int m_last_ping_time;					//�ϴ�ping��ʱ��

private:
	volatile int m_state;
	Socket* m_pSocket;

	//
	coqueue<Buffer*> m_sendqueHighPriority;

	coqueue<Buffer*> m_sendque;
	coqueue<Buffer*> m_recvque;

	// ������Ų������İ�, ��һ��������packet�������ټӽ�recvque
	char m_recv_packet_fragment_buf[PACKET_BUFFER_SIZE];
	int m_recv_packet_fragment_offset;

	char m_tempBuf[SOCK_READ_SIZE];

	// ͬ�ϣ����ڷ��ͻ���
	char m_send_packet_fragment_buf[PACKET_BUFFER_SIZE];
	int m_send_packet_fragment_offset;
	int m_send_packet_size;

	friend class Socket;

	volatile bool m_release;

	Mutex m_mutex;

	bool m_enbalehbt;

};


class SessionTask : public ThreadTask
{
public:
	SessionTask(Session* ss);
	~SessionTask();

	virtual bool DoTask();

private:
	Session* m_session;

};