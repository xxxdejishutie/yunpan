#ifndef CTCPNET_H
#define CTCPNET_H
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <list>
#include <map>
#include<thread>
#include"Inet.h"
#include"TCPkernel.h"

class CTCPNet : public Inet
{
public:
    CTCPNet(Ikernel* pkernel);
public:
    //1.初始化网络
    bool InitNetWork();
    void UnInitNetWork();
    //2.收发数据
    bool SendData(SOCKET sock,char *szbuf,int nlen);
    void RecvData();
public:
    static  unsigned _stdcall ThreadProc(void *);
    static  unsigned _stdcall ThreadRecv(void *);
private:
     SOCKET m_sock;
     SOCKET MLoadUdpSock;//用于负载均衡的udp套接字
     HANDLE m_hThread;
     std::list<HANDLE> m_lstThread;
     std::map<unsigned int,SOCKET> m_mapIdToSock;
     bool   m_bFlagQuit;
     Ikernel* m_pkernel;
     fd_set m_reset,m_allset;//select模型
     int m_arrClient[1024];//客户端套接字
     int m_connectnum;//已连接得数量


     bool *RunFlag;//控制线程结束
     int ConnCnt;//保存最大连接数
     //mutex MCntMutex;//互斥访问最大连接数
public:
    thread loadthread;//用于和代理服务器通信的线程
    friend void  loadfile(CTCPNet* pthis, char* ip, int port);

};

#endif // CTCPNET_H
