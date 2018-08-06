#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <error.h>
#include <iostream>

#define ERR_EXIT(m) \
		do \
		{ \
			perror(m);	\
			exit(EXIT_FAILURE);	\
		}while(0)

using namespace std;

void get_data(int conn)
{
    char recvbuf[1024];
    while( true )
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = read(conn, recvbuf, sizeof(recvbuf));
        if( ret == -1)
        {
            ERR_EXIT("read");
        }
        else if( ret == 0 )
        {
            printf("peer close\n");
            break;
        }
        fputs(recvbuf, stdout);
    }
}

void send_data(int conn)
{
    char sendbuf[1024];
    while( fgets(sendbuf, sizeof(sendbuf), stdin) != NULL )
    {
        write(conn, sendbuf, strlen(sendbuf));

        memset(sendbuf, 0, sizeof(sendbuf));    // 清空缓存
    }
}

void handler(int sig)
{
    printf("kill pid = %d\n", sig);
    exit(EXIT_SUCCESS);
}

int main()
{
	// 创建套接字
	int listenfd;
	if ( (listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))  < 0 ) 	// PF_INET（AF_INET）：IPv4 Internet protocols, SOCK_STREAM:流式套接字
	{
		ERR_EXIT("socket");
	}
//    cout << "socket ok " << endl;
	
	
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
//    cout << "setsockopt ok " << endl;

	// 将套接字与地址绑定
	if( bind(listenfd, (struct sockaddr*)(&servaddr), sizeof(servaddr)) < 0 )
	{
		ERR_EXIT("bind");
	}
//    cout << "bind ok " << endl;
	
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
	
	printf("ip=%s, port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));	// 打印客服端发送过来的ip和port
	
    // 创建一个进程，一个用来接收数据，一个用来发送数据
    pid_t pid;
    pid = fork();

    if( pid == -1 )
    {
        ERR_EXIT("fork");
    }
    else if( pid == 0 ) // 子进程用来发送数据
    {
        signal(SIGUSR1, handler);
        send_data(conn);
        exit(EXIT_SUCCESS);
    }
    else    // 父进程用来接收数据
    {
        get_data(conn);
        kill(pid, SIGUSR1);
        exit(EXIT_SUCCESS);
    }

	close(conn);
	close(listenfd);
	
	return 0;
}
