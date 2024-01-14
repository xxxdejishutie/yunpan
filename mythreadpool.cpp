#include"mythreadpool.h"


template<class T>
my_circul_queue<T>::my_circul_queue(int length):mlength(length)
{
	mhead = 0;
	mend = 0;
	ve_task_queue.resize(length);
}

template<class T>
bool my_circul_queue<T>::push(T tmp)
{
	if (mhead - mend == mlength)
	{
		return 0;
	}
	else
	{
		ve_task_queue[mhead % mlength] = move(tmp);
		mhead++;
	}
	return 1;
}

template<class T>
T my_circul_queue<T>::top()
{
	//循环确定队列非空
	while (mend == mhead);
	//输出当前位置的内容，并地址加一
	return ve_task_queue[mend++];
}

template<class T>
void my_circul_queue<T>::pop()
{
	mend++;
}

template<class T>
void mythread_pool<T>::push_task(T Task)
{
	int AimPos = rand() % MaxThreadNum;
	task_queue[AimPos].push(Task);
}

template<class T>
mythread_pool<T>::mythread_pool()
{
	//循环创建线程，为每个线程创建任务队列
	for (int i = 0; i < MaxThreadNum; i++)
	{
		auto th = thread(thread_run, this, my_circul_queue<T>);
		ve_thread.emplace_back(move(th));
		task_queue.emplace_back(my_circul_queue<T>{});
	}
}
	
template<class T>
mythread_pool<T>::~mythread_pool()
{
	//关闭线程

	//释放资源
	
}
	//void my_threadfun();
template<class T>
void mythread_pool<T>::thread_run(my_circul_queue<T>)
{
	//线程运行函数，不断从队列中取数据并处理
}

