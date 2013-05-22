#pragma once
#include "Buffer.h"

//�û��Զ�����̳�Packet,��ʼ��Χ: 0x0001 �� 0x0100

class Packet
{
public:
	virtual ~Packet(){}

	/**
	 * ���ذ���ID
     */
	virtual short GetID() = 0;

	/**
	 * ���ذ������ݴ�С���̳����������������ʵ��
     */
	virtual int GetDataSize() = 0;

	/**
	 * �����ݰ������Buffer�У��Ա����緢��
	 *
	 * @param out ���buffer
     */
	virtual void Encode(Buffer* out) = 0;

	/**
	 * ��Buffer�е����ݽ��
	 *
	 * @param in ����buffer
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