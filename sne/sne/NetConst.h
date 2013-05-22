#pragma once

//include 此头文件时放到最前面，FD_SETSIZE 必须放到 Winsock2.h 前才有效

#define FD_SETSIZE 1024

#define MAX_PACKET_LENGTH 65529
#define PACKET_BUFFER_SIZE (MAX_PACKET_LENGTH + 6)
#define SOCK_READ_SIZE 16400

#define HEARTBEAT_ID (0x1001)
#define HEARTBEAT_REPLY_ID (0x1002)