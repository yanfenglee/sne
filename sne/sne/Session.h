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
	//内部调用
	int GetState() { return m_state; }
	void SetState(int state) { m_state = state; }
	void OnRecv(const char* data, int len);
	int SendFromBuf();

	
	bool Select();

public:
	int m_id;								//session id
	int m_socket;							//套接字
	int m_local_ip;							//本地IP
	unsigned short m_local_port;			//本地端口
	int m_remote_ip;						//远程IP
	unsigned short m_remote_port;			//远程端口

	unsigned int m_last_recv_time;
	unsigned int m_heartbeat_send_time;

	int m_last_ping_time;					//上次ping的时间

private:
	volatile int m_state;
	Socket* m_pSocket;

	//
	coqueue<Buffer*> m_sendqueHighPriority;

	coqueue<Buffer*> m_sendque;
	coqueue<Buffer*> m_recvque;

	// 用来存放不完整的包, 等一个完整的packet接受完再加进recvque
	char m_recv_packet_fragment_buf[PACKET_BUFFER_SIZE];
	int m_recv_packet_fragment_offset;

	char m_tempBuf[SOCK_READ_SIZE];

	// 同上，用于发送缓冲
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