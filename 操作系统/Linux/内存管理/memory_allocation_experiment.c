#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 打印内存分配的信息
void print_memory_info(const char* tag, void* ptr, size_t size) {
    printf("%s - 地址: %p, 大小: %zu 字节\n", tag, ptr, size);
}

int main() {
    // 使用malloc分配内存
    size_t size1 = 1024 * 1024; // 1 MB
    void* ptr1 = malloc(size1);
    if (ptr1 == NULL) {
        perror("malloc分配内存失败");
        return 1;
    }
    print_memory_info("使用malloc分配1MB内存", ptr1, size1);

    // 使用calloc分配并初始化内存
    size_t num_elements = 256;
    size_t element_size = 1024; // 1 KB
    void* ptr2 = calloc(num_elements, element_size);
    if (ptr2 == NULL) {
        perror("calloc分配内存失败");
        free(ptr1); // 释放之前分配的内存
        return 1;
    }
    print_memory_info("使用calloc分配并初始化256个1KB块的内存", ptr2, num_elements * element_size);

    // 使用realloc调整内存大小
    size_t new_size = 2 * size1; // 2 MB
    void* ptr3 = realloc(ptr1, new_size);
    if (ptr3 == NULL) {
        perror("realloc调整内存大小失败");
        free(ptr1);
        free(ptr2);
        return 1;
    }
    print_memory_info("使用realloc调整内存大小到2MB", ptr3, new_size);

    // 释放内存
    free(ptr2);
    free(ptr3);

    printf("内存释放完成\n");
    return 0;
}
