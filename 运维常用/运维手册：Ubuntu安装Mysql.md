在Ubuntu 22.04上安装MySQL 8.0的步骤如下：

1. 更新包列表：首先，打开终端并更新包列表以确保您安装的是最新版本的软件。可以使用以下命令：

```SQL
sudo apt update
```

1. 安装MySQL服务器：接着，使用以下命令安装MySQL服务器：

```Plaintext
sudo apt install mysql-server
```

1. 安全配置：安装完成后，运行安全脚本来设置MySQL的安全选项，如设置root密码，删除匿名用户等。使用以下命令：

```Plaintext
sudo mysql_secure_installation
```

1. 这个脚本会引导您完成一些安全配置，例如设置root用户的密码、删除匿名用户、禁用root用户远程登录以及删除测试数据库。
2. 检查MySQL服务：确保MySQL服务已正确安装并正在运行，可以使用以下命令检查其状态：

```Lua
sudo systemctl status mysql.service
```

1. 登录MySQL：安装并配置完成后，您可以使用以下命令登录到MySQL数据库：

```CSS
sudo mysql -u root -p
```

1. 您将被提示输入在安全配置过程中设置的密码。
2. （可选）创建新的数据库用户：出于安全考虑，建议为日常任务创建一个新的数据库用户，而不是使用root用户。您可以在MySQL提示符下运行以下命令来创建一个新用户并授权：

```SQL
CREATE USER 'newuser'@'localhost' IDENTIFIED BY 'password';
GRANT ALL PRIVILEGES ON *.* TO 'newuser'@'localhost' WITH GRANT OPTION;
FLUSH PRIVILEGES;
```

1. 替换`'newuser'`和`'password'`为您想要的用户名和密码。
2. 退出MySQL：完成操作后，您可以输入`exit`命令退出MySQL。

这些步骤将帮助您在Ubuntu 22.04上安装和配置MySQL 8.0。记得定期检查更新并保持您的系统和应用程序是最新的。