#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <error.h>

#define ERR_EXIT(m) \
		do \
		{ \
			perror(m);	\
			exit(EXIT_FAILURE);	\
		}while(0)

int main()
{
	// 创建套接字
	int sock;
	if ( (sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))  < 0 ) 	// PF_INET（AF_INET）：IPv4 Internet protocols, SOCK_STREAM:流式套接字
	{
		ERR_EXIT("socket");
	}
	
	// 地址的初始化
	struct sockaddr_in cliaddr;	// IPv4的地址结构
	memset(&cliaddr, 0, sizeof(cliaddr));	// 初始化地址
	cliaddr.sin_family = AF_INET;		// 地址族
	cliaddr.sin_port = htons(5188);	// 指定端口号，并将端口号转化为2个字节的网络字节序
	cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");		// 指定地址  
	//inet_aton("127.0.0.1", &cliaddr.sin_addr);		// 指定地址


	// 连接 connect
	if( connect(sock, (struct sockaddr*)(&cliaddr), sizeof(cliaddr))  < 0 )
	{	
		ERR_EXIT("connect");
	}
	
	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
	while( fgets(sendbuf, sizeof(sendbuf), stdin) != NULL )		// fgets():从标准输入中读取字符
	{
		write(sock, sendbuf, strlen(sendbuf));	// 将sendbuf中的内容写入sock套接字中
		read(sock, recvbuf, sizeof(recvbuf));	// 获取套接字中的内容，存入recvbuf中
		
		fputs(recvbuf, stdout);
		memset(sendbuf, 0, sizeof(sendbuf));	// 清空缓冲区
		memset(recvbuf, 0, sizeof(recvbuf));	// 清空缓冲区
	}
	
	close(sock);	// 关闭套接字
	

	return 0;
}
