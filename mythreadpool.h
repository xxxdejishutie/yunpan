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
	vector<T&&> queue;//队列
public:
	my_circul_queue() = delete;
	//
	my_circul_queue(int length):mlength(length)
	{
		mhead = 0;
		mend = 0;
		queue.resize(length);
	}

public:
	bool push(T &&tmp)
	{
		if (mhead - mend == mlength)
		{
			return 0;
		}
		else
		{
			my_circul_queue[(mhead + 1) % mlength] = move(tmp);
			++mhead;
		}
	}


	T& pop()
	{
		//循环确定队列非空
		while (mend == mhead);
		//输出当前位置的内容，并地址加一
		return my_circul_queue[mend++];
	}


};
class mythread_pool
{
	int max_thread;//最大线程数
	int now_total_run;//当前存在的线程数
	int now_hav_run;//当前正在运行的线程数
	vector<int> ve_thread_id;//线程数组
public:
	mythread_pool();
	~mythread_pool();
	void my_threadfun();
};