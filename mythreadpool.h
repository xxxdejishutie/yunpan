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
	vector<T&&> ve_task_queue;//����
public:
	my_circul_queue() = delete;

	my_circul_queue(int length);
	

public:
	bool push(T&& tmp);
	T&& top();
	void pop();
};


template<class T>
class mythread_pool
{
	int max_thread;//����߳���
	int now_total_run;//��ǰ���ڵ��߳���
	int now_hav_run;//��ǰ�������е��߳���
	vector<int> ve_thread_id;//�߳�����
	my_circul_queue<T> task_queue;

public:
	push_task(T&&);

public:
	mythread_pool();
	~mythread_pool();
	void my_threadfun();
};