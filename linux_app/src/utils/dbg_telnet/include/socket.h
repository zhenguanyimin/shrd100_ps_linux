/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __SOCKET_H__
#define  __SOCKET_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int udp_socket_init(const char *ipstr, uint16_t port);
int udp_socket_init_no_addr();

int tcp_socket_init(const char *ipstr, uint16_t port);
int tcp_socket_init_no_addr();

int udp_socket_recvfrom(int sockfd, void *buf, int buf_size, struct sockaddr_in *peer_addr);
int udp_socket_sendto(int sockfd, void *buf, int buf_size, struct sockaddr_in *peer_addr);
void make_sockaddr(struct sockaddr_in *sock_addr, uint32_t ip, uint16_t port);
void resolve_sockaddr(struct sockaddr_in *sock_addr, char * ip, int len, uint16_t *port);

int sockaddr_equal(struct sockaddr_in *sock_addr1, struct sockaddr_in *sock_addr2);
char * get_ipstr(struct sockaddr_in *sock_addr, char *ip);
uint16_t get_port(struct sockaddr_in *sock_addr, uint16_t *port);
void set_socket_timeout(int sockfd, int snd, int rcv);
void set_useful_sock_opt(int sockfd);
int create_l2_raw_socket(const char *if_name);

#endif

