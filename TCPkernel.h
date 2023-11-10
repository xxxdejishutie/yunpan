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
//����ģʽ��������ֻ����һ��
#define Maxsocle 300
class TCPkernel :public Ikernel
{
private:
	Inet* m_pinet;
	CMySql* m_pmysql;
	static TCPkernel* m_tcpkernel;
	char m_path[64];
	map<long long, uploadfileinfo*> m_mapfileiftofileinfo;//���������ϴ���һ����ļ���Ϣ�����ļ�idѰ��
	map<pair<int,int>, uploadfileinfo*> m_mapfileinfoinstall;//���������ļ�����Ϣ
	shared_mutex s_mutex;//���ļ���Ϣ����б�����������д����
private:
	TCPkernel();
	~TCPkernel();
	
public:
	void do_REGISTER_RQ(shared_ptr<char[]>  buf,SOCKET sock);//ע��ģ��
	void do_LOGIN_RQ(shared_ptr<char[]>  buf, SOCKET sock);//��¼ģ��
	void do_GETFILELIST_RQ(shared_ptr<char[]>  buf, SOCKET sock);//��ȡ�ļ��б�ģ��


	//�����ļ��ϴ�ģ��
	void do_UPLOAD_FILEINFO(shared_ptr<char[]>  buf, SOCKET sock);//�ϴ��ļ���Ϣ����ظ���
	void do_UPLOAD_FILECONTENT_RQ(shared_ptr<char[]>  buf, SOCKET sock);//�����ļ�������

	//�ļ�����ģ��
	//�����ļ�����
	void do_DOWNLOAD_FILEINFO_RQ(shared_ptr<char[]>  buf, SOCKET sock);
    //�����ļ�����
	void DOWNLOAD_FILECONTENT(char*  buf, SOCKET sock);
    //�ļ�����ȷ��
	void do_DOWNLOAD_FILECONTENT_RELY(shared_ptr<char[]>  buf, SOCKET sock);


	//ɾ���ļ�ģ��
	void do_DELETE_FILE_RQ(shared_ptr<char[]>  buf, SOCKET sock);//ɾ��������


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

