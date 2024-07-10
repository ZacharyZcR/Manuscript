为了深入理解Linux中的进程控制块（PCB），我们可以设计一个实验，通过编写和运行C程序来观察和打印进程的一些属性，这些属性通常由PCB管理。尽管我们无法直接访问内核中的PCB数据结构，但我们可以使用系统调用和库函数获取PCB中的一些重要信息，如进程ID、父进程ID、进程组ID、会话ID、优先级等。

### 实验步骤

#### 1. 实验准备
确保系统上安装了GCC编译器，可以通过以下命令安装：
```bash
sudo apt-get update
sudo apt-get install gcc
```

#### 2. 编写C程序
编写一个C程序，创建子进程，并获取和打印父进程和子进程的各种信息。

**文件名：process_control_block.c**

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

void print_process_info(const char* process_name) {
    pid_t pid = getpid();        // 获取进程ID
    pid_t ppid = getppid();      // 获取父进程ID
    pid_t pgid = getpgid(0);     // 获取进程组ID
    pid_t sid = getsid(0);       // 获取会话ID
    int priority = getpriority(PRIO_PROCESS, 0); // 获取进程优先级

    printf("%s: \n", process_name);
    printf("进程ID: %d\n", pid);
    printf("父进程ID: %d\n", ppid);
    printf("进程组ID: %d\n", pgid);
    printf("会话ID: %d\n", sid);
    printf("优先级: %d\n", priority);
    printf("-------------------------\n");
}

int main() {
    pid_t pid;

    printf("父进程信息:\n");
    print_process_info("父进程");

    // 创建子进程
    pid = fork();

    if (pid < 0) {
        // fork失败
        perror("创建子进程失败");
        return 1;
    } else if (pid == 0) {
        // 子进程
        printf("子进程信息:\n");
        print_process_info("子进程");
        exit(0);
    } else {
        // 父进程等待子进程结束
        wait(NULL);
        printf("父进程: 子进程已结束\n");
    }

    return 0;
}
```

#### 3. 编译和运行程序
编译上述C程序并运行：
```bash
gcc process_control_block.c -o process_control_block
./process_control_block
```

#### 4. 观察输出
程序运行后，会打印出父进程和子进程的各种信息。输出示例如下：

```
父进程信息:
父进程: 
进程ID: 1234
父进程ID: 5678
进程组ID: 1234
会话ID: 1234
优先级: 0
-------------------------
子进程信息:
子进程: 
进程ID: 1235
父进程ID: 1234
进程组ID: 1234
会话ID: 1234
优先级: 0
-------------------------
父进程: 子进程已结束
```

### 预期结果

通过上述实验，我们可以观察到以下几点：
1. **进程ID**：每个进程都有唯一的进程ID。
2. **父进程ID**：子进程的父进程ID等于父进程的进程ID。
3. **进程组ID**：通常，子进程的进程组ID与父进程相同。
4. **会话ID**：通常，子进程的会话ID与父进程相同。
5. **优先级**：默认情况下，父进程和子进程的优先级相同，可以通过`getpriority`函数获取。

### 实验总结

通过这个实验，我们深入了解了Linux中的进程控制块（PCB）所包含的一些重要信息。尽管我们无法直接访问内核中的PCB结构体，但通过系统调用和库函数，我们可以获取到许多由PCB管理的进程属性。这些知识对于理解Linux的进程管理机制非常有帮助。