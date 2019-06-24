#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "EncodingPackage.h"
#include <signal.h>

char *rx_p;
int Package_Cnt=0;              //记录已经接收的数据包
int tcp_client_fd;              //客户端套接字描述符
int Server_Port;                //服务器端口号
struct sockaddr_in tcp_server;  //存放服务器的IP地址信息
int  rx_len;
fd_set readfds; 				//读事件的文件操作集合
int select_state,rx_cnt; 		//接收返回值
struct SocketPackageData RxTxData; //保存接收和发送的数据
struct SocketAckPackageData AckData; //保存客户端的应答状态
FILE *NewFile=NULL;    			//文件指针
unsigned int  SendOkByte=0; 	//记录已经接收的字节数量
unsigned int rx_size=0;
unsigned int all_size=0;

//计算接收速度
time_t time1;   //时间1
time_t time2; 	//时间2
unsigned int SendByteCnt;  //记录上一次的字节数据
char state=1;   //状态值
double speed=0.0;   //保存速度
char FileName[1024]; //存放文件的路径

//设置socket缓冲区大小
int snd_size = 0; /* 发送缓冲区大小 */
int rcv_size = 0; /* 接收缓冲区大小 */
socklen_t optlen; /* 选项值长度 */
int err = -1; 	  /* 返回值 */

/*
处理异常信号
*/
void sighandler(int a)
{
	printf("触发SIGSEGV信号: 产生段错误! 信号值:%d\n",a);
	exit(-1); //退出进程  return
}

