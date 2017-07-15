//
//  ViewController.m
//  SYTSocketServer
//
//  Created by dwf on 16/8/29.
//  Copyright © 2016年 dwf. All rights reserved.
//

#import "ViewController.h"
#import <DPCSocket_masOS/DPCSocketTest.h>

@interface ViewController (){
    DPCSocketTest *_testSocket;
}

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (void)viewDidAppear{
    [super viewDidAppear];
    _testSocket = [[DPCSocketTest alloc] init];
}

- (IBAction)startTCPServer:(NSButton *)sender {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
       [_testSocket creat_tcp_srv];
    });
}

- (IBAction)startUDPServer:(NSButton *)sender {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [_testSocket creat_udp_srv];
    });
}

@end
