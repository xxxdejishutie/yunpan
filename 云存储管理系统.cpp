#include<iostream>
#include"TCPkernel.h"
#include"C:\Program Files (x86)\Visual Leak Detector\include\vld.h"//查寻内存泄漏的软件
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
	delete k;

	return 0;
}