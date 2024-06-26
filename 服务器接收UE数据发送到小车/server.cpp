#include"server.h"
#define MAX_BUF_SIZE 1024 // 定义缓冲区大小为1024字节 
#define HEADER_SIZE 4 // 定义包头大小为4字节


struct ThreadArg {
	int Cfd;
	int Wfd;
	int epfd;
	int SQLfd;
};

char bufer[1024] = {"0.000#0.000"}; // 定义全局变量
pthread_t thread1, thread2, thread3;
struct ThreadArg* arg = (struct ThreadArg*)malloc(sizeof(struct ThreadArg));
int UE_Control_Car = 1;

int initListenFd(unsigned short port) {
	//1.创建监听的fd
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("socket");
		return -1;
	}
	//2.设置端口复用
	int opt = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret == -1) {
		perror("setsockopt");
		return -1;
	}
	//3.绑定
	struct sockaddr_in  addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		perror("bind");
		return -1;
	}
	//4.设置监听
	ret = listen(lfd, 128);
	if (ret == -1) {
		perror("listen");
		return -1;
	}
	//5.返回fd
	return lfd;
}

int epollRun(int lfd) {
	//1.创建epoll实列
	int epfd = epoll_create(1);
	if (epfd == -1) {
		perror("epoll_create");
		return -1;
	}
	arg->epfd = epfd;
	//2.lfd上树
	struct epoll_event ev;
	ev.data.fd = lfd;
	ev.events = EPOLLIN;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
	if (ret == -1) {
		perror("epoll_ctl");
		return -1;
	}
	//3.检测
	struct epoll_event evs[1024];
	int size = sizeof(evs) / sizeof(struct epoll_event);
	while (1) {
		int num = epoll_wait(epfd, evs, size, -1);
		if (num == -1) {
			perror("epoll_wait");
			return -1;
		}
		for (int i = 0; i < num; ++i) {
			int fd = evs[i].data.fd;
			if (fd == lfd) {
				//建立新连接accept
				printf("接收到新的客户端连接........\n");
				acceptClient(lfd, epfd);
			}
			else {
				//主要接收对端的数据
				//RecvMessage(fd,epfd);
			}
		}
	}
	return 0;
}

int acceptClient(int lfd, int epfd) {
	//建立连接
	int cfd = accept(lfd, NULL, NULL);
	if (cfd == -1) {
		perror("accpet");
		return -1;
	}
	//设置非阻塞
	int flag = fcntl(cfd, F_GETFL);

	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);
	//cfd添加到epoll
	struct epoll_event ev;
	ev.data.fd = cfd;
	ev.events = EPOLLIN | EPOLLET;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
	if (ret == -1) {
		perror("epoll_ctl");
		return -1;
	}
	sleep(1);
	char buf[4] = { 0 };
	recv(cfd, buf, sizeof(buf), 0);
	printf("---------%s---------\n", buf);
	if (buf[0] == 'C') {
		arg->Cfd = cfd;

		if (pthread_create(&thread1, NULL, thread_func1, arg) != 0) {
			printf("pthread_create failed\n");
		}
		//设置线程分离属性
		pthread_detach(thread1);
	}
	else if (buf[0] == 'W') {
		arg->Wfd = cfd;
		if (pthread_create(&thread2, NULL, thread_func2, arg) != 0) {
			printf("pthread_create failed\n");
		}
		//设置线程分离属性
		pthread_detach(thread2);
		//pthread_join(thread2,NULL);
	}
	else if (buf[0] == 'Q') {
		arg->SQLfd = cfd;
		if (pthread_create(&thread3, NULL, thread_func3, arg) != 0) {
			printf("pthread_create failed\n");
		}
		//设置线程分离属性
		pthread_detach(thread3);
	}
	return 0;
}

void* thread_func1(void* arg) {
	struct ThreadArg* thread_arg = (struct ThreadArg*)arg;
	pthread_mutex_t mutex;   // 定义互斥锁
	pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
	int len = 0, pos = 0;
	char buf[1024] = {"0.000#0.000"};
	if (UE_Control_Car == 0) {
		UE_Control_Car = 1;
	}
	while (1) {
		sleep(0.5);
		while ((len = recv(thread_arg->Cfd, buf + pos, sizeof(buf) - pos - 1, 0)) > 0) {
			pos += len;
			// 查找 \n，如果找到了就处理数据
			char* p = strchr(buf, '\n');
			while (p != NULL) {
				*p = '\0';
				// 处理数据，打印到屏幕上
				pthread_mutex_lock(&mutex); // 加锁
				strcpy(bufer, buf);
				pthread_mutex_unlock(&mutex); // 解锁
				//printf("%s\n",buf);
				// 从缓存中删除已经处理过的数据
				pos = pos - (p - buf + 1);
				memmove(buf, p + 1, pos);
				p = strchr(buf, '\n');
			}
		}
		if (len == 0) {
			//客户端断开了连接
			epoll_ctl(thread_arg->epfd, EPOLL_CTL_DEL, thread_arg->Cfd, NULL);
			close(thread_arg->Cfd);
			printf("ue客户端断开了连接\n");
			pthread_exit(NULL);
		}
		/*else if(len==-1){
			perror("xuyaorecv");
		}*/
	}
}


