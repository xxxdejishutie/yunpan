#pragma once
#include<functional>
#include<thread>
#include<vector>
using namespace std;
template <class T>
class my_circul_queue
{
	int mhead;//ͷָ��
	int mend;//βָ��
	int mlength;//���г���
	vector<T> ve_task_queue;//����
public:
	my_circul_queue() = delete;
	my_circul_queue(int length = 10);
	
public:
	bool push(T tmp);
	T top();
	void pop();
};


template<class T>
class mythread_pool
{
	int MaxThreadNum;//����߳���
	//int now_total_run;//��ǰ���ڵ��߳���
	//int now_hav_run;//��ǰ�������е��߳���
	vector<thread> ve_thread;//�߳�����
	vector<my_circul_queue<T>> task_queue;//������У�ÿ���߳�һ��

public:
	void push_task(T);
public:
	mythread_pool();
	/*mythread_pool(int length);
	mythread_pool(int length, int max_thread_ant);
	mythread_pool(int length, int max_thread_ant,int min_thread_ant);*/

	~mythread_pool();
	//void my_threadfun();
public:
	void thread_run(my_circul_queue<T>);//�߳����еĺ���
};