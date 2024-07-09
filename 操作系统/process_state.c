#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    // 创建子进程
    pid = fork();

    if (pid < 0) {
        // fork失败
        fprintf(stderr, "创建进程失败\n");
        return 1;
    } else if (pid == 0) {
        // 子进程
        printf("子进程: \n");
        printf("进程ID: %d\n", getpid());
        printf("父进程ID: %d\n", getppid());

        // 使子进程进入睡眠状态
        printf("子进程进入睡眠状态10秒...\n");
        sleep(10);

        // 子进程结束，进入僵尸状态
        printf("子进程结束，进入僵尸状态...\n");
        exit(0);
    } else {
        // 父进程
        printf("父进程: \n");
        printf("进程ID: %d\n", getpid());
        printf("父进程ID: %d\n", getppid());

        // 父进程等待5秒，然后查看子进程状态
        sleep(5);
        printf("父进程等待5秒后查看子进程状态:\n");
        char command[50];
        sprintf(command, "ps -o pid,ppid,state,cmd | grep %d", pid);
        system(command);

        // 等待子进程结束，进入僵尸状态
        sleep(7); // 等待子进程进入僵尸状态
        printf("查看子进程进入僵尸状态:\n");
        system(command);

        // 父进程回收子进程
        wait(NULL);
        printf("父进程回收子进程后查看状态:\n");
        system(command);

        printf("子进程已结束，父进程结束\n");
    }

    return 0;
}
