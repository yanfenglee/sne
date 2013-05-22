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


//��16�ֽ�IP�ַ���ת����4�ֽ�int�޷�������,�������紫�������
unsigned int ip_s2i(const char* ip);

//��int����ת����16�ֽ�IP�ַ���
char* ip_i2s(int ip);

int subip(int ip);


//�ַ���hash
unsigned int strhash(const char *str);

//�õ�����ip
char* getip();

//random
float getRand(float a, float b);

//�ж��Ƿ��ǺϷ�ip��ַ
bool isIp(const char* ip);

//��������
char* resolveIp(const char* hostname);

//�õ���ǰ������
unsigned int getTime();

