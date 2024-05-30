出现这个错误通常是因为MySQL的用户认证和权限配置问题。在MySQL 8.0及更高版本中，默认使用`auth_socket`插件来认证本地连接，而不是使用密码。这意味着，当使用`root`用户通过UNIX socket连接时，它不会要求密码，但是在尝试使用密码连接（例如通过您的Gin程序）时，它会失败。

要解决这个问题，您可以更改`root`用户的认证方法，使其使用密码认证。按照以下步骤操作：

1. 登录MySQL：首先，以`root`用户登录到MySQL：

```Bash
sudo mysql -u root
```

1. 更改认证插件：使用以下MySQL命令将`root`用户的认证插件更改为`mysql_native_password`：

```SQL
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '您的新密码';
FLUSH PRIVILEGES;
```

1. 将`'您的新密码'`替换为您想要设置的新密码。
2. 退出MySQL：执行完这些命令后，输入`exit`退出MySQL。
3. 重新启动MySQL服务：为了使更改生效，重新启动MySQL服务：

```Bash
sudo systemctl restart mysql
```

1. 更新Gin程序的数据库配置：确保您的Gin程序中的数据库配置使用了正确的用户名（在这种情况下是`root`）、密码（刚才设置的新密码）、主机名（通常是`localhost`）和数据库名。
2. 重新运行Gin程序：现在，重新运行您的Gin程序，它应该能够成功连接到MySQL数据库。

如果问题仍然存在，确保您的Gin程序使用的连接字符串是正确的，并检查是否有任何防火墙或安全组设置阻止了连接。此外，确保您的MySQL服务器正在运行并监听来自您的Gin程序的连接。