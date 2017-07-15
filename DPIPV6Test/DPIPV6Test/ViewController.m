//
//  ViewController.m
//  DPIPV6Test
//
//  Created by weifu Deng on 8/23/16.
//  Copyright © 2016 d-power. All rights reserved.
//

#import "ViewController.h"
#import <DPCSocket_iOS/DPCSocketTest.h>
#import <DPCSocket_iOS/DPCSocket.h>

#define remoteIP "121.42.169.247"

@interface ViewController (){
    DPCSocketTest *_testSocket;

}

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    [self creatSocket];
    //[self testGetnameinfo];
    //[self test];
    //[self addrtest];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)creatSocket{
    _testSocket = [[DPCSocketTest alloc] init];
    [_testSocket creat_tcp_clt];
    [_testSocket creat_udp_clt];
}

- (IBAction)clickOnBtn:(UIButton *)sender {
    if (sender.tag == 1001) {
        [_testSocket tcp_send_test];
        [_testSocket tcp_recv_test];
    }
    else if (sender.tag == 1002){
        [_testSocket udp_sendto_test];
        [_testSocket udp_recv_test];
    }
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
    if ((tset_socket != SOCKET_ERROR) &&
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

- (void)test{
    struct sockaddr_storage *dest_addr;
    dest_addr = dp_creat_remote_sockaddr("118.244.206.125", 3476);
    if (dest_addr) {
        [self test_inet_pton:dest_addr];
        dp_free_sockaddr(dest_addr);
    }
}

- (void)test_inet_pton:(struct sockaddr_storage *)addr{
    dp_logSockAddrInfo(addr, "testinetpto");
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)addr;
        NSLog(@"int ip: %d", ntohl(ipv4->sin_addr.s_addr));
    }
    else if (addr->ss_family == AF_INET6) {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)addr;
        NSLog(@"int ip: %d %d %d %d",
              ntohl(ipv6->sin6_addr.__u6_addr.__u6_addr32[0]),
              ntohl(ipv6->sin6_addr.__u6_addr.__u6_addr32[1]),
              ntohl(ipv6->sin6_addr.__u6_addr.__u6_addr32[2]),
              ntohl(ipv6->sin6_addr.__u6_addr.__u6_addr32[3]));
    }
}

@end

