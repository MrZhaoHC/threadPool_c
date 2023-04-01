#pragma once
#include<queue>
#include<pthread.h>

//任务结构体
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

	callback function;		//任务函数指针
	void* arg;				//传参
};

class TaskQueue
{
public:
	TaskQueue();
	~TaskQueue();

	//添加任务
	void addTask(Task task);
	void addTask(callback f, void* arg);
	//取出任务
	Task takeTask();
	//获取当前任务个数
	inline size_t taskNumber()
	{
		return m_taskQ.size();
	}
private:
	pthread_mutex_t m_mutex;		//访问类成员变量互斥锁
	std::queue<Task>m_taskQ;	//任务队列
};

