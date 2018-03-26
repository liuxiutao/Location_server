#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

//连接信息
#define BACKLOG 5         //完成三次握手但没有accept的队列的长度
#define CONCURRENT_MAX 10 //应用层同时可以处理的连接
#define SERVER_PORT 9001
#define BUFFER_SIZE 512

//场地信息
#define AREA_LENGTH 6  //长
#define AREA_WIDTH 6   //宽
#define SCALE 10        //精度

//MIC 信息
#define NODE_NUMBER 8
#define MIC_DISTANCE 0.135

//在communication.cc中定义
extern long double measure_data[];
extern int data_flag[];

//在main.cc中定义
extern double result[];
extern bool isAvailable;

void * calculation(void * ptr);
void * communication(void * ptr);
