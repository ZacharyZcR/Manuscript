#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void access_memory(char *memory, size_t size, const char *label) {
    printf("访问 %s: 地址 = %p, 大小 = %zu\n", label, memory, size);
    for (size_t i = 0; i < size; ++i) {
        memory[i] = 'A';
    }
    printf("访问 %s 成功\n", label);
}

int main() {
    size_t code_size = 4096; // 代码段大小
    size_t data_size = 4096; // 数据段大小
    size_t stack_size = 4096; // 栈段大小

    // 分配内存用于模拟不同的段
    char *code_segment = mmap(NULL, code_size, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    char *data_segment = mmap(NULL, data_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    char *stack_segment = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (code_segment == MAP_FAILED || data_segment == MAP_FAILED || stack_segment == MAP_FAILED) {
        perror("内存映射失败");
        return 1;
    }

    // 注册信号处理函数
    signal(SIGSEGV, signal_handler);

    // 访问代码段（只读执行）
    if (setjmp(env) == 0) {
        printf("尝试写入代码段\n");
        code_segment[0] = 'A';
        printf("写入代码段成功（意外情况）\n");
    } else {
        printf("写入代码段失败\n");
    }

    // 访问数据段（读写）
    if (setjmp(env) == 0) {
        access_memory(data_segment, data_size, "数据段");
    } else {
        printf("访问数据段失败\n");
    }

    // 访问栈段（读写）
    if (setjmp(env) == 0) {
        access_memory(stack_segment, stack_size, "栈段");
    } else {
        printf("访问栈段失败\n");
    }

    // 解除内存映射
    munmap(code_segment, code_size);
    munmap(data_segment, data_size);
    munmap(stack_segment, stack_size);

    printf("内存释放完成\n");

    return 0;
}