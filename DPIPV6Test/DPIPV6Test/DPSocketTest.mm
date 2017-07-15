//
//  DPSocketTest.m
//  DPIPV6Test
//
//  Created by weifu Deng on 8/24/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#import "DPSocketTest.h"
#include "DPSocket.hpp"

#ifndef DP_SOCKET_TCP_PORT
#define DP_SOCKET_TCP_PORT       (16001)
#endif

#ifndef DP_SOCKET_UDP_PORT
#define DP_SOCKET_UDP_PORT       (16003)
#endif

#ifndef DP_SOCKET_SERVER_IP
#define DP_SOCKET_SERVER_IP      "192.168.250.191"//"192.168.250.44"
#endif

@interface DPSocketTest(){
    DPSocket *_tcpSrv;
    DPSocket *_tcpClt;
    DPSocket *_udpSrv;
    DPSocket *_udpClt;
    sockaddr_storage *_pTCPRemoteAddr;
    sockaddr_storage *_pUDPRemoteAddr;
    sockaddr_storage *_pTCPLocalAdd;
    sockaddr_storage *_pUDPLocalAdd;
}

@end

@implementation DPSocketTest

- (instancetype)init{
    self = [super init];
    if (self) {
    }
    
    return self;
}

- (void)creatTCPServer{
    _tcpSrv = new DPSocket();
    _tcpSrv->creatSocket(SOCK_STREAM);
    _pTCPLocalAdd = _tcpSrv->creatLocalSockaddrByPort(DP_SOCKET_TCP_PORT);
    _tcpSrv->bindAddr((sockaddr *)_pTCPLocalAdd);
    _tcpSrv->listenCount(16);
    _tcpSrv->accept();
}

- (void)creatTCPClient{
    struct timeval tv_out;
    tv_out.tv_sec = 5.0f;
    tv_out.tv_usec = 0;
    
    _tcpClt = new DPSocket();
    _tcpClt->creatSocket(SOCK_STREAM);
    _pTCPRemoteAddr = _tcpClt->creatRemoteSockaddrByHostPort((char *)DP_SOCKET_SERVER_IP,
                                                             DP_SOCKET_TCP_PORT);
    _tcpClt->connectToHost((sockaddr *)_pTCPRemoteAddr, &tv_out);
}

- (void)tcpSendTest{
    char testData[] = "nihao";
    ssize_t iret = _tcpClt->sendData(testData, sizeof(testData));
    if (iret <= 0) {
        NSLog(@"tcp client send data failed");
    }
}

- (void)tcpRecvTest{
    char testData[13] = {0};
    ssize_t iret = _tcpClt->recvData(testData, sizeof(testData), NULL);
    if (iret > 0) {
        NSLog(@"tcp recv msg: %s", testData);
    }
}

- (void)creatUDPServer{
    
    _udpSrv = new DPSocket();
    _udpSrv->creatSocket(SOCK_DGRAM);
    _pUDPLocalAdd = _udpSrv->creatLocalSockaddrByPort(DP_SOCKET_UDP_PORT);
    _udpSrv->bindAddr((sockaddr *)_pUDPLocalAdd);
    
    char strBuff[6];
    sockaddr_storage *remoteAddr = (sockaddr_storage *)malloc(sizeof(sockaddr_storage));

    while (_udpSrv > 0) {
        memset(strBuff, 0, sizeof(strBuff));
        memset(remoteAddr, 0, sizeof(sockaddr_storage));
        ssize_t iret = _udpSrv->recvDataFrom(strBuff, sizeof(strBuff), (sockaddr *)remoteAddr);
        if (iret > 0) {
            char ack[] = "ack";
            _udpSrv->sendDataTo(ack, sizeof(ack), (sockaddr *)remoteAddr);
        }
    }
}

- (void)creatUDPClient{
    _udpClt = new DPSocket();
    _udpClt->creatSocket(SOCK_DGRAM);
    _pUDPRemoteAddr = _udpClt->creatRemoteSockaddrByHostPort((char *)DP_SOCKET_SERVER_IP,
                                                          DP_SOCKET_UDP_PORT);
}

- (void)UdpSendToTest{
    char testData[] = "nihao";
    ssize_t iret = _udpClt->sendDataTo(testData, sizeof(testData), (sockaddr *)_pUDPRemoteAddr);
    if (iret <= 0) {
        NSLog(@"udp client send data failed");
    }
}

- (void)UdpRecvFromTest{
    char rstr[6] = {0};
    sockaddr_storage *remoteAddr = (sockaddr_storage *)malloc(sizeof(sockaddr_storage));
    memset(remoteAddr, 0, sizeof(sockaddr_storage));
    ssize_t iret = _udpClt->recvDataFrom(rstr, sizeof(rstr), (sockaddr *)remoteAddr);
    if (iret > 0) {
        NSLog(@"udp recv msg: %s", rstr);
    }
}

- (void)dealloc{
    _tcpSrv->freeSockaddr(_pTCPLocalAdd);
    _tcpClt->freeSockaddr(_pTCPRemoteAddr);
    _udpSrv->freeSockaddr(_pUDPLocalAdd);
    _udpClt->freeSockaddr(_pUDPRemoteAddr);
    
    _tcpSrv->closeSocket();
    _tcpClt->closeSocket();
    _udpSrv->closeSocket();
    _udpClt->closeSocket();
}

@end
