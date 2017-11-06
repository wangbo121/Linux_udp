/*
 * udp.cpp
 *
 *  Created on: Nov 4, 2017
 *      Author: wangbo
 */

#include "udp.h"


#include<stdio.h>
#include <stdint.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/select.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<netdb.h>
#include<stdarg.h>
#include<string.h>
#include<math.h>

/*转换int或者short的字节顺序，该程序arm平台为大端模式，地面站x86架构为小端模式*/
#include <byteswap.h>


#include "udp.h"

uint64_t htonll(uint64_t n) {
return (((uint64_t)htonl(n)) << 32) | htonl(n >> 32);
}
uint64_t ntohll(uint64_t n) {
return (((uint64_t)ntohl(n)) << 32) | ntohl(n >> 32);
}

double ntoh_double(double net_double) {
uint64_t host_int64;
host_int64 = ntohll(*((uint64_t *) &net_double));
return *((double *) &host_int64);
}

double hton_double(double host_double) {
uint64_t net_int64;
net_int64 = htonll(*((uint64_t *) &host_double));
return *((double *) &net_int64);
}

float ntoh_float(float net_float) {
    uint32_t host_int32;
    host_int32 = ntohl(*((uint32_t *) &net_float));
    return *((double *) &host_int32);

}
float hton_float(float host_float) {
    uint32_t net_int32;
    net_int32 = htonl(*((uint32_t *) &host_float));
    return *((double *) &net_int32);

}

double htond (double x)
{
    int * p = (int*)&x;
    int tmp = p[0];
    p[0] = htonl(p[1]);
    p[1] = htonl(tmp);

    return x;
}

float htonf (float x)
{
    int * p = (int *)&x;
    *p = htonl(*p);
    return x;
}

int open_socket_udp_dev(int *ptr_fd_socket, char* ip, unsigned int port)
{
    int fd_socket;
    struct sockaddr_in socket_udp_addr;//保存ip和端口属性
    int sockaddr_size;

    /*设置发送目标的ip地址和发送目标的端口*/
    sockaddr_size = sizeof(struct sockaddr_in);
    bzero((char*)&socket_udp_addr, sockaddr_size);
    socket_udp_addr.sin_family = AF_INET;

    if(strcmp("0.0.0.0",ip) == 0)
    {
        printf("open_socket_udp_dev    :    创建套接字时没有绑定ip 也就是该套接字的ip由系统自动分配，而ip由网卡决定，一般系统会用最低序号的网卡的ip\n");
        socket_udp_addr.sin_addr.s_addr = INADDR_ANY;//INADDR_ANY是任意地址
    }
    else
    {
        inet_pton(AF_INET, ip, &socket_udp_addr.sin_addr);
    }

    socket_udp_addr.sin_port = htons(port);

    /*利用socket函数建立套接字*/
    fd_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_socket == -1)
    {
        printf("udp %s create socket failed",ip);
        close(fd_socket );
        return 0;
    }
    else
    {
        *ptr_fd_socket = fd_socket;
        printf("***udp IP:%s PORT:%d ini ok!\n***udp fd_socket = %d\n",ip, port,*ptr_fd_socket);
    }

    /*
     * 把fd_socket这个套接字绑定到socket_udp_addr所表示的ip和端口
     */
    if(-1 == (bind(fd_socket,(struct sockaddr*)&socket_udp_addr,sizeof(struct sockaddr_in))))
    {
        perror("Server Bind Failed:");
        //exit(1);
    }
    else
    {
        printf("bind success\n");
    }

    return 0;
}

int send_socket_udp_data(int fd_send_socket, unsigned char *buf, unsigned int len, char *target_ip, unsigned int target_port)
{
    int sockaddr_size;
    struct sockaddr_in udp_sendto_addr;//用于保存发送目标的ip和端口等信息

    sockaddr_size = sizeof(struct sockaddr_in);
    bzero((char*)&udp_sendto_addr, sockaddr_size);

    /* 1. 协议类型*/
    udp_sendto_addr.sin_family = AF_INET;//这个socket的协议类型

    /* 2. ip地址*/
    //udp_sendto_addr.sin_addr.s_addr = inet_addr(ip_sendto);//这个语句也可以设置ip地址，但是我用了下面的函数inet_pton
    //inet_pton(AF_INET, "10.108.16.163", &udp_sendto_addr.sin_addr);
    inet_pton(AF_INET, target_ip, &udp_sendto_addr.sin_addr);

    /* 3. 端口号*/
    udp_sendto_addr.sin_port = htons(target_port);

    int send_len;
    unsigned char send_buf[2000];

    send_len=len;
    memcpy(send_buf, buf, len);

    sendto(fd_send_socket, send_buf, send_len, 0, (struct sockaddr *)&udp_sendto_addr, sizeof(struct sockaddr_in));

    return 0;
}

