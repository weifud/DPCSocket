//
//  DPCSocket.h
//  DPCSocket
//
//  Support for ipv6
//
//  Created by weifu Deng on 9/8/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#ifndef DPCSocket_h
#define DPCSocket_h

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef dp_bool
#define dp_bool int
#endif

#ifndef DP_TRUE 
#define DP_TRUE (1)
#endif

#ifndef DP_FALSE
#define DP_FALSE (0)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

typedef int dp_socket;

/**
 打印socket信息

 @param iSocket 目标socket
 @param pFlag 打印标记
 */
void dp_logSocketInfo(dp_socket iSocket, char *pFlag);

/**
 打印地址信息

 @param pAddr 目标地址
 @param pFlag 打印标记
 */
void dp_logSockAddrInfo(struct sockaddr_storage *pAddr, char *pFlag);

/**
 ipv6地址判断

 @param pAddr 目标地址
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_is_ipv6addr(const char *pAddr);

/**
 ipv4地址转字符串

 @param ipv4Addr ipv4地址
 @param pAddr [in/out]ipv4地址字符串
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_format_ipv4_address(struct in_addr *ipv4Addr, char *pAddr);

/**
 ipv6地址转字符串

 @param ipv6Addr ipv6地址
 @param pAddr [in/out]ipv6地址字符串
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_format_ipv6_address(struct in6_addr *ipv6Addr, char *pAddr);

/**
 解析主机ip地址

 @param pHost 目标主机域名或IP地址
 @param pAddr [in/out]主机IP地址 (pAddr length: INET6_ADDRSTRLEN)
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_get_address_by_host(char *pHost, char *pAddr);

/**
 获取本地网络类型

 @return 本地网络类型 SOCK_STREAM(TCP), SOCK_DGRAM(UDP)， AF_UNSPEC(unspecified)
 */
sa_family_t dp_get_local_family();

/**
 获取本地ip地址字符串

 @param pLAddr [in/out] 本地IP地址字符串 (pAddr length: INET6_ADDRSTRLEN)
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_get_local_ipaddr(char *pLAddr);

/**
 获取本地IP地址信息

 @param pLAddr [in/out]本地IP地址信息
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_get_local_addrInfo(struct sockaddr_storage *pLAddr);

/**
 释放地址结构体

 @param pAddr 地址结构体
 */
void dp_free_sockaddr(struct sockaddr_storage *pAddr);

/**
 创建本地地址结构体

 @param iPort 端口号
 @return 本地地址结构体 / NULL
 */
struct sockaddr_storage * dp_creat_local_sockaddr(uint16_t iPort);

/**
 创建远程地址结构体

 @param pHost 远程地址字符串
 @param iPort 远程端口号
 @return 远程地址结构体 / NULL
 */
struct sockaddr_storage * dp_creat_remote_sockaddr(char *pHost, uint16_t iPort);

/**
 创建socket

 @param iType socket类型
 @return socket / INVALID_SOCKET
 */
dp_socket dp_creat_socket(int iType);//SOCK_STREAM(TCP), SOCK_DGRAM(UDP)

/**
 关闭socket

 @param iSocket 目标socket
 */
void dp_close_socket(dp_socket iSocket);

/**
 绑定socket和地址

 @param iSocket 目标socket
 @param pLAddr 目标地址
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_bind(dp_socket iSocket, struct sockaddr *pLAddr);

/**
 监听socket

 @param iSocket 目标socket
 @param iMaxCount 最大监听数
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_listen(dp_socket iSocket, uint32_t iMaxCount);

/**
 接听socket

 @param iSocket 目标socket
 @return DP_TRUE / DP_FALSE
 */
dp_socket dp_accept(dp_socket iSocket);

/**
 建立连接

 @param iSocket 目标socket
 @param pRaddr 远程地址
 @param iTimeout 超时时间（单位：ms）
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_connect(dp_socket iSocket, struct sockaddr *pRaddr, uint32_t iTimeout);//ms

/**
 接收数据流

 @param iSocket 目标socket
 @param pData 数据缓冲区
 @param iLen 接收数据的大小
 @param iTimeout 超时时间（单位：ms）
 @return 实际接收长度 / SOCKET_ERROR
 */
int32_t dp_recv(dp_socket iSocket, char *pData, uint32_t iLen, uint32_t iTimeout);//ms

/**
 发送数据流

 @param iSocket 目标socket
 @param pData 发送数据
 @param iLen 发送数据大小
 @return iLen / SOCKET_ERROR
 */
int32_t dp_send(dp_socket iSocket, char *pData, uint32_t iLen);

/**
 接收数据报

 @param iSocket 目标socket
 @param pData 数据缓冲区
 @param iLen [in/out]接收数据报的大小/实际接收数据报的大小
 @param pRaddr 远程地址
 @param iTimeout 超时时间（单位：ms）
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_recvfrom(dp_socket iSocket,
                    char *pData,
                    uint32_t *iLen,
                    struct sockaddr *pRaddr,
                    uint32_t iTimeout);//ms

/**
 发送数据报

 @param iSocket 目标socket
 @param pData 发送数据
 @param iLen 数据长度
 @param pRaddr 远程地址
 @return DP_TRUE / DP_FALSE
 */
dp_bool dp_sendto(dp_socket iSocket, char *pData, uint32_t iLen, struct sockaddr *pRaddr);

#endif /* DPCSocket_h */
