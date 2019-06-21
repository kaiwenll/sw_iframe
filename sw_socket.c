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
#include <signal.h>


char send_file_name[1024];
void *socket_pthread_fun(void *arg)
{
	
}

int main(int argc,char **argv)
{
	int listenfd,connfd;
	int *pconnfd=&connfd;
	struct sockaddr_in tcp_server,tcp_client;
	int sockaddr_len=sizeof(struct sockaddr);
	int tcp_server_port;

	 //判断传入的参数是否合理
	if(argc!=3)
	{
		printf("参数格式:./tcp_server <端口号> <FileName>\n");
		return -1;
	}	
	strcpy(send_file_name,argv[2]);
	tcp_server_port=atoi(argv[1]);

	//创建socket
	if(listenfd=socket(AF_INET,SOCK_STREAM,0)<0)
	{
		printf("TCP服务器端套接字创建失败!\n");
	}
	


	/*2. 绑定端口号,创建服务器*/
	tcp_server.sin_family=AF_INET; //IPV4协议类型
	tcp_server.sin_port=htons(tcp_server_port);//端口号赋值,将本地字节序转为网络字节序
	tcp_server.sin_addr.s_addr=INADDR_ANY; //将本地IP地址赋值给结构体成员

	if(bind(listenfd,(const struct sockaddr*)&tcp_server,sizeof(struct sockaddr))<0)
	{
		printf("TCP服务器端口绑定失败!\n");
	}

	/*3. 设置监听的客户端数量*/
	if(listen(listenfd,100))
	{
		printf("监听数量设置失败!\n");
	}
	
	/*4. 等待客户端连接*/
	pthread_t thread_id;


	while(1)
	{
		connfd=accept(listenfd,(struct sockaddr*)&tcp_client,&sockaddr_len);
		
//打印连接的客户端地址信息
		printf("客户端上线: %s:%d\n",inet_ntoa(tcp_client.sin_addr),ntohs(tcp_client.sin_port));
		/*1. 创建线程*/
		if(pthread_create(&thread_id,NULL,socket_pthread_fun,(void*)pconnfd)==0)
		{
			/*2. 设置分离属性，让线程结束之后自己释放资源*/
			pthread_detach(thread_id);
		}
	}



	close(listenfd);
	return 0;






}
