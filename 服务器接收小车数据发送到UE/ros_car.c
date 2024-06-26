#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include<errno.h>
#include <stdint.h>

//double data1 = 0.0,data2 = 0.0;
char bufer[1024] = {"0.000#0.000"};
#define MAX_BUF_SIZE 1024 // 定义缓冲区大小为1024字节 
#define PORT 7070
#define HEADER_SIZE 4 // 定义包头大小为4字节
struct ThreadArg {
	int Wfd;
	int Cfd;
};
struct ThreadArg* arg;
void* thread_func1(void* arg) {
	// 发送数据到对应客户端
	//int ac = 0;
	sleep(0.03);
	double a, b;
	struct ThreadArg* thread_arg = (struct ThreadArg*)arg;
	pthread_mutex_t mutex;   // 定义互斥锁
	pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
	signal(SIGPIPE, SIG_IGN);
	char bufeer[1024] = { 0 };
	//定义一个辅助指针，指向我们要发送的那个buf的起始地址
	/*char* buf;
	buf = bufeer;*/
	int size = 0;
	int biglen = 0, falg = 0;
	//srand(time(NULL));
	while (1) {
		//printf("进入一级循环\n");
		//a = (double)rand() / RAND_MAX * 1000000;
		//b = (double)rand() / RAND_MAX * 1000000;

		int count = 0;
		pthread_mutex_lock(&mutex); // 加锁
		strcpy(bufeer, bufer);
		//sprintf(bufeer, "%.6lf#%.6lf", a, b);
		pthread_mutex_unlock(&mutex); // 解锁
		size = strlen(bufeer);
		//printf("%s****%d\n",bufeer,ac);
		//ptintf("%s\n",bufeer);
		//定义一个整形数用于记录剩下待发送的字节数
		char* data = NULL;
		data = (char*)malloc(size + 4);
		char* data_start = data;  // 保存起始地址
		//printf("String = %c ,  Address = %p\n", data[4], data);
		if (data == NULL) {
			perror("malloc");
			falg = 1;
		}
		//把要发送的数据从主机字节序转为网络字节序
		biglen = htonl(size);
		//把biglen通过内存拷贝的方式把拷贝到data里
		memcpy(data, &biglen, 4);
		// 在写入数据之前检查内存块大小是否足够
		if (size > 0 && data != NULL && size <= (SIZE_MAX - 4)) {
			// 通过内存拷贝把要发送的数据拷贝到data里
			memcpy(data + 4, bufeer, size);
			// 在数据末尾添加空字符
			//data[size + 4] = '\0';
		}
		else {
			printf("内存泄漏！！！！！！！！！！");
			falg = 1;
		}
		//int ret = writen(thread_arg->Wfd, data, size + 4);
		count = size + 4;
		//printf("%s\n",bufeer);
		//char* originalData = data; // 保存 data 的初始值
		while (1) {
			sleep(0.5);
			//printf("进入二级循环: count = %d  data = %s\n",count,data);
			int len = send(thread_arg->Wfd, data, count, 0);
			if (len <= 0 || falg == 1) {
				if (errno == EPIPE || len == 0 || falg == 1) {
					pthread_mutex_destroy(&mutex); // 销毁互斥锁
					close(thread_arg->Wfd);
					printf("客户端断开了连接\n");
					if (data_start != NULL) {
						free(data_start);
						data_start = NULL;
					}
					pthread_exit(NULL);
				}
				else {
					//perror("send");
				}
			}
			else if (len < count) {
				data += len;
				count -= len;
			}
			else if (len == count) {
				break;
			}
		}
		//printf("数据已经发送: %s\n",data_start + 4);
		if (data_start != NULL) {
			free(data_start);
			data_start = NULL;
		}
		//printf("退出二级循环: data5 = %c,data19 = %c\n",data[4],data[18]);
	}
}

