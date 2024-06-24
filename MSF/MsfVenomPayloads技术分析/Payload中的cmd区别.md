在 MsfVenom 的 payload 中，`cmd/linux/tftp/x86/shell/bind_nonx_tcp` 和 `linux/tftp/x86/shell/bind_nonx_tcp` 这两个模块中，`cmd` 指的是命令（command）类型的 payload。

具体来说：

- `cmd` 类型的 payload 通常是指通过命令行执行的一些简单的命令或脚本。它们不依赖于特定的二进制文件，而是直接使用目标系统的命令解释器（如 `/bin/sh`）来执行命令。
- `linux/tftp/x86/shell/bind_nonx_tcp` 则是一个更加具体的 payload，它是针对 Linux 操作系统，通过 TFTP 服务，使用 x86 架构的 shellcode，绑定一个 TCP 端口，并且不依赖于 NX（No-eXecute）保护的 shell。

总结：

- `cmd/linux/tftp/x86/shell/bind_nonx_tcp`: 表示一个通过命令行的方式生成的 payload。
- `linux/tftp/x86/shell/bind_nonx_tcp`: 表示一个二进制 shellcode payload。

两者的主要区别在于 `cmd` 类型的 payload 更加灵活，可以在目标系统上执行任意命令，而 `linux/tftp/x86/shell/bind_nonx_tcp` 则是一个固定的 shellcode，专门用于创建一个绑定的 TCP shell。