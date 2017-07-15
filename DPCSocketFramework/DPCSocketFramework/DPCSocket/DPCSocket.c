//
//  DPCSocket.c
//  DPIPV6Test
//
//  Created by weifu Deng on 9/8/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#include "DPCSocket.h"

void dp_log(const char *fmt,...){
#if DEBUG
    printf("DPCSocket.c ");
    va_list args;
    va_start (args, fmt);
    vprintf(fmt, args);
    va_end (args);
    printf("\n");
#endif
}

void dp_logSocketInfo(dp_socket iSocket, char *pFlag){
    char ipstr[INET6_ADDRSTRLEN] = {0};
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof (struct sockaddr_storage);
    if (getsockname(iSocket, (struct sockaddr *) &addr, (socklen_t*)&addr_len) == 0) {
        if (addr.ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&addr;
            dp_format_ipv4_address(&ipv4->sin_addr, ipstr);
            dp_log("[%s]log socket:{\nfamily: AF_INET\nhost: %s\nport: %d\n}",
                   pFlag, ipstr, ntohs(ipv4->sin_port));
        }
        else if (addr.ss_family == AF_INET6){
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&addr;
            dp_format_ipv6_address(&ipv6->sin6_addr, ipstr);
            dp_log("[%s]log socket:{\nfamily: AF_INET6\nhost: %s\nport: %d\n}",
                   pFlag, ipstr, ntohs(ipv6->sin6_port));
        }
    }
}

void dp_logSockAddrInfo(struct sockaddr_storage *pAddr, char *pFlag){
    if (pAddr->ss_family == AF_INET) {
        char strip[INET_ADDRSTRLEN];
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)pAddr;
        dp_format_ipv4_address(&ipv4->sin_addr, strip);
        dp_log("[%s]log addr:{\nfamily: AF_INET\nip: %s\nport: %d\n}",
               pFlag, strip, htons(ipv4->sin_port));
    }
    else if (pAddr->ss_family == AF_INET6){
        char strip[INET6_ADDRSTRLEN];
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)pAddr;
        dp_format_ipv6_address(&ipv6->sin6_addr, strip);
        dp_log("[%s]log addr:{\nfamily: AF_INET6\nip: %s\nport: %d\n}",
               pFlag, strip, htons(ipv6->sin6_port));
    }
}

dp_bool dp_is_ipv6addr(const char *pAddr){
    if (!pAddr) {
        return DP_FALSE;
    }
    struct addrinfo hints, *res0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    int err = getaddrinfo(pAddr,"8000", &hints, &res0);
    if (err != 0) {
        dp_log("%s: %s", __func__, gai_strerror(err));
        return DP_FALSE;
    }
    
    dp_bool ret = (res0->ai_addr->sa_family == AF_INET6) ? DP_TRUE : DP_FALSE;
    freeaddrinfo(res0);
    return ret;
}

dp_bool dp_format_ipv4_address(struct in_addr *ipv4Addr, char *pAddr){
    if (!ipv4Addr || !pAddr) {
        return DP_FALSE;
    }
    memset(pAddr, 0, INET_ADDRSTRLEN);
    if(inet_ntop(AF_INET, ipv4Addr, pAddr, INET_ADDRSTRLEN) != NULL){
        return DP_TRUE;
    }
    
    return DP_FALSE;
}

dp_bool dp_format_ipv6_address(struct in6_addr *ipv6Addr, char *pAddr){
    if (!ipv6Addr || !pAddr) {
        return DP_FALSE;
    }
    memset(pAddr, 0, INET6_ADDRSTRLEN);
    if(inet_ntop(AF_INET6, ipv6Addr, pAddr, INET6_ADDRSTRLEN) != NULL){
        return DP_TRUE;
    }
    
    return DP_FALSE;
}

dp_bool dp_get_address_by_host(char *pHost, char *pAddr){
    if (!pHost || pAddr) {
        return DP_FALSE;
    }
    
    dp_bool bret = DP_FALSE;
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;//AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_DEFAULT;
    hints.ai_protocol = IPPROTO_IP;   // IP协议
    
    int status = getaddrinfo(pHost, "8000", &hints, &res);
    if (status != 0) {
        dp_log("%s error: getaddrinfo failed[%s(%d)]", strerror(status), status);
        return DP_FALSE;
    }
    
    for (cur = res; cur; cur = cur->ai_next) {
        if (cur->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)cur->ai_addr;
            bret = dp_format_ipv4_address(&ipv4->sin_addr, pAddr);
        }
        else if (cur->ai_family == AF_INET6){
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)cur->ai_addr;
            bret = dp_format_ipv6_address(&ipv6->sin6_addr, pAddr);
        }
    }
    
    freeaddrinfo(res);
    
    return bret;
}

