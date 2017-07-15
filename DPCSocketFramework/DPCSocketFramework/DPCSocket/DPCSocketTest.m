//
//  DPCSocketTest.m
//  DPIPV6Test
//
//  Created by weifu Deng on 9/8/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#import "DPCSocketTest.h"
#import "DPCSocket.h"

#ifndef DP_SOCKET_TCP_PORT
#define DP_SOCKET_TCP_PORT       (16001)
#endif

#ifndef DP_SOCKET_UDP_PORT
#define DP_SOCKET_UDP_PORT       (16003)
#endif

#ifndef DP_SOCKET_SERVER_IP
#define DP_SOCKET_SERVER_IP      "192.168.250.191"//"64:ff9b::192.168.250.191"
#endif

@interface DPCSocketTest (){
    dp_socket _tcpSrv_socket;
    dp_socket _tcpClt_socket;
    dp_socket _udpSrv_socket;
    dp_socket _udpClt_socket;
    
    struct sockaddr_storage *_pTCPRemoteAddr;
    struct sockaddr_storage *_pUDPRemoteAddr;
    struct sockaddr_storage *_pTCPLocalAdd;
    struct sockaddr_storage *_pUDPLocalAdd;
}

@end


@implementation DPCSocketTest

- (void)dealloc{
    dp_free_sockaddr(_pTCPRemoteAddr);
    dp_free_sockaddr(_pUDPRemoteAddr);
    dp_free_sockaddr(_pTCPLocalAdd);
    dp_free_sockaddr(_pUDPLocalAdd);
}

#pragma mark - tcp
- (void)tcp_srv_recv_loop:(dp_socket)socket{
    dp_socket isock = socket;
    char recvbuff[18];//tcp clt send test
    ssize_t iret = SOCKET_ERROR;
    
    uint32_t itimeout = 5000;
    
    char tcp_srv_ack[] = "tcp srv ack";
    
    NSLog(@"tcp srv (fd[%d]) recv loop start", socket);
    
    while (isock > INVALID_SOCKET) {
        memset(&recvbuff, 0, sizeof(recvbuff));
        iret = dp_recv(socket, recvbuff, sizeof(recvbuff), itimeout);
        if (iret < 0) {
            NSLog(@"tcp srv (fd[%d]) recv error: [%s(%d)]", socket, strerror(errno), errno);
            break;
        }
        else if (iret > 0){
            NSLog(@"tcp srv (fd[%d]) recv msg:\"%s\"", socket, recvbuff);
            dp_send(socket, tcp_srv_ack, sizeof(tcp_srv_ack));
        }
        else{
            NSLog(@"tcp srv (fd[%d]) recv: waiting msg", socket);
        }
    }
    
    NSLog(@"tcp srv (fd[%d]) recv loop end", socket);
}

- (void)tcp_srv_accept_loop{
    NSLog(@"tcp srv accept loop start");
    
    while (_tcpSrv_socket != INVALID_SOCKET) {
        dp_socket rwfd = dp_accept(_tcpSrv_socket);
        if (rwfd != SOCKET_ERROR) {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                [self tcp_srv_recv_loop:rwfd];
            });
        }
    }
    
    NSLog(@"tcp srv accept end");
}

- (BOOL)creat_tcp_srv{
    _pTCPLocalAdd = dp_creat_local_sockaddr(DP_SOCKET_TCP_PORT);
    _tcpSrv_socket = dp_creat_socket(SOCK_STREAM);
    if (_pTCPLocalAdd &&
        (_tcpSrv_socket != INVALID_SOCKET) &&
        dp_bind(_tcpSrv_socket, (struct sockaddr *)_pTCPLocalAdd) &&
        dp_listen(_tcpSrv_socket, 16)) {
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            [self tcp_srv_accept_loop];
        });
        
        NSLog(@"run tcp server success");
        
        return YES;
    }
    
    return NO;
}

- (BOOL)creat_tcp_clt{
    uint32_t itimeout = 5000;
    
    _pTCPRemoteAddr = dp_creat_remote_sockaddr(DP_SOCKET_SERVER_IP, DP_SOCKET_TCP_PORT);
    if (_pTCPRemoteAddr) {
        _tcpClt_socket = dp_creat_socket(SOCK_STREAM);
        if ((_tcpClt_socket != INVALID_SOCKET) &&
            dp_connect(_tcpClt_socket, (struct sockaddr *)_pTCPRemoteAddr, itimeout)) {
            NSLog(@"run tcp client success");
    
            return YES;
        }
    }
    
    return NO;
}

- (void)tcp_recv_test{
    uint32_t itimeout = 5000;
    
    char recvBuff[12] = {0};//tcp srv ack
    ssize_t iret = dp_recv(_tcpClt_socket, recvBuff, sizeof(recvBuff), itimeout);
    if (iret > 0) {
        NSLog(@"tcp client recv msg:\"%s\"", recvBuff);
    }
}

- (void)tcp_send_test{
    char sendBuff[] = "tcp clt send test";
    ssize_t iret = dp_send(_tcpClt_socket, sendBuff, sizeof(sendBuff));
    if (iret <= 0) {
        NSLog(@"tcp client send data failed");
    }
}

