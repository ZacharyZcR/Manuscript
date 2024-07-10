#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void create_child(int id) {
    pid_t pid = fork();

    if (pid < 0) {
        // fork失败
        fprintf(stderr, "创建子进程 %d 失败\n", id);
        exit(1);
    } else if (pid == 0) {
        // 子进程
        printf("子进程 %d: 进程ID: %d, 父进程ID: %d\n", id, getpid(), getppid());
        printf("子进程 %d 开始工作...\n", id);
        sleep(id * 2);  // 模拟不同的工作时间
        printf("子进程 %d 工作完成，退出\n", id);
        exit(0);
    }
}

int main() {
    printf("父进程: 进程ID: %d\n", getpid());

    // 创建三个子进程
    for (int i = 1; i <= 3; ++i) {
        create_child(i);
    }

    // 父进程等待子进程结束
    for (int i = 1; i <= 3; ++i) {
        pid_t pid = wait(NULL);
        printf("父进程: 子进程 %d 已结束\n", pid);
    }

    printf("所有子进程已结束，父进程结束\n");
    return 0;
}