sa_family_t dp_get_local_family(){
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
                        ifamily = AF_INET;
                    }
                }
                else if (temp_addr->ifa_addr->sa_family == AF_INET6){
                    char ipv6Addr[INET6_ADDRSTRLEN] = {0};
                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)temp_addr->ifa_addr;
                    if (dp_format_ipv6_address(&ipv6->sin6_addr, ipv6Addr)) {
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

dp_bool dp_get_local_ipaddr(char *pLAddr){
    dp_bool bresult = DP_FALSE;
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    
    if (getifaddrs(&interfaces) == 0) {  // 0 表示获取成功
        temp_addr = interfaces;
        while (temp_addr != NULL) {
            // Check if interface is en0 which is the wifi connection on the iPhone
            if ((strcmp(temp_addr->ifa_name, "en0") == 0) ||        //"en0" wifi
                (strcmp(temp_addr->ifa_name, "pdp_ip0")) == 0) {    //"pdp_ip0" 蜂窝网络
                if (temp_addr->ifa_addr->sa_family == AF_INET) {
                    char ipv4Addr[INET6_ADDRSTRLEN] = {0};
                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)temp_addr->ifa_addr;
                    if (dp_format_ipv4_address(&ipv4->sin_addr, ipv4Addr) &&
                        strcmp(ipv4Addr, "") != 0) {
                        memset(pLAddr, 0, INET6_ADDRSTRLEN);
                        memcpy(pLAddr, ipv4Addr, INET6_ADDRSTRLEN);
                        bresult = DP_TRUE;
                    }
                }
                else if (temp_addr->ifa_addr->sa_family == AF_INET6){
                    char ipv6Addr[INET6_ADDRSTRLEN] = {0};
                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)temp_addr->ifa_addr;
                    if (dp_format_ipv6_address(&ipv6->sin6_addr, ipv6Addr) &&
                        strcmp(ipv6Addr, "") != 0 &&
                        (strncasecmp(ipv6Addr, "fe80", strlen("fe80"))) != 0) {
                        memset(pLAddr, 0, INET6_ADDRSTRLEN);
                        memcpy(pLAddr, temp_addr->ifa_addr, INET6_ADDRSTRLEN);
                        bresult = DP_TRUE;
                        break;
                    }
                }
            }
            
            temp_addr = temp_addr->ifa_next;
        }
    }
    
    freeifaddrs(interfaces);
    return bresult;
}

dp_bool dp_get_local_addrInfo(struct sockaddr_storage *pLAddr){
    dp_bool bret = DP_FALSE;
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    if (getifaddrs(&interfaces) != 0) {// 0 表示获取成功
        return DP_FALSE;
    }
    
    temp_addr = interfaces;
    while (temp_addr != NULL) {
        // Check if interface is en0 which is the wifi connection on the iPhone
        if ((strcmp(temp_addr->ifa_name, "en0") == 0) ||        //"en0" wifi
            (strcmp(temp_addr->ifa_name, "pdp_ip0")) == 0) {    //"pdp_ip0" 蜂窝网络
            if (temp_addr->ifa_addr->sa_family == AF_INET) {
                char ipv4Addr[INET_ADDRSTRLEN] = {0};
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)temp_addr->ifa_addr;
                if (dp_format_ipv4_address(&ipv4->sin_addr, ipv4Addr) &&
                    strcmp(ipv4Addr, "") != 0) {
                    memset(pLAddr, 0, sizeof(struct sockaddr_storage));
                    memcpy(pLAddr, temp_addr->ifa_addr, sizeof(struct sockaddr_storage));
                    bret = DP_TRUE;
                }
            }
            else if (temp_addr->ifa_addr->sa_family == AF_INET6){
                char ipv6Addr[INET6_ADDRSTRLEN] = {0};
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)temp_addr->ifa_addr;
                if (dp_format_ipv6_address(&ipv6->sin6_addr, ipv6Addr) &&
                    strcmp(ipv6Addr, "") != 0 &&
                    (strncasecmp(ipv6Addr, "fe80", strlen("fe80"))) != 0) {
                    memset(pLAddr, 0, sizeof(struct sockaddr_storage));
                    memcpy(pLAddr, temp_addr->ifa_addr, sizeof(struct sockaddr_storage));
                    bret = DP_TRUE;
                    break;
                }
            }
        }
        
        temp_addr = temp_addr->ifa_next;
    }
    
    freeifaddrs(interfaces);
    
    return DP_TRUE;
}

