#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 100

void child_process(int pipe_fd[2]) {
    close(pipe_fd[1]);  // 关闭写端

    char buffer[BUFFER_SIZE];
    read(pipe_fd[0], buffer, sizeof(buffer));  // 从管道读取数据
    printf("子进程: 接收到消息: %s\n", buffer);

    close(pipe_fd[0]);  // 关闭读端
    exit(0);
}

int main() {
    int pipe_fd[2];
    pid_t pid;
    char message[] = "你好，子进程！这是父进程的消息。";

    // 创建管道
    if (pipe(pipe_fd) == -1) {
        perror("创建管道失败");
        return 1;
    }

    // 创建子进程
    pid = fork();
    if (pid < 0) {
        perror("创建子进程失败");
        return 1;
    } else if (pid == 0) {
        // 子进程
        child_process(pipe_fd);
    } else {
        // 父进程
        close(pipe_fd[0]);  // 关闭读端

        write(pipe_fd[1], message, strlen(message) + 1);  // 向管道写入数据
        printf("父进程: 发送消息: %s\n", message);

        close(pipe_fd[1]);  // 关闭写端

        // 等待子进程结束
        wait(NULL);
        printf("父进程: 子进程已结束\n");
    }

    return 0;
}
