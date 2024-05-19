#include <windows.h>
#include <stdio.h>
#include <psapi.h>

// 假设已知hello.dll及其函数地址
extern BOOL WINAPI MyWriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);

// 查找并修改指定模块的IAT
BOOL HijackIAT(const char* dllName, const char* funcName, PROC newFunc, DWORD procId) {
    HMODULE hMods[1024];
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, procId);
    DWORD cbNeeded;
    unsigned int i;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            MODULEINFO modInfo;
            GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(MODULEINFO));
            char* baseAddr = (char*)modInfo.lpBaseOfDll;
            char* modName = new char[MAX_PATH];
            GetModuleFileNameEx(hProcess, hMods[i], modName, MAX_PATH);

            // 确定模块是不是我们想要劫持的DLL
            if (strstr(modName, dllName)) {
                IMAGE_DOS_HEADER dosHeader;
                IMAGE_NT_HEADERS ntHeaders;

                // 读取DOS和NT头部
                ReadProcessMemory(hProcess, baseAddr, &dosHeader, sizeof(dosHeader), NULL);
                ReadProcessMemory(hProcess, baseAddr + dosHeader.e_lfanew, &ntHeaders, sizeof(ntHeaders), NULL);

                // 获取IAT位置和大小
                auto importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)(baseAddr + ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
                IMAGE_IMPORT_DESCRIPTOR singleImportDescriptor;

                // 遍历IAT寻找对应函数
                while (true) {
                    ReadProcessMemory(hProcess, importDescriptor, &singleImportDescriptor, sizeof(singleImportDescriptor), NULL);
                    if (singleImportDescriptor.Name == 0) break;

                    char* dllImportName = new char[MAX_PATH];
                    ReadProcessMemory(hProcess, baseAddr + singleImportDescriptor.Name, dllImportName, MAX_PATH, NULL);

                    if (strcmp(dllImportName, dllName) == 0) {
                        // 定位Thunk数据用于修改
                        auto firstThunk = (IMAGE_THUNK_DATA*)(baseAddr + singleImportDescriptor.FirstThunk);
                        IMAGE_THUNK_DATA thunkData;
                        while (true) {
                            ReadProcessMemory(hProcess, firstThunk, &thunkData, sizeof(IMAGE_THUNK_DATA), NULL);
                            if (thunkData.u1.Function == 0) break;

                            FARPROC* funcAddress = (FARPROC*)thunkData.u1.Function;
                            char funcNameBuffer[256];
                            DWORD bytes;
                            ReadProcessMemory(hProcess, (LPVOID)((PIMAGE_IMPORT_BY_NAME)funcAddress)->Name, funcNameBuffer, 256, &bytes);

                            if (strcmp(funcNameBuffer, funcName) == 0) {
                                DWORD oldProtect;
                                VirtualProtectEx(hProcess, &firstThunk->u1.Function, sizeof(FARPROC), PAGE_READWRITE, &oldProtect);
                                WriteProcessMemory(hProcess, &firstThunk->u1.Function, &newFunc, sizeof(newFunc), NULL);
                                VirtualProtectEx(hProcess, &firstThunk->u1.Function, sizeof(FARPROC), oldProtect, &oldProtect);
                                break;
                            }
                            firstThunk++;
                        }
                    }
                    importDescriptor++;
                }
            }
            delete[] modName;
        }
    }

    CloseHandle(hProcess);
    return TRUE;
}

int main() {
    DWORD pid = 1234; // notepad.exe的进程ID
    if (!HijackIAT("kernel32.dll", "WriteFile", (PROC)MyWriteFile, pid)) {
        printf("IAT 劫持失败。\n");
        return 1;
    }
    printf("IAT 劫持成功。\n");
    return 0;
}
