#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "47.94.128.29"
#define SERVER_PORT 8080
#define MAX_BUF_SIZE 1024 // 定义缓冲区大小为1024字节
#define HEADER_SIZE 4 // 定义包头大小为4字节
pthread_t thread1, thread2;
char bufeer[1024] = {0};
double val1 = 0.000, val2 = 0.000;
       
struct ThreadArg {
	int client_socket;
	ros::Publisher* pub_th;
	geometry_msgs::Twist* msg_th;
};

struct ThreadArg* arg;

int InitSocket() {
	// 创建socket
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	// 设置服务器地址信息
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_address.sin_port = htons(SERVER_PORT);

	// 连接服务器
	if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		perror("connection failed");
		exit(EXIT_FAILURE);
	}
	return client_socket;
}

void* thread_func1(void* arg){
	printf("线程开始运行....\n");
	struct ThreadArg* thread_arg = (struct ThreadArg*)arg;
	pthread_mutex_t mutex;   // 定义互斥锁
	pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
	int recv_len = 0; // 已接收数据长度 
	int data_len = 0; // 数据长度 
	char buf[MAX_BUF_SIZE]; // 缓冲区 
	int left_len = 0;
	int ret = 0;
	while (1) { // 处理包头 
		while (recv_len < HEADER_SIZE) {
			ret = recv(thread_arg->client_socket, buf + recv_len, HEADER_SIZE - recv_len, 0); // 接收包头 
			if (ret < 0) { // 接收数据失败 
				//perror("recv header error");
				//break;
				continue;
			}
			else if (ret == 0) { // 服务器已关闭连接 
				//printf("server closed connection\n");
				//break;
				printf("服务器断开了连接\n");
				pthread_mutex_destroy(&mutex); // 销毁互斥锁
				close(thread_arg->client_socket);
				pthread_exit(NULL);
			}
			recv_len += ret; // 更新已接收数据长度 
		}
		// 接收到完整的包头，获取数据长度 
		data_len = ntohl(*(int*)buf); // 注意字节序转换 
		if (data_len > MAX_BUF_SIZE - HEADER_SIZE) { // 数据长度超出缓冲区大小 
			printf("data too long\n"); 
			break;
		}

		// 处理包体 
		memset(buf, 0, sizeof(buf)); // 初始化并清空缓冲区 
		while (recv_len - HEADER_SIZE < data_len) {
			left_len = data_len - (recv_len - HEADER_SIZE); // 剩余数据长度 
			ret = recv(thread_arg->client_socket, buf + recv_len - HEADER_SIZE, left_len, 0); // 接收数据 
			if (ret < 0) { // 接收数据失败 
				//perror("recv data error");
				//break;
				continue;
			}
			else if (ret == 0) { // 服务器已关闭连接 
				//printf("server closed connection\n");
				//break;
				printf("服务器断开了连接\n");
				pthread_mutex_destroy(&mutex); // 销毁互斥锁
                                close(thread_arg->client_socket);
                                pthread_exit(NULL);
			}
			recv_len += ret; // 更新已接收数据长度 
		}
		//printf("%s\n", buf); // 输出接收到的数据
		char* token = NULL;
	        pthread_mutex_lock(&mutex); // 加锁
        	token = strtok(buf, "#");
            	val1 = atof(token);
        	token = strtok(NULL, "#");
            	val2 = atof(token);
		pthread_mutex_unlock(&mutex); //解锁
		recv_len = 0; // 重置已接收数据长度 
		data_len = 0; // 重置数据长度
	
	}
}


void* thread_func2(void* arg) {
	struct ThreadArg* thread_arg = (struct ThreadArg*)arg;
	double data1 = 0.000,data2 = 0.000;
	int flag = 1;
	pthread_mutex_t mutex;   // 定义互斥锁
        pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
    	while (ros::ok())
    	{
		pthread_mutex_lock(&mutex); // 加锁
		if(flag == 1){
		    data1 = val1/2;
		    data2 = val2/2;
	            flag = 2;
		}else if(flag == 2){
		   if(data1 == val1 && data2 ==val2){
		       pthread_mutex_unlock(&mutex); //解锁
		       continue;
		   }
		}
		data1 = val1/2;
		data2 = val2/2;
		pthread_mutex_unlock(&mutex); //解锁
		printf("***%f#%f***\n",data1,data2);
    		thread_arg->msg_th->linear.x = data1;
    		thread_arg->msg_th->linear.y = 0.0;
    		thread_arg->msg_th->linear.z = 0.0;
    		thread_arg->msg_th->angular.x = 0.0;
    		thread_arg->msg_th->angular.y = 0.0;
    		thread_arg->msg_th->angular.z = data2;
        	thread_arg->pub_th->publish(*(thread_arg->msg_th));
        	ros::spinOnce();
    	}
}

int main(int argc, char *argv[])
{
	arg = (struct ThreadArg*)malloc(sizeof(struct ThreadArg));
	printf("程序开始运行\n");
	setlocale(LC_ALL,"");
        // 2.初始化 ROS 节点
        ros::init(argc,argv,"car");
        ros::NodeHandle nh;
        // 3.创建发布者对象
        ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel",1000);
        // 4.循环发布运动控制消息
        //4-1.组织消息
        geometry_msgs::Twist msg;
	arg->pub_th = &pub;
	arg->msg_th = &msg;
	printf("ros初始化完成\n");
	int client_socket = InitSocket();
	//发送消息到服务器
	arg->client_socket = client_socket;
	char message[] = "W";
	if (send(client_socket, message, strlen(message), 0) < 0) {
		perror("send failed");
		exit(EXIT_FAILURE);
	}
     	if (pthread_create(&thread1, NULL, thread_func1, arg) != 0) {
		printf("pthread_create failed\n");
		}
	if (pthread_create(&thread2, NULL, thread_func2, arg) != 0) {
		printf("pthread_create failed\n");
	}
	printf("线程全部启动\n");
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	printf("线程结束\n");

    return 0;
}

