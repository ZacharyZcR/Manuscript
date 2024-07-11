#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// 打印文件内容
void print_file_content(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("打开文件失败");
        return;
    }
    char buffer[256];
    printf("文件内容: ");
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }
    printf("\n");
    fclose(file);
}

int main() {
    const char* filename = "testfile.txt";
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("打开文件失败");
        return 1;
    }

    // 获取文件大小
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("获取文件状态失败");
        close(fd);
        return 1;
    }

    // 将文件映射到内存
    char* map = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("内存映射失败");
        close(fd);
        return 1;
    }

    // 打印映射前的文件内容
    print_file_content(filename);

    // 读取映射的内容
    printf("映射的内容: %s\n", map);

    // 修改映射的内容
    const char* new_content = "这是修改后的内容。";
    size_t new_content_length = strlen(new_content);

    // 确保文件大小足够大以存储新的内容
    if (new_content_length > file_stat.st_size) {
        if (ftruncate(fd, new_content_length) == -1) {
            perror("调整文件大小失败");
            munmap(map, file_stat.st_size);
            close(fd);
            return 1;
        }

        // 重新映射文件以包含新的大小
        munmap(map, file_stat.st_size);
        map = mmap(NULL, new_content_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) {
            perror("重新内存映射失败");
            close(fd);
            return 1;
        }
        file_stat.st_size = new_content_length;
    }

    // 修改内容
    memset(map, 0, file_stat.st_size);  // 将整个映射区清零
    memcpy(map, new_content, new_content_length);
    printf("修改后的映射内容: %s\n", map);

    // 将修改同步到文件
    if (msync(map, file_stat.st_size, MS_SYNC) == -1) {
        perror("同步映射内容到文件失败");
        munmap(map, file_stat.st_size);
        close(fd);
        return 1;
    }

    // 解除内存映射
    if (munmap(map, file_stat.st_size) == -1) {
        perror("解除内存映射失败");
        close(fd);
        return 1;
    }

    close(fd);

    // 打印映射后的文件内容
    print_file_content(filename);

    return 0;
}
