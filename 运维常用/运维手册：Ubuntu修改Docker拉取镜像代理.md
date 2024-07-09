要在 Ubuntu 上配置 Docker 使用本地代理以拉取镜像，您可以按照以下步骤进行设置：

1. **创建 Docker 配置目录**：
   首先，确保 Docker 配置目录存在。如果不存在，请创建它。

   ```bash
   sudo mkdir -p /etc/systemd/system/docker.service.d
   ```

2. **创建或编辑代理配置文件**：
   创建一个名为 `http-proxy.conf` 的文件，并将代理配置添加到该文件中。

   ```bash
   sudo nano /etc/systemd/system/docker.service.d/http-proxy.conf
   ```

   在文件中添加以下内容，将 `http://proxy.example.com:8080` 替换为您的代理地址和端口：

   ```ini
   [Service]
   Environment="HTTP_PROXY=http://proxy.example.com:8080"
   Environment="HTTPS_PROXY=http://proxy.example.com:8080"
   Environment="NO_PROXY=localhost,127.0.0.1"
   ```

3. **重新加载系统守护进程并重启 Docker 服务**：

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl restart docker
   ```

4. **验证代理配置**：
   您可以通过以下命令验证 Docker 是否正确配置了代理：

   ```bash
   sudo systemctl show --property=Environment docker
   ```

   输出中应该包含您配置的 `HTTP_PROXY` 和 `HTTPS_PROXY` 环境变量。

5. **测试拉取镜像**：
   现在，您可以尝试从 Docker 仓库拉取镜像，以确保代理配置正常工作。

   ```bash
   docker pull ubuntu
   ```

完成以上步骤后，Docker 将使用您配置的代理服务器进行网络请求。这样您就可以通过本地代理拉取 Docker 镜像了。