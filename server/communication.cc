#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>
#include "location.h"

using namespace std;

long double measure_data[NODE_NUMBER] = {0};
int data_flag[NODE_NUMBER] = {0};


void *communication(void *ptr)
{
    char recv_msg[BUFFER_SIZE];
    //本地地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int res = -1;
    //创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket error");
        exit(1);
    }

    //绑定socket
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind error");
        exit(1);
    }

    //listen
    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("listen error");
        exit(1);
    }
    cout << "开始监听，等待连接中..." << endl;

    int max_fd = -1;
    //fd_set
    fd_set readfds;
    FD_ZERO(&readfds); //初始化文件集

    FD_SET(listenfd, &readfds); //将需要监视的listenfd放入readfds集中
    if (max_fd < listenfd)
    {
        max_fd = listenfd;
    }
    FD_SET(STDIN_FILENO, &readfds);
    if (max_fd < STDIN_FILENO)
    {
        max_fd = STDIN_FILENO;
    }

    //select超时设置
    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec = 0;

    while (1)
    {
        fd_set tmpfds = readfds;
        //设置监视，监视tmpfds内的子fd变化,发生变化的将会被保留在tmpfds中
        int res = select(max_fd + 1, &tmpfds, NULL, NULL, NULL);
        if (res <= 0)
        {
            continue;
        }
        else
        {
            for (int fd1 = 0; fd1 < max_fd + 1; fd1++)
            {
                if (FD_ISSET(fd1, &tmpfds))
                {
                    if (fd1 == listenfd) //有新的连接请求
                    {
                        struct sockaddr_in client_addr;
                        socklen_t addrlen = sizeof(client_addr);
                        int clientfd = accept(listenfd, (struct sockaddr *)&client_addr, &addrlen);
                        FD_SET(clientfd, &readfds);
                        if (clientfd > max_fd)
                        {
                            max_fd = clientfd;
                        }
                        cout << "新客户端加入成功：" << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << endl;
                    }
                    else //有数据到来
                    {
                        int recvSize = recv(fd1, recv_msg, BUFFER_SIZE, 0);
                        recv_msg[recvSize] = '\0';
                        if(recvSize == 0)
                        {
                            close(fd1);
                            FD_CLR(fd1, &readfds);
                            cout << "客户机断开连接" << endl;
                        }
                        else
                        {
                            if (fd1 != STDIN_FILENO)
                            {
                                int node_index = 0;
                                char offset_data[5];
                                for (int l = 0; l < recvSize; l++)
                                {
                                    if (recv_msg[l] == '#') //读取节点编号
                                    {
                                        node_index = recv_msg[++ l] - '0';
                                    }
                                    else if(recv_msg[l] == '!') //读取TDOA值
                                    {
                                        int k = 0;
                                        while(recv_msg[l + 1] != '~')
                                        {
                                            offset_data[k++] = recv_msg[++l];
                                        }
                                        offset_data[k] = '\0';
                                        break;
                                    }
                                }
                                measure_data[node_index] = atof(offset_data) / (44100 * 1.0);
                                //cout << atof(offset_data) << " "<<node_index <<endl;
                                data_flag[node_index] = 1;
                            }
                        }
                    }
                }
            }

        }
    }
    return 0;
}
