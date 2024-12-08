# NT与ZW

1. NTDLL (NT Dynamic Link Library)
- NT 代表 "New Technology"，是Windows NT操作系统家族的缩写
- NTDLL.DLL 是Windows的核心用户模式DLL
- 包含了用户模式到内核模式的转换层

2. NTDDK (NT Driver Development Kit)
- 是Windows驱动开发工具包的缩写
- ntddk.h 是驱动开发的核心头文件
- 包含了驱动开发需要的大部分定义和函数声明

3. NT前缀函数
- 来源于"New Technology"
- 这些函数直接在内核模式下执行
- 例如：NtCreateFile, NtReadFile

4. ZW前缀函数
- "ZW"本身没有特定含义
- Microsoft选择这个前缀只是为了区分
- 这些函数包含了用户模式到内核模式的转换代码
- 例如：ZwCreateFile, ZwReadFile

它们的调用关系是：
```
用户程序 -> NTDLL.DLL (NT函数) -> 系统服务调用 -> 内核(ZW/NT函数)
```

所以：
- NTDLL是用户模式下的库
- NTDDK是内核模式开发工具包
- NT和ZW是同一组函数的不同入口点
- ntddk.h头文件中同时定义了NT和ZW函数

## 主要区别

1. 安全检查
- NT函数：不做参数检查，假定参数已经验证过
- ZW函数：会进行完整的参数验证

2. 调用者模式
- NT函数：根据调用者的模式（用户/内核）确定当前执行线程的访问令牌
- ZW函数：总是使用进程的主令牌，不考虑调用者模式

3. 性能开销
- NT函数：开销较小，直接执行
- ZW函数：开销较大，包含附加的检查代码

## 使用场景

1. NT函数适用场景：
```c
// 1. 在文件系统过滤驱动中处理文件操作
NTSTATUS FileSystemFilter()
{
    // 使用NT函数保持原始调用上下文
    return NtCreateFile(...);
}

// 2. 需要模拟用户模式线程上下文
NTSTATUS SimulateUserContext()
{
    // NT函数会保持原始令牌上下文
    return NtOpenKey(...);
}
```

2. ZW函数适用场景：
```c
// 1. 驱动程序自身需要进行系统操作
NTSTATUS DriverOperation()
{
    // 使用ZW函数确保以驱动身份执行
    return ZwCreateFile(...);
}

// 2. 需要完整的参数验证
NTSTATUS SafeOperation()
{
    // ZW函数会进行参数检查
    return ZwReadFile(...);
}
```

## 具体示例

1. 文件系统过滤驱动：
```c
NTSTATUS FSFilterOperation(PIRP Irp)
{
    // 使用NT函数保持原始访问上下文
    NTSTATUS status = NtCreateFile(
        &FileHandle,
        DesiredAccess,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FileAttributes,
        ShareAccess,
        CreateDisposition,
        CreateOptions,
        NULL,
        0
    );
}
```

2. 驱动程序内部操作：
```c
NTSTATUS DriverInternalOperation()
{
    // 使用ZW函数进行驱动自身的操作
    NTSTATUS status = ZwCreateKey(
        &KeyHandle,
        DesiredAccess,
        &ObjectAttributes,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        NULL
    );
}
```

## 选择建议

1. 一般原则：
- 如果是过滤驱动，处理来自用户模式的请求，用NT函数
- 如果是驱动自身的操作，用ZW函数

2. 特殊考虑：
- 需要参数验证时，选择ZW函数
- 需要保持原始访问令牌时，选择NT函数
- 性能关键场景，优先考虑NT函数

3. 安全考虑：
- 处理不可信数据时，使用ZW函数
- 在确保参数有效性的情况下，可以使用NT函数

记住：选择NT还是ZW主要取决于：
1. 是否需要保持原始调用上下文
2. 是否需要参数验证
3. 操作的安全要求
4. 性能需求

## Win32k
Win32k是Windows操作系统内核中的一个重要组成部分，主要负责：

1. 系统组件
- win32k.sys：内核模式图形驱动
- 处理用户界面相关的内核模式代码
- 管理窗口、菜单、输入等GUI元素

2. 主要功能
```c
// 图形处理
GDI (Graphics Device Interface)
- 绘图
- 字体渲染
- 打印支持

// 窗口管理
Window Manager
- 窗口创建和管理
- 消息处理
- 输入处理
```

## Win32 API
Win32 API是Windows提供给应用程序开发者的编程接口：

1. 主要组成部分：
```c
// 基础服务
kernel32.dll  // 核心系统功能
- 文件操作
- 内存管理
- 进程/线程管理

user32.dll    // 用户界面
- 窗口管理
- 消息处理
- 输入处理

gdi32.dll     // 图形接口
- 绘图函数
- 设备上下文
- 字体管理
```

2. 使用示例：
```c
// 创建窗口
HWND hwnd = CreateWindow(
    className,      // 窗口类名
    windowName,     // 窗口标题
    WS_OVERLAPPED,  // 窗口样式
    CW_USEDEFAULT,  // X坐标
    CW_USEDEFAULT,  // Y坐标
    CW_USEDEFAULT,  // 宽度
    CW_USEDEFAULT,  // 高度
    NULL,           // 父窗口
    NULL,           // 菜单
    hInstance,      // 应用程序实例
    NULL            // 额外参数
);

// 文件操作
HANDLE hFile = CreateFile(
    "test.txt",                   // 文件名
    GENERIC_READ | GENERIC_WRITE, // 访问模式
    0,                           // 共享模式
    NULL,                        // 安全特性
    CREATE_NEW,                  // 创建方式
    FILE_ATTRIBUTE_NORMAL,       // 文件属性
    NULL                         // 模板文件句柄
);
```

3. 调用链关系：
```
应用程序
    ↓
Win32 API (kernel32.dll, user32.dll, gdi32.dll等)
    ↓
NTDLL.DLL (NT函数)
    ↓
系统调用
    ↓
内核服务 (包括Win32k.sys)
```

总结：
1. Win32k是内核中处理GUI相关的组件
2. Win32 API是应用程序使用的编程接口
3. 它们共同构成了Windows的用户界面系统
4. Win32 API通过系统调用最终会使用Win32k的服务