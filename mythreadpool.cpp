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
	//ѭ��ȷ�����зǿ�
	while (mend == mhead);
	//�����ǰλ�õ����ݣ�����ַ��һ
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
	//ѭ�������̣߳�Ϊÿ���̴߳����������
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
	//�ر��߳�

	//�ͷ���Դ
	
}
	//void my_threadfun();
template<class T>
void mythread_pool<T>::thread_run(my_circul_queue<T>)
{
	//�߳����к��������ϴӶ�����ȡ���ݲ�����
}

