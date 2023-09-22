#pragma once
#include<WinSock2.h>
#pragma comment(lib,"Ws2_32")
class Inet
{
public:
    Inet()
    {}
    virtual ~Inet()
    {}
public:
    virtual bool InitNetWork() = 0;
    virtual void UnInitNetWork() = 0;
    virtual bool SendData(SOCKET sock, char* szbuf, int nlen) = 0;
    virtual void RecvData() = 0;

};

