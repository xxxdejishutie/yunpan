#pragma once
#include"mythreadpool.h"
#include "Ikernel.h"
#include"ctcpnet.h"
#include"CMySql.h"
#include<iostream>
#include"packdef.h"
#include<map>
#include<fstream>
#include<memory>
#include<thread>
#include<mutex>
#include<shared_mutex>


class TCPkernel;
typedef void (TCPkernel::*PFUN)(shared_ptr<char[]>  buf, SOCKET sock);

struct protomap
{
	char n_type;
	PFUN fun;
};

struct uploadfileinfo
{
	long long filesize;
	long long user_id;
	long long fileposition;
	char pfilepath[MAXFILENUM];
	int file_id;
	ofstream* of;
};
//单例模式，服务器只能有一个
#define Maxsocle 300
class TCPkernel :public Ikernel
{
private:
	Inet* m_pinet;
	CMySql* m_pmysql;
	static TCPkernel* m_tcpkernel;
	char m_path[64];
	map<long long, uploadfileinfo*> m_mapfileiftofileinfo;//保存所有上传了一半的文件信息，用文件id寻找
	map<pair<int,int>, uploadfileinfo*> m_mapfileinfoinstall;//保存下载文件的信息
	shared_mutex s_mutex;//对文件信息表进行保护，读共享写互斥
private:
	TCPkernel();
	~TCPkernel();
	
public:
	void do_REGISTER_RQ(shared_ptr<char[]>  buf,SOCKET sock);//注册模块
	void do_LOGIN_RQ(shared_ptr<char[]>  buf, SOCKET sock);//登录模块
	void do_GETFILELIST_RQ(shared_ptr<char[]>  buf, SOCKET sock);//获取文件列表模块


	//这是文件上传模块
	void do_UPLOAD_FILEINFO(shared_ptr<char[]>  buf, SOCKET sock);//上传文件信息请求回复包
	void do_UPLOAD_FILECONTENT_RQ(shared_ptr<char[]>  buf, SOCKET sock);//发送文件报处理

	//文件下载模块
	//下载文件请求
	void do_DOWNLOAD_FILEINFO_RQ(shared_ptr<char[]>  buf, SOCKET sock);
    //下载文件发送
	void DOWNLOAD_FILECONTENT(char*  buf, SOCKET sock);
    //文件接收确认
	void do_DOWNLOAD_FILECONTENT_RELY(shared_ptr<char[]>  buf, SOCKET sock);


	//删除文件模块
	void do_DELETE_FILE_RQ(shared_ptr<char[]>  buf, SOCKET sock);//删除处理函数


public:
	bool Opensqlandnet();
	void Closesqlandnet();


	void dealtext(shared_ptr<char[]>, SOCKET);
	static TCPkernel* gettcpkernel()
	{
		/*if (m_tcpkernel == NULL)
			m_tcpkernel = new TCPkernel;*/
		return m_tcpkernel;
	}
};

