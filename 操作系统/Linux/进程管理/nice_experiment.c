#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

// 计算密集型任务
void compute_task(int id) {
    unsigned long long counter = 0;
    for (unsigned long long i = 0; i < 1000000000ULL; ++i) {
        counter += i;
    }
    printf("进程 %d (PID: %d) 计算完成\n", id, getpid());
}

int main() {
    pid_t pid1, pid2, pid3;

    // 创建第一个子进程，设置Nice值为-10
    pid1 = fork();
    if (pid1 == 0) {
        if (nice(-10) == -1) {
            perror("设置Nice值失败");
        }
        printf("进程 1 (PID: %d) Nice值: %d\n", getpid(), getpriority(PRIO_PROCESS, 0));
        compute_task(1);
        exit(0);
    }

    // 创建第二个子进程，设置Nice值为0
    pid2 = fork();
    if (pid2 == 0) {
        printf("进程 2 (PID: %d) Nice值: %d\n", getpid(), getpriority(PRIO_PROCESS, 0));
        compute_task(2);
        exit(0);
    }

    // 创建第三个子进程，设置Nice值为10
    pid3 = fork();
    if (pid3 == 0) {
        if (nice(10) == -1) {
            perror("设置Nice值失败");
        }
        printf("进程 3 (PID: %d) Nice值: %d\n", getpid(), getpriority(PRIO_PROCESS, 0));
        compute_task(3);
        exit(0);
    }

    // 父进程等待所有子进程结束
    for (int i = 0; i < 3; ++i) {
        wait(NULL);
    }

    printf("所有子进程已结束\n");
    return 0;
}
