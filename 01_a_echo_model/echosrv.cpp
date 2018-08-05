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

using namespace std;

int main()
{
	// 创建套接字
	int listenfd;
	if ( (listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))  < 0 ) 	// PF_INET（AF_INET）：IPv4 Internet protocols, SOCK_STREAM:流式套接字
	{
		ERR_EXIT("socket");
	}
	
	
	// 地址的初始化
	struct sockaddr_in servaddr;	// IPv4的地址结构
	memset(&servaddr, 0, sizeof(servaddr));	// 初始化地址
	servaddr.sin_family = AF_INET;		// 地址族
	servaddr.sin_port = htons(5188);	// 指定端口号，并将端口号转化为2个字节的网络字节序
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	// 绑定主机任意地址
	//servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");		// 指定地址  
	//inet_aton("127.0.0.1", &servaddr.sin_addr);		// 指定地址

	// 将套接字与地址绑定
	if( bind(listenfd, (struct sockaddr*)(&servaddr), sizeof(servaddr)) < 0 )
	{
		ERR_EXIT("bind");
	}
	
	// 监听
	if( listen(listenfd, SOMAXCONN) < 0 )
	{
		ERR_EXIT("listen");
	}
	
	// 接收
	struct sockaddr_in peeraddr;	// 对方的地址
	socklen_t peerlen = sizeof(peeraddr);
	int conn;		// 定义已连接套接字
	if( (conn = accept(listenfd, (struct sockaddr*)(&peeraddr), &peerlen)) < 0 )
	{
		ERR_EXIT("accept");
	}
	
	char recvbuf[1024];
	while(true)
	{
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = read(conn, recvbuf, sizeof(recvbuf));
		fputs(recvbuf, stdout);
		write(conn, recvbuf, ret);
	}

	close(conn);
	close(listenfd);
	
	return 0;
}
