#include <ntddk.h>

#define DEVICE_NAME L"\\Device\\MyTestDevice"
#define SYMBOLIC_LINK_NAME L"\\??\\MyTestDevice"

#define IOCTL_TEST \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

PDEVICE_OBJECT g_DeviceObject = NULL;
UNICODE_STRING g_DeviceName;
UNICODE_STRING g_SymbolicLinkName;

// 添加Create和Close的处理函数
NTSTATUS DeviceCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    DbgPrint("Device opened\n");
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DeviceClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    DbgPrint("Device closed\n");
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    switch (stack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_TEST:
        DbgPrint("Received message from user mode!\n");
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

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    IoDeleteSymbolicLink(&g_SymbolicLinkName);
    IoDeleteDevice(DriverObject->DeviceObject);
    DbgPrint("Driver Unloaded\n");
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status;

    RtlInitUnicodeString(&g_DeviceName, DEVICE_NAME);
    RtlInitUnicodeString(&g_SymbolicLinkName, SYMBOLIC_LINK_NAME);

    status = IoCreateDevice(DriverObject,
        0,
        &g_DeviceName,
        FILE_DEVICE_UNKNOWN,
        0,
        FALSE,
        &g_DeviceObject);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = IoCreateSymbolicLink(&g_SymbolicLinkName, &g_DeviceName);
    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(g_DeviceObject);
        return status;
    }

    // 设置所有必需的分发函数
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
    DriverObject->DriverUnload = DriverUnload;

    DbgPrint("Driver Loaded\n");
    return STATUS_SUCCESS;
}