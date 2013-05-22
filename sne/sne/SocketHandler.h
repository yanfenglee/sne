#pragma once
#include "Session.h"
#include "Packet.h"

class SocketHandler
{
public:
	
	SocketHandler(){}
	virtual ~SocketHandler() {}

	virtual void OnSessionOpened(Session* ss);
	virtual void OnSessionClosed(Session* ss);
	virtual void OnMessageRecv(Session* ss, Packet* pkt);

protected:
	
};