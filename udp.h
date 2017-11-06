/*
 * udp.h
 *
 *  Created on: Nov 4, 2017
 *      Author: wangbo
 */

#ifndef UDP_H_
#define UDP_H_

/*
 * 网络与主机字节转换函数:htons ntohs htonl ntohl (s 就是short l是long h是host n是network)
 * htons 把unsigned short类型从主机序转换到网络序
 * htonl 把unsigned long类型从主机序转换到网络序
 * ntohs 把unsigned short类型从网络序转换到主机序
 * ntohl 把unsigned long类型从网络序转换到主机序
 */

/*
 *其实htonl这些函数最后调用的还是__bswap
 */
#if 0
uint32_t htonl (uint32_t x)
{
    #if BYTE_ORDER == BIG_ENDIAN
       return x;
    #elif BYTE_ORDER == LITTLE_ENDIAN
       return __bswap_32 (x);
    #else
       # error "What kind of system is this?"
    #endif
}
#endif

#include <stdint.h>

#define IP_NATIVE_NULL "0.0.0.0"
#define IP_RECEIVE_ALL "0.0.0.0"//这个用来表示接收来自任意ip地址的数据
#define UDP_RCV_BUF_SIZE 2000//udp接收数据最大缓存数组大小

struct T_UDP_DEVICE
{
    int fd_socket;//表示socket套接字的文件描述符，而这个socket描述符在创建的时候，我在用open_socket_udp_dev函数创建的时候就绑定了一个本地的ip和端口
    int (*ptr_fun)(unsigned char *buf,unsigned int len);////如果该T_UDP_DEVICE用于接收线程，则接收到数据后的处理函数的指针就是这个，如果该T_UDP_DEVICE用于发送，则不需要
    unsigned long int fd_receive_thread;//如果该T_UDP_DEVICE用于接收线程，则线程标识符号存在这里，如果该T_UDP_DEVICE用于发送，则不需要
    char *ip;//如果该T_UDP_DEVICE用于接收线程，那么这个ip指的就是本机准备接收来自该ip的地址的数据，比如，我只想接收来自“110.110.110.110”地址的数据，我就把这个ip赋值
    unsigned int port;//如果该T_UDP_DEVICE用于接收线程，那么这个port指的就是本机准备接收来自该port的地址的数据
};

uint64_t htonll(uint64_t n) ;
uint64_t ntohll(uint64_t n) ;
float    hton_float(float host_float) ;
float    ntoh_float(float net_float) ;
double   hton_double(double host_double) ;
double   ntoh_double(double net_double) ;
//float htonf (float x);
//double htond (double x);

/*
 * 创建本地的socket套接字，
 * 绑定该socekt套接字的ip和端口，该ip和端口是本地套接字的ip和端口，该ip表示着本地电脑有多少个网卡就会有多少个ip
 * 该端口号是表示通过该socket套接字，使用sendto或者recfrom时用到的本地端口，
 * 而sendto和recvfrom函数中的ip和端口
 * 分别对应的是发送目标的ip和端口，以及从哪里接收的ip和端口，跟open_socket_udp_dev这个函数的ip和端口参数没有关系
 * 该socket套接字通信类型是udp，
 * 绑定该socket套接字的文件描述符到ptr_fd_socket指向的变量
 */
int open_socket_udp_dev(int *ptr_fd_socket, char* ip, unsigned int port);

/*
 * 通过fd_send_socket绑定的套接字往外发送数据
 * 发送目标的ip是target_ip
 * 发送目标的端口是target_port
 */
int send_socket_udp_data(int fd_send_socket, unsigned char *buf, unsigned int len, char *target_ip, unsigned int target_port);

/*
 * 创建socket udp接收线程
 * fd_receive_socket:本地接收数据的socket套接字
 * ptr_fun:接收到数据后的处理函数
 * from_ip:接收从from_ip发送过来的数据，如果是null，则接收任意地址发送过来的数据
 * from_port:接收从from_port发送过来的数据
 */
int create_socket_udp_receive_thread(int fd_receive_socket,int (*ptr_fun)(unsigned char*,unsigned int),char *from_ip, unsigned int from_port);

/*
 * udp_recvbuf_and_process
 * 创建udp接收线程时的，该线程所调用的函数
 */
void *udp_recvbuf_and_process(void * ptr_udp_device);

extern int fd_sock_send;
extern int fd_sock_recv;

extern struct T_UDP_DEVICE udp_send;
extern struct T_UDP_DEVICE udp_receive;

#endif /* UDP_H_ */
