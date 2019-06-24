#include "EncodingPackage.h"

/*
函数功能:   封装数据包的数据
函数形参:   *datapack :存放数据包结构体的地址
函数返回值: 0表示成功 其他值表示失败
*/
int SetDataPackage(struct SocketPackageData *datapack)
{
	/*1. 封装帧头*/
	datapack->FrameHead[0]=0xA1;
	datapack->FrameHead[1]=0xA2;
	datapack->FrameHead[2]=0xA3;
	datapack->FrameHead[3]=0xA4;
	
	/*2. 计算校验和*/
	datapack->CheckSum=0;
	int i;
	for(i=0;i<sizeof(datapack->SrcDataBuffer)/sizeof(datapack->SrcDataBuffer[0]);i++)
	{
		datapack->CheckSum+=datapack->SrcDataBuffer[i];
	}
}


/*
函数功能: 校验数据包是否正确
函数形参:   data :校验的数据包结构
函数返回值: 0表示成功 其他值表示失败
*/
int CheckDataPackage(struct SocketPackageData *datapack)
{
	unsigned int checksum=0;
	int i;
	/*1. 判断帧头是否正确*/
	if(datapack->FrameHead[0]!=0xA1|| datapack->FrameHead[1]!=0xA2||
	   datapack->FrameHead[2]!=0xA3||datapack->FrameHead[3]!=0xA4)
	   {
		   return -1;
	   }
	/*2. 判断校验和*/
	for(i=0;i<sizeof(datapack->SrcDataBuffer)/sizeof(datapack->SrcDataBuffer[0]);i++)
	{
		checksum+=datapack->SrcDataBuffer[i];
	}
	if(checksum!=datapack->CheckSum)
	{
		return -1;
	}
	return 0;
}
