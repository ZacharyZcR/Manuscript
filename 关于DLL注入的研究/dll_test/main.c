#include <windows.h>
#include <stdio.h>

typedef void (*HelloFunc)();

int main() {
    HMODULE hModule = LoadLibrary("hello.dll");
    if (hModule != NULL) {
        HelloFunc hello = (HelloFunc)GetProcAddress(hModule, "hello");
        if (hello != NULL) {
            hello();
        }
        FreeLibrary(hModule);
    } else {
        printf("Failed to load DLL\n");
    }
    return 0;
}
