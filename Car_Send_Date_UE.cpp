#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include<errno.h>
#include <stdint.h>

#define SERVER_IP "47.94.128.29"
#define SERVER_PORT 7070
struct ThreadArg {
    int Wfd;
    pthread_mutex_t mutex;
};
pthread_t thread1;
struct ThreadArg* arg;
double data1=0.0,data2=0.0,data3=0.0,data4=0.0,data5=0.0,data6=0.0,data7=0.0,data8=0.0,data9=10.0;

void odomCallback(const nav_msgs::Odometry::ConstPtr&msg){
	//pthread_mutex_lock(&(arg->mutex)); // 加锁
        data1=msg->twist.twist.linear.x/8;
        data2=msg->twist.twist.angular.z/8;
        data3=msg->twist.twist.angular.z/2;
        data4=msg->twist.twist.linear.x/2;
        data5=msg->twist.twist.angular.z/3;
        data6=msg->twist.twist.angular.z/6;
        data7=msg->twist.twist.angular.z/1;
        data8=msg->twist.twist.angular.z/7;
	printf("%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",data1, data2,data3,data4,data5,data6,data7,data8,data9);
	//pthread_mutex_unlock(&(arg->mutex)); // 解锁
}

void* thread_func1(void* arg) {
    // 发送数据到对应客户端
    //double a, b;
    struct ThreadArg* thread_arg = (struct ThreadArg*)arg;
    pthread_mutex_t mutex;   // 定义互斥锁
    pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
    char bufeer[1024] = { 0 };
    int size = 0;
    int biglen = 0, falg = 0;
    //srand(time(NULL));
    while (1) {
        //a = (double)rand() / RAND_MAX * 1000000;
        //b = (double)rand() / RAND_MAX * 1000000;

        int count = 0;
        pthread_mutex_lock(&mutex); // 加锁
        //strcpy(bufeer, bufer);
        sprintf(bufeer, "%.10lf#%.10lf#%.10lf#%.10lf#%.10lf#%.10lf#%.10lf#%.10lf#%.10lf", data1, data2,data3,data4,data5,data6,data7,data8,data9);
        //printf("发送前bufeer = %s\n",bufeer);
        pthread_mutex_unlock(&mutex); // 解锁
        size = strlen(bufeer);
        //定义一个整形数用于记录剩下待发送的字节数
        char* data = NULL;
        data = (char*)malloc(size + 4);
        char* data_start = data;  // 保存起始地址
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
        count = size + 4;
        while (1) {
            sleep(0.8);
            int len = send(thread_arg->Wfd, data, count, 0);
            if (len <= 0 || falg == 1) {
                if (errno == EPIPE || len == 0 || falg == 1) {
                    //pthread_mutex_destroy(&mutex); // 销毁互斥锁
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
            //printf("数据已经发送:  %s\n", data_start + 4);
            if (data_start != NULL) {
                free(data_start);
                data_start = NULL;
            }
        }
    
}
int main(int argc, char* argv[])
{
	pthread_mutex_t mutex;   // 定义互斥锁
        pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
	//arg->mutex = mutex;
        int client_sockfd;
        struct sockaddr_in server_addr;
        char buffer[1024];
        arg = (struct ThreadArg*)malloc(sizeof(struct ThreadArg));
        // 创建客户端套接字
	arg->mutex = mutex;
        client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sockfd < 0) {
            perror("socket error");
            exit(EXIT_FAILURE);
        }

        // 设置服务器地址
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
        server_addr.sin_port = htons(SERVER_PORT);

        // 连接服务器
        if (connect(client_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect error");
            exit(EXIT_FAILURE);
        }
        arg->Wfd = client_sockfd; strcpy(buffer, "C");
        if (write(client_sockfd, buffer, strlen(buffer)) < 0) {
            perror("write error");
            exit(EXIT_FAILURE);
        }
	printf("开始执行\n");
        sleep(1);
        if (pthread_create(&thread1, NULL, thread_func1, arg) != 0) {
            printf("pthread_create failed\n");
        }
        //设置线程分离属性
        pthread_detach(thread1);
        ros::init(argc,argv,"car_pub_UE");
        ros::NodeHandle nh;
        ros::Subscriber sub = nh.subscribe("/odom",1000,odomCallback);
        ros::spin();
        return 0;
}



