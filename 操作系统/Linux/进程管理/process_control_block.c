#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

void print_process_info(const char* process_name) {
    pid_t pid = getpid();        // 获取进程ID
    pid_t ppid = getppid();      // 获取父进程ID
    pid_t pgid = getpgid(0);     // 获取进程组ID
    pid_t sid = getsid(0);       // 获取会话ID
    int priority = getpriority(PRIO_PROCESS, 0); // 获取进程优先级

    printf("%s: \n", process_name);
    printf("进程ID: %d\n", pid);
    printf("父进程ID: %d\n", ppid);
    printf("进程组ID: %d\n", pgid);
    printf("会话ID: %d\n", sid);
    printf("优先级: %d\n", priority);
    printf("-------------------------\n");
}

int main() {
    pid_t pid;

    printf("父进程信息:\n");
    print_process_info("父进程");

    // 创建子进程
    pid = fork();

    if (pid < 0) {
        // fork失败
        perror("创建子进程失败");
        return 1;
    } else if (pid == 0) {
        // 子进程
        printf("子进程信息:\n");
        print_process_info("子进程");
        exit(0);
    } else {
        // 父进程等待子进程结束
        wait(NULL);
        printf("父进程: 子进程已结束\n");
    }

    return 0;
}
