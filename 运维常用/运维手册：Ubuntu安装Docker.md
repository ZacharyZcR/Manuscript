在Ubuntu上安装Docker包括以下步骤：

1. 更新现有的软件包列表： 打开终端，并运行以下命令来更新Ubuntu的软件包列表：

```Bash
sudo apt update
```

1. 安装一些必要的软件包： 这些软件包允许`apt`通过HTTPS使用仓库：

```Bash
sudo apt install apt-transport-https ca-certificates curl software-properties-common
```

1. 添加Docker的官方GPG密钥： 这确保你下载的软件是由Docker创建的：

```Bash
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
```

1. 添加Docker的APT仓库： 将Docker的官方APT仓库添加到你的系统中：

```Bash
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
```

1. 再次更新软件包列表： 由于你添加了新的仓库，需要再次更新软件包列表：

```Bash
sudo apt update
```

1. 安装Docker CE（社区版）： 现在，安装最新版本的Docker CE：

```Bash
sudo apt install docker-ce
```

1. 启动Docker服务（如果尚未自动启动）：

```Bash
sudo systemctl start docker
```

1. 使Docker在启动时自动运行：

```Bash
sudo systemctl enable docker
```

1. 验证安装： 运行hello-world容器来验证Docker是否正确安装：

```Bash
sudo docker run hello-world
```

1. 如果安装正确，你将看到一条消息，表明你的Docker安装正常运行。
2. （可选）将你的用户添加到`docker`组： 默认情况下，运行Docker命令需要`sudo`权限。为了避免每次使用Docker都输入`sudo`，可以将你的用户添加到`docker`组：

```Bash
sudo usermod -aG docker ${USER}
```

1. 注销并重新登录以应用这些更改。

完成这些步骤后，Docker就会在你的Ubuntu系统上安装并准备就绪。