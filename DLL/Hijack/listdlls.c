#include <windows.h>
#include <stdio.h>
#include <psapi.h>

// 打印给定进程加载的所有DLL
void ListDLLs(DWORD processID) {
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;

    // 打开进程句柄
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess) {
        printf("打开进程失败。\n");
        return;
    }

    // 列出所有加载的模块（DLL）
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            TCHAR szModName[MAX_PATH];

            // 获取模块的文件名
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
                printf("%lu. %s\n", i + 1, szModName);
            }
        }
    } else {
        printf("枚举进程模块失败。\n");
    }

    // 关闭进程句柄
    CloseHandle(hProcess);
}

// 主函数
int main(int argc, char *argv[]) {
    DWORD pid;

    // 检查命令行参数
    if (argc != 2) {
        printf("使用方法: %s <进程ID>\n", argv[0]);
        return 1;
    }

    pid = atoi(argv[1]);
    printf("列出进程ID %d 的所有DLL文件:\n", pid);
    ListDLLs(pid);

    return 0;
}
