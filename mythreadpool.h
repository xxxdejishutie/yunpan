#pragma once
#include<functional>
#include<thread>
#include<vector>
using namespace std;
template <class T>
class my_circul_queue
{
	int mhead;//头指针
	int mend;//尾指针
	int mlength;//队列长度
	vector<T&&> ve_task_queue;//队列
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
	int max_thread;//最大线程数
	int now_total_run;//当前存在的线程数
	int now_hav_run;//当前正在运行的线程数
	vector<int> ve_thread_id;//线程数组
	my_circul_queue<T> task_queue;

public:
	push_task(T&&);

public:
	mythread_pool();
	~mythread_pool();
	void my_threadfun();
};