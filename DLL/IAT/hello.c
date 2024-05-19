#include <windows.h>
#include <stdio.h>

// 定义导出函数
__declspec(dllexport) BOOL WINAPI MyWriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
) {
    MessageBox(NULL, "Intercepted WriteFile!", "Hello DLL", MB_OK);
    // 可以在这里调用原始的WriteFile函数或执行其他代码
    return TRUE; // 假设操作总是成功
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            MessageBox(NULL, "DLL Loaded", "Hello DLL", MB_OK);
        break;
        case DLL_PROCESS_DETACH:
            MessageBox(NULL, "DLL Unloaded", "Hello DLL", MB_OK);
        break;
        default:
            break;
    }
    return TRUE;
}
