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
	virtual void do_REGISTER_RQ(char* buf,SOCKET sock) = 0;//ע��
	virtual void do_LOGIN_RQ(char* buf, SOCKET sock) = 0;//��¼
	virtual void do_GETFILELIST_RQ(char* buf, SOCKET sock) = 0;//��ȡ�ļ��б�
	virtual void do_UPLOAD_FILEINFO(char* buf, SOCKET sock) = 0;//�ļ���Ϣ�ظ���
public:
	virtual bool Opensqlandnet() = 0;//�������ݿ������
	virtual void Closesqlandnet() = 0;//�ر�
	virtual void dealtext(char *,SOCKET sock) = 0;//����
};

