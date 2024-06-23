`ssh-copy-id` 是一个在 Unix-like 系统中常用的命令，用于将本地的 SSH 公钥复制到远程主机的 `~/.ssh/authorized_keys` 文件中，从而实现免密码登录。然而，Windows 系统中没有内置的 `ssh-copy-id` 命令，但我们可以通过其他方式来实现相同的功能。

### 方法一：使用 PowerShell 复制 SSH 公钥

1. **生成 SSH 密钥对（如果还没有）**： 打开 PowerShell，并运行以下命令生成 SSH 密钥对：

   ```powershell
   ssh-keygen
   ```

2. **手动复制公钥到远程主机**： 将生成的公钥（通常位于 `~/.ssh/id_rsa.pub`）手动复制到远程主机。

3. **使用 PowerShell 连接并复制公钥**： 假设你已经在远程主机上创建了 `~/.ssh` 目录并设置了正确的权限，你可以使用以下 PowerShell 命令将本地的公钥内容追加到远程主机的 `~/.ssh/authorized_keys` 文件中：

   ```powershell
   type $env:USERPROFILE\.ssh\id_rsa.pub | ssh username@remote_host "mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys"
   ```

### 方法二：使用 `scp` 传输公钥文件

1. **生成 SSH 密钥对（如果还没有）**： 打开 PowerShell，并运行以下命令生成 SSH 密钥对：

   ```powershell
   ssh-keygen
   ```

2. **使用 `scp` 复制公钥文件**： 将本地的公钥文件传输到远程主机的临时目录：

   ```powershell
   scp $env:USERPROFILE\.ssh\id_rsa.pub username@remote_host:~/id_rsa.pub
   ```

3. **在远程主机上设置公钥**： 使用 SSH 连接到远程主机，并将公钥追加到 `~/.ssh/authorized_keys` 文件中：

   ```bash
   ssh username@remote_host
   mkdir -p ~/.ssh
   cat ~/id_rsa.pub >> ~/.ssh/authorized_keys
   rm ~/id_rsa.pub
   chmod 600 ~/.ssh/authorized_keys
   ```

### 方法三：使用 `ssh-copy-id` for Windows

可以使用第三方工具 `ssh-copy-id for Windows`。这是一个移植到 Windows 的版本，使用它可以简化上述过程。可以从网上找到并下载这个工具，然后按照以下步骤操作：

1. **下载并安装 `ssh-copy-id for Windows`**： 从可靠的来源下载并安装。

2. **使用 `ssh-copy-id` 复制公钥**： 打开 PowerShell 或 CMD，并运行以下命令：

   ```powershell
   ssh-copy-id username@remote_host
   ```

通过以上方法，你可以在 Windows 系统中实现类似于 `ssh-copy-id` 的功能，从而简化 SSH 公钥的传输和配置过程。