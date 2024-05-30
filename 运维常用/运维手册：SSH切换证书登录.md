要在 Ubuntu 系统中关闭 SSH 的密码登录并改为使用 SSH 证书登录，你需要进行以下步骤：

1. 生成 SSH 密钥对（如果你还没有）：
   1. 打开终端。
   2. 输入 `ssh-keygen` 并按照提示操作。这将在你的家目录的 `.ssh` 文件夹中生成一对密钥（默认是 `id_rsa` 和 `id_rsa.pub`）。
2. 将公钥复制到服务器：
   1. 使用 `ssh-copy-id user@your_server_ip` 命令，将你的公钥（`.pub` 文件）复制到服务器的 `~/.ssh/authorized_keys` 文件中。这里的 `user` 是你的用户名，`your_server_ip` 是服务器的 IP 地址。
3. 配置 SSH 以禁用密码登录：
   1. 登录到你的服务器。
   2. 编辑 SSH 配置文件。通常，这可以通过输入 `sudo nano /etc/ssh/sshd_config` 来完成。
   3. 在配置文件中找到以下行（如果不存在，则添加它们）：
   4. ```Perl
      PasswordAuthentication no
      ChallengeResponseAuthentication no
      UsePAM no
      ```

   5. 确保 `PubkeyAuthentication` 项设置为 `yes`（这通常是默认设置）。
4. 重启 SSH 服务：
   1. 在服务器上，执行命令 `sudo systemctl restart ssh` 以应用更改。

完成以上步骤后，SSH 的密码登录将被禁用，只能通过 SSH 密钥对进行登录。

注意：在禁用密码登录之前，请确保你的 SSH 密钥可以正常工作，以防被锁在系统外。同时，请确保你有其他访问服务器的方式（比如直接控制台访问），以防出现配置错误。