#ifndef CTCPNET_H
#define CTCPNET_H
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <list>
#include <map>
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
     HANDLE m_hThread;
     std::list<HANDLE> m_lstThread;
     std::map<unsigned int,SOCKET> m_mapIdToSock;
     bool   m_bFlagQuit;
     Ikernel* m_pkernel;
     fd_set m_reset,m_allset;//select模型
     int m_arrClient[1024];//客户端套接字
     int m_connectnum;//已连接得数量
};

#endif // CTCPNET_H
