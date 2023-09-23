#include<iostream>
#include"TCPkernel.h"
using namespace std;
int main()
{
	cout << "服务器正在启动" << endl;
	Ikernel* k = TCPkernel::gettcpkernel();
	cout << "文件存储位置为d:/disk" << endl;

	if (k->Opensqlandnet())//初始化网络服务，sql数据库服务
	{
		cout << "服务器已经成功启动" << endl;
	}

	system("pause");
	return 0;
}