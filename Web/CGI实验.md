## Linux CGI实践步骤

在Linux系统上进行CGI编程和配置实验需要以下步骤：

1. 安装和配置Web服务器

首先，我们需要安装Apache Web服务器。在Ubuntu系统上，可以使用以下命令安装Apache：

```
sudo apt-get update
sudo apt-get install apache2
```

安装完成后，需要配置Apache以支持CGI。编辑Apache配置文件：

```
sudo nano /etc/apache2/apache2.conf
```

在配置文件中添加或修改以下内容：

```

    ScriptAlias /cgi-bin/ "/usr/lib/cgi-bin/"



    AllowOverride None
    Options +ExecCGI -MultiViews +SymLinksIfOwnerMatch
    Order allow,deny
    Allow from all

```

这些配置将允许Apache执行/usr/lib/cgi-bin/目录中的CGI程序。

2. 编写CGI程序

接下来，我们创建一个简单的CGI程序。使用任何文本编辑器创建一个名为test.cgi的文件：

```
sudo nano /usr/lib/cgi-bin/test.cgi
```

在文件中输入以下内容：

```bash
#!/bin/sh
echo -e "Content-Type:text/html\n\n"
echo "Hello, CGI!"
```

保存文件并赋予执行权限：

```
sudo chmod +x /usr/lib/cgi-bin/test.cgi
```

3. 重启Apache服务

配置完成后，重启Apache服务以使更改生效：

```
sudo service apache2 restart
```

4. 测试CGI程序

打开Web浏览器，访问以下URL：

```
http://localhost/cgi-bin/test.cgi
```

如果一切配置正确，你应该能看到"Hello, CGI!"的输出。

5. 观察CGI进程

为了观察CGI进程的运行，我们可以修改test.cgi文件，添加一个延迟：

```bash
#!/bin/sh
sleep 20
echo -e "Content-Type:text/html\n\n"
echo "Hello, CGI!"
```

保存文件后，再次访问CGI程序的URL。在浏览器加载期间，打开终端并运行以下命令：

```
ps -ef | grep test.cgi
```

你应该能看到一个正在运行的test.cgi进程。这证明每次CGI请求都会创建一个新的进程来处理。

通过这个简单的实验，我们可以了解CGI的基本工作原理，包括Web服务器的配置、CGI程序的编写，以及CGI进程的创建和运行过程。这为进一步学习和开发CGI应用奠定了基础。