#pragma mark - udp
- (void)udp_srv_recv_loop{
    char recvBuff[18];//udp clt send test
    struct sockaddr_storage *remoteAddr = (struct sockaddr_storage *)malloc(sizeof(struct sockaddr_storage));
    
    NSLog(@"udp srv recv loop start");
    uint32_t itimeout = 5000;
    
    while (_udpSrv_socket > 0) {
        memset(recvBuff, 0, sizeof(recvBuff));
        memset(remoteAddr, 0, sizeof(struct sockaddr_storage));
        ssize_t iret = dp_recvfrom(_udpSrv_socket,
                                   recvBuff,
                                   sizeof(recvBuff),
                                   (struct sockaddr *)remoteAddr,
                                   itimeout);
        if (iret > 0) {
            char srcipstr[INET6_ADDRSTRLEN] = {0};
            memset(srcipstr, 0, sizeof(srcipstr));
            if (remoteAddr->ss_family == AF_INET) {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)remoteAddr;
                dp_format_ipv4_address(&ipv4->sin_addr, srcipstr);
            }
            else if (remoteAddr->ss_family == AF_INET6){
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)remoteAddr;
                dp_format_ipv6_address(&ipv6->sin6_addr, srcipstr);
            }
            
            NSLog(@"udp srv recv msg:\"%s\" from [%s]", recvBuff, srcipstr);
            
            char ackBuff[] = "udp srv ack";
            dp_sendto(_udpSrv_socket, ackBuff, sizeof(ackBuff), (struct sockaddr *)remoteAddr);
        }
    }
    
    NSLog(@"udp srv recv loop end");
}

- (BOOL)creat_udp_srv{
    _pUDPLocalAdd = dp_creat_local_sockaddr(DP_SOCKET_UDP_PORT);
    _udpSrv_socket = dp_creat_socket(SOCK_DGRAM);
    if (_pUDPLocalAdd &&
        (_udpSrv_socket != INVALID_SOCKET) &&
        dp_bind(_udpSrv_socket, (struct sockaddr *)_pUDPLocalAdd)) {
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            [self udp_srv_recv_loop];
        });
        
        NSLog(@"run udp server success");
        
        return YES;
    }
    
    return NO;
}

- (BOOL)creat_udp_clt{
    _pUDPRemoteAddr = dp_creat_remote_sockaddr(DP_SOCKET_SERVER_IP, DP_SOCKET_UDP_PORT);
    if (_pUDPRemoteAddr) {
        _udpClt_socket = dp_creat_socket(SOCK_DGRAM);
        if (_udpClt_socket != INVALID_SOCKET) {
            NSLog(@"run udp client success");
            return YES;
        }
    }
    
    return NO;
}

- (void)udp_recv_test{
    char recvBuff[12]={0};//udp srv ack
    struct sockaddr_storage *remoteAddr = (struct sockaddr_storage *)malloc(sizeof(struct sockaddr_storage));
    memset(remoteAddr, 0, sizeof(struct sockaddr_storage));
    uint32_t itimeout = 5000;
    ssize_t iret = dp_recvfrom(_udpClt_socket,
                               recvBuff,
                               sizeof(recvBuff),
                               (struct sockaddr *)remoteAddr,
                               itimeout);
    if (iret > 0) {
        char srcipstr[INET6_ADDRSTRLEN] = {0};
        memset(srcipstr, 0, sizeof(srcipstr));
        if (remoteAddr->ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)remoteAddr;
            dp_format_ipv4_address(&ipv4->sin_addr, srcipstr);
        }
        else if (remoteAddr->ss_family == AF_INET6){
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)remoteAddr;
            dp_format_ipv6_address(&ipv6->sin6_addr, srcipstr);
        }
        
        NSLog(@"udp clt recv msg:\"%s\" from [%s]", recvBuff, srcipstr);
    }
}

- (void)udp_sendto_test{
    char sendbuff[] = "udp clt send test";
    ssize_t iret = dp_sendto(_udpClt_socket, sendbuff, sizeof(sendbuff), (struct sockaddr *)_pUDPRemoteAddr);
    if (iret <= 0) {
        NSLog(@"udp client send data failed");
    }
}

- (void)closeSocket:(dp_socket)socket{
    dp_close_socket(socket);
}

- (void)testGetnameinfo{
    int err = 0;
    dp_socket tset_socket;
    char loc[128] = {0};
    int size = sizeof(loc);
    struct sockaddr_storage *dest_addr;
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof (struct sockaddr_storage);
    
    uint32_t itimeout = 5000;
    
    dest_addr = dp_creat_remote_sockaddr("www.d-powercloud.net", 8060);
    if (!dest_addr) {
        NSLog(@"testGetnameinfo: creat remote sockaddr failed");
        return;
    }
    
    tset_socket = dp_creat_socket(SOCK_STREAM);
    if ((tset_socket != INVALID_SOCKET) &&
        dp_connect(tset_socket, (struct sockaddr *)dest_addr, itimeout)) {
        NSLog(@"testGetnameinfo: connect success");
        err = getsockname(tset_socket, (struct sockaddr *) &addr, (socklen_t*)&addr_len);
        if (err == 0){
            NSLog(@"testGetnameinfo: getsockname success");
            err = getnameinfo ((struct sockaddr *) &addr, addr_len, loc, size, NULL, 0, NI_NUMERICHOST);
            if (err == 0){
                NSLog(@"testGetnameinfo: loc[%s]", loc);
            }
        }
    }

    dp_free_sockaddr(dest_addr);
    dp_close_socket(tset_socket);
}

@end
