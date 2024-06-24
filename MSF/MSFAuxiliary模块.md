在Metasploit框架中，Auxiliary模块（辅助模块）用于执行各种辅助任务，例如信息收集、服务扫描、漏洞验证和DoS（拒绝服务）攻击等。与Exploit模块不同，Auxiliary模块通常不直接用于利用漏洞获取访问权限，而是用于支持和增强渗透测试过程中的其他活动。

### Auxiliary模块的功能
Auxiliary模块的功能非常广泛，包括但不限于：
- 扫描端口和服务
- 信息收集（如枚举用户、主机、服务等）
- 验证和确认漏洞
- 执行拒绝服务攻击
- 与其他工具和服务集成

### 使用Auxiliary模块的步骤
使用Auxiliary模块的步骤与使用Exploit或Post模块类似。下面是一个基本的使用流程，包括选择、配置和运行Auxiliary模块：

1. **启动msfconsole**：
   打开终端并启动Metasploit控制台。
   ```bash
   msfconsole
   ```

2. **列出可用的Auxiliary模块**：
   在msfconsole中，可以使用`search`命令来查找可用的Auxiliary模块。例如，要查找端口扫描器：
   ```bash
   search auxiliary/scanner/portscan
   ```

3. **选择一个Auxiliary模块**：
   使用`use`命令选择一个你要使用的Auxiliary模块。例如，选择TCP端口扫描器：
   ```bash
   use auxiliary/scanner/portscan/tcp
   ```

4. **查看模块信息**：
   使用`info`命令查看该模块的详细信息，包括需要的选项和描述：
   ```bash
   info
   ```

5. **设置选项**：
   根据模块的信息设置所需的选项，例如目标主机（RHOSTS）和端口范围（PORTS）。例如，设置目标主机为192.168.1.100：
   ```bash
   set RHOSTS 192.168.1.100
   set PORTS 1-1000
   ```

6. **运行模块**：
   使用`run`命令运行配置好的Auxiliary模块：
   ```bash
   run
   ```

以下是一个具体的例子，详细演示如何使用Auxiliary模块进行端口扫描：

### 示例：使用Auxiliary模块扫描端口

1. **启动msfconsole**：
   ```bash
   msfconsole
   ```

2. **搜索端口扫描器**：
   ```bash
   search auxiliary/scanner/portscan
   ```

3. **选择TCP端口扫描器模块**：
   ```bash
   use auxiliary/scanner/portscan/tcp
   ```

4. **查看模块信息**：
   ```bash
   info
   ```

5. **设置目标主机和端口范围**：
   ```bash
   set RHOSTS 192.168.1.100
   set PORTS 1-1000
   ```

6. **运行模块**：
   ```bash
   run
   ```

运行后，你将看到扫描结果，显示目标主机上开放的端口。

### 其他常用Auxiliary模块
- **信息收集模块**：
  - `auxiliary/gather/`: 用于收集各种信息，如邮件地址、用户名等。
- **漏洞验证模块**：
  - `auxiliary/scanner/http/`: 用于扫描和验证HTTP服务上的漏洞。
- **DoS攻击模块**：
  - `auxiliary/dos/`: 用于执行拒绝服务攻击。

通过以上步骤和示例，你可以选择并使用各种Auxiliary模块来支持和增强你的渗透测试活动。这些模块在信息收集、漏洞验证和攻击准备阶段都非常有用。