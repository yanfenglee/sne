#pragma once

#include "sne/sne/Buffer.h"
#include "sne/sne/Packet.h"
#include "sne/sne/PacketFactoryMgr.h"

enum
{
	ID_HELLO = 10,
	ID_FILE,
};

class helloPacket : public Packet
{
public:
	int index;
	char hello[16];

	short GetID() { return ID_HELLO; }
	int GetDataSize() { return sizeof(hello)+4; }

	void Encode(Buffer* out)
	{
		out->putInt(index);
		out->putRaw(hello, sizeof(hello));
	}

	void Decode(Buffer* in)
	{
		index = in->getInt();
		in->getRaw(hello, sizeof(hello));
	}
};

//========================================================================
class filePacket : public Packet
{
public:
	char filename[32];
	int block_index;
	int len;
	char buf[16384];

	short GetID() { return ID_FILE; }
	int GetDataSize() { return sizeof(filename) + sizeof(len) + sizeof(buf) + sizeof(block_index); }

	void Encode(Buffer* out)
	{
		out->putRaw(filename, sizeof(filename));
		out->putInt(block_index);
		out->putInt(len);
		out->putRaw(buf, sizeof(buf));
	}

	void Decode(Buffer* in)
	{
		in->getRaw(filename, sizeof(filename));
		block_index = in->getInt();
		len = in->getInt();
		in->getRaw(buf, sizeof(buf));
	}
};