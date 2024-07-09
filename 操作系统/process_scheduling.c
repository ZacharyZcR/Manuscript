#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>

void child_process(int id, int policy, int priority) {
    struct sched_param param;
    param.sched_priority = priority;
    if (sched_setscheduler(0, policy, &param) == -1) {
        perror("设置调度策略失败");
        exit(1);
    }

    printf("子进程 %d (PID: %d) 开始，调度策略: %d，优先级: %d\n", id, getpid(), policy, priority);
    for (int i = 0; i < 5; ++i) {
        printf("子进程 %d 正在运行 (循环 %d)\n", id, i+1);
        sleep(1);  // 模拟计算任务
    }
    printf("子进程 %d 结束\n", id);
    exit(0);
}

int main() {
    pid_t pid1, pid2, pid3;

    // 创建子进程 1
    pid1 = fork();
    if (pid1 == 0) {
        // 子进程 1，使用 SCHED_OTHER 策略
        child_process(1, SCHED_OTHER, 0);
    }

    // 创建子进程 2
    pid2 = fork();
    if (pid2 == 0) {
        // 子进程 2，使用 SCHED_FIFO 策略，优先级 10
        child_process(2, SCHED_FIFO, 10);
    }

    // 创建子进程 3
    pid3 = fork();
    if (pid3 == 0) {
        // 子进程 3，使用 SCHED_RR 策略，优先级 5
        child_process(3, SCHED_RR, 5);
    }

    // 父进程等待所有子进程结束
    for (int i = 0; i < 3; ++i) {
        wait(NULL);
    }

    printf("所有子进程已结束，父进程结束\n");
    return 0;
}
