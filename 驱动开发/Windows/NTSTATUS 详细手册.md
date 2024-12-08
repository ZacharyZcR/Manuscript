# NTSTATUS 详细手册

## 一、基本概念

NTSTATUS 是Windows驱动开发中用于表示操作结果的32位值。其结构如下：

```c
typedef long NTSTATUS;

// 结构分解：
// [31]     - 严重性位
// [30-29]  - 保留位
// [28-16]  - 设备码
// [15-0]   - 错误码
```

## 二、常见状态码分类

### 1. 成功状态码
```c
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // 操作成功
#define STATUS_PENDING                   ((NTSTATUS)0x00000103L)    // 操作挂起
#define STATUS_REPARSE                   ((NTSTATUS)0x00000104L)    // 重解析
```

### 2. 信息状态码
```c
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)    // 缓冲区溢出
#define STATUS_NO_MORE_FILES            ((NTSTATUS)0x80000006L)    // 没有更多文件
```

### 3. 警告状态码
```c
#define STATUS_WARNING                   ((NTSTATUS)0x80000000L)    // 一般警告
#define STATUS_OBJECT_NAME_EXISTS       ((NTSTATUS)0x40000000L)    // 对象名已存在
```

### 4. 错误状态码
```c
// 常见错误
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)    // 一般错误
#define STATUS_NOT_IMPLEMENTED         ((NTSTATUS)0xC0000002L)    // 未实现
#define STATUS_INVALID_PARAMETER       ((NTSTATUS)0xC000000DL)    // 无效参数
#define STATUS_ACCESS_DENIED           ((NTSTATUS)0xC0000022L)    // 访问拒绝
#define STATUS_BUFFER_TOO_SMALL       ((NTSTATUS)0xC0000023L)    // 缓冲区太小

// 对象相关
#define STATUS_OBJECT_NAME_NOT_FOUND   ((NTSTATUS)0xC0000034L)    // 对象名未找到
#define STATUS_OBJECT_NAME_COLLISION   ((NTSTATUS)0xC0000035L)    // 对象名冲突
#define STATUS_OBJECT_PATH_NOT_FOUND   ((NTSTATUS)0xC000003AL)    // 对象路径未找到

// 资源相关
#define STATUS_INSUFFICIENT_RESOURCES  ((NTSTATUS)0xC000009AL)    // 资源不足
#define STATUS_DEVICE_NOT_READY       ((NTSTATUS)0xC00000A3L)    // 设备未就绪
```

## 三、状态码检查宏

```c
// 检查是否成功
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

// 检查是否为信息状态
#define NT_INFORMATION(Status) ((((NTSTATUS)(Status)) >> 30) == 1)

// 检查是否为警告
#define NT_WARNING(Status) ((((NTSTATUS)(Status)) >> 30) == 2)

// 检查是否为错误
#define NT_ERROR(Status) ((((NTSTATUS)(Status)) >> 30) == 3)
```

## 四、使用示例

```c
NTSTATUS DeviceIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    NTSTATUS status;
    
    // 参数验证
    if (inputBuffer == NULL) {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    
    // 缓冲区大小检查
    if (inputBufferLength < sizeof(INPUT_STRUCT)) {
        status = STATUS_BUFFER_TOO_SMALL;
        goto Exit;
    }
    
    // 资源分配
    buffer = ExAllocatePoolWithTag(NonPagedPool, size, 'Tag1');
    if (buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }
    
    // 访问检查
    if (!AccessAllowed) {
        status = STATUS_ACCESS_DENIED;
        goto Exit;
    }
    
    // 正常处理
    status = STATUS_SUCCESS;

Exit:
    // 状态检查示例
    if (NT_SUCCESS(status)) {
        // 成功处理
    } else if (NT_WARNING(status)) {
        // 警告处理
    } else if (NT_ERROR(status)) {
        // 错误处理
    }
    
    return status;
}
```

## 五、错误处理最佳实践

1. 错误传播：
```c
NTSTATUS Function1()
{
    NTSTATUS status;
    
    status = LowerLevelFunction();
    if (!NT_SUCCESS(status)) {
        return status;  // 保持原始错误码
    }
    
    return STATUS_SUCCESS;
}
```

2. 资源清理：
```c
NTSTATUS AllocateAndProcess()
{
    NTSTATUS status;
    PVOID buffer = NULL;
    
    buffer = ExAllocatePoolWithTag(NonPagedPool, size, 'Tag1');
    if (buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }
    
    // 处理逻辑
    status = ProcessBuffer(buffer);
    
Exit:
    if (buffer) {
        ExFreePoolWithTag(buffer, 'Tag1');
    }
    return status;
}
```

3. 状态码转换：
```c
// 将Win32错误码转换为NTSTATUS
NTSTATUS status = RtlNtStatusToDosError(Win32Error);

// 将NTSTATUS转换为Win32错误码
ULONG win32Error = RtlNtStatusToDosError(Status);
```

使用NTSTATUS时，关键是要：
1. 选择准确的状态码
2. 正确传播错误
3. 妥善处理各种状态
4. 确保资源在任何状态下都能正确清理
