# MsfVenom - 独立的 Metasploit 有效载荷生成器

同时也是 msfpayload 和 msfencode 的替代品。

## 使用方法

## 选项

- **-l, --list `<type>`**  
  列出所有 [type] 类型的模块。类型包括：
  - `payloads`（有效载荷）
  - `encoders`（编码器）
  - `nops`
  - `platforms`（平台）
  - `archs`（架构）
  - `encrypt`（加密）
  - `formats`（格式）
  - `all`（全部）

- **-p, --payload `<payload>`**  
  使用的有效载荷（使用 `--list payloads` 列出，`--list-options` 查看参数）。指定 `-` 或 STDIN 输入自定义有效载荷

- **--list-options**  
  列出 `--payload <value>` 的标准、高级和逃避检测选项

- **-f, --format `<format>`**  
  输出格式（使用 `--list formats` 列出）

- **-e, --encoder `<encoder>`**  
  使用的编码器（使用 `--list encoders` 列出）

- **--service-name `<value>`**  
  生成服务二进制文件时使用的服务名称

- **--sec-name `<value>`**  
  生成大型 Windows 二进制文件时使用的新节名称。默认值：随机 4 字符的字母字符串

- **--smallest**  
  使用所有可用的编码器生成尽可能小的有效载荷

- **--encrypt `<value>`**  
  应用于 shellcode 的加密或编码类型（使用 `--list encrypt` 列出）

- **--encrypt-key `<value>`**  
  用于 `--encrypt` 的密钥

- **--encrypt-iv `<value>`**  
  用于 `--encrypt` 的初始化向量

- **-a, --arch `<arch>`**  
  为 `--payload` 和 `--encoders` 使用的架构（使用 `--list archs` 列出）

- **--platform `<platform>`**  
  `--payload` 的平台（使用 `--list platforms` 列出）

- **-o, --out `<path>`**  
  将有效载荷保存到文件

- **-b, --bad-chars `<list>`**  
  需要避免的字符，例如：`\x00\xff`

- **-n, --nopsled `<length>`**  
  在有效载荷前添加一段长度为 [length] 的 NOP 滑梯

- **--pad-nops**  
  使用由 `-n <length>` 指定的 nopsled 大小作为总有效载荷大小，自动在前面添加 nopsled

- **-s, --space `<length>`**  
  结果有效载荷的最大大小

- **--encoder-space `<length>`**  
  编码后有效载荷的最大大小（默认为 `-s` 值）

- **-i, --iterations `<count>`**  
  编码有效载荷的次数

- **-c, --add-code `<path>`**  
  指定一个额外的 win32 shellcode 文件包含进来

- **-x, --template `<path>`**  
  指定一个自定义的可执行文件作为模板

- **-k, --keep**  
  保留 `--template` 行为，并将有效载荷作为一个新线程注入

- **-v, --var-name `<value>`**  
  指定用于某些输出格式的自定义变量名称

- **-t, --timeout `<second>`**  
  从 STDIN 读取有效载荷时的等待秒数（默认 30 秒，0 为禁用）

- **-h, --help**  
  显示此消息