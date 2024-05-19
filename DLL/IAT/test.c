#include <windows.h>
#include <stdio.h>
#include <psapi.h>

// 功能：打印指定模块的所有IAT条目
void PrintModuleIAT(HANDLE hProcess, HMODULE hModule) {
    // 获取模块信息
    MODULEINFO modInfo;
    if (!GetModuleInformation(hProcess, hModule, &modInfo, sizeof(MODULEINFO))) {
        printf("获取模块信息失败。\n");
        return;
    }

    // 计算模块的范围
    BYTE *pBase = (BYTE*)modInfo.lpBaseOfDll;
    IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER*)pBase;
    IMAGE_NT_HEADERS *pNtHeaders = (IMAGE_NT_HEADERS*)(pBase + pDosHeader->e_lfanew);

    // 定位导入表
    IMAGE_IMPORT_DESCRIPTOR *pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)(pBase +
        pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    if (pImportDesc == NULL) {
        printf("未找到导入表。\n");
        return;
    }

    // 遍历所有导入表项
    while (pImportDesc->Name != 0) {
        // 输出导入的模块名
        char *szModName = (char*)(pBase + pImportDesc->Name);
        printf("模块: %s\n", szModName);

        // 遍历该模块的所有导入函数
        IMAGE_THUNK_DATA *pThunkOrig = (IMAGE_THUNK_DATA *)(pBase + pImportDesc->OriginalFirstThunk);
        IMAGE_THUNK_DATA *pThunk = (IMAGE_THUNK_DATA *)(pBase + pImportDesc->FirstThunk);
        while (pThunkOrig->u1.AddressOfData != 0 && pThunk->u1.Function != 0) {
            FARPROC pFunc = (FARPROC)pThunk->u1.Function;
            if (pThunkOrig->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
                printf("\t函数地址: 0x%p (序数: %d)\n", pFunc, IMAGE_ORDINAL(pThunkOrig->u1.Ordinal));
            } else {
                IMAGE_IMPORT_BY_NAME *pImportByName = (IMAGE_IMPORT_BY_NAME *)(pBase + pThunkOrig->u1.AddressOfData);
                printf("\t函数地址: 0x%p, 函数名: %s\n", pFunc, pImportByName->Name);
            }
            pThunk++;
            pThunkOrig++;
        }

        pImportDesc++;
    }
}

int main() {
    // 获取当前进程的句柄和模块句柄
    HANDLE hProcess = GetCurrentProcess();
    HMODULE hModule = GetModuleHandle(NULL);  // NULL 表示获取主模块

    // 打印主模块的IAT
    PrintModuleIAT(hProcess, hModule);

    // 关闭进程句柄
    CloseHandle(hProcess);

    return 0;
}