void dp_free_sockaddr(struct sockaddr_storage *pAddr){
    if (pAddr) {
        free(pAddr);
        pAddr = NULL;
    }
}

struct sockaddr_storage * dp_creat_local_sockaddr(uint16_t iPort){
    if (iPort <= 0x400 ) {
        dp_log("%s: illegal parameter", __func__);
        return NULL;
    }
    
    struct sockaddr_storage pladdr = {0};
    sa_family_t ifamily = dp_get_local_family();
    if (ifamily == AF_INET) {
        struct sockaddr_in *sockaddr4 = (struct sockaddr_in *)&pladdr;
        sockaddr4->sin_len         = sizeof(struct sockaddr_in);
        sockaddr4->sin_family      = AF_INET;
        sockaddr4->sin_port        = htons(iPort);
        sockaddr4->sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else if (ifamily == AF_INET6){
        struct sockaddr_in6 *sockaddr6 = (struct sockaddr_in6 *)&pladdr;
        sockaddr6->sin6_len       = sizeof(struct sockaddr_in6);
        sockaddr6->sin6_family    = AF_INET6;
        sockaddr6->sin6_port      = htons(iPort);
        sockaddr6->sin6_addr      = in6addr_any;
    }
    else{
        dp_log("%s: protocol not supported", __func__);
        return NULL;
    }
    
    struct sockaddr_storage * pLocalAddr = (struct sockaddr_storage *)malloc(sizeof(struct sockaddr_storage));
    if (!pLocalAddr) {
        dp_log("%s: malloc failed", __func__);
        return NULL;
    }
    
    memset(pLocalAddr, 0, sizeof(struct sockaddr_storage));
    memcpy(pLocalAddr, &pladdr, sizeof(struct sockaddr_storage));
    
    return pLocalAddr;
}

struct sockaddr_storage * dp_creat_remote_sockaddr(char *pHost, uint16_t iPort){
    if (!pHost) {
        dp_log("%s: illegal parameter", __func__);
        return NULL;
    }
    
    struct sockaddr_storage praddr = {0};
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_DEFAULT;
    hints.ai_protocol = IPPROTO_IP;   // IP协议
    
    int status = getaddrinfo(pHost, "8000", &hints, &res);
    if (status != 0) {
        dp_log("%s: %s(%d)", __func__, strerror(status), status);
        return NULL;
    }
    
    dp_bool bflag = DP_FALSE;
    for (cur = res; cur; cur = cur->ai_next) {
        if (cur->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&praddr;
            memcpy(ipv4, (struct sockaddr_in *)cur->ai_addr, sizeof(struct sockaddr_in));
            ipv4->sin_port = htons(iPort);
            
            bflag = DP_TRUE;
        }
        else if (cur->ai_family == AF_INET6){
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&praddr;
            memcpy(ipv6, (struct sockaddr_in6 *)cur->ai_addr, sizeof(struct sockaddr_in6));
            ipv6->sin6_port = htons(iPort);
            
            bflag = DP_TRUE;
        }
    }
    freeaddrinfo(res);
    
    if (!bflag) {
        dp_log("%s: protocol not supported", __func__);
        return NULL;
    }
    
    struct sockaddr_storage *pRemoteAddr = (struct sockaddr_storage *)malloc(sizeof(struct sockaddr_storage));
    if (!pRemoteAddr) {
        dp_log("%s: malloc failed", __func__);
        return NULL;
    }
    
    memset(pRemoteAddr, 0, sizeof(struct sockaddr_storage));
    memcpy(pRemoteAddr, &praddr, sizeof(struct sockaddr_storage));
    
    return pRemoteAddr;
}

//SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
dp_socket dp_creat_socket(int iType){
    if (iType != SOCK_STREAM && iType != SOCK_DGRAM) {
        dp_log("%s: itype not supported", __func__);
        return INVALID_SOCKET;
    }
    
    int nosigpipe = 1;
    int reuseOn = 1;
    int status = SOCKET_ERROR;
    int isocket = INVALID_SOCKET;
    
    sa_family_t ifamily = dp_get_local_family();
    if (ifamily == AF_INET6) {
        isocket = socket(AF_INET6, iType, 0);
    }
    else if (ifamily == AF_INET){
        isocket = socket(AF_INET, iType, 0);
    }
    else{
        dp_log("%s error: protocol not supported", __func__);
        return INVALID_SOCKET;
    }
    
    /*!
     *屏蔽SIGPIPE信号,等同于signal(SIGPIPE,SIG_IGN);
     *SIGPIPE信号
     *在linux下写socket的程序的时候，如果尝试send到一个disconnected socket上，就会让底层抛出一个SIGPIPE信号。
     *该信号的缺省处理方法是退出进程。
     */
    status = setsockopt(isocket, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe));
    if (status == SOCKET_ERROR) {
        dp_log("%s error: shielded signal sigpipe failed", __func__);
        close(isocket);
        return INVALID_SOCKET;
    }
    
