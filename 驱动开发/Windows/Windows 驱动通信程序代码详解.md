# Windows 驱动程序代码详解

## 头文件和宏定义部分
```c
#include <ntddk.h>  // Windows驱动开发核心头文件，包含驱动开发所需的基本定义和函数

// 设备名称，用于内核层识别设备
#define DEVICE_NAME L"\\Device\\MyTestDevice"  
// 符号链接名称，用于用户层访问设备
#define SYMBOLIC_LINK_NAME L"\\??\\MyTestDevice"  

// 定义用户态与驱动通信的控制码
#define IOCTL_TEST \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
```

## 全局变量定义
```c
PDEVICE_OBJECT g_DeviceObject = NULL;  // 设备对象指针
UNICODE_STRING g_DeviceName;           // 设备名称字符串
UNICODE_STRING g_SymbolicLinkName;     // 符号链接名称字符串
```

## 设备操作处理函数
### 创建操作处理
```c
NTSTATUS DeviceCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);  // 标记未使用的参数，避免编译警告
    
    DbgPrint("Device opened\n");  // 打印调试信息
    
    // 设置IRP完成状态
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    
    // 完成IRP请求
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
```

### 关闭操作处理
```c
NTSTATUS DeviceClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    
    DbgPrint("Device closed\n");
    
    // 处理方式与Create类似
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
```

### 设备控制处理
```c
NTSTATUS DeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    
    // 获取IRP堆栈位置，包含请求的详细信息
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    // 根据控制码处理不同的请求
    switch (stack->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_TEST:
            DbgPrint("Received message from user mode!\n");
            break;
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    // 完成IRP请求
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
```

## 驱动卸载函数
```c
VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    // 删除符号链接
    IoDeleteSymbolicLink(&g_SymbolicLinkName);
    // 删除设备对象
    IoDeleteDevice(DriverObject->DeviceObject);
    DbgPrint("Driver Unloaded\n");
}
```

## 驱动入口函数
```c
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status;

    // 初始化设备名称和符号链接名称
    RtlInitUnicodeString(&g_DeviceName, DEVICE_NAME);
    RtlInitUnicodeString(&g_SymbolicLinkName, SYMBOLIC_LINK_NAME);

    // 创建设备对象
    status = IoCreateDevice(DriverObject, 
                          0,                // 扩展大小
                          &g_DeviceName,    // 设备名
                          FILE_DEVICE_UNKNOWN, // 设备类型
                          0,                // 设备特征
                          FALSE,            // 独占访问标志
                          &g_DeviceObject); // 返回的设备对象

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    // 创建符号链接
    status = IoCreateSymbolicLink(&g_SymbolicLinkName, &g_DeviceName);
    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(g_DeviceObject);
        return status;
    }

    // 注册驱动派遣函数
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
    DriverObject->DriverUnload = DriverUnload;

    DbgPrint("Driver Loaded\n");
    return STATUS_SUCCESS;
}
```

## 关键概念说明

1. **IRP (I/O Request Packet)**
   - Windows驱动中的I/O请求数据结构
   - 包含了请求的类型、参数和状态信息

2. **设备对象和符号链接**
   - 设备对象：驱动程序在系统中的表示
   - 符号链接：允许用户态程序访问设备的机制

3. **派遣函数**
   - 处理不同类型I/O请求的函数
   - IRP_MJ_CREATE：处理设备打开请求
   - IRP_MJ_CLOSE：处理设备关闭请求
   - IRP_MJ_DEVICE_CONTROL：处理设备控制请求

4. **驱动生命周期**
   - DriverEntry：驱动加载时调用
   - DriverUnload：驱动卸载时调用

这个驱动程序实现了一个基本的用户态和内核态通信框架，通过设备控制码实现了简单的消息传递功能。要扩展功能，可以：
1. 添加更多控制码处理不同类型的请求
2. 实现数据传输功能
3. 添加安全检查机制
4. 实现更复杂的驱动功能