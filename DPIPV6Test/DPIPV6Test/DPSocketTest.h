//
//  DPSocketTest.h
//  DPIPV6Test
//
//  Created by weifu Deng on 8/24/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface DPSocketTest : NSObject

- (void)creatTCPServer;
- (void)creatTCPClient;

- (void)tcpSendTest;
- (void)tcpRecvTest;

- (void)creatUDPServer;
- (void)creatUDPClient;

- (void)UdpSendToTest;
- (void)UdpRecvFromTest;


@end