    //地址复用，解决端口被占用问题
    status = setsockopt(isocket, SOL_SOCKET, SO_REUSEADDR, &reuseOn, sizeof(reuseOn));
    if (status == SOCKET_ERROR) {
        dp_log("%s error: enabling address reuse failed", __func__);
        close(isocket);
        return INVALID_SOCKET;
    }
    
    return isocket;
}

void dp_close_socket(dp_socket iSocket){
    if (iSocket != INVALID_SOCKET) {
        shutdown(iSocket, SHUT_RDWR);
        close(iSocket);
    }
}

dp_bool dp_bind(dp_socket iSocket, struct sockaddr *pLAddr){
    if (iSocket == INVALID_SOCKET || !pLAddr) {
        dp_log("%s error: illegal parameter", __func__);
        return DP_FALSE;
    }
    
    int iret = bind(iSocket, pLAddr, pLAddr->sa_len);
    if (iret == SOCKET_ERROR) {
        dp_log("%s error: %s(%d)", __func__, strerror(errno), errno);
        return DP_FALSE;
    }
    
    return DP_TRUE;
}

dp_bool dp_listen(dp_socket iSocket, uint32_t iMaxCount){
    if (iSocket == INVALID_SOCKET) {
        dp_log("%s error: illegal parameter", __func__);
        return DP_FALSE;
    }
    
    int iret = listen(iSocket, iMaxCount);
    if (iret == SOCKET_ERROR) {
        dp_log("%s error: %s(%d)", __func__, strerror(errno), errno);
        return DP_FALSE;
    }

    return DP_TRUE;
}

dp_socket dp_accept(dp_socket iSocket){
    if (iSocket == INVALID_SOCKET) {
        dp_log("%s error: illegal parameter", __func__);
        return DP_FALSE;
    }
    
    struct sockaddr client = {0};
    socklen_t addrLen = sizeof(socklen_t);
    dp_socket rwfd = accept(iSocket, &client, &addrLen);
    if (rwfd == SOCKET_ERROR){
        dp_log("%s error: %s(%d)", __func__, strerror(errno), errno);
    }
    
    return rwfd;
}

