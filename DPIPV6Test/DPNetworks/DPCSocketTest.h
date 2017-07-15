//
//  DPCSocketTest.h
//  DPIPV6Test
//
//  Created by weifu Deng on 9/8/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface DPCSocketTest : NSObject

- (BOOL)creat_tcp_srv;

- (BOOL)creat_tcp_clt;
- (void)tcp_recv_test;
- (void)tcp_send_test;

- (BOOL)creat_udp_srv;

- (BOOL)creat_udp_clt;
- (void)udp_recv_test;
- (void)udp_sendto_test;

@end
