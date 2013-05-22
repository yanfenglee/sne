#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <winsock2.h> 

#pragma comment(lib,"ws2_32.lib") 
#pragma comment(lib, "winmm.lib")

#include "NetUtil.h"


#include <time.h>
#include <sys/timeb.h>

unsigned int ip_s2i( const char* ip )
{
	int  nIp[4];
	unsigned char Ip[4];

	if (!ip || INADDR_NONE == inet_addr(ip))
	{
		return 0;
	}

	sscanf_s(ip, "%d.%d.%d.%d", &nIp[0], &nIp[1], &nIp[2], &nIp[3]);

	for (int i = 0; i < 4; i++)
	{
		Ip[i] = nIp[i];
	}

	unsigned int v = *(unsigned int*)Ip;

	return v;
}

char* ip_i2s( int ip )
{
	unsigned char* nIp = (unsigned char *)&ip;

	const int MAX_IP = 16;
	static char ipstr[MAX_IP][16];
	static int idx = 0;
	int r = idx;

	memset(ipstr[r], 0, sizeof(ipstr[r]));

	sprintf_s(ipstr[r], 16, "%d.%d.%d.%d", nIp[0],   nIp[1],   nIp[2],   nIp[3]);

	idx = (idx+1)%MAX_IP;

	return ipstr[r];
}

unsigned int strhash( const char *str )
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

char* getip()
{
	WSAData data;

	if(WSAStartup(MAKEWORD(2,2), &data)!=0)
	{
		printf("Winsock Initiliaze Failed.\n");
	}


	char host_name[255]; 
	static char ip[16];

	//获取本地主机名称 
	if (gethostname(host_name, sizeof(host_name)) == SOCKET_ERROR) { 
		printf("Error %d when getting local host name.\n", WSAGetLastError()); 
		return 0; 
	} 
	//printf("Host name is: %s\n", host_name); 

	//从主机名数据库中得到对应的“主机” 
	struct hostent *phe = gethostbyname(host_name); 
	if (phe == 0) { 
		printf("Yow! Bad host lookup."); 
		return 0; 
	} 

	//循环得出本地机器所有IP地址 
	for (int i = 0; phe->h_addr_list[i] != 0; ++i) { 
		struct in_addr addr; 
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr)); 
		//printf("Address %d : %s\n" , i, inet_ntoa(addr)); 

		sprintf_s(ip, sizeof(ip), "%s", inet_ntoa(addr));
		break;
	} 

	return ip; 

}

float getRand( float a, float b )
{
	float r = (float)(rand()%1000)/1000.0f;
	float ret = (a + (b-a)*r);

	//printf("----------rand result: %f\n", ret);

	return ret;
}

bool isIp( const char* ip )
{
	if (INADDR_NONE == inet_addr(ip))
	{
		return false;
	}
	else
	{
		return true;
	}
}

int subip( int ip )
{
	unsigned char* nIp = (unsigned char *)&ip;

	return nIp[3];
}

char* resolveIp( const char* hostname )
{                
	static char ip[64];
	memset(ip, 0, sizeof(ip));

	if (isIp(hostname))
	{
		strcpy_s(ip, sizeof(ip), hostname);
	}
	else
	{
		hostent* host = gethostbyname(hostname);

		if (!host)
		{
			sprintf_s(ip, sizeof(ip), "255.255.255.255");
			printf("resolve ip error\n");
		}
		else
		{
			sprintf_s(ip, sizeof(ip), "%s", inet_ntoa(*( (struct in_addr *)host->h_addr) ) ); 
		}
	}

	return ip;
	
}

unsigned int getTime()
{
	return timeGetTime();
}