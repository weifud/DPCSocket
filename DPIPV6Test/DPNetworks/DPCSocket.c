//
//  DPCSocket.c
//  DPIPV6Test
//
//  Created by weifu Deng on 9/8/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#include "DPCSocket.h"

static char g_ThisFile[] = "DPCSocket.c";

sa_family_t dp_curnetwork_family(){
    sa_family_t ifamily = AF_UNSPEC;
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    
    if (getifaddrs(&interfaces) == 0) {  // 0 表示获取成功
        temp_addr = interfaces;
        while (temp_addr != NULL) {
            // Check if interface is en0 which is the wifi connection on the iPhone
            if ((strcmp(temp_addr->ifa_name, "en0") == 0) ||        //"en0" wifi
                (strcmp(temp_addr->ifa_name, "pdp_ip0")) == 0) {    //"pdp_ip0" 蜂窝网络
                if (temp_addr->ifa_addr->sa_family == AF_INET){
                    char ipv4Addr[INET_ADDRSTRLEN] = {0};
                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)temp_addr->ifa_addr;
                    if (dp_format_ipv4_address(&ipv4->sin_addr, ipv4Addr)) {
                        //printf("%s %s: ipv4 ipaddress: %s\n", g_ThisFile, __func__, ipv4Addr);
                        ifamily = AF_INET;
                    }
                }
                else if (temp_addr->ifa_addr->sa_family == AF_INET6){
                    char ipv6Addr[INET6_ADDRSTRLEN] = {0};
                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)temp_addr->ifa_addr;
                    if (dp_format_ipv6_address(&ipv6->sin6_addr, ipv6Addr)) {
                        //printf("%s %s: ipv6 ipaddress: %s\n", g_ThisFile, __func__, ipv6Addr);
                        ifamily = AF_INET6;
                        if ((strcmp(ipv6Addr, "") != 0) &&
                            ((strncasecmp(ipv6Addr, "fe80", strlen("fe80"))) != 0)) {
                            break;
                        }
                    }
                }
            }
            
            temp_addr = temp_addr->ifa_next;
        }
    }
    
    freeifaddrs(interfaces);
    
    return ifamily;
}

dp_bool dp_format_ipv4_address(struct in_addr *ipv4Addr, char *dstAddress){
    memset(dstAddress, 0, INET_ADDRSTRLEN);
    if(inet_ntop(AF_INET, ipv4Addr, dstAddress, INET_ADDRSTRLEN) != NULL){
        return DP_TRUE;
    }
    
    return DP_FALSE;
}

dp_bool dp_format_ipv6_address(struct in6_addr *ipv6Addr, char *dstAddress){
    memset(dstAddress, 0, INET_ADDRSTRLEN);
    if(inet_ntop(AF_INET6, ipv6Addr, dstAddress, INET6_ADDRSTRLEN) != NULL){
        return DP_TRUE;
    }
    
    return DP_FALSE;
}

struct sockaddr_storage * dp_creat_local_sockaddr(uint16_t iport){
    struct sockaddr_storage * pLocalAddr = (struct sockaddr_storage *)malloc(sizeof(struct sockaddr_storage));
    if (!pLocalAddr) {
        printf("%s %s: malloc failed\n", g_ThisFile, __func__);
        return NULL;
    }
    memset(pLocalAddr, 0, sizeof(struct sockaddr_storage));
    
    int ifamily = dp_curnetwork_family();
    if (ifamily == AF_INET) {
        struct sockaddr_in *sockaddr4 = (struct sockaddr_in *)pLocalAddr;
        sockaddr4->sin_len         = sizeof(struct sockaddr_in);
        sockaddr4->sin_family      = AF_INET;
        sockaddr4->sin_port        = htons(iport);
        sockaddr4->sin_addr.s_addr = htonl(INADDR_ANY);
        
        char ipstr[INET_ADDRSTRLEN] = {0};
        dp_format_ipv4_address(&sockaddr4->sin_addr, ipstr);
        printf("%s %s: ipv4 host(%s) port(%d)\n",
               g_ThisFile, __func__,ipstr, ntohs(sockaddr4->sin_port));
    }
    else if (ifamily == AF_INET6){
        struct sockaddr_in6 *sockaddr6 = (struct sockaddr_in6 *)pLocalAddr;
        sockaddr6->sin6_len       = sizeof(struct sockaddr_in6);
        sockaddr6->sin6_family    = AF_INET6;
        sockaddr6->sin6_port      = htons(iport);
        sockaddr6->sin6_addr      = in6addr_any;
        
        char ipstr[INET6_ADDRSTRLEN] = {0};
        dp_format_ipv6_address(&sockaddr6->sin6_addr, ipstr);
        printf("%s %s: ipv6 host(%s) port(%d)\n",
               g_ThisFile, __func__,ipstr, ntohs(sockaddr6->sin6_port));
    }
    
    return pLocalAddr;
}

