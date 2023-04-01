#pragma once
#include<queue>
#include<pthread.h>

//����ṹ��
using callback = void (*)(void* arg);
struct Task
{
	Task()
	{
		function = nullptr;
		arg = nullptr;
	}
	Task(callback f, void* arg)
	{
		this->arg = arg;
		function = f;
	}

	callback function;		//������ָ��
	void* arg;				//����
};

class TaskQueue
{
public:
	TaskQueue();
	~TaskQueue();

	//�������
	void addTask(Task task);
	void addTask(callback f, void* arg);
	//ȡ������
	Task takeTask();
	//��ȡ��ǰ�������
	inline size_t taskNumber()
	{
		return m_taskQ.size();
	}
private:
	pthread_mutex_t m_mutex;		//�������Ա����������
	std::queue<Task>m_taskQ;	//�������
};