void* thread_func2(void* arg) {
	// 发送数据到对应客户端
	//double a, b;
	struct ThreadArg* thread_arg = (struct ThreadArg*)arg;
	pthread_mutex_t mutex;   // 定义互斥锁
	pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
	signal(SIGPIPE, SIG_IGN);
	char bufeer[1024] = { 0 };
	//定义一个辅助指针，指向我们要发送的那个buf的起始地址
	/*char* buf;
	buf = bufeer;*/
	int size = 0;
	int biglen = 0 , falg = 0;
	//srand(time(NULL));
	while (1) {
		//printf("进入一级循环\n");
		/*a = (double)rand() / RAND_MAX * 1000000;
		b = (double)rand() / RAND_MAX * 1000000;*/

		int count = 0;
		pthread_mutex_lock(&mutex); // 加锁
		strcpy(bufeer, bufer);
		//sprintf(bufeer, "%.6lf#%.6lf", a, b);
		pthread_mutex_unlock(&mutex); // 解锁
		size = strlen(bufeer);
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
			sleep(0.8);
			//printf("进入二级循环: count = %d  data = %s\n",count,data);
			int len = send(thread_arg->Wfd, data, count, 0);
			if (len <= 0 || falg == 1) {
				if (errno == EPIPE || len == 0 || falg == 1) {
					pthread_mutex_destroy(&mutex); // 销毁互斥锁
					epoll_ctl(thread_arg->epfd, EPOLL_CTL_DEL, thread_arg->Wfd, NULL);
					close(thread_arg->Wfd);
					printf("nano客户端断开了连接\n");
					if (data_start != NULL) {
						free(data_start);
						data_start = NULL;
					}
					pthread_exit(NULL);
				}
				else {
					//perror("send");
					continue;
				}
			}
			else if(len < count){
				data += len;
				count -= len;
			}
			else if (len == count) {
				break;
			}
		}
		//printf("数据已经发送: %s\n",data_start);
		if (data_start != NULL) {
			free(data_start);
			data_start = NULL;
		}
		//printf("退出二级循环: data5 = %c,data19 = %c\n",data[4],data[18]);
	}
}

void* thread_func3(void* arg) {
	struct ThreadArg* thread_arg = (struct ThreadArg*)arg;
	int recv_len = 0; // 已接收数据长度 
	int data_len = 0; // 数据长度 
	char buf[MAX_BUF_SIZE]; // 缓冲区 
	int left_len = 0;
	int ret = 0;
	while (1) { // 处理包头 
		//sleep(0.5);
		//printf("123\n");
		while (recv_len < HEADER_SIZE) {
			ret = recv(thread_arg->SQLfd, buf + recv_len, HEADER_SIZE - recv_len, 0); // 接收包头 
			if (ret < 0) { // 接收数据失败 
				//perror("recv header error");
			}
			else if (ret == 0) { // 服务器已关闭连接 
				printf("server closed connection\n");
				break;
			}
			recv_len += ret; // 更新已接收数据长度 
		}
		if (recv_len < 4) {
			continue; // 接收到的数据不足4个字节，继续接收 
		}

		printf("recv_len = %d\n",recv_len);
		// 接收到完整的包头，获取数据长度
		printf("buf中的数据 %s\n",buf);
		data_len = ntohl(*(int*)buf); // 注意字节序转换 
		printf("包体长度 = %d\n", data_len);
		if (data_len > MAX_BUF_SIZE - HEADER_SIZE) { // 数据长度超出缓冲区大小 
			printf("data too long\n");
			break;
		}

		// 处理包体 
		printf("123");
		memset(buf, 0, sizeof(buf)); // 初始化并清空缓冲区 
		printf("456");
		while (recv_len - HEADER_SIZE < data_len) {
			left_len = data_len - (recv_len - HEADER_SIZE); // 剩余数据长度 
			ret = recv(thread_arg->SQLfd, buf + recv_len - HEADER_SIZE, left_len, 0); // 接收数据 
			if (ret < 0) { // 接收数据失败 
				//perror("recv data error");
				//break;
			}
			else if (ret == 0) { // 服务器已关闭连接 
				printf("server closed connection\n");
				break;
			}
			recv_len += ret; // 更新已接收数据长度 
		}
		printf("接收到的数据%s\n", buf); // 输出接收到的数据 
		recv_len = 0; // 重置已接收数据长度 
		data_len = 0; // 重置数据长度
	}
}