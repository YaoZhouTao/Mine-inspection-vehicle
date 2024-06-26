#include "server.h"
#include <stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main(int argc, char* argv[])
{
    printf("程序开始运行\n");
    unsigned short port = atoi("8080");
    //切换服务器的工作路径
    //chdir(argv[2]);
    //初始化用于监听的套接字
    int lfd = initListenFd(port);
    //启动服务器程序
    epollRun(lfd);
    //pthread_join(thread1, NULL);
    printf("程序结束运行\n");
    return 0;
}