/*
 * 收到udp的数据后，最终的处理函数在这里，根据通信协议一个个字节解析
 */
int read_socket_udp_data(unsigned char *buf, unsigned int len)
{

    return 0;
}

void *udp_recvbuf_and_process(void * ptr_udp_device)
{
    char buf[UDP_RCV_BUF_SIZE] = { 0 };
    unsigned int read_len;

    struct T_UDP_DEVICE udp;
    udp.fd_socket=((struct T_UDP_DEVICE *)ptr_udp_device)->fd_socket;
    udp.ptr_fun=((struct T_UDP_DEVICE *)ptr_udp_device)->ptr_fun;
    udp.fd_receive_thread=((struct T_UDP_DEVICE *)ptr_udp_device)->fd_receive_thread;
    udp.ip = ((struct T_UDP_DEVICE *)ptr_udp_device)->ip;
    udp.port = ((struct T_UDP_DEVICE *)ptr_udp_device)->port;

    struct sockaddr_in from_addr;//接收数据时，数据包中有ip地址和端口的，这个函数把ip和端口等socket属性保存在from_addr中
    unsigned int from_addr_len=0;
    from_addr_len=sizeof(struct sockaddr);

    int sockaddr_size;

    sockaddr_size = sizeof(struct sockaddr_in);
    bzero((char*)&from_addr, sockaddr_size);

    from_addr.sin_family = AF_INET;//这个socket的协议类型

    //from_addr.sin_addr.s_addr = INADDR_ANY;//INADDR_ANY是任意地址
    //from_addr.sin_addr.s_addr = inet_addr("10.108.16.163");;//任意地址
    //from_addr.sin_port = htons(49005);

    while(1)
    {
        if(-1!=(read_len=recvfrom(udp.fd_socket, buf, sizeof(buf), 0, (struct sockaddr *)&from_addr, &from_addr_len)))
        {
#if 0
            for(unsigned int i=0;i<read_len;i++)
            {
                printf("recvfrom = %x ",buf[i]);
            }
            printf("\n");
#endif
            //printf("udp.ip = %s\n",udp.ip);

            char *temp=NULL;
            temp = inet_ntoa(from_addr.sin_addr);
            //printf("temp = %s\n",temp);

            if(strcmp(udp.ip,IP_RECEIVE_ALL) == 0)
            {
                printf("udp_recvbuf_and_process    :    接收来自任意ip地址的数据\n");
                if(read_len>0)
                {
                    udp.ptr_fun((unsigned char*)buf,read_len);
                }
            }
            else if(strcmp(udp.ip,temp) == 0)
            {
                printf("udp_recvbuf_and_process    :    收到数据的发送方的ip地址与我们所希望的ip地址一致，准备接收\n");
                if(read_len>0)
                {
                    udp.ptr_fun((unsigned char*)buf,read_len);
                }
            }
            else
            {
                printf("udp_recvbuf_and_process    :    收到的ip地址不正确，不接收\n");
            }
        }
        else
        {
            //printf("没有收到数据\n");
        }
    }

    pthread_exit(NULL);
}


int create_socket_udp_receive_thread(int fd_receive_socket,int (*ptr_fun)(unsigned char*,unsigned int),char *from_ip, unsigned int from_port)
{
    //struct T_UDP_DEVICE udp_dev;//这个警示不要删除 pthread_create函数中的第4个参数必须是全局变量，我一开始没有加static找了半天错误
    static struct T_UDP_DEVICE udp_dev;

    int ret=0;
    udp_dev.fd_socket = fd_receive_socket;
    udp_dev.ptr_fun=ptr_fun;
    udp_dev.ip = from_ip;
    udp_dev.port = from_port;

    printf("udp_dev.fd_socket = %d \n",udp_dev.fd_socket);

    /*udp_recvbuf_and_process函数中调用了fd_sock_recv来确定从哪个socket获取数据*/
    ret = pthread_create (&udp_dev.fd_receive_thread,            //线程标识符指针
                          NULL,                            //默认属性
                          udp_recvbuf_and_process,//运行函数
                          (void *)&udp_dev);                 //运行函数的参数
    if (0 != ret)
    {
       perror ("pthread create error\n");
    }

    return 0;
}

