#pragma once
#include "Buffer.h"

//用户自定义包继承Packet,起始范围: 0x0001 ～ 0x0100

class Packet
{
public:
	virtual ~Packet(){}

	/**
	 * 返回包的ID
     */
	virtual short GetID() = 0;

	/**
	 * 返回包的数据大小，继承类根据自身数据量实现
     */
	virtual int GetDataSize() = 0;

	/**
	 * 将数据包打包的Buffer中，以便网络发送
	 *
	 * @param out 输出buffer
     */
	virtual void Encode(Buffer* out) = 0;

	/**
	 * 将Buffer中的数据解包
	 *
	 * @param in 输入buffer
     */
	virtual void Decode(Buffer* in) = 0;
};

class AbstractPacketFactory
{
public:
	virtual Packet* CreatePacket() = 0;
	virtual void DestroyPacket(Packet* pkt) = 0;
};

template<class T> class PacketFactory : public AbstractPacketFactory
{
public:
	Packet* CreatePacket()
	{
		return new T();
	}

	void DestroyPacket(Packet* pkt)
	{
		delete pkt;
	}
};