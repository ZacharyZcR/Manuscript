#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALLOCATIONS 1000
#define MAX_SIZE 1024 * 1024 // 1 MB

int main() {
    void* allocations[ALLOCATIONS];
    size_t sizes[ALLOCATIONS];
    srand(time(NULL));

    // 分配不同大小的内存块
    for (int i = 0; i < ALLOCATIONS; i++) {
        sizes[i] = rand() % MAX_SIZE + 1;
        allocations[i] = malloc(sizes[i]);
        if (allocations[i] == NULL) {
            perror("内存分配失败");
            return 1;
        }
        printf("分配内存块：大小 = %zu 字节\n", sizes[i]);
    }

    // 随机释放部分内存块
    for (int i = 0; i < ALLOCATIONS / 2; i++) {
        int idx = rand() % ALLOCATIONS;
        if (allocations[idx] != NULL) {
            free(allocations[idx]);
            printf("释放内存块：大小 = %zu 字节\n", sizes[idx]);
            allocations[idx] = NULL;
        }
    }

    // 释放剩余的内存块
    for (int i = 0; i < ALLOCATIONS; i++) {
        if (allocations[i] != NULL) {
            free(allocations[i]);
        }
    }

    printf("内存分配和释放实验完成\n");
    return 0;
}