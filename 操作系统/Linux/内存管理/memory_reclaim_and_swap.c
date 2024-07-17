#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

// 打印内存使用信息
void print_memory_usage(const char* tag) {
    char command[256];
    sprintf(command, "echo \"%s\" && vmstat -s | grep -E 'total memory|free memory|used memory|swap total|swap free|swap used'", tag);
    system(command);
}

int main() {
    // 打印初始内存使用情况
    print_memory_usage("初始内存使用情况:");

    // 分配大块内存，尽可能接近物理内存限制
    size_t alloc_size = 1024 * 1024 * 100; // 每次分配100MB
    int num_allocs = 20; // 分配次数
    char **memory = (char **)malloc(num_allocs * sizeof(char *));
    if (memory == NULL) {
        perror("内存分配失败");
        return 1;
    }

    for (int i = 0; i < num_allocs; i++) {
        memory[i] = (char *)malloc(alloc_size);
        if (memory[i] == NULL) {
            perror("内存分配失败");
            break;
        }
        // 触发实际内存分配
        for (size_t j = 0; j < alloc_size; j += 4096) {
            memory[i][j] = 'A';
        }
        printf("已分配 %d MB 内存\n", (i + 1) * 100);
        print_memory_usage("当前内存使用情况:");
    }

    // 打印分配完成后的内存使用情况
    print_memory_usage("分配完成后的内存使用情况:");

    // 释放内存
    for (int i = 0; i < num_allocs; i++) {
        if (memory[i] != NULL) {
            free(memory[i]);
        }
    }
    free(memory);

    // 打印释放内存后的内存使用情况
    print_memory_usage("释放内存后的内存使用情况:");

    return 0;
}
