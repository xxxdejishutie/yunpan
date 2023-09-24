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
			my_circul_queue[(mhead + 1) % mlength] = tmp;
		}
	}
};
class mythread_pool
{
	int max_thread;//最大线程数
	int now_total_run;//当前存在的线程数
	int now_hav_run;//当前正在运行的线程数
	int arr_thread_id[1000];
	mythread_pool()
	{
		
	}
};