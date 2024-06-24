以下是翻译后的表格：

| 名称                         | 等级 | 描述                                           |
| ---------------------------- | ---- | ---------------------------------------------- |
| cmd/base64                   | 良好 | Base64 命令编码器                              |
| cmd/brace                    | 低   | Bash Brace 扩展命令编码器                      |
| cmd/echo                     | 良好 | Echo 命令编码器                                |
| cmd/generic_sh               | 手动 | 通用 Shell 变量替换命令编码器                  |
| cmd/ifs                      | 低   | Bourne ${IFS} 替换命令编码器                   |
| cmd/perl                     | 正常 | Perl 命令编码器                                |
| cmd/powershell_base64        | 优秀 | Powershell Base64 命令编码器                   |
| cmd/printf_php_mq            | 手动 | printf(1) 通过 PHP magic_quotes 实用命令编码器 |
| generic/eicar                | 手动 | EICAR 编码器                                   |
| generic/none                 | 正常 | "无" 编码器                                    |
| mipsbe/byte_xori             | 正常 | Byte XORi 编码器                               |
| mipsbe/longxor               | 正常 | XOR 编码器                                     |
| mipsle/byte_xori             | 正常 | Byte XORi 编码器                               |
| mipsle/longxor               | 正常 | XOR 编码器                                     |
| php/base64                   | 优秀 | PHP Base64 编码器                              |
| ppc/longxor                  | 正常 | PPC LongXOR 编码器                             |
| ppc/longxor_tag              | 正常 | PPC LongXOR 编码器                             |
| ruby/base64                  | 优秀 | Ruby Base64 编码器                             |
| sparc/longxor_tag            | 正常 | SPARC DWORD XOR 编码器                         |
| x64/xor                      | 正常 | XOR 编码器                                     |
| x64/xor_context              | 正常 | 基于主机名的上下文键控载荷编码器               |
| x64/xor_dynamic              | 正常 | 动态密钥 XOR 编码器                            |
| x64/zutto_dekiru             | 手动 | Zutto Dekiru                                   |
| x86/add_sub                  | 手动 | 加/减 编码器                                   |
| x86/alpha_mixed              | 低   | Alpha2 字母数字混合大小写编码器                |
| x86/alpha_upper              | 低   | Alpha2 字母数字大写编码器                      |
| x86/avoid_underscore_tolower | 手动 | 避免下划线/小写编码器                          |
| x86/avoid_utf8_tolower       | 手动 | 避免 UTF8/小写编码器                           |
| x86/bloxor                   | 手动 | BloXor - 基于块的变形 XOR 编码器               |
| x86/bmp_polyglot             | 手动 | BMP 多种格式                                   |
| x86/call4_dword_xor          | 正常 | Call+4 Dword XOR 编码器                        |
| x86/context_cpuid            | 手动 | 基于 CPUID 的上下文键控载荷编码器              |
| x86/context_stat             | 手动 | 基于 stat(2) 的上下文键控载荷编码器            |
| x86/context_time             | 手动 | 基于 time(2) 的上下文键控载荷编码器            |
| x86/countdown                | 正常 | 单字节 XOR 倒计时编码器                        |
| x86/fnstenv_mov              | 正常 | 可变长度 Fnstenv/mov Dword XOR 编码器          |
| x86/jmp_call_additive        | 正常 | 跳跃/调用 XOR 加法反馈编码器                   |
| x86/nonalpha                 | 低   | 非字母编码器                                   |
| x86/nonupper                 | 低   | 非大写编码器                                   |
| x86/opt_sub                  | 手动 | 子编码器（优化）                               |
| x86/service                  | 手动 | 注册服务                                       |
| x86/shikata_ga_nai           | 优秀 | 多态 XOR 加法反馈编码器                        |
| x86/single_static_bit        | 手动 | 单个静态位编码器                               |
| x86/unicode_mixed            | 手动 | Alpha2 字母数字 Unicode 混合大小写编码器       |
| x86/unicode_upper            | 手动 | Alpha2 字母数字 Unicode 大写编码器             |
| x86/xor_dynamic              | 正常 | 动态密钥 XOR 编码器                            |
| x86/xor_poly                 | 正常 | XOR 多态编码器                                 |