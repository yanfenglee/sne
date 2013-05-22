#pragma once
#include "Singleton.h"


#define REGISTER_PACKET(pkt) { static PacketFactory<pkt> spfabc; PacketFactoryMgr::Get()->RegisterFactory(&spfabc);}

class AbstractPacketFactory;
class Packet;

class PacketFactoryMgr : public Singleton<PacketFactoryMgr>
{
public:
	~PacketFactoryMgr();
	
	Packet* CreatePacket(short id);
	void DestroyPacket(Packet* pkt);

	void RegisterFactory( AbstractPacketFactory* pf );
	
	AbstractPacketFactory* GetFactory( short id );

	void Clear();
	

};