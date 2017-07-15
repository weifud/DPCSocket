//
//  DPCSocket.h
//  DPIPV6Test
//
//  Created by weifu Deng on 9/8/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#ifndef DPCSocket_h
#define DPCSocket_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread/pthread.h>

#ifndef dp_bool
#define dp_bool int
#endif

#ifndef DP_TRUE 
#define DP_TRUE (1)
#endif

#ifndef DP_FALSE
#define DP_FALSE (0)
#endif

#ifndef NEGATIVE_ONE
#define NEGATIVE_ONE (-1)
#endif

typedef int dp_socket;

sa_family_t dp_curnetwork_family();

dp_bool dp_format_ipv4_address(struct in_addr *ipv4Addr, char *dstAddress);
dp_bool dp_format_ipv6_address(struct in6_addr *ipv6Addr, char *dstAddress);

struct sockaddr_storage * dp_creat_local_sockaddr(uint16_t iport);
struct sockaddr_storage * dp_creat_remote_sockaddr(char *hostName, uint16_t iport);
void dp_free_sockaddr(struct sockaddr_storage *paddr);

dp_socket dp_creat_socket(sa_family_t ifamily, int isocketType);
void dp_close_socket(dp_socket socket);

dp_bool dp_bind(dp_socket socket, struct sockaddr *ipaddr);

dp_bool dp_listen(dp_socket socket, int icount);
dp_socket dp_accept(dp_socket socket);

dp_bool dp_connect(dp_socket socket, struct sockaddr *ipaddr, struct timeval* timeout);
ssize_t dp_recv(dp_socket socket, char *pData, int ilenOfData, struct timeval *timeout);
ssize_t dp_send(dp_socket socket, char *pData, int ilenOfData);

ssize_t dp_recvfrom(dp_socket socket, char *pData, int ilenOfData, struct sockaddr *remoteAddr);
ssize_t dp_sendto(dp_socket socket, char *pData, int ilenOfData, struct sockaddr *remoteAddr);

#endif /* DPCSocket_h */
