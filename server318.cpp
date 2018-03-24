#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include<unistd.h>
#include<pthread.h>
#include<iostream>
//#include"tdoa.h"
#include"tdoa318.h"
//#include"tdoa_n.h"
//#include"tdoa_relax.h"
#define BACKLOG 5 //完成三次握手但没有accept的队列的长度
#define CONCURRENT_MAX 10 //应用层同时可以处理的连接
#define SERVER_PORT 9001
#define BUFFER_SIZE 1024
#define Node_number 8
using namespace std;

int client_fds[CONCURRENT_MAX];
long double measure_data[Node_number]={0};
int data_flag[Node_number]={0};
double Microphone_Cita[Node_number]={0,270,0,270,0,270,0,270};
double Microphone_Center_Location[Node_number][2]={3,3,3,3,6,3,6,3,3,6,3,6,6,6,6,6};
double Microphone_Distance=0.135;
double Room_Length=9;
double Room_Width=9;
int scale=10;
double result[2]={0,0};

void *calculate(void *ptr)
{
		while(1)

		{
		sleep(1);
		int flag_count=0;
		for(int i=0;i<Node_number;i++)
		{
				if(data_flag[i]==1)
						flag_count=flag_count+1;
		}
		if(flag_count>3)
		{
	    tdoa(Node_number, measure_data,Microphone_Cita, Microphone_Center_Location, Microphone_Distance,Room_Length,Room_Width,scale,result,data_flag);
				cout<<"结果为：X="<<result[0]<<"  Y="<<result[1]<<endl;
				for (int i=0;i<Node_number;i++)
						data_flag[i]=0;
		}
		}

		return 0;
}

int main (int argc, const char * argv[])
{
		char input_msg[BUFFER_SIZE];
        char recv_msg[BUFFER_SIZE];   
        //本地地址
        struct sockaddr_in server_addr;
        //server_addr.sin_len = sizeof(struct sockaddr_in);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = inet_addr("192.168.191.3");
        bzero(&(server_addr.sin_zero),8);
        //创建socket
        int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock_fd == -1) {
            perror("socket error");
            return 1;
        }
        //绑定socket
        int bind_result = bind(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (bind_result == -1) {
            perror("bind error");
            return 1;
        }
        //listen
        if (listen(server_sock_fd, BACKLOG) == -1) {
            perror("listen error");
            return 1;
        }
        //fd_set
        fd_set server_fd_set;
        int max_fd = -1;
        struct timeval tv;
        tv.tv_sec = 20;
        tv.tv_usec = 0;
		pthread_t id;
		int ret = pthread_create(&id, NULL, calculate, NULL);
       // pthread_join(id, NULL);
		
        while (1) {
            FD_ZERO(&server_fd_set);
            //标准输入
            FD_SET(STDIN_FILENO, &server_fd_set);
            if (max_fd < STDIN_FILENO) {
                max_fd = STDIN_FILENO;
            }
            //服务器端socket
            FD_SET(server_sock_fd, &server_fd_set);
            if (max_fd < server_sock_fd) {
                max_fd = server_sock_fd;
            }
            //客户端连接
            for (int i = 0; i < CONCURRENT_MAX; i++) {
                if (client_fds[i]!=0) {
                    FD_SET(client_fds[i], &server_fd_set);

                    if (max_fd < client_fds[i]) {
                        max_fd = client_fds[i];
                    }
                }
            }
            int ret = select(max_fd+1, &server_fd_set, NULL, NULL, &tv);
            if (ret < 0) {
                perror("select 出错\n");
                continue;
            }else if(ret == 0){
                printf("select 超时\n");
                continue;
            }else{
               /* //ret为未状态发生变化的文件描述符的个数
                if (FD_ISSET(STDIN_FILENO, &server_fd_set)) {
                    //标准输入
                    bzero(input_msg, BUFFER_SIZE);
                    fgets(input_msg, BUFFER_SIZE, stdin);
                    //输入 ".quit" 则退出服务器
                    if (strcmp(input_msg, QUIT_CMD) == 0) {
                        exit(0);
                    }
                    for (int i=0; i<CONCURRENT_MAX; i++) {
                        if (client_fds[i]!=0) {
                            send(client_fds[i], input_msg, BUFFER_SIZE, 0);
                        }
                    }
                }*/
                if (FD_ISSET(server_sock_fd, &server_fd_set)) {
                    //有新的连接请求
                    struct sockaddr_in client_address;
                    socklen_t address_len;
                    int client_socket_fd = accept(server_sock_fd, (struct sockaddr *)&client_address, &address_len);
                    if (client_socket_fd > 0) {
                        int index = -1;
                        for (int i = 0; i < CONCURRENT_MAX; i++) {
                            if (client_fds[i] == 0) {
                                index = i;
                                client_fds[i] = client_socket_fd;
                                break;
                            }
                        }
                        if (index >= 0) {
                            printf("新客户端(%d)加入成功 %s:%d \n",index,inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
                        }else{
                            bzero(input_msg, BUFFER_SIZE);
                            strcpy(input_msg, "服务器加入的客户端数达到最大值,无法加入!\n");
                            send(client_socket_fd, input_msg, BUFFER_SIZE, 0);
                            printf("客户端连接数达到最大值，新客户端加入失败 %s:%d \n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
                        }
                    }
                }
                for (int i = 0; i <CONCURRENT_MAX; i++) {
                    if (client_fds[i]!=0) {
                        if (FD_ISSET(client_fds[i], &server_fd_set)) {
                            //处理某个客户端过来的消息
                            bzero(recv_msg, BUFFER_SIZE);
                            long byte_num = recv(client_fds[i],recv_msg,BUFFER_SIZE,0);
                            if (byte_num > 0) 
							{
                                if (byte_num > BUFFER_SIZE)
								{
                                    byte_num = BUFFER_SIZE;
                                }
                                recv_msg[byte_num] = '\0';
								//int ret = pthread_create(&id, NULL, calculate, NULL);
								char data_temp[byte_num];
								for(int l=0;l<byte_num;l++)
								{
										if(recv_msg[l]=='!')
										{
												int k=l+1;
												for(int j=0;j<byte_num;j++)
												{
														if(recv_msg[k]!='~')
														{
																data_temp[j]=recv_msg[k];
																k++;
														}
														else
																break;
												}
										}
								}

								int index_phone=0;
								for(int l=0;l<byte_num;l++)
								{
										if(recv_msg[l]=='#')
										{
												index_phone=recv_msg[l+1]-'0';
												break;
										}
								}

                                //printf("客户端(%d):%s\n",i,recv_msg);
								if(data_temp[0]!='^')
								{
										double b=atof(data_temp);
										if(abs(b)<22)
										{
										measure_data[index_phone]=b/(44100*1.0);
										data_flag[index_phone]=1;  
										}

                            }
							}
								else if(byte_num < 0){
                                printf("从客户端(%d)接受消息出错.\n",i);
                            }else{
                                FD_CLR(client_fds[i], &server_fd_set);
                                client_fds[i] = 0;
                                printf("客户端(%d)退出了\n",i);
                            }
                        }
                    }
                }
            }
        }
//		pthread_join(id, NULL);
        return 0;
}
