#pragma once

//include ��ͷ�ļ�ʱ�ŵ���ǰ�棬FD_SETSIZE ����ŵ� Winsock2.h ǰ����Ч

#define FD_SETSIZE 1024

#define MAX_PACKET_LENGTH 65529
#define PACKET_BUFFER_SIZE (MAX_PACKET_LENGTH + 6)
#define SOCK_READ_SIZE 16400

#define HEARTBEAT_ID (0x1001)
#define HEARTBEAT_REPLY_ID (0x1002)