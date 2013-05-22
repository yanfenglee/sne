// ***************************************************************
//  NetUtil.h
//  -------------------------------------------------------------
//  @Author: GLB
//	@Date:   2012/3/12
//  -------------------------------------------------------------
//  Copyright (C) 2012 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************


#pragma once
#include "SNE_LogAssert.h"

#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }


//将16字节IP字符串转换成4字节int无符号整数,减少网络传输的容量
unsigned int ip_s2i(const char* ip);

//将int整数转换成16字节IP字符串
char* ip_i2s(int ip);

int subip(int ip);


//字符串hash
unsigned int strhash(const char *str);

//得到本机ip
char* getip();

//random
float getRand(float a, float b);

//判断是否是合法ip地址
bool isIp(const char* ip);

//域名解析
char* resolveIp(const char* hostname);

//得到当前毫秒数
unsigned int getTime();

