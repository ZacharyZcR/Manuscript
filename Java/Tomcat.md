Tomcat 是一个开源的 Servlet 容器，也是一个 Web 服务器，主要用于运行 Java Servlets 和呈现动态 Web 内容。以下是 Tomcat 的搭建和使用详解：

### 一、环境准备

1. **安装 Java Development Kit (JDK)**

   Tomcat 依赖于 JDK 环境，因此需要先安装 JDK。
   - 从 [Oracle](https://www.oracle.com/java/technologies/javase-downloads.html) 或 [OpenJDK](https://jdk.java.net/) 下载并安装 JDK。
   - 配置环境变量：
     ```sh
     export JAVA_HOME=/path/to/jdk
     export PATH=$JAVA_HOME/bin:$PATH
     ```

2. **下载 Tomcat**

   从 Apache Tomcat 官方网站下载所需版本的 Tomcat：
   - [Tomcat 下载页面](https://tomcat.apache.org/download-90.cgi)

### 二、安装和配置 Tomcat

1. **解压 Tomcat**

   将下载的 Tomcat 压缩包解压到你想要安装的目录：
   ```sh
   tar -xvzf apache-tomcat-9.x.x.tar.gz
   mv apache-tomcat-9.x.x /usr/local/tomcat
   ```

2. **配置环境变量**

   设置 Tomcat 的环境变量以便于启动和停止 Tomcat：
   ```sh
   export CATALINA_HOME=/usr/local/tomcat
   export PATH=$CATALINA_HOME/bin:$PATH
   ```

### 三、启动和停止 Tomcat

1. **启动 Tomcat**

   进入 Tomcat 的 `bin` 目录，并运行启动脚本：
   ```sh
   cd $CATALINA_HOME/bin
   ./startup.sh
   ```

   你可以通过浏览器访问 `http://localhost:8080` 来检查 Tomcat 是否成功启动。

2. **停止 Tomcat**

   进入 Tomcat 的 `bin` 目录，并运行停止脚本：
   ```sh
   cd $CATALINA_HOME/bin
   ./shutdown.sh
   ```

### 四、部署 Web 应用

1. **手动部署**

   将你的 `.war` 文件复制到 `webapps` 目录中，Tomcat 会自动解压并部署该应用：
   ```sh
   cp yourapp.war $CATALINA_HOME/webapps/
   ```

2. **使用 Tomcat Manager**

   Tomcat 提供了一个 Web 界面来管理和部署应用，访问 `http://localhost:8080/manager/html` 并使用管理员账号登录（默认情况下需要配置 `tomcat-users.xml` 文件来设置管理员账号）。

### 五、配置 Tomcat

1. **修改默认端口**

   编辑 `conf/server.xml` 文件，找到以下内容并修改端口号：
   ```xml
   <Connector port="8080" protocol="HTTP/1.1"
              connectionTimeout="20000"
              redirectPort="8443" />
   ```

2. **设置环境变量**

   在 `bin/setenv.sh` 文件中配置自定义的环境变量：
   ```sh
   export JAVA_OPTS="-Xms512m -Xmx1024m"
   ```

3. **配置用户**

   编辑 `conf/tomcat-users.xml` 文件，添加管理员用户：
   ```xml
   <user username="admin" password="password" roles="manager-gui,admin-gui"/>
   ```

### 六、日志管理

1. **查看日志**

   Tomcat 的日志存储在 `logs` 目录中，包括启动日志、应用日志等。常用日志文件：
   - `catalina.out`：主日志文件，包含 Tomcat 启动和运行时的信息。
   - `localhost_access_log.*.txt`：访问日志，记录所有 HTTP 请求。

2. **配置日志**

   编辑 `conf/logging.properties` 文件来配置日志级别和输出格式。

### 七、性能调优

1. **调整线程池**

   修改 `conf/server.xml` 文件中连接器的线程配置：
   ```xml
   <Connector port="8080" protocol="HTTP/1.1"
              maxThreads="200"
              minSpareThreads="25"
              maxSpareThreads="75"
              ...
   ```

2. **优化内存设置**

   在 `bin/setenv.sh` 文件中调整 JVM 参数以优化内存使用：
   ```sh
   export JAVA_OPTS="-Xms1024m -Xmx2048m -XX:MaxPermSize=256m"
   ```

### 八、安全配置

1. **启用 HTTPS**

   配置 SSL 证书并修改 `conf/server.xml` 文件启用 HTTPS：
   ```xml
   <Connector port="8443" protocol="org.apache.coyote.http11.Http11NioProtocol"
              maxThreads="150" SSLEnabled="true">
       <SSLHostConfig>
           <Certificate certificateKeystoreFile="conf/keystore.jks"
                        type="RSA" />
       </SSLHostConfig>
   </Connector>
   ```

2. **防止目录列出**

   在 `conf/web.xml` 文件中禁用目录列出：
   ```xml
   <init-param>
       <param-name>listings</param-name>
       <param-value>false</param-value>
   </init-param>
   ```

3. **限制访问权限**

   编辑 `conf/tomcat-users.xml` 文件，设置不同角色和用户的访问权限。

### 总结

通过上述步骤，你可以成功搭建和使用 Tomcat 服务器。根据具体需求，你还可以进行更多高级配置和优化。Tomcat 的官方文档也提供了详尽的信息，推荐参考官方文档获取更多帮助。