#pragma once
#include "TaskQueue.h"

class ThreadPool
{
public:
    //初始化线程池
    ThreadPool(int min,int max);//最小线程，最大线程

    //销毁线程池
    ~ThreadPool();
    
    //给线程池添加任务
    void addTask(Task task);

    //获取忙的线程个数 
    int getBusyNum();

    //获取活着的线程个数
    int getAliveNum();

private:
    /////////////////////////////////////
    //工作的线程的任务函数
    static void* worker(void* arg);

    // 管理者线程的任务函数
    static void* manager(void* arg);

    //单个线程退出
    void threadExit();
     
private:
	TaskQueue* m_taskQ;               //任务队列
    pthread_t* m_threadIDs;         //工作的线程ID
    pthread_t m_managerID;          //管理者线程ID
    int m_minNum;                   //最小线程数
    int m_maxNum;                   //最大线程数
    int m_busyNum;                  //正在工作的线程数
    int m_aliveNum;                 //活着的线程数
    int m_exitNum;                  //要销毁的线程数
    bool m_shutdown = false;        //线程池是否关闭 false：开启 true：关闭 
    pthread_mutex_t m_lock;         //整个线程池锁
    pthread_cond_t m_notEmpty;      //任务队列是不是为空
};

