#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

// 共享内存大小
#define SHM_SIZE 4096

// 信号处理函数，用于同步
void signal_handler(int sig) {
    // 处理信号，不做任何事，只是用于同步
}

int main() {
    const char* shm_name = "/my_shared_memory";
    int fd;
    char* shared_mem;

    // 创建共享内存对象
    fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("创建共享内存对象失败");
        return 1;
    }

    // 配置共享内存大小
    if (ftruncate(fd, SHM_SIZE) == -1) {
        perror("配置共享内存大小失败");
        shm_unlink(shm_name);
        return 1;
    }

    // 映射共享内存
    shared_mem = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("映射共享内存失败");
        shm_unlink(shm_name);
        return 1;
    }

    // 注册信号处理函数
    signal(SIGUSR1, signal_handler);

    pid_t pid = fork();
    if (pid == -1) {
        perror("创建子进程失败");
        shm_unlink(shm_name);
        return 1;
    }

    if (pid == 0) {
        // 子进程1：写入共享内存
        const char* message = "这是共享内存中的数据。";
        printf("子进程1写入: %s\n", message);
        strncpy(shared_mem, message, SHM_SIZE);

        // 向父进程发送信号
        kill(getppid(), SIGUSR1);

        // 解除共享内存映射
        munmap(shared_mem, SHM_SIZE);
        exit(0);
    } else {
        // 父进程等待子进程1写入完成
        pause();

        pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("创建子进程失败");
            shm_unlink(shm_name);
            return 1;
        }

        if (pid2 == 0) {
            // 子进程2：读取共享内存
            printf("子进程2读取: %s\n", shared_mem);

            // 解除共享内存映射
            munmap(shared_mem, SHM_SIZE);
            exit(0);
        } else {
            // 父进程等待所有子进程结束
            wait(NULL);
            wait(NULL);

            // 解除共享内存映射并删除共享内存对象
            munmap(shared_mem, SHM_SIZE);
            shm_unlink(shm_name);
        }
    }

    return 0;
}
