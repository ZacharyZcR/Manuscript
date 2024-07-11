#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf env;

void signal_handler(int sig) {
    if (sig == SIGSEGV) {
        printf("捕获到信号SIGSEGV: 内存访问违规\n");
        longjmp(env, 1);
    }
}

// 打印内存保护信息
void print_memory_protection_info(const char* tag, void* ptr, size_t size) {
    printf("%s - 地址: %p, 大小: %zu 字节\n", tag, ptr, size);
}

int main() {
    size_t size = 4096; // 4KB内存页大小
    void* memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("内存映射失败");
        return 1;
    }
    print_memory_protection_info("分配并映射4KB内存", memory, size);

    // 注册信号处理函数
    signal(SIGSEGV, signal_handler);

    // 设置内存保护为只读
    if (mprotect(memory, size, PROT_READ) == -1) {
        perror("设置内存保护为只读失败");
        munmap(memory, size);
        return 1;
    }
    printf("设置内存保护为只读\n");

    if (setjmp(env) == 0) {
        // 尝试写入只读内存，预期会导致段错误
        printf("尝试写入只读内存\n");
        ((char*)memory)[0] = 'A';
        printf("写入只读内存成功（意外情况）\n");
    } else {
        printf("写入只读内存失败\n");
    }

    // 设置内存保护为读写
    if (mprotect(memory, size, PROT_READ | PROT_WRITE) == -1) {
        perror("设置内存保护为读写失败");
        munmap(memory, size);
        return 1;
    }
    printf("设置内存保护为读写\n");

    // 尝试写入读写内存
    ((char*)memory)[0] = 'A';
    printf("写入读写内存成功\n");

    // 释放内存
    if (munmap(memory, size) == -1) {
        perror("解除内存映射失败");
        return 1;
    }
    printf("内存释放完成\n");

    return 0;
}
