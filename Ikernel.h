#pragma once
#include <winsock2.h>
#pragma comment(lib,"Ws2_32")
#define Maxpath 64
class Ikernel
{
public:
	Ikernel()
	{}
	virtual ~Ikernel()
	{}
public:
	virtual void do_REGISTER_RQ(char* buf,SOCKET sock) = 0;//注册
	virtual void do_LOGIN_RQ(char* buf, SOCKET sock) = 0;//登录
	virtual void do_GETFILELIST_RQ(char* buf, SOCKET sock) = 0;//获取文件列表
	virtual void do_UPLOAD_FILEINFO(char* buf, SOCKET sock) = 0;//文件信息回复包
public:
	virtual bool Opensqlandnet() = 0;//启动数据库和网络
	virtual void Closesqlandnet() = 0;//关闭
	virtual void dealtext(char *,SOCKET sock) = 0;//处理
};

