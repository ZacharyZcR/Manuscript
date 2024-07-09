在Ubuntu上安装Java 1.8 JDK可以通过以下步骤完成：

### 步骤 1：更新包索引
首先，更新系统的包索引：

```bash
sudo apt update
```

### 步骤 2：安装Java 1.8 JDK
使用以下命令安装Java 1.8 JDK：

```bash
sudo apt install openjdk-8-jdk
```

### 步骤 3：验证安装
安装完成后，验证Java 1.8 JDK是否安装成功：

```bash
java -version
```

应该看到类似以下的输出：

```plaintext
openjdk version "1.8.0_xxx"
OpenJDK Runtime Environment (build 1.8.0_xxx-xxx)
OpenJDK 64-Bit Server VM (build 25.171-b11, mixed mode)
```

### 步骤 4：配置默认Java版本（可选）
如果系统上有多个Java版本，可以使用`update-alternatives`命令配置默认的Java版本。

首先，更新`update-alternatives`系统：

```bash
sudo update-alternatives --config java
```

会显示一个列表，选择Java 1.8对应的编号。

### 步骤 5：设置环境变量（可选）
你可以通过修改`/etc/profile`或`~/.bashrc`文件来设置Java环境变量。

打开文件：

```bash
nano ~/.bashrc
```

添加以下行：

```bash
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=$PATH:$JAVA_HOME/bin
```

保存文件并使更改生效：

```bash
source ~/.bashrc
```

完成这些步骤后，Java 1.8 JDK应该已经成功安装并配置在你的Ubuntu系统上了。