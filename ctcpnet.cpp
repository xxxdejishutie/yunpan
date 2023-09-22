﻿#include "ctcpnet.h"
#include<iostream>.
#include<WS2tcpip.h>
using namespace std;

CTCPNet::CTCPNet(Ikernel* pkernel)
{
    m_sock = 0;
    m_bFlagQuit = true;
    m_pkernel = pkernel;
    m_connectnum = 0;
    memset(m_arrClient, -1, sizeof(m_arrClient));
}
//1.同步阻塞+ 多线程
//2.单线程处理客户端


bool CTCPNet::InitNetWork()
{
      WORD wVersionRequested;
      WSADATA wsaData;
      int err;


      wVersionRequested = MAKEWORD(2, 2);

      err = WSAStartup(wVersionRequested, &wsaData);
      if (err != 0) {

          return false;
      }

      if (LOBYTE(wsaData.wVersion) != 2 ||
          HIBYTE(wsaData.wVersion) != 2)
      {
          UnInitNetWork();
          return false;
      }

      //创建监听套接字
      m_sock =  socket(AF_INET,SOCK_STREAM,0);
      if(m_sock == INVALID_SOCKET)
      {
           UnInitNetWork();
           return false;
      }

      //3.找个地方
      sockaddr_in addrserver;
      addrserver.sin_family = AF_INET;
      addrserver.sin_port = htons(8899);
     addrserver.sin_addr.S_un.S_addr = 0;
      //addrserver.sin_addr.S_un.S_addr = inet_addr("192.168.137.1");
      //InetPtonW(AF_INET, L"192.168.137.1", &addrserver.sin_addr);
     if(SOCKET_ERROR == bind(m_sock,( const sockaddr *)&addrserver,sizeof(addrserver)))
     {
        // int n = GetLastError();
         UnInitNetWork();
         return false;
     }
     //fd_set reset, allset;
      //4.店长宣传
     if( SOCKET_ERROR == listen(m_sock,10))
     {
         UnInitNetWork();
         return false;
     }

     FD_ZERO(&m_allset);
     FD_SET(m_sock, &m_allset);
     //4.创建线程池
     m_connectnum++;
      m_hThread = (HANDLE)_beginthreadex(0,0,&ThreadProc,this,0,0);
      if(m_hThread)
           m_lstThread.push_back(m_hThread);


    return true;
}

