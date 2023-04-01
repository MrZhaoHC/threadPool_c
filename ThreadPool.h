#pragma once
#include "TaskQueue.h"

class ThreadPool
{
public:
    //��ʼ���̳߳�
    ThreadPool(int min,int max);//��С�̣߳�����߳�

    //�����̳߳�
    ~ThreadPool();
    
    //���̳߳��������
    void addTask(Task task);

    //��ȡæ���̸߳��� 
    int getBusyNum();

    //��ȡ���ŵ��̸߳���
    int getAliveNum();

private:
    /////////////////////////////////////
    //�������̵߳�������
    static void* worker(void* arg);

    // �������̵߳�������
    static void* manager(void* arg);

    //�����߳��˳�
    void threadExit();
     
private:
	TaskQueue* m_taskQ;               //�������
    pthread_t* m_threadIDs;         //�������߳�ID
    pthread_t m_managerID;          //�������߳�ID
    int m_minNum;                   //��С�߳���
    int m_maxNum;                   //����߳���
    int m_busyNum;                  //���ڹ������߳���
    int m_aliveNum;                 //���ŵ��߳���
    int m_exitNum;                  //Ҫ���ٵ��߳���
    bool m_shutdown = false;        //�̳߳��Ƿ�ر� false������ true���ر� 
    pthread_mutex_t m_lock;         //�����̳߳���
    pthread_cond_t m_notEmpty;      //��������ǲ���Ϊ��
};

