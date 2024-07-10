为了深入理解Linux中的优先级与Nice值，我们可以设计一个实验，通过编写和运行C程序来观察不同Nice值对进程调度的影响。我们将创建多个进程，分别设置不同的Nice值，并通过运行计算密集型任务来观察它们的执行情况。

### 实验步骤

#### 1. 实验准备
确保系统上安装了GCC编译器，可以通过以下命令安装：
```bash
sudo apt-get update
sudo apt-get install gcc
```

#### 2. 编写C程序
编写一个C程序，创建多个子进程，并设置不同的Nice值，观察它们的执行情况。

**文件名：nice_experiment.c**

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

// 计算密集型任务
void compute_task(int id) {
    unsigned long long counter = 0;
    for (unsigned long long i = 0; i < 1000000000ULL; ++i) {
        counter += i;
    }
    printf("进程 %d (PID: %d) 计算完成\n", id, getpid());
}

int main() {
    pid_t pid1, pid2, pid3;

    // 创建第一个子进程，设置Nice值为-10
    pid1 = fork();
    if (pid1 == 0) {
        if (nice(-10) == -1) {
            perror("设置Nice值失败");
        }
        printf("进程 1 (PID: %d) Nice值: %d\n", getpid(), getpriority(PRIO_PROCESS, 0));
        compute_task(1);
        exit(0);
    }

    // 创建第二个子进程，设置Nice值为0
    pid2 = fork();
    if (pid2 == 0) {
        printf("进程 2 (PID: %d) Nice值: %d\n", getpid(), getpriority(PRIO_PROCESS, 0));
        compute_task(2);
        exit(0);
    }

    // 创建第三个子进程，设置Nice值为10
    pid3 = fork();
    if (pid3 == 0) {
        if (nice(10) == -1) {
            perror("设置Nice值失败");
        }
        printf("进程 3 (PID: %d) Nice值: %d\n", getpid(), getpriority(PRIO_PROCESS, 0));
        compute_task(3);
        exit(0);
    }

    // 父进程等待所有子进程结束
    for (int i = 0; i < 3; ++i) {
        wait(NULL);
    }

    printf("所有子进程已结束\n");
    return 0;
}
```

#### 3. 编译和运行程序
编译上述C程序并运行：
```bash
gcc nice_experiment.c -o nice_experiment
./nice_experiment
```

#### 4. 观察输出
程序运行后，会打印出每个子进程的Nice值和计算完成的信息。输出示例如下：

```
进程 1 (PID: 1235) Nice值: -10
进程 2 (PID: 1236) Nice值: 0
进程 3 (PID: 1237) Nice值: 10
进程 1 (PID: 1235) 计算完成
进程 2 (PID: 1236) 计算完成
进程 3 (PID: 1237) 计算完成
所有子进程已结束
```

### 预期结果

通过上述实验，我们可以观察到以下几点：
1. **Nice值设置**：不同的进程可以设置不同的Nice值，通过`nice()`系统调用实现。Nice值范围从-20（最高优先级）到19（最低优先级）。
2. **优先级影响**：优先级较高的进程（Nice值低）应比优先级较低的进程（Nice值高）更快完成计算密集型任务。在实际运行中，具有更高优先级的进程将会得到更多的CPU时间片。
3. **输出顺序**：根据优先级的不同，计算完成的顺序可能会有所不同。通常情况下，Nice值为-10的进程应该最先完成，Nice值为0的进程其次，Nice值为10的进程最后完成。

### 实验总结

通过这个实验，我们深入了解了Linux中的优先级与Nice值的关系及其对进程调度的影响。我们通过设置不同的Nice值，观察到进程执行顺序的变化。这些知识对于理解Linux调度机制和优化系统性能非常有帮助。