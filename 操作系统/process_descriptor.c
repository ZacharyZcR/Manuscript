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
        printf("组ID: %d\n", getpgid(0));
        printf("会话ID: %d\n", getsid(0));

        // 模拟子进程工作
        sleep(2);
        printf("子进程结束\n");
        exit(0);
    } else {
        // 父进程
        printf("父进程: \n");
        printf("进程ID: %d\n", getpid());
        printf("父进程ID: %d\n", getppid());
        printf("组ID: %d\n", getpgid(0));
        printf("会话ID: %d\n", getsid(0));

        // 等待子进程结束
        wait(NULL);
        printf("子进程已结束，父进程结束\n");
    }

    return 0;
}
