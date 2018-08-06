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

// 通信处理函数
void do_servece(int conn)
{
    char recvbuf[1024];
    while(true)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = read(conn, recvbuf, sizeof(recvbuf));
        if( ret == 0 )          // 返回值为0，表示客户端关闭
        {
            printf("Client close");
            break;              // 当客户端关闭，退出循环
        }
        else if( ret == -1)
        {
            ERR_EXIT("read");
        }
        fputs(recvbuf, stdout);
        write(conn, recvbuf, ret);
    }
}

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

	// 设置地址重复利用 REUSEADDR
	int on = 1;
	if( (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))  < 0) )
	{
		ERR_EXIT("setsockopt");
	} 

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

    // 通过多进程来使得服务器可以接收多个客户端的消息，从而达到并发效果
    // 父进程用来监听，子进程用来处理通信
    pid_t pid;
    while(true)
    {
        if( (conn = accept(listenfd, (struct sockaddr*)(&peeraddr), &peerlen)) < 0 )
        {
            ERR_EXIT("accept");
        }

        printf("ip=%s, port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));	// 打印客服端发送过来的ip和port

        pid = fork();
        if( pid == -1 )
        {
            ERR_EXIT("fork");
        }
        else if( pid == 0 )     // 子进程来处理通信
        {
            close(listenfd);    // 子进程不需要监听，将监听套接口关闭
            do_servece(conn);   // 通信处理函数
            exit(EXIT_SUCCESS); // 当客户端关闭后，子进程销毁
        }
        else    // 父进程
        {
            close(conn);    // 父进程不需要通信，将通信套接口关闭
        }
    }

	close(conn);
	close(listenfd);
	
	return 0;
}
