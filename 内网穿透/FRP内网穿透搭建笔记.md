# FRP内网穿透搭建笔记

## 实验目标

在公网VPS上使用FRP将内网服务（如Docker中的Nginx）暴露到公网，而不需要在内网机器上进行任何配置。

## 部署步骤

### 1. 准备工作

- **VPS要求**：确保你有一台可以访问互联网的VPS。
- **Docker要求**：如果打算在Docker中运行服务，确保VPS上已安装Docker。

### 2. 在VPS上搭建Nginx服务（可选）

#### 2.1 安装Docker

在VPS上安装Docker（以Ubuntu为例）：

```bash
sudo apt update
sudo apt install -y docker.io
```

#### 2.2 启动Nginx容器

启动Nginx容器，但不映射端口以避免对外开放：

```bash
sudo docker run -d --name nginx-server nginx
```

### 3. 下载和配置FRP

#### 3.1 下载FRP

在VPS上下载FRP：

```bash
wget https://github.com/fatedier/frp/releases/download/v0.60.0/frp_0.60.0_linux_amd64.tar.gz
tar -zxvf frp_0.60.0_linux_amd64.tar.gz
cd frp_0.60.0_linux_amd64
```

#### 3.2 配置FRP服务器

创建`frps.ini`配置文件，内容如下：

```ini
[common]
bind_port = 7000        # FRP服务器监听的端口
```

#### 3.3 启动FRP服务器

运行以下命令启动FRP服务器：

```bash
./frps -c frps.ini
```

### 4. 配置FRP客户端

#### 4.1 创建FRP客户端配置文件

创建`frpc.ini`配置文件，内容如下：

```ini
[common]
server_addr = 127.0.0.1   # 使用localhost，因为frps和frpc都在同一台机器上
server_port = 7000         # FRP服务器的端口

[nginx]
type = tcp                 # 使用TCP协议
local_ip = 127.0.0.1       # Nginx服务的IP地址（如果是在Docker中运行，则使用容器的IP地址）
local_port = 80            # Nginx服务的端口
remote_port = 8080         # 映射到FRP服务器的端口
```

> **注意**：如果Nginx是在Docker中运行，需要获取Docker容器的IP地址并替换`local_ip`。

#### 4.2 启动FRP客户端

运行以下命令启动FRP客户端：

```bash
./frpc -c frpc.ini
```

### 5. 测试连接

在浏览器中输入你的VPS公网IP地址和映射的端口，例如：`http://xxx:8080`。如果设置正确，你应该能够看到Nginx的欢迎页面。

### 注意事项

- 确保防火墙设置允许相应端口的流量。
- 如果使用云服务提供商，请确保安全组规则允许外部访问所需端口。
- 根据需要调整FRP的配置文件，以适应不同的服务或需求。

## 总结

通过上述步骤，可以成功地在单个公网VPS上部署FRP，将内网服务暴露到公网。这种方法简化了网络架构，同时确保了内网服务能够安全地被外部访问。