#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <string.h>

#define PAGE_SIZE 4096

// 打印内存映射信息
void print_memory_map(pid_t pid) {
    char command[256];
    sprintf(command, "cat /proc/%d/maps", pid);
    printf("进程 %d 的内存映射信息:\n", pid);
    fflush(stdout);
    system(command);
}

// 读取并打印页表信息
void read_pagemap(pid_t pid, unsigned long vaddr) {
    char pagemap_path[64];
    sprintf(pagemap_path, "/proc/%d/pagemap", pid);

    int fd = open(pagemap_path, O_RDONLY);
    if (fd == -1) {
        perror("打开pagemap文件失败");
        exit(EXIT_FAILURE);
    }

    unsigned long offset = (vaddr / PAGE_SIZE) * sizeof(uint64_t);
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("lseek失败");
        close(fd);
        exit(EXIT_FAILURE);
    }

    uint64_t page_info;
    if (read(fd, &page_info, sizeof(uint64_t)) != sizeof(uint64_t)) {
        perror("读取pagemap失败");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    uint64_t frame_num = page_info & ((1ULL << 55) - 1);
    int present = (page_info >> 63) & 1;

    if (present) {
        printf("虚拟地址 0x%lx 映射到物理帧号 %" PRIu64 "\n", vaddr, frame_num);
    } else {
        printf("虚拟地址 0x%lx 未映射到物理内存\n", vaddr);
    }
}

int main() {
    pid_t pid = getpid();
    size_t size = 1024 * 1024 * 10; // 10MB
    char *memory;

    // 分配10MB内存
    memory = (char *)malloc(size);
    if (memory == NULL) {
        perror("内存分配失败");
        return 1;
    }

    // 打印初始内存映射信息
    print_memory_map(pid);

    // 访问分配的内存以触发页表填充
    for (size_t i = 0; i < size; i += PAGE_SIZE) {
        memory[i] = 'A'; // 每4KB访问一次
    }

    // 打印访问内存后的内存映射信息
    print_memory_map(pid);

    // 读取并打印页表信息
    printf("页表信息:\n");
    for (size_t i = 0; i < size; i += PAGE_SIZE) {
        read_pagemap(pid, (unsigned long)(memory + i));
    }

    // 释放内存
    free(memory);

    return 0;
}
