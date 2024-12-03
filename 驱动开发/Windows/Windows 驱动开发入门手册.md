# Windows 驱动开发入门手册

## 1. 开发环境搭建
1. 安装 Visual Studio（推荐使用最新版本）
2. 安装 Windows Driver Kit (WDK)
   - 确保 VS 和 WDK 版本匹配
   - 建议先安装 VS，再安装 WDK
   - 安装完成后重启电脑

## 2. 创建驱动项目
1. 打开 Visual Studio
2. 创建新项目，选择 "Kernel Mode Driver, Empty (KMDF)"
3. 配置项目属性
   - 确保目标平台正确（x64/x86）
   - 配置驱动签名选项

## 3. 编写基础驱动代码
在 Driver.c 中添加基本框架：
```c
#include <ntddk.h>
#include <wdf.h>
DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD KmdfHelloWorldEvtDeviceAdd;

NTSTATUS 
DriverEntry(
    _In_ PDRIVER_OBJECT     DriverObject, 
    _In_ PUNICODE_STRING    RegistryPath
)
{
    // NTSTATUS variable to record success or failure
    NTSTATUS status = STATUS_SUCCESS;

    // Allocate the driver configuration object
    WDF_DRIVER_CONFIG config;

    // Print "Hello World" for DriverEntry
    KdPrintEx(( DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfHelloWorld: DriverEntry\n" ));

    // Initialize the driver configuration object to register the
    // entry point for the EvtDeviceAdd callback, KmdfHelloWorldEvtDeviceAdd
    WDF_DRIVER_CONFIG_INIT(&config, 
                           KmdfHelloWorldEvtDeviceAdd
                           );

    // Finally, create the driver object
    status = WdfDriverCreate(DriverObject, 
                             RegistryPath, 
                             WDF_NO_OBJECT_ATTRIBUTES, 
                             &config, 
                             WDF_NO_HANDLE
                             );
    return status;
}

NTSTATUS 
KmdfHelloWorldEvtDeviceAdd(
    _In_    WDFDRIVER       Driver, 
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    // We're not using the driver object,
    // so we need to mark it as unreferenced
    UNREFERENCED_PARAMETER(Driver);

    NTSTATUS status;

    // Allocate the device object
    WDFDEVICE hDevice;    

    // Print "Hello World"
    KdPrintEx(( DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfHelloWorld: KmdfHelloWorldEvtDeviceAdd\n" ));

    // Create the device object
    status = WdfDeviceCreate(&DeviceInit, 
                             WDF_NO_OBJECT_ATTRIBUTES,
                             &hDevice
                             );
    return status;
}
```

## 4. 编译和准备文件
1. 编译项目（Release x64）
2. 在输出目录找到：
   - .sys 文件（驱动文件）
   - .inf 文件（安装信息文件）
3. 将这些文件复制到目标机器

## 5. 目标机器配置
1. 启用测试模式：
```powershell
bcdedit /set testsigning on
```
2. 重启计算机

## 6. 驱动安装步骤
1. 先使用 pnputil 安装驱动包：
```powershell
pnputil -i -a <驱动的inf文件路径>
```

2. 使用 devcon 创建设备实例：
```powershell
devcon install <inf文件> root\<设备ID>
```

注意：不要直接使用 devcon 安装，这可能会失败。应该先用 pnputil 安装。

## 7. 调试配置
1. 安装 DebugView
2. 配置 DebugView：
   - 以管理员身份运行
   - 勾选 Capture -> Capture Kernel
   - 勾选 Capture -> Enable Verbose Kernel Output

## 8. 验证驱动
1. 使用 devcon 查看设备：
```powershell
devcon find *<驱动名>*
```

2. 重启设备来触发调试输出：
```powershell
devcon restart root\<设备ID>
```

## 常见问题解决
1. 驱动安装失败
   - 检查签名设置
   - 确认测试模式已开启
   - 使用正确的安装顺序（先 pnputil，后 devcon）

2. 看不到调试输出
   - 确认 DebugView 以管理员权限运行
   - 检查调试筛选器设置
   - 确认驱动中的打印级别正确
