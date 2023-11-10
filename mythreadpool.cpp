#include"mythreadpool.h"


template<class T>
my_circul_queue<T>::my_circul_queue(int length):mlength(length)
{
	mhead = 0;
	mend = 0;
	queue.resize(length);
}

template<class T>
bool my_circul_queue<T>::push(T&& tmp)
{
	if (mhead - mend == mlength)
	{
		return 0;
	}
	else
	{
		ve_task_queue[mhead % mlength] = move(tmp);
	}
	return 1;
}

template<class T>
T&& my_circul_queue<T>::top()
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