struct sockaddr_storage * dp_creat_remote_sockaddr(char *hostName, uint16_t iport){
    struct sockaddr_storage *pRemoteAddr = (struct sockaddr_storage *)malloc(sizeof(struct sockaddr_storage));
    if (!pRemoteAddr) {
        printf("%s %s: malloc failed\n", g_ThisFile, __func__);
        return NULL;
    }
    memset(pRemoteAddr, 0, sizeof(struct sockaddr_storage));
    
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_DEFAULT;
    hints.ai_protocol = IPPROTO_IP;   // IP协议
    
    int status = getaddrinfo(hostName, "8000", &hints, &res);
    if (status != 0) {
        free(pRemoteAddr);
        printf("%s %s error: [%s(%d)]\n", g_ThisFile, __func__, strerror(status), status);
        return NULL;
    }
    
    for (cur = res; cur; cur = cur->ai_next) {
        if (cur->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)pRemoteAddr;
            memcpy(ipv4, (struct sockaddr_in *)cur->ai_addr, sizeof(struct sockaddr_in));
            ipv4->sin_port = htons(iport);
            
            char ipstr[INET_ADDRSTRLEN] = {0};
            dp_format_ipv4_address(&ipv4->sin_addr, ipstr);
            printf("%s %s: ipv4 host(%s) port(%d)\n",
                   g_ThisFile, __func__,ipstr, ntohs(ipv4->sin_port));
        }
        else if (cur->ai_family == AF_INET6){
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)pRemoteAddr;
            memcpy(ipv6, (struct sockaddr_in6 *)cur->ai_addr, sizeof(struct sockaddr_in6));
            ipv6->sin6_port = htons(iport);
            
            char ipstr[INET6_ADDRSTRLEN] = {0};
            dp_format_ipv6_address(&ipv6->sin6_addr, ipstr);
            printf("%s %s: ipv6 host(%s) port(%d)\n",
                   g_ThisFile, __func__, ipstr, ntohs(ipv6->sin6_port));
        }
    }
    
    freeaddrinfo(res);
    
    return pRemoteAddr;
}

void dp_free_sockaddr(struct sockaddr_storage *paddr){
    if (paddr) {
        free(paddr);
        paddr = NULL;
    }
}

//SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
dp_socket dp_creat_socket(sa_family_t ifamily, int isocketType){
    int nosigpipe = 1;
    int reuseOn = 1;
    int status = NEGATIVE_ONE;
    int isocket = NEGATIVE_ONE;
    
    if (ifamily == AF_INET6) {
        isocket = socket(AF_INET6, isocketType, 0);
    }
    else if (ifamily == AF_INET){
        isocket = socket(AF_INET, isocketType, 0);
    }
    else{
        printf("%s %s: protocol not supported\n", g_ThisFile, __func__);
        return NEGATIVE_ONE;
    }
    
    /*!
     *屏蔽SIGPIPE信号,等同于signal(SIGPIPE,SIG_IGN);
     *SIGPIPE信号
     *在linux下写socket的程序的时候，如果尝试send到一个disconnected socket上，就会让底层抛出一个SIGPIPE信号。
     *该信号的缺省处理方法是退出进程。
     */
    status = setsockopt(isocket, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe));
    if (status == NEGATIVE_ONE) {
        printf("%s %s: shielded signal sigpipe failed\n", g_ThisFile, __func__);
        close(isocket);
        return NEGATIVE_ONE;
    }
    
    //地址复用，解决端口被占用问题
    status = setsockopt(isocket, SOL_SOCKET, SO_REUSEADDR, &reuseOn, sizeof(reuseOn));
    if (status == NEGATIVE_ONE) {
        printf("%s %s: enabling address reuse failed\n", g_ThisFile, __func__);
        close(isocket);
        return NEGATIVE_ONE;
    }
    
    printf("%s %s: creat socket success\n", g_ThisFile, __func__);
    return isocket;
}

void dp_close_socket(dp_socket socket){
    shutdown(socket, SHUT_RDWR);
    close(socket);
    socket = NEGATIVE_ONE;
}

dp_bool dp_bind(dp_socket socket, struct sockaddr *ipaddr){
    int iret = bind(socket, ipaddr, ipaddr->sa_len);
    if (iret == NEGATIVE_ONE) {
        printf("%s %s: bind failed [%s(%d)]\n", g_ThisFile, __func__, strerror(errno), errno);
        return DP_FALSE;
    }
    
    printf("%s %s: bind success\n", g_ThisFile, __func__);
    return DP_TRUE;
}

dp_bool dp_listen(dp_socket socket, int icount){
    int iret = listen(socket, icount);
    if (iret == NEGATIVE_ONE) {
        printf("%s %s: listen failed [%s(%d)]\n", g_ThisFile, __func__, strerror(errno), errno);
        return DP_FALSE;
    }
    
    printf("%s %s: listen success\n", g_ThisFile, __func__);
    return DP_TRUE;
}

