# 分发函数(Dispatch Functions)详细手册

## 一、基本概念

### 1. IRP(I/O Request Packet)
```c
typedef struct _IRP {
    CSHORT Type;
    USHORT Size;
    PMDL MdlAddress;
    ULONG Flags;
    union {
        struct _IRP *MasterIrp;
        LONG IrpCount;
        PVOID SystemBuffer;
    } AssociatedIrp;
    LIST_ENTRY ThreadListEntry;
    IO_STATUS_BLOCK IoStatus;
    KPROCESSOR_MODE RequestorMode;
    // ... 更多字段
} IRP, *PIRP;
```

### 2. 常见的IRP主功能码
```c
#define IRP_MJ_CREATE                   0x00  // 创建/打开
#define IRP_MJ_CLOSE                    0x02  // 关闭
#define IRP_MJ_READ                     0x03  // 读取
#define IRP_MJ_WRITE                    0x04  // 写入
#define IRP_MJ_DEVICE_CONTROL           0x0E  // 设备控制
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0F  // 内部设备控制
#define IRP_MJ_SHUTDOWN                 0x10  // 系统关机
#define IRP_MJ_CLEANUP                  0x12  // 清理
```

## 二、分发函数注册

### 1. 在DriverEntry中注册
```c
NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
)
{
    // 设置默认处理函数
    for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = DefaultDispatch;
    }
    
    // 设置特定处理函数
    DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlDispatch;
    DriverObject->MajorFunction[IRP_MJ_READ] = ReadDispatch;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = WriteDispatch;
}
```

## 三、典型分发函数实现

### 1. 通用分发函数模板
```c
NTSTATUS GenericDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION deviceExtension;
    
    // 获取当前IRP堆栈位置
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    
    // 获取设备扩展
    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    
    // 处理请求
    // ...
    
    // 设置IRP完成状态
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    
    // 完成IRP
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;
}
```

### 2. CREATE请求处理示例
```c
NTSTATUS CreateDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    
    // 这里可以执行:
    // - 设备初始化
    // - 资源分配
    // - 访问权限检查
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;
}
```

### 3. DEVICE_CONTROL请求处理示例
```c
NTSTATUS DeviceControlDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    ULONG ioControlCode;
    PVOID inputBuffer;
    PVOID outputBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    
    // 获取IOCTL相关信息
    ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    outputBuffer = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    
    switch (ioControlCode) {
        case IOCTL_CUSTOM_CODE:
            // 处理特定IOCTL请求
            break;
            
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;
}
```

### 4. READ/WRITE请求处理示例
```c
NTSTATUS ReadDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    LARGE_INTEGER offset;
    ULONG length;
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    
    // 获取读取参数
    offset = irpStack->Parameters.Read.ByteOffset;
    length = irpStack->Parameters.Read.Length;
    
    // 执行读取操作
    // ...
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = length; // 实际读取的字节数
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;
}
```

## 四、重要注意事项

1. IRP完成规则：
   - 每个IRP必须且只能完成一次
   - 使用IoCompleteRequest完成IRP
   - 设置正确的完成状态

2. 错误处理：
   - 始终返回适当的NTSTATUS码
   - 处理所有可能的错误情况
   - 清理分配的资源

3. 缓冲区访问：
   - 验证缓冲区长度
   - 使用正确的缓冲区访问方法
   - 考虑缓冲区方法(METHOD_BUFFERED, METHOD_DIRECT等)

4. 性能考虑：
   - 避免长时间持有资源
   - 考虑使用分页/非分页内存
   - 适当使用延迟处理
