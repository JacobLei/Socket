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

#define ERR_EXIT(m) \
		do \
		{ \
			perror(m);	\
			exit(EXIT_FAILURE);	\
		}while(0)

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

    pid_t pid;
    pid = fork();

    if( pid == -1 )
    {
        ERR_EXIT("fork");
    }
    else if( pid == 0 )
    {
        signal(SIGUSR1, handler);   // 信号函数，父进程通知子进程退出
        send_data(sock);
        exit(EXIT_SUCCESS);
    }
    else
    {
        get_data(sock);
        kill(pid, SIGUSR1);     // 当父进程退出时，自进程也许要退出
        exit(EXIT_SUCCESS);
    }
	
	close(sock);	// 关闭套接字
	

	return 0;
}
