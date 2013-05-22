#include "PacketFactoryMgr.h"
#include <map>
#include "Packet.h"
#include "SNE_LogAssert.h"

typedef std::map<short, AbstractPacketFactory*> TPacketFactory;

static TPacketFactory g_factories;

PacketFactoryMgr::~PacketFactoryMgr() 
{
	Clear();
}

Packet* PacketFactoryMgr::CreatePacket(short id)
{
	AbstractPacketFactory* pf = GetFactory(id);
	if (pf)
	{
		return pf->CreatePacket();
	}
	return NULL;
}

void PacketFactoryMgr::RegisterFactory( AbstractPacketFactory* pf )
{
	Packet* pkt = pf->CreatePacket();
	short id = pkt->GetID();

	TPacketFactory::iterator it = g_factories.find(id);
	if (it != g_factories.end())
	{
		UTL_ASSERT(false, "packet id exist: %d", id);
	}

	g_factories[id] = pf;

	pf->DestroyPacket(pkt);
}

AbstractPacketFactory* PacketFactoryMgr::GetFactory( short id )
{
	TPacketFactory::iterator it = g_factories.find(id);
	if (it != g_factories.end())
	{
		return it->second;
	}

	return NULL;
}

void PacketFactoryMgr::Clear()
{
	//由外部删除，因为指针是外部传入，可以传入static的指针，由系统回收
	//TPacketFactory::iterator it = g_factories.begin();
	//for (; it != g_factories.end(); ++it)
	//{
	//	delete it->second;
	//}

	g_factories.clear();
}

void PacketFactoryMgr::DestroyPacket( Packet* pkt )
{
	AbstractPacketFactory* pf = GetFactory(pkt->GetID());
	if (pf != NULL)
	{
		pf->DestroyPacket(pkt);
	}
}