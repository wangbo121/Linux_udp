/*
 * main.cpp
 *
 *  Created on: Nov 4, 2017
 *      Author: wangbo
 */


#include <stdio.h>
#include <unistd.h>

#include "udp.h"

#define IP_NATIVE_SOCKET "127.0.0.1"//本机所用网卡的ip地址
#define PORT_NATIVE_SOCKET_SEND 49000//本机要往外发送的套接字的端口
#define PORT_NATIVE_SOCKET_RECEIVE 49005//本机要接收数据的套接字所使用的端口

#define IP_SEND_TO "127.0.0.1"//发送目标的ip地址
#define PORT_SENT_TO 49005//发送目标的port端口

#define IP_RECEIVE_FROM "127.0.0.1"//只接收来自该ip地址的数据
#define PORT_RECEIVE_FROM 49005//只接收来自该端口的数据

struct T_UDP_DEVICE udp_send;
struct T_UDP_DEVICE udp_receive;

int read_udp_data_test(unsigned char *temp, unsigned int len);

int read_udp_data_test(unsigned char *temp, unsigned int len)
{
    printf("read_udp_data_test\n");
    return 0;
}

int main()
{
    printf("hello Linux_udp\n");

    char send_test[]={'w','a','n','g','\0'};

    /*
     * 本机用于往外发送数据的socket
     * 即，用49000的端口给49005的端口发送数据
     */

    //udp_send.ip = "127.0.0.2";//改变发送的地址，做测试用
    udp_send.ip = IP_NATIVE_SOCKET;
    udp_send.port = PORT_NATIVE_SOCKET_SEND;
    open_socket_udp_dev(&udp_send.fd_socket, udp_send.ip, udp_send.port);

    /*
     * 用49005的端口接收数据
     */
    udp_receive.ip = IP_NATIVE_SOCKET;
    udp_receive.port = PORT_NATIVE_SOCKET_RECEIVE;
    open_socket_udp_dev(&udp_receive.fd_socket, udp_receive.ip, udp_receive.port);

    /*
     * 创建接收的线程，
     * 只接收来自IP_RECEIVE_FROM的ip地址的数据
     * 只接收来自PORT_RECEIVE_FROM的端口的数据
     * 并且用read_udp_data_test函数处理接收到的数据
     */
    udp_receive.ip = IP_RECEIVE_FROM;
    udp_receive.port = PORT_RECEIVE_FROM;
    udp_receive.ptr_fun = read_udp_data_test;
    create_socket_udp_receive_thread(udp_receive.fd_socket,udp_receive.ptr_fun, udp_receive.ip, udp_receive.port);

    while ( 1 )
    {
        printf("udp_send.fd_socket = %d\n",udp_send.fd_socket);

        /*
         * 设置本机要给谁发送，
         * 设置发送目标的ip为IP_SEND_TO
         * 设置发送目标的端口为PORT_SENT_TO
         */
        udp_send.ip = IP_SEND_TO;
        udp_send.port = PORT_SENT_TO;
        send_socket_udp_data(udp_send.fd_socket, (unsigned char*)send_test, sizeof(send_test), udp_send.ip, udp_send.port);
        sleep(1);
    }

    return 0;
}

