在Ubuntu中配置APT使用本地代理，可以通过以下步骤来实现：

1. **安装代理软件（如Squid）**

   如果你没有本地代理服务器，可以安装并配置Squid作为本地代理。

   ```bash
   sudo apt update
   sudo apt install squid
   ```

2. **配置Squid**

   编辑Squid配置文件 `/etc/squid/squid.conf`，添加或修改以下配置项：

   ```plaintext
   acl localnet src 192.168.1.0/24
   http_access allow localnet
   ```

   确保你使用了正确的网络范围，保存并关闭文件。

   然后，重启Squid服务以应用更改：

   ```bash
   sudo systemctl restart squid
   ```

3. **配置APT使用代理**

   编辑或创建APT配置文件 `/etc/apt/apt.conf.d/01proxy`，并添加以下行：

   ```plaintext
   Acquire::http::Proxy "http://your.proxy.server:port/";
   Acquire::https::Proxy "https://your.proxy.server:port/";
   ```

   将 `your.proxy.server` 替换为你的代理服务器地址，将 `port` 替换为你的代理端口号。

4. **验证配置**

   运行以下命令，检查APT是否通过代理服务器进行连接：

   ```bash
   sudo apt update
   ```

   如果配置正确，APT应能通过代理服务器访问并更新软件包列表。

通过上述步骤，Ubuntu的APT就会通过配置的本地代理进行网络访问，从而可以加速下载速度，减少外网流量使用。