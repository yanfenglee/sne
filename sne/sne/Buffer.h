#pragma once
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

//typedef unsigned short int unsigned short int;
//typedef unsigned long int unsigned long int;

// �����ʹ�С�˻���
#define BigLittleSwap16(A)        ((((unsigned short int)(A) & 0xff00) >> 8) | \
	(((unsigned short int)(A) & 0x00ff) << 8))

// �����ʹ�С�˻���
#define BigLittleSwap32(A)        ((((unsigned long int)(A) & 0xff000000) >> 24) | \
	(((unsigned long int)(A) & 0x00ff0000) >> 8) | \
	(((unsigned long int)(A) & 0x0000ff00) << 8) | \
	(((unsigned long int)(A) & 0x000000ff) << 24))

//�˳���ֻ�ʺ�x86ƽ̨

// ģ��htonl�����������ֽ���ת�����ֽ���
#define HtoNl(A)        (BigLittleSwap32(A))

// ģ��ntohl�����������ֽ���ת�����ֽ���
#define NtoHl(A)        HtoNl(A)

// ģ��htons�����������ֽ���ת�����ֽ���
#define HtoNs(A)        (BigLittleSwap16(A))

// ģ��ntohs�����������ֽ���ת�����ֽ���
#define NtoHs(A)        HtoNs(A)


class Buffer
{
public:

	Buffer::Buffer( int alloc_size )
		: m_buf(0)
		, m_write_pos(0)
		, m_read_pos(0)
		, m_size(alloc_size)
	{
		m_buf = (char*)malloc(m_size);
	}

	Buffer::~Buffer()
	{
		free(m_buf);
	}

	int getSize() {	return m_size;}
	int getReadPos() { return m_read_pos; }
	void setReadPos(int pos) { m_read_pos = pos; }


	void putRaw( const void* data, int len )
	{
		int len1 = len;
		int left = m_size - m_write_pos;
		if (len1 > left)
		{
			len1 = left;
			//UTL_LOG_HIGH("sne", "Buffer put out of range, ʣ�ࣺ%d, д�룺%d", left, len);
			
			assert(false && "Buffer put out of range");
		}

		memcpy(m_buf + m_write_pos, data, len1);
		m_write_pos += len1;
	}

	void getRaw( void* out, int len )
	{
		int len1 = len;
		int left = m_size - m_read_pos;
		if (len1 > left)
		{
			len1 = left;
			//UTL_LOG_HIGH("sne", "Buffer read out of range, read pos: %d, ʣ�ࣺ%d, ��ȡ��%d", m_read_pos, left, len);
			assert(false && "Buffer read out of range");
		}

		memcpy(out, m_buf + m_read_pos, len1);
		m_read_pos += len1;
	}

	void getRaw( int pos, void* out, int len )
	{
		int len1 = len;
		int left = m_size - pos;
		if (len1 > left)
		{
			len1 = left;
			//UTL_LOG_HIGH("sne", "Buffer read out of range, read pos: %d, ʣ�ࣺ%d, ��ȡ��%d", pos, left, len);
			assert(false && "get raw pos, Buffer read out of range");
		}

		memcpy(out, m_buf + pos, len1);
	}

	void putInt( int v )
	{
		long v1 = HtoNl(v);
		putRaw(&v1, 4);
	}

	void putShort( short v )
	{
		short v1 = HtoNs(v);
		putRaw(&v1, 2);
	}

	int getInt()
	{
		int v;
		getRaw(&v, 4);

		return NtoHl(v);
	}

	int getInt( int pos )
	{
		int v;
		getRaw(pos, &v, 4);

		return NtoHl(v);
	}
	short getShort()
	{
		short v;
		getRaw(&v, 2);

		return NtoHs(v);
	}

	short getShort( int pos )
	{
		short v;
		getRaw(pos, &v, 2);

		return NtoHs(v);
	}

	void putFloat( float v )
	{
		int v1 = (*((int*)&v));
		putInt(v1);
	}

	float getFloat()
	{
		int v = getInt();

		return *((float*)&v);
	}

	float getFloat( int pos )
	{
		int v = getInt(pos);

		return *((float*)&v);
	}
private:
	Buffer(){}


private:
	char* m_buf;
	int m_write_pos;
	int m_read_pos;
	int m_size;
};