dp_socket dp_accept(dp_socket socket){
    struct sockaddr client = {0};
    socklen_t addrLen = sizeof(socklen_t);
    dp_socket rwfd = accept(socket, &client, &addrLen);
    if (rwfd == NEGATIVE_ONE){
        printf("%s %s: accept failed [%s (%d)]\n", g_ThisFile, __func__, strerror(errno), errno);
    }
    
    return rwfd;
}

dp_bool dp_connect(dp_socket socket, struct sockaddr *ipaddr, struct timeval* timeout){
    /*
     *1.设置成非阻塞模式来控制链接超时
     *2.成功链接后再设为阻塞模式
     */
    //1.设置非阻塞
    int status = NEGATIVE_ONE;
    int flags = fcntl(socket, F_GETFL,0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    
    status = connect(socket, ipaddr, ipaddr->sa_len);
    if (status == NEGATIVE_ONE){
        if (errno == EINPROGRESS){
            //it is in the connect process
            fd_set fdwrite;
            int error;
            int len = sizeof(int);
            FD_ZERO(&fdwrite);
            FD_SET(socket, &fdwrite);
            
            int ret = select(socket + 1, NULL, &fdwrite, NULL, timeout);
            if( ret <= 0){//time out
                printf("%s %s: connect timeout\n", g_ThisFile, __func__);
                return DP_FALSE;
            }
            
            //判断是否链接成功
            getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            if (error != 0) {
                printf("%s %s: connect failed [%s(%d)]\n",
                       g_ThisFile, __func__, strerror(error), error);
                return DP_FALSE;
            }
        }
        else{
            printf("%s %s: connect failed --- [%s(%d)]\n",
                   g_ThisFile, __func__, strerror(errno), errno);
            close(socket);
            socket = NEGATIVE_ONE;
            return DP_FALSE;
        }
    }
    
    //2.连接成功后设置阻塞模式
    flags = fcntl(socket, F_GETFL,0);
    flags &= ~ O_NONBLOCK;
    fcntl(socket,F_SETFL, flags);
    
    printf("%s %s: conncet success\n", g_ThisFile, __func__);
    return DP_TRUE;
}

/*
 *@return value:
 *            0: time out or iLen <=0
 *           -1: error
 *           -2: remote close
 *        other: length of recv data
 */
ssize_t dp_recv(dp_socket socket, char *pData, int ilenOfData, struct timeval *timeout){
    int ret;
    ssize_t rlen = 0;
    ssize_t len;
    
    fd_set	readfds;
    while (rlen < ilenOfData){
        FD_ZERO(&readfds);
        FD_SET(socket, &readfds);
        
        ret = select(socket + 1, &readfds, NULL, NULL, timeout);
        if (ret < 0) {// err
            return -1;
        }
        else if (ret == 0){// time out
            return 0;
        }
        
        if (FD_ISSET(socket, &readfds)){  //测试mSock是否可读，即是否网络上有数据
            len = recv(socket, pData + rlen, ilenOfData - rlen, 0);
            if (len < 0) {//err
                return -1;
            }
            else if (len == 0) {//connection closed by remote
                return -2;
            }
            else{
                rlen += len;
            }
        }
    }
    
    return rlen;
}

ssize_t dp_send(dp_socket socket, char *pData, int ilenOfData){
    ssize_t wlen = 0;
    ssize_t len;
    
    while (wlen < ilenOfData){
        len = send(socket, pData + wlen, ilenOfData - wlen, 0);
        if (len == -1) {
            return wlen;
        }
        else if (len == 0) {
            return 0;
        }
        else {
            wlen += len;
        }
    }
    
    return wlen;

}

ssize_t dp_recvfrom(dp_socket socket, char *pData, int iLenOfData, struct sockaddr *remoteAddr){
    socklen_t addrLen = 0;
    int ifamily = dp_curnetwork_family();
    if (ifamily == AF_INET) {
        addrLen = sizeof(struct sockaddr_in);
    }
    else if (ifamily == AF_INET6) {
        addrLen = sizeof(struct sockaddr_in6);
    }
    
    ssize_t iret = recvfrom(socket,
                            pData,
                            iLenOfData,
                            0,
                            remoteAddr,
                            &addrLen);
    if (iret < 0) {
        printf("%s %s recv data error: [%s(%d)]\n", g_ThisFile, __func__, strerror(errno), errno);
    }
    
    return iret;
}

ssize_t dp_sendto(dp_socket socket, char *pData, int iLenOfData, struct sockaddr *remoteAddr){
    ssize_t iret = sendto(socket, pData, iLenOfData, 0, remoteAddr, remoteAddr->sa_len);
    if (iret < 0) {
        printf("%s %s send data error: [%s(%d)]\n", g_ThisFile, __func__, strerror(errno), errno);
    }
    
    return iret;
}
