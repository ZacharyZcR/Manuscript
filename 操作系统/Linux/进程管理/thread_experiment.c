#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

// 线程函数
void* thread_function(void* arg) {
    pid_t pid = getpid();          // 获取进程ID
    pthread_t tid = pthread_self(); // 获取线程ID

    printf("线程 %d: 进程ID: %d, 线程ID: %lu\n", *(int*)arg, pid, tid);
    sleep(2);  // 模拟线程工作
    printf("线程 %d: 完成工作\n", *(int*)arg);
    return NULL;
}

int main() {
    pthread_t threads[3];
    int thread_ids[3];

    printf("主线程: 进程ID: %d, 线程ID: %lu\n", getpid(), pthread_self());

    // 创建三个线程
    for (int i = 0; i < 3; ++i) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]) != 0) {
            perror("创建线程失败");
            return 1;
        }
    }

    // 等待所有线程结束
    for (int i = 0; i < 3; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("所有线程已结束\n");
    return 0;
}
