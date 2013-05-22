#pragma once
#include "NetConst.h"
#include <WinSock2.h>

#include <map>

#include "Session.h"
#include "SocketHandler.h"

class Adns;

class Socket
{

public:
	Socket(SocketHandler* handler);
	virtual ~Socket();

	virtual void Reset(bool force = false);

	void UpdateAll();

	Session* GetSession(int id);
	void AddSession(Session* ss);
	void RemoveSession(int id);

	int GetSessionNum();

	void Clear();

	int NewSessionID();

	void CloseSessions(int* sessions, int num, bool keep = false);

	void EnableHeartbeat(bool hbt);

	virtual void Connect(const char* ip, unsigned short port);


	void UpdateDNS();

private:
	Socket(){}

	int m_session_count;



public:
	typedef std::map<int, Session*> TSession;

protected:
	int CreateSocket(int af, int type);
	virtual void _Update();

	void _Dispatch();
	void _UpdateSessionState();

	void _ProcessReadWrite();

	void ResolveAndConnect(const char* hostname);

	static void ConnectCb(const char* ip, void* arg);

protected:
	TSession m_sessions;
	
	
	SocketHandler* m_handler;

	int m_org_socket;

	bool m_needUpdateSessionState;

	int m_dnsLookupState;

	int m_remoteIP;
	unsigned short m_remotePort;

	Adns* m_dns;

};