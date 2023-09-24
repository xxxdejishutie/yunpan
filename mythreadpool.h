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
	vector<T&&> queue;//����
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
	int max_thread;//����߳���
	int now_total_run;//��ǰ���ڵ��߳���
	int now_hav_run;//��ǰ�������е��߳���
	int arr_thread_id[1000];
	mythread_pool()
	{
		
	}
};