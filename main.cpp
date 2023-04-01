#include "ThreadPool.h"
#include<unistd.h>
#include<iostream>


void taskFun(void* arg)
{
    int num = *static_cast<int*>(arg);
    std::cout << "thread" <<pthread_self()<<" is working, num = " << num << std::endl;
    sleep(1);
    return;
}

int main()
{
    //创建线程池
    ThreadPool pool(3, 10);
    for (int i = 0; i < 10; i++)
    {
        int* num = new int(i+100);
        pool.addTask(Task(taskFun, num));
    }

    sleep(20);

    return 0;
}