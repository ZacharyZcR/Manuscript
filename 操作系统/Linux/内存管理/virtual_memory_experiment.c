#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#define MEMORY_SIZE (1024 * 1024 * 100) // 100 MB

void print_memory_usage(pid_t pid) {
    char command[256];
    sprintf(command, "cat /proc/%d/status | grep -i 'VmSize\\|VmRSS'", pid);
    system(command);
}

int main() {
    pid_t pid = getpid();
    printf("进程ID: %d\n", pid);

    // 打印初始内存使用情况
    printf("初始内存使用情况:\n");
    print_memory_usage(pid);

    // 分配100MB内存
    char *memory = (char *)malloc(MEMORY_SIZE);
    if (memory == NULL) {
        perror("内存分配失败");
        return 1;
    }

    // 打印分配内存后的内存使用情况
    printf("分配100MB内存后:\n");
    print_memory_usage(pid);

    // 访问分配的内存以触发页错误并分配物理内存
    for (size_t i = 0; i < MEMORY_SIZE; i += 4096) {
        memory[i] = 'A'; // 每4KB访问一次
    }

    // 打印访问内存后的内存使用情况
    printf("访问内存后:\n");
    print_memory_usage(pid);

    // 释放内存
    free(memory);

    // 打印释放内存后的内存使用情况
    printf("释放内存后:\n");
    print_memory_usage(pid);

    return 0;
}
