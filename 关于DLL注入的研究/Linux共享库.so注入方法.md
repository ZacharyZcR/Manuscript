# Linux共享库.so注入方法

Linux 中的共享库文件（.so 文件）也有类似于 Windows 中 DLL 注入的技术。这些技术可以用来将代码注入到运行中的进程中。以下是几种常见的 Linux 下的共享库注入方法：

### 1. LD_PRELOAD

**LD_PRELOAD** 是一个环境变量，可以在运行程序时指定一个共享库，这个共享库会在程序启动时被加载，并覆盖原有库中的符号。

```bash
LD_PRELOAD=/path/to/malicious.so /path/to/victim_program
```

- **优点**：实现简单，不需要修改目标程序。
- **缺点**：需要在程序启动时设置，无法对已运行的进程进行注入。

### 2. ptrace

`ptrace` 是一个系统调用，常用于调试器（如 gdb）来观察和控制进程的执行。可以利用 `ptrace` 将共享库注入到目标进程。

```c
ptrace(PTRACE_ATTACH, pid, NULL, NULL);
ptrace(PTRACE_POKETEXT, pid, remote_address, value);
ptrace(PTRACE_DETACH, pid, NULL, NULL);
```

- **优点**：可以注入到已运行的进程。
- **缺点**：需要较高的权限（通常是 root），实现复杂。

### 3. 直接修改 /proc/[pid]/mem

可以直接修改 `/proc/[pid]/mem` 来注入代码到目标进程。通过查找内存地址并写入恶意代码。

```c
int fd = open("/proc/[pid]/mem", O_RDWR);
lseek(fd, address, SEEK_SET);
write(fd, payload, sizeof(payload));
close(fd);
```

- **优点**：可以注入到已运行的进程。
- **缺点**：需要 root 权限，实现复杂且容易导致系统不稳定。

### 4. 使用 gdb 调试器

利用 gdb 调试器来注入共享库文件。

```bash
gdb -p [pid]
(gdb) call dlopen("/path/to/malicious.so", RTLD_LAZY)
```

- **优点**：实现相对简单。
- **缺点**：需要较高的权限，容易被检测到。

### 5. 使用动态链接器（ld.so）

在启动目标程序时，通过 `ld.so` 注入共享库。

```bash
ld.so --library-path /path/to/malicious.so /path/to/victim_program
```

- **优点**：实现简单。
- **缺点**：需要在程序启动时设置，无法对已运行的进程进行注入。

### 6. 注入自定义 ELF 加载器

使用自定义的 ELF 加载器来加载目标程序，并在加载过程中注入共享库。

```c
// 自定义 ELF 加载器代码
```

- **优点**：高度灵活。
- **缺点**：实现非常复杂，需要深入了解 ELF 格式。

### 防范措施

1. **限制 ptrace 使用**：通过修改内核参数限制 `ptrace` 的使用。
2. **监控 LD_PRELOAD**：对环境变量进行监控，防止恶意注入。
3. **权限管理**：限制用户权限，防止未经授权的访问和修改。
4. **进程隔离**：使用容器化技术（如 Docker）隔离进程，限制其对系统的影响。

与 Windows 的 DLL 注入类似，Linux 的共享库注入技术也可以用来进行调试、性能分析以及恶意攻击等用途。理解这些技术并采取相应的防范措施对于保护系统安全至关重要。