dp_bool dp_connect(dp_socket iSocket, struct sockaddr *pRaddr, uint32_t iTimeout){
    if (iSocket == INVALID_SOCKET || !pRaddr) {
        dp_log("%s error: illegal parameter", __func__);
        return DP_FALSE;
    }

    /*
     *1.设置成非阻塞模式来控制链接超时
     *2.成功链接后再设为阻塞模式
     */
    //1.设置非阻塞
    int status = SOCKET_ERROR;
    int flags = fcntl(iSocket, F_GETFL,0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    
    status = connect(iSocket, pRaddr, pRaddr->sa_len);
    if (status == SOCKET_ERROR){
        if (errno == EINPROGRESS){
            //it is in the connect process
            fd_set fdwrite;
            int error;
            int len = sizeof(int);
            
            struct timeval tv;
            tv.tv_sec = iTimeout / 1000;
            tv.tv_usec = (iTimeout % 1000) * 1000;
            
            FD_ZERO(&fdwrite);
            FD_SET(iSocket, &fdwrite);
            
            int ret = select(iSocket + 1, NULL, &fdwrite, NULL, &tv);
            if (ret <= 0) {//err
                dp_log("%s error: %s(%d)", __func__, strerror(errno), errno);
                return DP_FALSE;
            }
            
            //判断是否链接成功
            getsockopt(iSocket, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            if (error != 0) {
                dp_log("%s error: %s(%d)", __func__, strerror(error), error);
                return DP_FALSE;
            }
        }
        else{
            dp_log("%s error: %s(%d)", __func__, strerror(errno), errno);
            return DP_FALSE;
        }
    }
    
    //2.连接成功后设置阻塞模式
    flags = fcntl(iSocket, F_GETFL,0);
    flags &= ~ O_NONBLOCK;
    fcntl(iSocket,F_SETFL, flags);
    
    return DP_TRUE;
}

int32_t dp_recv(dp_socket iSocket, char *pData, uint32_t iLen, uint32_t iTimeout){
    if (iSocket == INVALID_SOCKET || !pData) {
        dp_log("%s error: illegal parameter", __func__);
        return SOCKET_ERROR;
    }
    
    uint32_t ret;
    uint32_t rlen = 0;
    uint32_t rslt = 0;
    
    fd_set	readfds;
    while (rlen < iLen){
        struct timeval tv;
        tv.tv_sec = iTimeout / 1000;
        tv.tv_usec = (iTimeout % 1000) * 1000;
        
        FD_ZERO(&readfds);
        FD_SET(iSocket, &readfds);
        
        ret = select(iSocket + 1, &readfds, NULL, NULL, &tv);
        if (ret < 0) {// err
            dp_log("%s error: %s(%d)", __func__, strerror(errno), errno);
            return SOCKET_ERROR;
        }
        else if (ret == 0){// time out
            //dp_log("%s error:recv timeout", __func__);
            return 0;
        }
        
        if (FD_ISSET(socket, &readfds)){  //测试mSock是否可读
            rslt = recv(iSocket, pData + rlen, iLen - rlen, 0);
            if (rslt <= 0) {//err
                dp_log("%s error: %s(%d)", __func__, strerror(errno), errno);
                return SOCKET_ERROR;
            }
            else{
                rlen += rslt;
            }
        }
    }
    
    return rlen;
}

int32_t dp_send(dp_socket iSocket, char *pData, uint32_t iLen){
    if (iSocket == INVALID_SOCKET || !pData) {
        dp_log("%s error: illegal parameter", __func__);
        return SOCKET_ERROR;
    }
    
    int32_t wlen = 0;
    ssize_t rlen;
    while (wlen < iLen){
        rlen = send(iSocket, pData + wlen, iLen - wlen, 0);
        if (rlen <= 0) {
            dp_log("%s error: %s(%d)", __func__, strerror(errno), errno);
            return SOCKET_ERROR;
        }
        else {
            wlen += rlen;
        }
    }
    
    return wlen;
}

dp_bool dp_recvfrom(dp_socket iSocket, char *pData, uint32_t *iLen, struct sockaddr *pRaddr, uint32_t iTimeout){
    
    uint32_t originalSize = *iLen;
    if (iSocket == INVALID_SOCKET || !pData || !pRaddr ) {
        dp_log("%s: illegal parameter", __func__);
        return DP_FALSE;
    }
    
    socklen_t addrLen = 0;
    sa_family_t ifamily = dp_get_local_family();
    if (ifamily == AF_INET) {
        addrLen = sizeof(struct sockaddr_in);
    }
    else if (ifamily == AF_INET6) {
        addrLen = sizeof(struct sockaddr_in6);
    }
    else{
        dp_log("%s: unknown ifamily", __func__);
        return DP_FALSE;
    }
    
    struct timeval tv;
    tv.tv_sec = iTimeout / 1000;
    tv.tv_usec = (iTimeout % 1000) * 1000;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(iSocket, &readfds);
    int irslt = select(iSocket + 1, &readfds, NULL, NULL, &tv);
    if (irslt <= 0 ){
        dp_log("%s: irslt(%d) error[%s(%d)]", __func__, irslt, strerror(errno), errno);
        return DP_FALSE;
    }
    
    if (FD_ISSET (socket, &readfds)){
        *iLen = (uint32_t)recvfrom(iSocket, pData, originalSize, 0, pRaddr, &addrLen);
        if (*iLen <= 0){
            dp_log("%s: %s(%d)", __func__, strerror(errno), errno);
            return DP_FALSE;
        }
        
        if ((*iLen) + 1 >= originalSize){
            dp_log("%s: received a message that was too large", __func__);
            return DP_FALSE;
        }
        
        pData[*iLen]=0;
        
        return DP_TRUE;
    }
    
    return DP_FALSE;
}

dp_bool dp_sendto(dp_socket iSocket, char *pData, uint32_t iLen, struct sockaddr *pRaddr){
    if (iSocket == INVALID_SOCKET || !pData || !pRaddr){
        dp_log("%s: illegal parameter", __func__);
        return DP_FALSE;
    }
    
    ssize_t iret = sendto(iSocket, pData, iLen, 0, pRaddr, pRaddr->sa_len);
    if (iret != iLen) {
        dp_log("%s: %s(%d)", __func__, strerror(errno), errno);
        return DP_FALSE;
    }
    
    return DP_TRUE;
}