void* thread_func2(void* arg) {
	struct ThreadArg* thread_arg = (struct ThreadArg*)arg;
	pthread_mutex_t mutex;   // 定义互斥锁
        pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
	int recv_len = 0; // 已接收数据长度 
	int data_len = 0; // 数据长度 
	char buf[MAX_BUF_SIZE] = {0}; // 缓冲区 
	int left_len = 0;
	int ret = 0;
	while (1) { // 处理包头 
		//sleep(0.8);
		//printf("123\n");
		memset(buf, 0, sizeof(buf)); // 初始化并清空缓冲区
		//char buf[4];
		while (recv_len < HEADER_SIZE) {
			ret = recv(thread_arg->Cfd, buf + recv_len, HEADER_SIZE - recv_len, 0); // 接收包头 
			if (ret < 0) { // 接收数据失败 
				//perror("recv header error");
				continue;
			}
			else if (ret == 0) { // 服务器已关闭连接 
				//printf("server closed connection\n");
				pthread_mutex_destroy(&mutex); // 销毁互斥锁
                                close(thread_arg->Cfd);
                                printf("客户端断开了连接\n");
				pthread_exit(NULL);
			}
			recv_len += ret; // 更新已接收数据长度 
		}
		if (recv_len < 4) {
			continue; // 接收到的数据不足4个字节，继续接收 
		}
		//printf("recv_len = %d\n",recv_len);
		// 接收到完整的包头，获取数据长度
		//printf("buf中的数据 %s\n",buf);
		data_len = ntohl(*(int*)buf); // 注意字节序转换 
		//printf("包体长度 = %d\n", data_len);
		if (data_len > MAX_BUF_SIZE - HEADER_SIZE) { // 数据长度超出缓冲区大小 
			printf("data too long\n");
			break;
		}

		// 处理包体 
		//printf("123");
		memset(buf, 0, sizeof(buf)); // 初始化并清空缓冲区 
		//printf("456");
		//char bufeer[1024];
		while (recv_len - HEADER_SIZE < data_len) {
			left_len = data_len - (recv_len - HEADER_SIZE); // 剩余数据长度 
			ret = recv(thread_arg->Cfd, buf + recv_len - HEADER_SIZE, left_len, 0); // 接收数据 
			if (ret < 0) { // 接收数据失败 
				//perror("recv data error");
				//break;
				continue;
			}
			else if (ret == 0) { // 服务器已关闭连接 
				//printf("server closed connection\n");
				//break;
				pthread_mutex_destroy(&mutex); // 销毁互斥锁
                                close(thread_arg->Cfd);
                                printf("客户端断开了连接\n");
                                pthread_exit(NULL);
			}
			recv_len += ret; // 更新已接收数据长度 
		}
		//printf("接收到的数据%s\n", buf); // 输出接收到的数据
	        pthread_mutex_lock(&mutex); // 加锁
                strcpy(bufer, buf);
                //sprintf(bufeer, "%.6lf#%.6lf", a, b);
                pthread_mutex_unlock(&mutex); // 解锁	
		recv_len = 0; // 重置已接收数据长度 
		data_len = 0; // 重置数据长度
	}
}

int main(int argc, char const* argv[]) {
    arg = (struct ThreadArg*)malloc(sizeof(struct ThreadArg));
    pthread_t thread1,thread2;
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char* hello = "Hello from server";

    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置socket选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 设置地址族、IP地址和端口号
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定socket到地址
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 开始监听
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("开始监听....\n");
    while (1) {
        // 接受客户端连接
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        else {
            char buf[4] = { 0 };
            recv(new_socket, buf, sizeof(buf), 0);
            printf("---------%s---------\n", buf);
            if (buf[0] == 'W') {
		arg->Wfd = new_socket;
                if (pthread_create(&thread1, NULL, thread_func1, arg) != 0) {
                    printf("pthread_create failed\n");
                }
                //设置线程分离属性
                pthread_detach(thread1);
            }
	    if (buf[0] == 'C') {
                arg->Cfd = new_socket;
                if (pthread_create(&thread2, NULL, thread_func2, arg) != 0) {
                    printf("pthread_create failed\n");
                }
                //设置线程分离属性
                pthread_detach(thread2);
            }

        }
    }

    return 0;
}
