#include <windows.h>
#include <stdio.h>

// 导出函数的声明
__declspec(dllexport) void hello();

// 线程函数声明
DWORD WINAPI showMessage(LPVOID lpParameter);

void hello() {
    // 创建一个线程来显示消息框
    CreateThread(NULL, 0, showMessage, NULL, 0, NULL);
}

// 线程函数定义
DWORD WINAPI showMessage(LPVOID lpParameter) {
    MessageBox(NULL, "Hello, World!", "Hello", MB_OK);
    return 0;
}

// DllMain函数是DLL的入口点
BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      ) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            hello(); // 当进程附加时调用hello()
        break;
        case DLL_THREAD_ATTACH:
            // 当线程附加时执行的代码
                break;
        case DLL_THREAD_DETACH:
            // 当线程分离时执行的代码
                break;
        case DLL_PROCESS_DETACH:
            // 当进程分离时执行的代码
                break;
    }
    return TRUE;
}
