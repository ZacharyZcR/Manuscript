#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALLOCATIONS 10000
#define OBJECT_SIZE 64 // 64 字节的小对象

int main() {
    void* allocations[ALLOCATIONS];
    srand(time(NULL));

    // 分配小对象
    for (int i = 0; i < ALLOCATIONS; i++) {
        allocations[i] = malloc(OBJECT_SIZE);
        if (allocations[i] == NULL) {
            perror("内存分配失败");
            return 1;
        }
        printf("分配小对象：大小 = %d 字节\n", OBJECT_SIZE);
    }

    // 随机释放部分小对象
    for (int i = 0; i < ALLOCATIONS / 2; i++) {
        int idx = rand() % ALLOCATIONS;
        if (allocations[idx] != NULL) {
            free(allocations[idx]);
            printf("释放小对象：大小 = %d 字节\n", OBJECT_SIZE);
            allocations[idx] = NULL;
        }
    }

    // 释放剩余的小对象
    for (int i = 0; i < ALLOCATIONS; i++) {
        if (allocations[i] != NULL) {
            free(allocations[i]);
        }
    }

    printf("小对象内存分配和释放实验完成\n");
    return 0;
}
