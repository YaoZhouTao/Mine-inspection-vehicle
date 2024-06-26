#include<arpa/inet.h>
#include<sys/epoll.h>
#include<stdio.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<sys/stat.h>
#include<strings.h>
#include<assert.h>
#include<unistd.h>
#include<sys/sendfile.h>
#include<dirent.h>
#include<stdlib.h>
#include<ctype.h>
#include <pthread.h>
#include<string.h>
#include <signal.h>


extern pthread_t thread1;



//初始化监听的套接字
int initListenFd(unsigned short port);
//启动epoll
int epollRun(int lfd);
//和客户端建立连接的函数
int acceptClient(int lfd, int epfd);
//接收客户端发送过的信息
//void RecvMessage(int lfd,int epfd);
//发送信息
//void SendMessage(const char* buf,int wfd, int epfd);

void* thread_func1(void* arg);

void* thread_func2(void* arg);

void* thread_func3(void* arg);

//发送指定长度字符串
//pthreadint writen(int fd,const char* msg,int size);