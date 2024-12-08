我来编写一份详细的IRP手册：

# Windows驱动程序IRP完整指南

## 一、IRP基础概念

IRP (I/O Request Packet) 是Windows驱动程序中用于描述I/O请求的数据结构，是内核态驱动程序处理I/O请求的基本单位。

### 1.1 IRP结构
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
    BOOLEAN PendingReturned;
    CHAR StackCount;
    CHAR CurrentLocation;
    BOOLEAN Cancel;
    KIRQL CancelIrql;
    CCHAR ApcEnvironment;
    UCHAR AllocationFlags;
    PIO_STATUS_BLOCK UserIosb;
    PKEVENT UserEvent;
    union {
        struct {
            PIO_APC_ROUTINE UserApcRoutine;
            PVOID UserApcContext;
        } AsynchronousParameters;
        LARGE_INTEGER AllocationSize;
    } Overlay;
    volatile PDRIVER_CANCEL CancelRoutine;
    PVOID UserBuffer;
    union {
        struct {
            union {
                KDEVICE_QUEUE_ENTRY DeviceQueueEntry;
                struct {
                    PVOID DriverContext[4];
                } Driver;
            };
            PETHREAD Thread;
            PCHAR AuxiliaryBuffer;
            struct {
                LIST_ENTRY ListEntry;
                union {
                    struct _IO_STACK_LOCATION *CurrentStackLocation;
                    ULONG PacketType;
                };
            };
            PFILE_OBJECT OriginalFileObject;
        } Overlay;
        KAPC Apc;
        PVOID CompletionKey;
    } Tail;
} IRP, *PIRP;
```

### 1.2 重要字段说明

- MdlAddress：内存描述符列表指针
- SystemBuffer：系统缓冲区指针
- UserBuffer：用户缓冲区指针
- IoStatus：I/O操作状态信息
- StackCount：IRP栈位置数量
- CurrentLocation：当前栈位置

## 二、IRP的主要类型

### 2.1 标准IRP类型
```c
#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04
#define IRP_MJ_DEVICE_CONTROL           0x0E
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0F
#define IRP_MJ_CLEANUP                  0x12
#define IRP_MJ_CLOSE                    0x13
```

### 2.2 PnP和电源管理IRP
```c
#define IRP_MJ_PNP                      0x1b
#define IRP_MJ_POWER                    0x16
```

## 三、IRP处理流程

### 3.1 基本处理步骤
1. 接收IRP
2. 获取当前I/O栈位置
3. 处理请求
4. 完成IRP或传递给下层驱动

```c
NTSTATUS DeviceIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    // 处理请求
    switch(irpStack->Parameters.DeviceIoControl.IoControlCode)
    {
        // 处理具体的控制码
    }

    // 完成IRP
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
```

### 3.2 异步处理
```c
NTSTATUS AsyncIoHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    IoMarkIrpPending(Irp);
    IoQueueWorkItem(...);  // 队列化工作项
    return STATUS_PENDING;
}
```

## 四、IRP缓冲方式

### 4.1 METHOD_BUFFERED
系统自动创建缓冲区，数据通过SystemBuffer访问：
```c
PVOID inputBuffer = Irp->AssociatedIrp.SystemBuffer;
PVOID outputBuffer = Irp->AssociatedIrp.SystemBuffer;
```

### 4.2 METHOD_DIRECT
使用MDL进行直接内存访问：
```c
PVOID buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
```

### 4.3 METHOD_NEITHER
直接访问用户模式缓冲区：
```c
PVOID inputBuffer = irpStack->Parameters.DeviceIoControl.Type3InputBuffer;
PVOID outputBuffer = Irp->UserBuffer;
```

## 五、IRP取消处理

### 5.1 设置取消例程
```c
void SetCancelRoutine(PIRP Irp)
{
    IoSetCancelRoutine(Irp, CancelRoutine);
}

VOID CancelRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    // 清理资源
    IoReleaseCancelSpinLock(Irp->CancelIrql);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}
```

### 5.2 检查取消标志
```c
if (Irp->Cancel) {
    // IRP已被取消
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_CANCELLED;
}
```

## 六、常见错误处理

1. 忘记完成IRP
```c
// 错误示例
if (error) return STATUS_UNSUCCESSFUL; // IRP未完成

// 正确做法
if (error) {
    Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_UNSUCCESSFUL;
}
```

2. 重复完成IRP
```c
// 错误示例
IoCompleteRequest(Irp, IO_NO_INCREMENT);
IoCompleteRequest(Irp, IO_NO_INCREMENT); // 重复完成

// 正确做法
IoCompleteRequest(Irp, IO_NO_INCREMENT);
return status;
```

## 七、性能优化建议

1. 避免不必要的缓冲区复制
2. 合理使用缓冲方式
3. 尽量使用异步处理大型I/O请求
4. 正确设置IoStatus.Information字段
5. 合理使用IRP优先级

这份手册涵盖了IRP的主要概念和使用方法。在实际开发中，需要根据具体需求选择合适的处理方式。特别是在EDR驱动开发中，需要特别注意性能和可靠性的平衡。