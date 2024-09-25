#!/bin/bash

# Tomcat一键部署脚本

# 更新系统并安装Java
sudo apt update
sudo apt install -y openjdk-11-jdk

# 下载Tomcat
TOMCAT_VERSION="9.0.91"
wget https://dlcdn.apache.org/tomcat/tomcat-9/v${TOMCAT_VERSION}/bin/apache-tomcat-${TOMCAT_VERSION}.tar.gz

# 解压Tomcat
tar zxvf apache-tomcat-${TOMCAT_VERSION}.tar.gz

# 移动Tomcat到/opt目录
sudo mv apache-tomcat-${TOMCAT_VERSION} /opt/tomcat

# 创建tomcat用户
sudo useradd -r -m -U -d /opt/tomcat -s /bin/false tomcat

# 设置权限
sudo chown -R tomcat: /opt/tomcat
sudo sh -c 'chmod +x /opt/tomcat/bin/*.sh'

# 创建systemd服务文件
cat << EOF | sudo tee /etc/systemd/system/tomcat.service
[Unit]
Description=Apache Tomcat Web Application Container
After=network.target

[Service]
Type=forking
Environment=JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
Environment=CATALINA_PID=/opt/tomcat/temp/tomcat.pid
Environment=CATALINA_HOME=/opt/tomcat
Environment=CATALINA_BASE=/opt/tomcat
Environment='CATALINA_OPTS=-Xms512M -Xmx1024M -server -XX:+UseParallelGC'
ExecStart=/opt/tomcat/bin/startup.sh
ExecStop=/opt/tomcat/bin/shutdown.sh
User=tomcat
Group=tomcat
UMask=0007
RestartSec=10
Restart=always

[Install]
WantedBy=multi-user.target
EOF

# 重新加载systemd，启动并启用Tomcat服务
sudo systemctl daemon-reload
sudo systemctl start tomcat
sudo systemctl enable tomcat

# 配置防火墙（如果启用）
sudo ufw allow 8080/tcp

# 清理下载文件
rm apache-tomcat-${TOMCAT_VERSION}.tar.gz

echo "Tomcat安装完成！请访问http://your_server_ip:8080检查安装结果。"