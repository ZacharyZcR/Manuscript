#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

// 线程函数
void* thread_function(void* arg) {
    int num_switches = *(int*)arg;
    for (int i = 0; i < num_switches; ++i) {
        sched_yield(); // 让出CPU，模拟上下文切换
    }
    return NULL;
}

// 获取当前时间（微秒）
unsigned long long get_current_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main() {
    const int num_switches = 100000; // 上下文切换次数
    pthread_t thread1, thread2;
    pid_t pid;
    unsigned long long start_time, end_time;

    printf("上下文切换实验\n");

    // 创建两个线程并测量上下文切换时间
    start_time = get_current_time_us();
    if (pthread_create(&thread1, NULL, thread_function, &num_switches) != 0) {
        perror("创建线程1失败");
        return 1;
    }
    if (pthread_create(&thread2, NULL, thread_function, &num_switches) != 0) {
        perror("创建线程2失败");
        return 1;
    }
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    end_time = get_current_time_us();
    printf("线程上下文切换时间: %llu 微秒\n", end_time - start_time);

    // 创建两个进程并测量上下文切换时间
    start_time = get_current_time_us();
    pid = fork();
    if (pid == 0) {
        // 子进程
        for (int i = 0; i < num_switches; ++i) {
            sched_yield(); // 让出CPU，模拟上下文切换
        }
        exit(0);
    } else if (pid > 0) {
        for (int i = 0; i < num_switches; ++i) {
            sched_yield(); // 让出CPU，模拟上下文切换
        }
        wait(NULL); // 等待子进程结束
        end_time = get_current_time_us();
        printf("进程上下文切换时间: %llu 微秒\n", end_time - start_time);
    } else {
        perror("创建子进程失败");
        return 1;
    }

    return 0;
}