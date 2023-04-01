#include "ThreadPool.h"
#include<iostream>
#include<string.h>
#include<string>
#include<unistd.h>
using namespace std;

ThreadPool::ThreadPool(int min, int max)
{
	
	do
		{
		//ʵ����������
		m_taskQ = new TaskQueue;
		if (m_taskQ == nullptr)
		{
			cout << "new taskQ failed...." << endl;
			break;
		}
		m_threadIDs = new pthread_t[max];
		if (m_threadIDs==nullptr)
		{
			cout << "new threadIDs failed...." << endl;
			break;
		}
		memset(m_threadIDs, 0, sizeof(pthread_t)*max); //��ʼ������Ϊ0,ͷ�ļ�string.h
		m_minNum = min;
		m_maxNum = max;
		m_busyNum = 0;
		m_aliveNum = min;		//����С������ͬ
		m_exitNum = 0;

		//��ʼ��������
		if (pthread_mutex_init(&m_lock, NULL) != 0 ||
			pthread_cond_init(&m_notEmpty, NULL) != 0)
		{
			cout << "mutex or condition lock init failed...." << endl;
			break;
		}

		//�̳߳�Ĭ���ǲ��رյ�
		m_shutdown = false;

		//�����߳�									//���ݲ���thisָ�룬��Ϊ�˲�����ǰ���еĳ�Ա��������̬��Ա����ֻ�ܷ��ʾ�̬��Ա������
		pthread_create(&m_managerID, NULL, manager, this);	//�����������߳�

		for (int i = 0; i < max; ++i)						//�����������߳�
		{
			pthread_create(&m_threadIDs[i], NULL, worker, this);
		}
		return;
	} while (0);
	
	//���ִ��ʧ������dowhileѭ���ͷ����������Դ 
	if (m_threadIDs) delete[]m_threadIDs;
	if (m_taskQ)delete m_taskQ;
}

ThreadPool::~ThreadPool()
{
	m_shutdown = 1;
	// ���ٹ������߳�
	pthread_join(m_managerID, NULL);
	// ���������������߳�
	for (int i = 0; i < m_aliveNum; ++i)
	{
		pthread_cond_signal(&m_notEmpty);
	}

	if (m_taskQ) delete m_taskQ;
	if (m_threadIDs) delete[]m_threadIDs;
	pthread_mutex_destroy(&m_lock);
	pthread_cond_destroy(&m_notEmpty);
}

void ThreadPool::addTask(Task task)
{
	if (m_shutdown)
	{
		return;
	}
	// ������񣬲���Ҫ�������������������
	m_taskQ->addTask(task);
	// ���ѹ������߳�
	pthread_cond_signal(&m_notEmpty);
}

int ThreadPool::getBusyNum()
{
	int busyNum = 0;
	pthread_mutex_lock(&m_lock);
	busyNum = m_busyNum;
	pthread_mutex_unlock(&m_lock);
	return busyNum;
}

int ThreadPool::getAliveNum()
{
	int threadNum = 0;
	pthread_mutex_lock(&m_lock);
	threadNum = m_aliveNum;
	pthread_mutex_unlock(&m_lock);
	return threadNum;
}

void* ThreadPool::worker(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);//c++�﷨static_cast<ThreadPool*>(arg) �൱��c�﷨(ThreadPool*)arg
	// һֱ��ͣ�Ĺ���
	while (true)
	{
		// �����������(������Դ)����
		pthread_mutex_lock(&pool->m_lock);
		// �ж���������Ƿ�Ϊ��, ���Ϊ�չ����߳�����
		while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown)
		{
			//cout << "thread " << to_string(pthread_self()) << " waiting..." << endl;
			// �����߳�
			pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);

			// �������֮��, �ж��Ƿ�Ҫ�����߳�
			if (pool->m_exitNum > 0)
			{
				pool->m_exitNum--;
				if (pool->m_aliveNum > pool->m_minNum)
				{
					pool->m_aliveNum--;
					pthread_mutex_unlock(&pool->m_lock);
					pool->threadExit();
				}
			}
		}
		// �ж��̳߳��Ƿ񱻹ر���
		if (pool->m_shutdown)
		{
			pthread_mutex_unlock(&pool->m_lock);
			pool->threadExit();
		}

		// �����������ȡ��һ������
		Task task = pool->m_taskQ->takeTask();
		// �������߳�+1
		pool->m_busyNum++;
		// �̳߳ؽ���
		pthread_mutex_unlock(&pool->m_lock);
		// ִ������
		cout << "thread " << to_string(pthread_self()) << " start working..." << endl;
		task.function(task.arg);
		delete task.arg;
		task.arg = nullptr;

		// ���������
		cout << "thread " << to_string(pthread_self()) << " end working..."<<endl;
		pthread_mutex_lock(&pool->m_lock);
		pool->m_busyNum--;
		pthread_mutex_unlock(&pool->m_lock);
	}
	return nullptr;
}

void* ThreadPool::manager(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	// ����̳߳�û�йر�, ��һֱ���
	while (!pool->m_shutdown)
	{
		// ÿ��5s���һ��
		sleep(5);			//<unistd.h>
		// ȡ���̳߳��е����������߳�����
		//  ȡ���������̳߳�����
		pthread_mutex_lock(&pool->m_lock);
		int queueSize = pool->m_taskQ->taskNumber();
		int liveNum = pool->m_aliveNum;
		int busyNum = pool->m_busyNum;
		pthread_mutex_unlock(&pool->m_lock);

		// �����߳�
		const int NUMBER = 2;
		// ��ǰ�������>�����߳��� && �����߳���<����̸߳���
		if (queueSize > liveNum && liveNum < pool->m_maxNum)
		{
			// �̳߳ؼ���
			pthread_mutex_lock(&pool->m_lock);
			int num = 0;
			for (int i = 0; i < pool->m_maxNum && num < NUMBER
				&& pool->m_aliveNum < pool->m_maxNum; ++i)
			{
				if (pool->m_threadIDs[i] == 0)
				{
					pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
					num++;
					pool->m_aliveNum++;
				}
			}
			pthread_mutex_unlock(&pool->m_lock);
		}

		// ���ٶ�����߳�
		// æ�߳�*2 < �����߳���Ŀ && �����߳��� > ��С�߳�����
		if (busyNum * 2 < liveNum && liveNum > pool->m_minNum)
		{
			pthread_mutex_lock(&pool->m_lock);
			pool->m_exitNum = NUMBER;
			pthread_mutex_unlock(&pool->m_lock);
			for (int i = 0; i < NUMBER; ++i)
			{
				pthread_cond_signal(&pool->m_notEmpty);
			}
		}
	}
	return nullptr;
}

void ThreadPool::threadExit()
{
	pthread_t tid = pthread_self();
	for (int i = 0; i < m_maxNum; ++i)
	{
		if (m_threadIDs[i] == tid)
		{
			cout << "threadExit() function: thread "<< to_string(pthread_self()) << " exiting..." << endl;
			m_threadIDs[i] = 0;
			break;
		}
	}
	pthread_exit(NULL);
}
