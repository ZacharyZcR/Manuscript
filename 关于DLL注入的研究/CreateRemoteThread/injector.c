#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

// 获取目标进程ID的函数
DWORD FindProcessId(const char *processname) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    DWORD result = 0;

    // 创建进程快照
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printf("创建进程快照失败。\n");
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    // 遍历进程列表，查找目标进程
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, processname) == 0) {
                result = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
    return result;
}

// 主函数
int main() {
    const char *dllPath = "hello.dll";
    const char *targetProcess = "notepad.exe";
    DWORD pid = FindProcessId(targetProcess);

    if (pid == 0) {
        printf("找不到目标进程：%s\n", targetProcess);
        return 1;
    } else {
        printf("找到 %s 进程，PID = %u\n", targetProcess, pid);
    }

    // 打开目标进程
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        printf("打开目标进程失败。\n");
        return 1;
    }

    // 在目标进程中为DLL路径分配内存
    LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pDllPath == NULL) {
        printf("在目标进程中分配内存失败。\n");
        CloseHandle(hProcess);
        return 1;
    }

    // 将DLL路径写入目标进程
    WriteProcessMemory(hProcess, pDllPath, (LPVOID)dllPath, strlen(dllPath) + 1, NULL);

    // 获取LoadLibraryA函数的地址
    HMODULE hKernel32 = GetModuleHandle("Kernel32");
    FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");

    // 创建远程线程来加载DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pDllPath, 0, NULL);
    if (hThread == NULL) {
        printf("创建远程线程失败。\n");
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    printf("成功创建远程线程，DLL注入中。\n");

    // 等待远程线程结束
    WaitForSingleObject(hThread, INFINITE);

    // 清理
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    printf("DLL注入完成。\n");

    return 0;
}