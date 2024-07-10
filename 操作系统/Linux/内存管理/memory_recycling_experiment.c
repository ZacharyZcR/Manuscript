#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 打印内存使用信息
void print_memory_usage(const char* tag) {
    char command[256];
    sprintf(command, "echo \"%s\" && vmstat -s | grep 'free memory'", tag);
    system(command);
}

int main() {
    // 打印初始内存使用情况
    print_memory_usage("初始内存使用情况:");

    // 分配100MB内存
    size_t size = 100 * 1024 * 1024; // 100 MB
    void* memory = malloc(size);
    if (memory == NULL) {
        perror("内存分配失败");
        return 1;
    }
    print_memory_usage("分配100MB内存后:");

    // 使用分配的内存
    for (size_t i = 0; i < size; i += 4096) {
        ((char*)memory)[i] = 'A'; // 每4KB访问一次
    }
    print_memory_usage("使用100MB内存后:");

    // 释放内存
    free(memory);
    print_memory_usage("释放100MB内存后:");

    // 模拟内存泄漏
    for (int i = 0; i < 10; ++i) {
        void* leak_memory = malloc(size);
        if (leak_memory == NULL) {
            perror("内存分配失败");
            return 1;
        }
    }
    print_memory_usage("模拟内存泄漏后:");

    return 0;
}
