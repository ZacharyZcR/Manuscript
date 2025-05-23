# 常用与不常用的DLL注入方法

DLL注入有许多方法，包括常用和不常用的技术。以下是几种常用和不常用的DLL注入方法：

### 常用的DLL注入方法

1. **CreateRemoteThread 方法**：
   - 使用 Windows API 函数 `CreateRemoteThread` 在目标进程中创建一个新线程，并调用 `LoadLibrary` 加载恶意DLL。
   - 优点：实现简单，效果可靠。
   - 缺点：容易被检测到。
2. **APC 注入（Asynchronous Procedure Call Injection）**：
   - 将 `LoadLibrary` 函数排队到目标进程的 APC 队列中，当目标线程进入可警报状态时，APC 会调用 `LoadLibrary` 加载恶意DLL。
   - 优点：隐蔽性较好。
   - 缺点：依赖目标线程的状态，复杂度较高。
3. **DLL 劫持**：
   - 将恶意DLL放置在应用程序搜索路径中的某个位置，使得应用程序优先加载恶意DLL而不是合法DLL。
   - 优点：实现简单。
   - 缺点：需要特定的目录访问权限。
4. **劫持IAT（Import Address Table Hijacking）**：
   - 修改目标进程的导入地址表，使其指向恶意DLL中的函数。
   - 优点：高效。
   - 缺点：需要深入了解目标进程的内存结构。

### 不常用的DLL注入方法

1. **PE（Portable Executable）注入**：
   - 手动修改目标进程的PE结构，将恶意DLL注入其中。
   - 优点：灵活性高。
   - 缺点：实现复杂，需要深入了解PE文件格式。
2. **线程劫持**：
   - 暂停目标进程中的现有线程，修改其上下文以执行恶意DLL加载代码，然后恢复线程执行。
   - 优点：隐蔽性好。
   - 缺点：实现复杂，对目标进程的稳定性影响较大。
3. **截获 API 调用（API Hooking）**：
   - 拦截目标进程中的特定API调用（如 `LoadLibrary`），在调用时加载恶意DLL。
   - 优点：隐蔽性好。
   - 缺点：实现复杂，容易引发兼容性问题。
4. **内核模式注入（Kernel-Mode Injection）**：
   - 利用内核模式代码（如驱动程序）直接注入DLL到目标进程。
   - 优点：强大的控制能力。
   - 缺点：需要管理员权限，容易导致系统不稳定。
5. **反射式DLL注入（Reflective DLL Injection）**：
   - 使用自定义加载器将DLL注入到目标进程内存中，而不通过常规的 `LoadLibrary` 函数。
   - 优点：隐蔽性极好，不需要写入目标进程的磁盘。
   - 缺点：实现复杂，需要自定义加载器。

这些方法各有优缺点，选择哪种方法取决于具体的需求、目标进程的环境和安全性要求。在实际应用中，应尽量避免使用DLL注入等不安全的技术，维护系统的整体安全性。