unsigned _stdcall CTCPNet::ThreadProc(void *lpvoid)
{
    CTCPNet *pthis= (CTCPNet*)lpvoid;
    unsigned dwthreadid;
    while(pthis->m_bFlagQuit)
    {
        pthis->m_reset = pthis->m_allset;
        //select 返回有多少个读就绪事件 
           // rset是传入传出参数，传入的时候 告诉select我需要监听哪些事件，传出的时候rset表示事件就绪集 
        int nready = select(1025, &pthis->m_reset, 0, 0, 0);
        if (nready < 0)
        {
            perror("select error");
            return 0;
        }
        nready = select(pthis->m_connectnum + 1, &pthis->m_reset, NULL, NULL, NULL);
        std::cout << "nready" << nready << endl;
        if (nready < 0) {
            fputs("select error\n", stderr);
            exit(1);
        }
        int maxi = pthis->m_connectnum - 1;
        //看看位图里有没有连接事件  
        char cli_ip[45] = "";
        if (FD_ISSET(pthis->m_sock, &pthis->m_reset)) {
            //客户端地址长度 
            sockaddr_in cliaddr;
            int cliaddr_len = sizeof(cliaddr);
            //三次握手接收连接 返回connfd ，accept不阻塞 
            SOCKET connfd = accept(pthis->m_sock, (struct sockaddr*)&cliaddr, &cliaddr_len);
            std::printf("connfd:%d\n", connfd);
            std::printf("received from %s at PORT %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, cli_ip, sizeof(cli_ip)),
                ntohs(cliaddr.sin_port));
            //client数组记录下connfd 
            int i;
            for (i = 0; i < FD_SETSIZE; i++)
                if (pthis->m_arrClient[i] < 0) {
                    pthis->m_arrClient[i] = connfd;
                    break;
                }
            //打印client数组 有效数据 
            int j;
            for (j = 0; j < pthis->m_connectnum - 1; j++) {
                if (pthis->m_arrClient[j] != -1) {
                    std::printf("client index=%d value=%d \n", j, pthis->m_arrClient[j]);
                }
            }


            if (i == FD_SETSIZE) {
                fputs("too many clients\n", stderr);
                exit(1);
            }
            //将位图 connfd 通过位图算法计算出来的所在位设为1 
            FD_SET(connfd, &pthis->m_allset);
            //更新 已连接数量       
            pthis->m_connectnum++;
            //如果 nready==0,说明没有事件了 ，那么没有必要再去读取客户端发送过来的数据了 
            if (--nready == 0)
                continue;

        }
        SOCKET sockfd;
        //处理客户端发送过来的数据  ，遍历多路io，这在以前bio是不可能的，bio只会在read死等数据 
        for (int i = 0; i < maxi; i++) {
            // 这个if一般不会进去，因为 0~maxi的数据都是有效的 
            if ((sockfd = pthis->m_arrClient[i]) < 0)
                continue;
            printf("sockfd:%d\n", sockfd);
            //看看位图里有没有客户端发送数据事件 
           int nPackSize;
            if (FD_ISSET(sockfd, &pthis->m_reset)) {
                //客户端断开连接 ，这个read不阻塞 
                int n;
                if ((n = recv(sockfd, (char*)&nPackSize, sizeof(int), 0)) <= 0) {
                    //关闭 sockfd
                    closesocket(sockfd);
                    //把 allset里的 sockfd所在位 置为0 
                    FD_CLR(sockfd, &pthis->m_allset);
                    //client数组清为-1 
                    pthis->m_arrClient[i] = -1;
                }
                else {
                    //有数据发送 
                    char *pszbuf = new char[nPackSize];
                    int noffset = 0;
                    while (nPackSize)
                    {
                        int nRecvNum = recv(sockfd, pszbuf + noffset, nPackSize, 0);
                        noffset += nRecvNum;
                        nPackSize -= nRecvNum;
                    }
                    //处理数据
                    pthis->m_pkernel->dealtext(pszbuf, sockfd);
                }
                if (--nready == 0)//如果 nready==0 ，没必要再去读数据了 
                    break;
            }
        }
    }

           
            

        //SOCKET sockWaiter = accept(pthis->m_sock,0,0);
        //if(sockWaiter == INVALID_SOCKET)continue;

        //HANDLE m_hThread = (HANDLE)_beginthreadex(0,0,&ThreadRecv,pthis,0,&dwthreadid);
        //if(m_hThread)
        //{
        //    pthis->m_mapIdToSock[dwthreadid] = sockWaiter;//用线程id映射sock套接字
        //    pthis->m_lstThread.push_back(m_hThread);//把套接字交给链表管理
        //}

    

    return 0;
}



unsigned _stdcall CTCPNet::ThreadRecv(void *lpvoid)
{
    CTCPNet *pthis= (CTCPNet*)lpvoid;
    pthis->RecvData();
    return 0;
}

void CTCPNet::RecvData()
{
    SOCKET sockWaiter = m_mapIdToSock[GetCurrentThreadId()];
    int nPackSize;
    int nRecvNum;
    char *pszbuf = NULL;
    int noffset ;
    while(m_bFlagQuit)
    {
        nRecvNum= recv(sockWaiter,(char*)&nPackSize,sizeof(int),0);
        if(nRecvNum <=0)
        {
            //如果客户端下线,则退出
            if(WSAGetLastError() == WSAECONNRESET)
                break;

            continue;
        }
        if(nPackSize <=0)continue;

        pszbuf = new char[nPackSize];
        noffset = 0;
        while(nPackSize)
        {
            nRecvNum = recv(sockWaiter,pszbuf+noffset,nPackSize,0);
            noffset += nRecvNum;
            nPackSize -= nRecvNum;
        }
        //处理数据
        m_pkernel->dealtext(pszbuf,sockWaiter);


        

        delete []pszbuf;
        pszbuf = NULL;
    }
}

void CTCPNet::UnInitNetWork()
{
    m_bFlagQuit = false;
    std::list<HANDLE>::iterator ite = m_lstThread.begin();
    while(ite != m_lstThread.end())
    {
        if(*ite)
        {
            if(WAIT_TIMEOUT == WaitForSingleObject(*ite,100))
                TerminateThread(*ite,-1);

            CloseHandle(*ite);
            *ite = NULL;
        }
         ite++;
    }
    m_lstThread.clear();

    if(m_sock)
    {
        closesocket(m_sock);
        m_sock = 0;
    }
     WSACleanup();
}

bool CTCPNet::SendData(SOCKET sock,char *szbuf,int nlen)
{
    if(sock == INVALID_SOCKET || !szbuf || nlen <=0)
        return false;
    //发送大小
     if(send(sock,(char*)&nlen,sizeof(int),0) <=0)
         return false;
    //发送内容
    if(send(sock,szbuf,nlen,0) <=0)
        return false;
    return true;
}