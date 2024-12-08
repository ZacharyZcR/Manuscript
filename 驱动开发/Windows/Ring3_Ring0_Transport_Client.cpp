#include <windows.h>
#include <stdio.h>

#define DEVICE_NAME L"\\\\.\\MyTestDevice"
#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

int main()
{
    printf("Opening device...\n");

    HANDLE hDevice = CreateFileW(DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open device: %d\n", GetLastError());
        return 1;
    }

    printf("Device opened successfully\n");

    DWORD bytesReturned;
    BOOL success = DeviceIoControl(hDevice,
        IOCTL_TEST,
        NULL,
        0,
        NULL,
        0,
        &bytesReturned,
        NULL);

    if (success)
    {
        printf("Successfully sent message to driver\n");
    }
    else
    {
        printf("Failed to send message: %d\n", GetLastError());
    }

    CloseHandle(hDevice);
    printf("Device closed\n");
    return 0;
}