int main(int argc,char **argv)
{
	//捕获段错误信号-内存溢出产生的错误
	signal(SIGSEGV,sighandler);
	
	if(argc!=4)
	{
		printf("客户端形参格式:./tcp_client <服务器IP地址>  <服务器端口号>  <文件存放的目录>\n");
		return -1;
	}
	
	Server_Port=atoi(argv[2]); //将字符串的端口号转为整型
	
	/*1. 创建网络套接字*/
	tcp_client_fd=socket(AF_INET,SOCK_STREAM,0);
	if(tcp_client_fd<0)
	{
		printf("客户端提示:服务器端套接字创建失败!\n");
		goto ERROR;
	}

	/*
	* 设置发送缓冲区大小
	*/
	snd_size = 20*1024; /* 发送缓冲区大小为*/
	optlen = sizeof(snd_size);
	err = setsockopt(tcp_client_fd, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen);
	if(err<0)
	{
		printf("服务器提示:设置发送缓冲区大小错误\n");
	}
	
	/*
	* 设置接收缓冲区大小
	*/
	rcv_size = 20*1024; /* 接收缓冲区大小*/
	optlen = sizeof(rcv_size);
	err = setsockopt(tcp_client_fd,SOL_SOCKET,SO_RCVBUF, (char *)&rcv_size, optlen);
	if(err<0)
	{
		printf("服务器提示:设置接收缓冲区大小错误\n");
	}
	
	/*2. 连接到指定的服务器*/
	tcp_server.sin_family=AF_INET; //IPV4协议类型
	tcp_server.sin_port=htons(Server_Port);//端口号赋值,将本地字节序转为网络字节序
	tcp_server.sin_addr.s_addr=inet_addr(argv[1]); //IP地址赋值
	
	if(connect(tcp_client_fd,(const struct sockaddr*)&tcp_server,sizeof(const struct sockaddr))<0)
	{
		 printf("客户端提示: 连接服务器失败!\n");
		 goto ERROR;
	}
	
	rx_p=(char*)&RxTxData; //指针
	rx_size=sizeof(struct SocketPackageData);
	all_size=0;
	while(1)
	{
		/*5.1 清空文件操作集合*/
		FD_ZERO(&readfds);
        /*5.2 添加要监控的文件描述符*/
		FD_SET(tcp_client_fd,&readfds);
		/*5.3 监控文件描述符*/
		select_state=select(tcp_client_fd+1,&readfds,NULL,NULL,NULL);
		if(select_state>0)//表示有事件产生
		{
			/*5.4 测试指定的文件描述符是否产生了读事件*/
			if(FD_ISSET(tcp_client_fd,&readfds))
			{
				/*5.5 读取数据*/
				rx_cnt=read(tcp_client_fd,rx_p,rx_size);
				if(rx_cnt>0) //收到不完整的数据
				{
					all_size+=rx_cnt; //记录收到的字节数量
					//收到数据包
					if(all_size>=sizeof(struct SocketPackageData))
					{
						printf("rx_cnt=%d,all_size=%d\n",rx_cnt,all_size);
						
						//出现错误的的处理方法
						if(all_size!=sizeof(struct SocketPackageData))
						{
							printf("all_size值超出限制=%d\n",all_size);
							all_size=0;
							rx_size=sizeof(struct SocketPackageData); //总大小归位
							rx_p=(char*)&RxTxData; //指针归位
							AckData.AckStat=0x81; //表示接收失败
							printf("all_size的值恢复正常=%d\n",all_size);
							continue;  //结束本次循环
						}
						
						all_size=0; //当前已经接收的字节归0
						rx_size=sizeof(struct SocketPackageData); //总大小归位
						rx_p=(char*)&RxTxData; //指针归位
						
						/*校验数据包是否正确*/
						if(CheckDataPackage(&RxTxData)==0)
						{
							//判断之前是否已经接收到相同的一次数据包了,如果接收过就不需要再继续写入到文件
							//原因: 可能服务器没有收到客户端发送的应答,触发了数据重发
							if(Package_Cnt!=RxTxData.NumCnt) 
							{
								printf("包编号:%d,有效数据:%d\n",RxTxData.NumCnt,RxTxData.CurrentSize);
								Package_Cnt=RxTxData.NumCnt; //记录上一次的接收编号
								
								if(RxTxData.NumCnt==1)  //表示第一次接收数据包
								{
									printf("第一次接收数据包\n");
									strcpy(FileName,argv[3]); //拷贝路径  /123/456.c
									strcat(FileName,RxTxData.FileName); //文件名称
									NewFile=fopen(FileName,"wb"); //创建文件
									if(NewFile==NULL)
									{
										printf("客户端提示: %s 文件创建失败!\n",FileName);
										fclose(NewFile);
										goto ERROR; //退出连接
									}
									if(fwrite(RxTxData.SrcDataBuffer,1,RxTxData.CurrentSize,NewFile)!=RxTxData.CurrentSize)
									{
										printf("客户端提示: 第%d包向%s文件写入失败!\n",RxTxData.NumCnt,FileName);
										fclose(NewFile);
										goto ERROR; //退出连接
									}
								}
								else  //继续接收数据包
								{
									if(fwrite(RxTxData.SrcDataBuffer,1,RxTxData.CurrentSize,NewFile)!=RxTxData.CurrentSize)
									{
										printf("客户端提示: 第%d包向%s文件写入失败!\n",RxTxData.NumCnt,FileName);
										fclose(NewFile);
										goto ERROR; //退出连接
									}
								}
								SendOkByte+=RxTxData.CurrentSize; 	//记录已经接收的字节数量
								

								printf("客户端接收进度提示: 总大小:%d字节,已接收:%d字节,百分比:%0.0f%%\n",RxTxData.FileSize,SendOkByte,SendOkByte/1.0/RxTxData.FileSize*100.0);
							}
							//接收到数据包之后向服务器回发应答信号
							AckData.AckStat=0x80; //表示接收成功
							if(write(tcp_client_fd,&AckData,sizeof(struct SocketAckPackageData))!=sizeof(struct SocketAckPackageData))
							{
								printf("客户端提示: 向服务器应答失败!");
							}
							//判断数据是否接收完毕
							if(RxTxData.CurrentSize!=sizeof(RxTxData.SrcDataBuffer))
							{
								printf("客户端提示:文件接收成功!\n");
								break; //退出接收
							}
						}
						else
						{
							AckData.AckStat=0x81; //表示接收失败
							if(write(tcp_client_fd,&AckData,sizeof(struct SocketAckPackageData))!=sizeof(struct SocketAckPackageData))
							{
								printf("客户端提示: 向服务器应答失败!");
							}
							printf("客户端提示:校验数据包不正确\n");
						}
					}
					else
					{
						rx_size=sizeof(struct SocketPackageData)-rx_cnt;
						rx_p+=rx_cnt; //偏移文件指针
					}
					
					time1=time(NULL);   //获取时间1
					if(state)
					{
						state=0;
						time2=time1; 	//保存时间1
						SendByteCnt=SendOkByte;  //记录上一次的字节数据
					}
					
					if(time1-time2>=1) //1秒时间到达
					{
						state=1;
						speed=(SendOkByte-SendByteCnt)*1.0/1024; //按每秒KB算
					}
					
					if(speed>1024) //大于1024字节
					{
						printf("实际接收速度:%0.2f mb/s\n",speed*1.0/1024);
					}
					else  
					{
						printf("接收速度:%0.2f kb/s\n",speed);
					}
					
		
					//printf("客户端提示: 应该接收%d字节,实际接收%d字节\n",sizeof(struct SocketPackageData),rx_cnt);
					//printf("客户端提示:数据大小接收不正确....\n");
				}
				
				if(rx_cnt==0)
				{
					printf("客户端提示:服务器已经断开连接!\n");
					fclose(NewFile);
					break;
				}
			}
		}
		else if(select_state<0) //表示产生了错误
		{
			printf("客户端提示:select函数产生异常!\n");
			break;
		}
	}
ERROR:	
	/*4. 关闭连接*/
	//close(tcp_client_fd);
	shutdown(tcp_client_fd,SHUT_WR);  //TCP半关闭，保证缓冲区内的数据全部写完
	return 0;
}

