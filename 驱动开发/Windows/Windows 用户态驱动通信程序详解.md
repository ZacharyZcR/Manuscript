# Windows 用户态驱动通信程序详解

## 头文件引入部分
```cpp
#include <windows.h>  // Windows API的核心头文件
#include <stdio.h>    // 标准输入输出函数头文件
```

## 宏定义部分
```cpp
// 设备符号链接名称，必须与驱动端的符号链接名称匹配
#define DEVICE_NAME L"\\\\.\\MyTestDevice"  

// 设备控制码，必须与驱动端定义的控制码完全一致
#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
```

## 主函数详解
```cpp
int main()
{
    // 输出程序开始执行的信息
    printf("Opening device...\n");
    
    // 打开设备，获取设备句柄
    HANDLE hDevice = CreateFileW(
        DEVICE_NAME,              // 设备名称
        GENERIC_READ | GENERIC_WRITE,  // 请求读写权限
        0,                        // 不共享设备
        NULL,                     // 默认安全属性
        OPEN_EXISTING,           // 打开存在的设备
        FILE_ATTRIBUTE_NORMAL,   // 普通文件属性
        NULL                     // 没有模板文件
    );

    // 检查设备是否成功打开
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open device: %d\n", GetLastError());
        return 1;
    }

    printf("Device opened successfully\n");

    // 与驱动通信
    DWORD bytesReturned;  // 用于存储操作返回的字节数
    BOOL success = DeviceIoControl(
        hDevice,           // 设备句柄
        IOCTL_TEST,       // 控制码
        NULL,             // 输入缓冲区
        0,                // 输入缓冲区大小
        NULL,             // 输出缓冲区
        0,                // 输出缓冲区大小
        &bytesReturned,   // 返回的字节数
        NULL              // 不使用重叠IO
    );

    // 检查通信是否成功
    if (success)
    {
        printf("Successfully sent message to driver\n");
    }
    else
    {
        printf("Failed to send message: %d\n", GetLastError());
    }

    // 关闭设备句柄
    CloseHandle(hDevice);
    printf("Device closed\n");
    return 0;
}
```

## 关键函数说明

### 1. CreateFileW
```cpp
HANDLE CreateFileW(
    LPCWSTR lpFileName,            // 文件/设备名称
    DWORD dwDesiredAccess,         // 访问模式
    DWORD dwShareMode,             // 共享模式
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  // 安全属性
    DWORD dwCreationDisposition,    // 创建方式
    DWORD dwFlagsAndAttributes,     // 文件属性
    HANDLE hTemplateFile            // 模板文件句柄
);
```
- 用于打开或创建文件/设备
- 返回设备句柄，用于后续通信
- 失败返回INVALID_HANDLE_VALUE

### 2. DeviceIoControl
```cpp
BOOL DeviceIoControl(
    HANDLE hDevice,                // 设备句柄
    DWORD dwIoControlCode,         // 控制码
    LPVOID lpInBuffer,            // 输入缓冲区
    DWORD nInBufferSize,          // 输入缓冲区大小
    LPVOID lpOutBuffer,           // 输出缓冲区
    DWORD nOutBufferSize,         // 输出缓冲区大小
    LPDWORD lpBytesReturned,      // 返回字节数
    LPOVERLAPPED lpOverlapped     // 重叠结构
);
```
- 用于与驱动程序通信
- 可以发送和接收数据
- 通过控制码区分不同操作

### 3. CloseHandle
```cpp
BOOL CloseHandle(
    HANDLE hObject   // 要关闭的句柄
);
```
- 关闭设备句柄
- 释放系统资源

## 错误处理
```cpp
GetLastError()  // 获取最后的错误码
```
- 当操作失败时获取具体错误原因
- 常见错误码：
  - ERROR_FILE_NOT_FOUND：设备未找到
  - ERROR_ACCESS_DENIED：访问被拒绝
  - ERROR_INVALID_HANDLE：无效句柄

## 程序扩展建议

1. 数据传输功能
```cpp
// 定义数据结构
typedef struct _DEVICE_DATA {
    ULONG Command;
    UCHAR Data[256];
} DEVICE_DATA, *PDEVICE_DATA;

// 发送数据
DEVICE_DATA inputData = {0};
DeviceIoControl(hDevice, 
    IOCTL_TEST, 
    &inputData, 
    sizeof(DEVICE_DATA),
    NULL, 
    0, 
    &bytesReturned, 
    NULL);
```

2. 异步通信
```cpp
// 使用重叠IO
OVERLAPPED overlapped = {0};
DeviceIoControl(hDevice,
    IOCTL_TEST,
    NULL,
    0,
    NULL,
    0,
    &bytesReturned,
    &overlapped);
```

3. 错误处理优化
```cpp
void PrintError(const char* operation) {
    DWORD error = GetLastError();
    char buffer[256];
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        error,
        0,
        buffer,
        sizeof(buffer),
        NULL
    );
    printf("%s failed: %s (Error: %d)\n", operation, buffer, error);
}
```

这个程序提供了基本的用户态驱动通信框架，可以根据具体需求进行扩展：
1. 添加更多控制码支持不同操作
2. 实现数据传输功能
3. 添加异步通信支持
4. 增强错误处理和日志记录
5. 添加安全检查机制