// VideoRTSPServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "RTSPServer.h"
#include "RTPHelper.h"
#include <iostream>


int main()
{
    RTPHeader header;
    RTPHeader* pheader = &header;
    memset(&header, 0, sizeof(header));
    printf("header size %d\r\n", sizeof(header));
    header.version = 2;
    header.padding = 1;
    header.extension = 0;
    header.csrccount = 5;

    //RTSPServer server;
    //server.Init();
    //server.Invoke();
    //printf("press any key to stop...\r\n");
    //getchar();
    //server.Stop();
    return 0;
}