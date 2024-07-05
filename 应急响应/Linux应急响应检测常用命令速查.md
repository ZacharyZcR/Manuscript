# Linux应急响应检测常用命令速查

### 进程信息检查

#### CPU占用TOP 15
```bash
ps aux | grep -v ^'USER' | sort -rn -k3 | head -15
```
- **解释**: `ps aux` 列出所有进程信息，`grep -v ^'USER'` 排除标题行，`sort -rn -k3` 按照CPU使用率从高到低排序，`head -15` 取前15行。
- **用法**: 检查系统中CPU占用最高的15个进程。

#### 内存占用TOP 15
```bash
ps aux | grep -v ^'USER' | sort -rn -k4 | head -15
```
- **解释**: `ps aux` 列出所有进程信息，`grep -v ^'USER'` 排除标题行，`sort -rn -k4` 按照内存使用率从高到低排序，`head -15` 取前15行。
- **用法**: 检查系统中内存占用最高的15个进程。

#### 父进程为1的进程信息
```bash
ps -e -o user,pid,ppid,cmd | awk '$3 == 1' | egrep -v "containerd-shim|/lib/systemd/systemd|/usr/sbin/cron|dbus|rsyslogd|containerd|/usr/sbin/sshd|/usr/bin/dockerd|/usr/sbin/arpd|/bin/login|/usr/sbin/vnstatd"
```
- **解释**: `ps -e -o user,pid,ppid,cmd` 列出所有进程的用户、PID、PPID和命令，`awk '$3 == 1'` 过滤出父进程ID为1的进程，`egrep -v` 排除常见的系统进程。
- **用法**: 检查由init（PID 1）直接启动的进程，通常这些进程可能是异常的。

#### bash反弹shell进程
```bash
ps -ef | grep -P 'sh -i' | egrep -v 'grep' | awk '{print $2}' | xargs -i{} lsof -p {} | grep 'ESTAB'
```
- **解释**: `ps -ef` 列出所有进程信息，`grep -P 'sh -i'` 查找使用bash反弹shell的进程，`egrep -v 'grep'` 排除grep本身，`awk '{print $2}'` 提取PID，`xargs -i{} lsof -p {}` 列出对应PID的打开文件，`grep 'ESTAB'` 查找处于ESTABLISHED状态的网络连接。
- **用法**: 检测是否存在通过bash反弹的shell进程以及对应的网络连接。

#### SSH 软连接后门进程
```bash
ps -ef | grep -P '\s+\-oport=\d+'
```
- **解释**: `ps -ef` 列出所有进程信息，`grep -P '\s+\-oport=\d+'` 查找匹配SSH软连接后门特征的进程。
- **用法**: 检查是否存在利用SSH软连接后门进行连接的可疑进程。

以下是从代码中提取的常用Linux网络应急响应检测命令，并附有解释和用法：

### 网络/流量检查

#### 网卡信息
```bash
/sbin/ifconfig -a
```
- **解释**: 显示所有网络接口的详细信息。
- **用法**: 检查所有网卡的配置信息，包括IP地址、子网掩码、广播地址等。

#### 网络流量
```bash
awk ' NR>2' /proc/net/dev | while read line; do
  echo "$line" | awk -F ':' '{print "  "$1"  " $2}' | awk '{print $1"   "$2 "    "$3"   "$10"  "$11}'
done
```
- **解释**: 读取`/proc/net/dev`文件中的网络流量信息，过滤掉前两行标题，格式化显示网络接口名称、接收字节数、接收包数、发送字节数和发送包数。
- **用法**: 监控网络接口的流量统计信息。

#### 端口监听
```bash
netstat -tulpen | grep -P 'tcp|udp.*'
```
- **解释**: `netstat -tulpen` 显示所有监听的TCP和UDP端口信息，`grep -P 'tcp|udp.*'` 过滤出TCP和UDP协议的监听端口。
- **用法**: 检查系统中所有正在监听的端口和服务。

#### 对外开放端口
```bash
netstat -tulpen | awk '{print $1,$4}' | grep -P -o '.*0.0.0.0:(\d+)|:::\d+'
```
- **解释**: `netstat -tulpen` 显示所有监听端口信息，`awk '{print $1,$4}'` 提取协议和地址信息，`grep -P -o '.*0.0.0.0:(\d+)|:::\d+'` 过滤出对外开放的端口。
- **用法**: 检查所有对外开放的端口，识别潜在的安全风险。

#### 网络连接
```bash
netstat -antop | grep -P ESTAB
```
- **解释**: `netstat -antop` 显示所有TCP连接信息，`grep -P ESTAB` 过滤出状态为ESTABLISHED的连接。
- **用法**: 检查所有已经建立的TCP连接，监控活动连接状态。

```bash
netstat -anp | grep -P udp
```
- **解释**: `netstat -anp` 显示所有网络连接信息，`grep -P udp` 过滤出所有UDP连接。
- **用法**: 检查系统中的UDP连接，监控活动状态。

#### TCP连接状态
```bash
netstat -n | awk '/^tcp/ {++S[$NF]} END {for(a in S) print a, S[a]}'
```
- **解释**: `netstat -n` 显示所有TCP连接信息，`awk '/^tcp/ {++S[$NF]} END {for(a in S) print a, S[a]}'` 统计不同TCP状态的连接数。
- **用法**: 检查系统中各个TCP连接状态的数量。

#### 路由表
```bash
/sbin/route -nee
```
- **解释**: 显示详细的路由表信息，包括目的地、网关、掩码等。
- **用法**: 检查系统的路由表配置。

#### 路由转发
```bash
more /proc/sys/net/ipv4/ip_forward | awk -F: '{if ($1==1) print "1"}'
```
- **解释**: 检查`/proc/sys/net/ipv4/ip_forward`文件内容，如果值为1表示开启了路由转发。
- **用法**: 确认系统是否开启了路由转发功能。

#### DNS Server
```bash
grep -oP '\d+\.\d+\.\d+\.\d+' </etc/resolv.conf
```
- **解释**: 从`/etc/resolv.conf`文件中提取所有的DNS服务器IP地址。
- **用法**: 检查系统配置的DNS服务器。

#### ARP表
```bash
arp -n -a
```
- **解释**: 显示ARP缓存表信息，`-n` 选项表示以数字格式显示地址，`-a` 显示所有条目。
- **用法**: 检查系统的ARP缓存，识别和诊断网络问题。

#### 网卡混杂模式
```bash
ip link | grep -P PROMISC
```
- **解释**: 检查网卡是否处于混杂模式，`ip link` 显示网络接口状态，`grep -P PROMISC` 过滤出混杂模式的网卡。
- **用法**: 检查是否有网卡处于混杂模式，混杂模式可能被用于网络嗅探。

#### IPTABLES防火墙
```bash
iptables -L
```
- **解释**: 列出当前iptables防火墙规则。
- **用法**: 检查系统配置的防火墙规则，确保没有异常或不安全的规则。

### 任务计划检查

#### 查看Crontab文件
```bash
crontab -u root -l | egrep -v '#'
```
- **解释**: 显示root用户的crontab任务，`egrep -v '#'` 排除注释行。
- **用法**: 检查root用户的计划任务，确保没有异常或恶意的任务。

#### 列出/etc/cron.*目录下的文件
```bash
ls -alht /etc/cron.*/*
```
- **解释**: 列出`/etc/cron.hourly`、`/etc/cron.daily`、`/etc/cron.weekly`和`/etc/cron.monthly`目录下的所有文件，按时间排序并显示详细信息。
- **用法**: 检查系统定时任务目录中的文件，识别是否有可疑文件或脚本。

#### 查看/var/spool/cron目录下的文件内容
```bash
find /var/spool/cron/ -type f -print0 | xargs -0 sudo cat | egrep -v '#'
```
- **解释**: 查找`/var/spool/cron/`目录下的所有文件，并显示其内容，`egrep -v '#'` 排除注释行。
- **用法**: 检查所有用户的crontab任务，确保没有异常或恶意的任务。

### 环境变量检查

#### 查看所有环境变量
```bash
env
```
- **解释**: 列出当前用户会话中的所有环境变量。
- **用法**: 检查所有环境变量，识别是否存在异常或恶意的变量配置。

#### 查看PATH变量
```bash
echo $PATH
```
- **解释**: 显示当前用户的PATH环境变量内容。
- **用法**: 检查PATH变量，确保没有包含不安全的目录。

### Linux 动态链接库变量检查

#### 检查LD_PRELOAD变量
```bash
if [[ -n $LD_PRELOAD ]]; then
  echo "**LD_PRELOAD**"
  echo $LD_PRELOAD
fi
```
- **解释**: 检查是否设置了LD_PRELOAD变量，并输出其内容。
- **用法**: LD_PRELOAD用于指定在程序运行前需要加载的共享库，可能被用于劫持系统调用。

#### 检查LD_ELF_PRELOAD变量
```bash
if [[ -n $LD_ELF_PRELOAD ]]; then
  echo "**LD_ELF_PRELOAD**"
  echo $LD_ELF_PRELOAD
fi
```
- **解释**: 检查是否设置了LD_ELF_PRELOAD变量，并输出其内容。
- **用法**: LD_ELF_PRELOAD用于指定ELF格式程序在加载时需要预加载的库，可能被用于恶意目的。

#### 检查LD_AOUT_PRELOAD变量
```bash
if [[ -n $LD_AOUT_PRELOAD ]]; then
  echo "**LD_AOUT_PRELOAD**"
  echo $LD_AOUT_PRELOAD
fi
```
- **解释**: 检查是否设置了LD_AOUT_PRELOAD变量，并输出其内容。
- **用法**: LD_AOUT_PRELOAD用于指定a.out格式程序在加载时需要预加载的库，可能被用于恶意目的。

#### 检查PROMPT_COMMAND变量
```bash
if [[ -n $PROMPT_COMMAND ]]; then
  echo "**PROMPT_COMMAND**"
  echo $PROMPT_COMMAND
fi
```
- **解释**: 检查是否设置了PROMPT_COMMAND变量，并输出其内容。
- **用法**: PROMPT_COMMAND在每次显示提示符之前执行，可能被用于执行恶意命令。

#### 检查LD_LIBRARY_PATH变量
```bash
if [[ -n $LD_LIBRARY_PATH ]]; then
  echo "**LD_LIBRARY_PATH**"
  echo $LD_LIBRARY_PATH
fi
```
- **解释**: 检查是否设置了LD_LIBRARY_PATH变量，并输出其内容。
- **用法**: LD_LIBRARY_PATH指定了共享库搜索路径，可能被用于加载恶意库。

#### 检查ld.so.preload文件
```bash
preload='/etc/ld.so.preload'
if [ -e "${preload}" ]; then
  echo "**ld.so.preload**"
  cat ${preload}
fi
```
- **解释**: 检查是否存在`/etc/ld.so.preload`文件，并输出其内容。
- **用法**: `ld.so.preload`文件用于指定需要预加载的共享库，可能被用于系统劫持。

### 正在运行的进程环境变量检查
```bash
grep -P 'LD_PRELOAD|LD_ELF_PRELOAD|LD_AOUT_PRELOAD|PROMPT_COMMAND|LD_LIBRARY_PATH' /proc/*/environ
```
- **解释**: 检查所有正在运行的进程的环境变量，过滤出包含指定关键字的变量。
- **用法**: 检查系统中正在运行的进程是否包含可疑的环境变量配置。

### 用户信息检查

#### 可登陆用户
```bash
cat /etc/passwd | egrep -v 'nologin$|false$'
```
- **解释**: 查看`/etc/passwd`文件，过滤掉shell为`nologin`或`false`的用户，列出可以登录系统的用户。
- **用法**: 检查系统中所有有权登录的用户，识别是否有异常用户。

#### 拥有Root权限的非root账号
```bash
cat /etc/passwd | awk -F ':' '$3==0' | egrep -v root:
```
- **解释**: 查看`/etc/passwd`文件，过滤出UID为0且用户名不是root的用户。
- **用法**: 检查是否有除root外拥有超级用户权限的账号，识别潜在的安全风险。

#### /etc/passwd文件修改日期
```bash
stat /etc/passwd | grep -P -o '(?<=Modify: ).*'
```
- **解释**: 使用`stat`命令查看`/etc/passwd`文件的属性，过滤出文件的修改时间。
- **用法**: 检查`/etc/passwd`文件的修改时间，识别是否有异常修改行为。

#### sudoers文件中含有ALL权限的用户（注意NOPASSWD）
```bash
cat /etc/sudoers | egrep -v '#' | sed -e '/^$/d' | grep -P ALL
```
- **解释**: 查看`/etc/sudoers`文件，过滤掉注释行和空行，查找所有具有ALL权限的用户。
- **用法**: 检查sudoers配置，识别具有超级用户权限的账号，特别注意是否有无需密码（NOPASSWD）验证的配置。

#### 当前登录用户信息
```bash
w
```
- **解释**: 显示当前登录的用户信息，包括用户名、登录时间、使用的终端等。
- **用法**: 检查当前登录的用户，识别是否有异常登录活动。

#### 历史登录信息
```bash
last
```
- **解释**: 显示系统上所有用户的登录历史记录。
- **用法**: 检查历史登录信息，识别是否有异常的登录行为。

#### 用户最后登录信息
```bash
lastlog
```
- **解释**: 显示系统上所有用户的最后登录时间。
- **用法**: 检查每个用户的最后登录时间，识别是否有长期未登录或突然登录的用户。

#### 登录IP地址
```bash
grep -i -a Accepted /var/log/secure /var/log/auth.* 2>/dev/null | grep -Po '\d+\.\d+\.\d+\.\d+' | sort | uniq
```
- **解释**: 在`/var/log/secure`和`/var/log/auth.*`日志文件中查找包含"Accepted"的行，提取其中的IP地址，并去重排序。
- **用法**: 检查所有成功登录的IP地址，识别是否有异常IP登录。

### Linux启动项排查

#### 查看/etc/init.d目录下的记录
```bash
ls -alhtR /etc/init.d | head -n 30
```
- **解释**: 递归列出`/etc/init.d`目录及其子目录下的所有文件，按时间排序并显示前30行。
- **用法**: 检查系统启动脚本目录，查看最近修改的文件，识别是否有异常或未授权的修改。

### 服务状态检查

#### 正在运行的Service
```bash
systemctl -l | grep running | awk '{print $1}'
```
- **解释**: 使用`systemctl -l`命令列出所有服务，过滤出状态为`running`的服务，并输出其名称。
- **用法**: 检查当前正在运行的服务，识别是否有异常或未授权的服务在运行。

#### 最近添加的Service
```bash
ls -alhtR /etc/systemd/system/multi-user.target.wants
```
- **解释**: 递归列出`/etc/systemd/system/multi-user.target.wants`目录下的所有文件及其详细信息，按时间排序。
- **用法**: 检查最近添加到该目录中的服务，识别是否有异常或未授权的服务被添加。

```bash
ls -alht /etc/systemd/system/*.service | egrep -v 'dbus-org'
```
- **解释**: 列出`/etc/systemd/system`目录下的所有`.service`文件，按时间排序并显示详细信息，排除文件名中包含`dbus-org`的服务。
- **用法**: 检查最近添加的`.service`文件，识别是否有异常或未授权的服务文件。

### Bash配置检查

#### 查看History文件
```bash
ls -alht /root/.*_history
```
- **解释**: 列出`/root`目录下所有以`.`开头且包含`_history`的文件，按时间排序并显示详细信息。
- **用法**: 检查root用户的历史命令文件，识别最近的命令活动。

#### 检查History中的敏感操作
```bash
cat ~/.*history | grep -P '(?<![0-9])(?:(?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2}))(?![0-9])|http://|https://|\bssh\b|\bscp\b|\.tar|\bwget\b|\bcurl\b|\bnc\b|\btelnet\b|\bbash\b|\bsh\b|\bchmod\b|\bchown\b|/etc/passwd|/etc/shadow|/etc/hosts|\bnmap\b|\bfrp\b|\bnfs\b|\bsshd\b|\bmodprobe\b|\blsmod\b|\bsudo\b|mysql\b|mysqldump' | egrep -v 'man\b|ag\b|cat\b|sed\b|git\b|docker\b|rm\b|touch\b|mv\b|\bapt\b|\bapt-get\b'
```
- **解释**: 检查用户历史命令中是否包含敏感操作命令，并过滤掉常见无害命令。
- **用法**: 检查用户的命令历史，识别是否有潜在的恶意操作或异常活动。

#### 查看/etc/profile文件
```bash
cat /etc/profile | egrep -v '#'
```
- **解释**: 显示`/etc/profile`文件内容，并排除注释行。
- **用法**: 检查系统级的profile配置，识别是否有可疑的环境变量设置或命令。

#### 查看用户的.profile文件
```bash
cat $HOME/.profile | egrep -v '#'
```
- **解释**: 显示当前用户的`.profile`文件内容，并排除注释行。
- **用法**: 检查用户级的profile配置，识别是否有可疑的环境变量设置或命令。

#### 查看/etc/rc.local文件
```bash
cat /etc/rc.local | egrep -v '#'
```
- **解释**: 显示`/etc/rc.local`文件内容，并排除注释行。
- **用法**: 检查系统启动时执行的脚本，识别是否有可疑的启动项。

#### 查看用户的.bash_profile文件
```bash
if [ -e "$HOME/.bash_profile" ]; then
  cat ~/.bash_profile | egrep -v '#'
fi
```
- **解释**: 如果存在`.bash_profile`文件，则显示其内容，并排除注释行。
- **用法**: 检查用户的bash配置文件，识别是否有可疑的命令或配置。

#### 查看用户的.bashrc文件
```bash
cat ~/.bashrc | egrep -v '#' | sort | uniq
```
- **解释**: 显示当前用户的`.bashrc`文件内容，排除注释行，并进行排序和去重。
- **用法**: 检查用户的bash配置文件，识别是否有可疑的命令或配置。

#### 查看用户的.zshrc文件
```bash
cat ~/.zshrc | egrep -v '#' | sort | uniq
```
- **解释**: 显示当前用户的`.zshrc`文件内容，排除注释行，并进行排序和去重。
- **用法**: 检查用户的zsh配置文件，识别是否有可疑的命令或配置。

### 文件检查

#### 系统文件修改时间
```bash
cmdline=(
  "/sbin/ifconfig"
  "/bin/ls"
  "/bin/login"
  "/bin/netstat"
  "/bin/top"
  "/bin/ps"
  "/bin/find"
  "/bin/grep"
  "/etc/passwd"
  "/etc/shadow"
  "/usr/bin/curl"
  "/usr/bin/wget"
  "/root/.ssh/authorized_keys"
)
for soft in "${cmdline[@]}"; do
  echo -e "文件：$soft\t\t\t修改日期：$(stat $soft | grep -P -o '(?<=Modify: )[\d-\s:]+')"
done
```
- **解释**: 检查一组关键系统文件的修改时间。
- **用法**: 确定这些关键文件是否被修改，识别潜在的安全威胁。

#### 查找隐藏文件
```bash
find / ! -path "/proc/*" ! -path "/sys/*" ! -path "/run/*" ! -path "/boot/*" -name ".*."
```
- **解释**: 查找系统中所有名称为`.`开头且以`.`结尾的隐藏文件。
- **用法**: 检查系统中是否有可疑的隐藏文件。

#### 检查/tmp目录
```bash
ls /tmp /var/tmp /dev/shm -alht
```
- **解释**: 列出`/tmp`、`/var/tmp`和`/dev/shm`目录下的所有文件，按时间排序并显示详细信息。
- **用法**: 检查临时目录中的文件，识别是否有可疑文件或脚本。

#### 别名检查
```bash
alias | egrep -v 'git'
```
- **解释**: 显示当前用户的所有别名配置，排除包含`git`的别名。
- **用法**: 检查别名配置，识别是否有可疑的命令别名。

#### SUID文件检查
```bash
find / ! -path "/proc/*" -perm -004000 -type f | egrep -v 'snap|docker|pam_timestamp_check|unix_chkpwd|ping|mount|su|pt_chown|ssh-keysign|at|passwd|chsh|crontab|chfn|usernetctl|staprun|newgrp|chage|dhcp|helper|pkexec|top|Xorg|nvidia-modprobe|quota|login|security_authtrampoline|authopen|traceroute6|traceroute|ps'
```
- **解释**: 查找系统中所有设置了SUID位的文件，并排除常见的合法文件。
- **用法**: 检查是否有可疑的SUID文件，这些文件可能被用于提权攻击。

#### 检查已删除但仍在使用的文件
```bash
lsof +L1
```
- **解释**: 使用`lsof`命令查找所有已删除但仍在使用的文件。
- **用法**: 检查是否有被删除但仍在使用的文件，这些文件可能包含恶意软件或敏感数据。

#### 近7天内修改的文件（mtime）
```bash
find /etc /bin /lib /sbin /dev /root/ /home /tmp /var /usr ! -path "/var/log*" ! -path "/var/spool/exim4*" ! -path "/var/backups*" -mtime -7 -type f | egrep -v '\.log|cache|vim|/share/|/lib/|.zsh|.gem|\.git|LICENSE|README|/_\w+\.\w+|\blogs\b|elasticsearch|nohup|i18n' | xargs -i{} ls -alh {}
```
- **解释**: 查找近7天内修改的文件，并排除常见的无害文件。
- **用法**: 检查系统中最近修改的文件，识别潜在的安全威胁。

#### 近7天内修改的文件（ctime）
```bash
find /etc /bin /lib /sbin /dev /root/ /home /tmp /var /usr ! -path "/var/log*" ! -path "/var/spool/exim4*" ! -path "/var/backups*" -ctime -7 -type f | egrep -v '\.log|cache|vim|/share/|/lib/|.zsh|.gem|\.git|LICENSE|README|/_\w+\.\w+|\blogs\b|elasticsearch|nohup|i18n' | xargs -i{} ls -alh {}
```
- **解释**: 查找近7天内改变权限或所有者的文件，并排除常见的无害文件。
- **用法**: 检查系统中最近改变权限或所有者的文件，识别潜在的安全威胁。

#### 大文件（大于200MB）
```bash
find / ! -path "/proc/*" ! -path "/sys/*" ! -path "/run/*" ! -path "/boot/*" -size +200M -exec ls -alht {} + 2>/dev/null | grep -P '\.gif|\.jpeg|\.jpg|\.png|\.zip|\.tar.gz|\.tgz|\.7z|\.log|\.xz|\.rar|\.bak|\.old|\.sql|\.1|\.txt|\.tar|\.db|/\w+$' | egrep -v 'ib_logfile|ibd|mysql-bin|mysql-slow|ibdata1|overlay2'
```
- **解释**: 查找系统中所有大于200MB的文件，排除常见的无害文件类型。
- **用法**: 检查是否有异常的大文件，识别潜在的恶意活动，如打包和下载数据。

#### 敏感文件检查
```bash
find / ! -path "/lib/modules*" ! -path "/usr/src*" ! -path "/snap*" ! -path "/usr/include/*" -regextype posix-extended -regex '.*sqlmap|.*msfconsole|.*\bncat|.*\bnmap|.*nikto|.*ettercap|.*tunnel\.(php|jsp|asp|py)|.*/nc\b|.*socks.(php|jsp|asp|py)|.*proxy.(php|jsp|asp|py)|.*brook.*|.*frps|.*frpc|.*aircrack|.*hydra|.*miner|.*/ew$' -type f | egrep -v '/lib/python' | xargs -i{} ls -alh {}
```
- **解释**: 查找系统中所有包含常见黑客工具或后门文件的文件。
- **用法**: 检查是否存在恶意工具或后门文件，识别潜在的安全威胁。

#### 可疑黑客文件检查
```bash
find /root /home /opt /tmp /var/ /dev -regextype posix-extended -regex '.*wget|.*curl|.*openssl|.*mysql' -type f 2>/dev/null | xargs -i{} ls -alh {} | egrep -v '/pkgs/|/envs/|overlay2'
```
- **解释**: 查找指定目录中包含常见黑客工具或命令的文件，排除常见无害目录。
- **用法**: 检查是否存在可疑的黑客工具或命令，识别潜在的安全威胁。

### Rootkit检查

#### lsmod 可疑模块
```bash
lsmod | egrep -v 'ablk_helper|ac97_bus|acpi_power_meter|aesni_intel|ahci|ata_generic|ata_piix|auth_rpcgss|binfmt_misc|bluetooth|bnep|bnx2|bridge|cdrom|cirrus|coretemp|crc_t10dif|crc32_pclmul|crc32c_intel|crct10dif_common|crct10dif_generic|crct10dif_pclmul|cryptd|dca|dcdbas|dm_log|dm_mirror|dm_mod|dm_region_hash|drm|drm_kms_helper|drm_panel_orientation_quirks|e1000|ebtable_broute|ebtable_filter|ebtable_nat|ebtables|edac_core|ext4|fb_sys_fops|floppy|fuse|gf128mul|ghash_clmulni_intel|glue_helper|grace|i2c_algo_bit|i2c_core|i2c_piix4|i7core_edac|intel_powerclamp|ioatdma|ip_set|ip_tables|ip6_tables|ip6t_REJECT|ip6t_rpfilter|ip6table_filter|ip6table_mangle|ip6table_nat|ip6table_raw|ip6table_security|ipmi_devintf|ipmi_msghandler|ipmi_si|ipmi_ssif|ipt_MASQUERADE|ipt_REJECT|iptable_filter|iptable_mangle|iptable_nat|iptable_raw|iptable_security|iTCO_vendor_support|iTCO_wdt|jbd2|joydev|kvm|kvm_intel|libahci|libata|libcrc32c|llc|lockd|lpc_ich|lrw|mbcache|megaraid_sas|mfd_core|mgag200|Module|mptbase|mptscsih|mptspi|nf_conntrack|nf_conntrack_ipv4|nf_conntrack_ipv6|nf_defrag_ipv4|nf_defrag_ipv6|nf_nat|nf_nat_ipv4|nf_nat_ipv6|nf_nat_masquerade_ipv4|nfnetlink|nfnetlink_log|nfnetlink_queue|nfs_acl|nfsd|parport|parport_pc|pata_acpi|pcspkr|ppdev|rfkill|sch_fq_codel|scsi_transport_spi|sd_mod|serio_raw|sg|shpchp|snd|snd_ac97_codec|snd_ens1371|snd_page_alloc|snd_pcm|snd_rawmidi|snd_seq|snd_seq_device|snd_seq_midi|snd_seq_midi_event|snd_timer|soundcore|sr_mod|stp|sunrpc|syscopyarea|sysfillrect|sysimgblt|tcp_lp|ttm|tun|uvcvideo|videobuf2_core|videobuf2_memops|videobuf2_vmalloc|videodev|virtio|virtio_balloon|virtio_console|virtio_net|virtio_pci|virtio_ring|virtio_scsi|vmhgfs|vmw_balloon|vmw_vmci|vmw_vsock_vmci_transport|vmware_balloon|vmwgfx|vsock|xfs|xt_CHECKSUM|xt_conntrack|xt_state|raid*|tcpbbr|btrfs|.*diag|psmouse|ufs|linear|msdos|cpuid|veth|xt_tcpudp|xfrm_user|xfrm_algo|xt_addrtype|br_netfilter|input_leds|sch_fq|ib_iser|rdma_cm|iw_cm|ib_cm|ib_core|.*scsi.*|tcp_bbr|pcbc|autofs4|multipath|hfs.*|minix|ntfs|vfat|jfs|usbcore|usb_common|ehci_hcd|uhci_hcd|ecb|crc32c_generic|button|hid|usbhid|evdev|hid_generic|overlay|xt_nat|qnx4|sb_edac|acpi_cpufreq|ixgbe|pf_ring|tcp_htcp|cfg80211|x86_pkg_temp_thermal|mei_me|mei|processor|thermal_sys|lp|enclosure|ses|ehci_pci|igb|i2c_i801|pps_core|isofs|nls_utf8|xt_REDIRECT|xt_multiport|iosf_mbi|qxl|cdc_ether|usbnet|ip6table_raw|skx_edac|intel_rapl|wmi|acpi_pad|ast|i40e|ptp|nfit|libnvdimm|bpfilter|failover|toa|tls|nft_|qemu_fw_cfg'
```
- **解释**: 列出当前加载的内核模块，并排除常见合法模块。
- **用法**: 检查系统中是否有可疑的内核模块加载，识别潜在的Rootkit。

#### Rootkit 内核模块
```bash
kernel=$(grep -E 'hide_tcp4_port|hidden_files|hide_tcp6_port|diamorphine|module_hide|module_hidden|is_invisible|hacked_getdents|hacked_kill|heroin|kernel_unlink|hide_module|find_sys_call_tbl|h4x_delete_module|h4x_getdents64|h4x_kill|h4x_tcp4_seq_show|new_getdents|old_getdents|should_hide_file_name|should_hide_task_name' </proc/kallsyms)
if [ -n "$kernel" ]; then
  echo "存在内核敏感函数！疑似Rootkit内核模块"
  echo "$kernel"
else
  echo "未找到内核敏感函数"
fi
```
- **解释**: 在`/proc/kallsyms`文件中搜索包含常见Rootkit内核函数的条目。
- **用法**: 检查系统中是否存在常见Rootkit内核函数，识别潜在的Rootkit内核模块。

#### 可疑的.ko模块
```bash
find / ! -path '/var/lib/docker/overlay2/*' ! -path '/proc/*' ! -path '/usr/lib/modules/*' ! -path '/lib/modules/*' ! -path '/boot/*' -regextype posix-extended -regex '.*\.ko' | egrep -v 'tutor.ko'
```
- **解释**: 查找系统中所有`.ko`（内核模块）文件，并排除常见的合法文件路径。
- **用法**: 检查系统中是否存在可疑的内核模块文件，识别潜在的Rootkit。

### SSH检查

#### SSH爆破IP
```bash
if [ $OS = 'Centos' ]; then
  grep -P -i -a 'authentication failure' /var/log/secure* | awk '{print $14}' | awk -F '=' '{print $2}' | grep -P '\d+\.\d+\.\d+\.\d+' | sort | uniq -c | sort -nr | head -n 25
else
  grep -P -i -a 'authentication failure' /var/log/auth.* | awk '{print $14}' | awk -F '=' '{print $2}' | grep -P '\d+\.\d+\.\d+\.\d+' | sort | uniq -c | sort -nr | head -n 25
fi
```
- **解释**: 检查SSH认证失败的日志记录，并统计尝试爆破的IP地址，输出最频繁的25个IP地址。根据操作系统的不同，日志文件路径有所不同。
- **用法**: 识别和防范SSH爆破攻击，找到尝试登录的恶意IP地址。

#### 检查SSHD文件的访问、修改和更改时间
```bash
stat /usr/sbin/sshd | grep -P 'Access|Modify|Change'
```
- **解释**: 使用`stat`命令查看`/usr/sbin/sshd`文件的访问、修改和更改时间。
- **用法**: 检查SSHD文件的时间戳，识别是否有异常修改行为。

#### SSH后门配置检查
```bash
if [ -e "$HOME/.ssh/config" ]; then
  grep LocalCommand <~/.ssh/config
  grep ProxyCommand <~/.ssh/config
else
  echo "未发现ssh配置文件"
fi
```
- **解释**: 检查当前用户的`~/.ssh/config`文件，查找是否有设置`LocalCommand`或`ProxyCommand`，这些配置可能被用于后门。
- **用法**: 识别是否有SSH后门配置。

#### PAM后门检查
```bash
ls -la /usr/lib/security 2>/dev/null
ls -la /usr/lib64/security 2>/dev/null
```
- **解释**: 列出`/usr/lib/security`和`/usr/lib64/security`目录下的文件，查看PAM模块是否被篡改。
- **用法**: 检查PAM模块，识别是否存在恶意的PAM后门。

#### SSH inetd后门检查
```bash
if [ -e "/etc/inetd.conf" ]; then
  grep -E '(bash -i)' </etc/inetd.conf
fi
```
- **解释**: 检查`/etc/inetd.conf`文件，查找是否有包含`bash -i`的行，这可能表示存在inetd后门。
- **用法**: 识别是否有通过inetd配置的SSH后门。

#### SSH key检查
```bash
user_dirs=$(ls /home)
for user_dir in $user_dirs; do
  sshkey="/home/${user_dir}/.ssh/authorized_keys"
  if [ -s "${sshkey}" ]; then
    echo "User: ${user_dir}\n"
    cat ${sshkey}
  fi
done
```
- **解释**: 检查每个用户目录下的`.ssh/authorized_keys`文件，输出文件内容。
- **用法**: 识别用户是否有未经授权的SSH公钥。

#### 检查/root目录的authorized_keys文件
```bash
root_sshkey="/root/.ssh/authorized_keys"
if [ -s "${root_sshkey}" ]; then
  cat ${root_sshkey}
else
  echo "User: root - SSH key文件不存在"
fi
```
- **解释**: 检查`/root/.ssh/authorized_keys`文件，输出文件内容。
- **用法**: 识别root用户是否有未经授权的SSH公钥。

### Webshell检查

#### PHP Webshell查杀
```bash
# 使用各种模式匹配查找可疑的PHP文件
grep -P -i -r -l 'array_map\(|pcntl_exec\(|proc_open\(|popen\(|assert\(|phpspy|c99sh|milw0rm|eval?\(|\(gunerpress|\(base64_decoolcode|spider_bc|shell_exec\(|passthru\(|base64_decode\s?\(|gzuncompress\s?\(|gzinflate|\(\$\$\w+|call_user_func\(|call_user_func_array\(|preg_replace_callback\(|preg_replace\(|register_shutdown_function\(|register_tick_function\(|mb_ereg_replace_callback\(|filter_var\(|ob_start\(|usort\(|uksort\(|uasort\(|GzinFlate\s?\(|\$\w+\(\d+\)\.\$\w+\(\d+\)\.|\$\w+=str_replace\(|eval\/\*.*\*\/\(' $webpath --include='*.php*' --include='*.phtml'
grep -P -i -r -l '^(\xff\xd8|\x89\x50|GIF89a|GIF87a|BM|\x00\x00\x01\x00\x01)[\s\S]*<\?\s*php' $webpath --include='*.php*' --include='*.phtml'
grep -P -i -r -l '\b(assert|eval|system|exec|shell_exec|passthru|popen|proc_open|pcntl_exec)\b[\/*\s]*\(+[\/*\s]*((\$_(GET|POST|REQUEST|COOKIE)\[.{0,25})|(base64_decode|gzinflate|gzuncompress|gzdecode|str_rot13)[\s\(]*(\$_(GET|POST|REQUEST|COOKIE)\[.{0,25}))' $webpath --include='*.php*' --include='*.phtml'
grep -P -i -r -l '\$\s*(\w+)\s*=[\s\(\{]*(\$_(GET|POST|REQUEST|COOKIE)\[.{0,25});[\s\S]{0,200}\b(assert|eval|system|exec|shell_exec|passthru|popen|proc_open|pcntl_exec)\b[\/*\s]*\(+[\s"\/*]*(\$\s*\1|((base64_decode|gzinflate|gzuncompress|gzdecode|str_rot13)[\s\("]*\$\s*\1))' $webpath --include='*.php*' --include='*.phtml'
grep -P -i -r -l '\b(filter_var|filter_var_array)\b\s*\(.*FILTER_CALLBACK[^;]*((\$_(GET|POST|REQUEST|COOKIE|SERVER)\[.{0,25})|(eval|assert|ass\\x65rt|system|exec|shell_exec|passthru|popen|proc_open|pcntl_exec))' $webpath --include='*.php*' --include='*.phtml'
grep -P -i -r -l "\b(assert|eval|system|exec|shell_exec|passthru|popen|proc_open|pcntl_exec|include)\b\s*\(\s*(file_get_contents\s*\(\s*)?[\'\"]php:\/\/input" $webpath --include='*.php*' --include='*.phtml'
grep -P -i -r -l 'getruntime|processimpl|processbuilder|defineclass|classloader|naming.lookup|internaldofilter|elprocessor|scriptenginemanager|urlclassloader|versionhelper|registermapping|registerhandler|detecthandlermethods|\\u0063\\u006c\\u0061\\u0073\\u0073' $webpath --include='*.php*' --include='*.phtml'
grep -P -i -r -l 'phpinfo|move_uploaded_file|system|shell_exec|passthru|popen|proc_open|pcntl_exec|call_user_func|ob_start' $webpath --include='*.php*' --include='*.phtml'
grep -P -i -r -l 'array_map|uasort|uksort|array_diff_uassoc|array_diff_ukey|array_intersect_uassoc|array_intersect_ukey|array_reduce|array_filter|array_udiff|array_udiff_assoc|array_udiff_uassoc|array_uintersect|array_uintersect_assoc|array_uintersect_uassoc|array_walk|array_walk_recursive|register_shutdown_function|register_tick_function|filter_var_array|yaml_parse|sqlite_create_function|fgetc|fgets|fgetss|fpassthru|fread|file_get_contents|readfile|stream_get_contents|stream_get_line|highlight_file|show_source|file_put_contents|pfsockopen|fsockopen' $webpath --include='*.php*' --include='*.phtml'
```
- **解释**: 通过多种模式匹配规则搜索PHP文件中的可疑代码和常见Webshell特征。
- **用法**: 在指定的web路径中查找可能存在的PHP Webshell文件。

#### JSP Webshell查杀
```bash
# 使用模式匹配查找可疑的JSP文件
grep -P -i -r -l '<%@\spage\simport=[\s\S]*\\u00\d+\\u00\d+|<%@\spage\simport=[\s\S]*Runtime.getRuntime\(\).exec\(request.getParameter\(|Runtime.getRuntime\(\)' $webpath --include='*.jsp*' --include='*.jhtml'
```
- **解释**: 使用正则表达式搜索JSP文件中的可疑代码和常见Webshell特征。
- **用法**: 在指定的web路径中查找可能存在的JSP Webshell文件。

### 供应链投毒检测

#### Python2 pip 检测
```bash
pip freeze | grep -P 'istrib|djanga|easyinstall|junkeldat|libpeshka|mumpy|mybiubiubiu|nmap-python|openvc|python-ftp|pythonkafka|python-mongo|python-mysql|python-mysqldb|python-openssl|python-sqlite|virtualnv|mateplotlib|request=|aioconsol'
```
- **解释**: 使用`pip freeze`命令列出已安装的Python2包，并使用`grep -P`命令筛选出名称中包含可疑字符串的包。
- **用法**: 检查已安装的Python2包中是否包含可能存在投毒风险的包。

#### Python3 pip 检测
```bash
pip3 freeze | grep -P 'istrib|djanga|easyinstall|junkeldat|libpeshka|mumpy|mybiubiubiu|nmap-python|openvc|python-ftp|pythonkafka|python-mongo|python-mysql|python-mysqldb|python-openssl|python-sqlite|virtualnv|mateplotlib|request=|aioconsol'
```
- **解释**: 使用`pip3 freeze`命令列出已安装的Python3包，并使用`grep -P`命令筛选出名称中包含可疑字符串的包。
- **用法**: 检查已安装的Python3包中是否包含可能存在投毒风险的包。

### 挖矿木马检查

#### 常规挖矿进程检测
```bash
ps aux | grep -P "systemctI|kworkerds|init10.cfg|wl.conf|crond64|watchbog|sustse|donate|proxkekman|test.conf|/var/tmp/apple|/var/tmp/big|/var/tmp/small|/var/tmp/cat|/var/tmp/dog|/var/tmp/mysql|/var/tmp/sishen|ubyx|cpu.c|tes.conf|psping|/var/tmp/java-c|pscf|cryptonight|sustes|xmrig|xmr-stak|suppoie|ririg|/var/tmp/ntpd|/var/tmp/ntp|/var/tmp/qq|/tmp/qq|/var/tmp/aa|gg1.conf|hh1.conf|apaqi|dajiba|/var/tmp/look|/var/tmp/nginx|dd1.conf|kkk1.conf|ttt1.conf|ooo1.conf|ppp1.conf|lll1.conf|yyy1.conf|1111.conf|2221.conf|dk1.conf|kd1.conf|mao1.conf|YB1.conf|2Ri1.conf|3Gu1.conf|crant|nicehash|linuxs|linuxl|Linux|crawler.weibo|stratum|gpg-daemon|jobs.flu.cc|cranberry|start.sh|watch.sh|krun.sh|killTop.sh|cpuminer|/60009|ssh_deny.sh|clean.sh|\./over|mrx1|redisscan|ebscan|barad_agent|\.sr0|clay|udevs|\.sshd|/tmp/init|xmr|xig|ddgs|minerd|hashvault|geqn|\.kthreadd|httpdz|pastebin.com|sobot.com|kerbero|2t3ik|ddgs|qW3xt|ztctb|i2pd" | egrep -v 'grep'
```
- **解释**: 使用`ps aux`命令列出所有进程，并通过`grep -P`命令筛选出名称中包含常见挖矿木马特征的进程。
- **用法**: 检查系统中是否有常见的挖矿进程在运行。

```bash
find / ! -path "/proc/*" ! -path "/sys/*" ! -path "/run/*" ! -path "/boot/*" -regextype posix-extended -regex '.*systemctI|.*kworkerds|.*init10.cfg|.*wl.conf|.*crond64|.*watchbog|.*sustse|.*donate|.*proxkekman|.*cryptonight|.*sustes|.*xmrig|.*xmr-stak|.*suppoie|.*ririg|gg1.conf|.*cpuminer|.*xmr|.*xig|.*ddgs|.*minerd|.*hashvault|\.kthreadd|.*httpdz|.*kerbero|.*2t3ik|.*qW3xt|.*ztctb|.*miner.sh' -type f
```
- **解释**: 使用`find`命令查找系统中包含常见挖矿木马特征的文件。
- **用法**: 检查系统中是否存在常见挖矿木马文件。

#### Ntpclient 挖矿木马检测
```bash
find / ! -path "/proc/*" ! -path "/sys/*" ! -path "/boot/*" -regextype posix-extended -regex 'ntpclient|Mozz'
```
- **解释**: 使用`find`命令查找系统中包含`ntpclient`或`Mozz`特征的文件。
- **用法**: 检查系统中是否存在与`ntpclient`或`Mozz`相关的挖矿木马文件。

```bash
ls -alh /tmp/.a /var/tmp/.a /run/shm/a /dev/.a /dev/shm/.a 2>/dev/null
```
- **解释**: 列出一些常见的隐藏挖矿木马文件路径，显示详细信息。
- **用法**: 检查系统中是否存在常见的隐藏挖矿木马文件。

#### WorkMiner 挖矿木马检测
```bash
ps aux | grep -P "work32|work64|/tmp/secure.sh|/tmp/auth.sh" | egrep -v 'grep'
```
- **解释**: 使用`ps aux`命令列出所有进程，并通过`grep -P`命令筛选出名称中包含`work32`、`work64`、`/tmp/secure.sh`或`/tmp/auth.sh`特征的进程。
- **用法**: 检查系统中是否有与WorkMiner相关的挖矿进程在运行。

```bash
ls -alh /tmp/xmr /tmp/config.json /tmp/secure.sh /tmp/auth.sh /usr/.work/work64 2>/dev/null
```
- **解释**: 列出一些常见的WorkMiner挖矿木马文件路径，显示详细信息。
- **用法**: 检查系统中是否存在常见的WorkMiner挖矿木马文件。

### 服务器风险/漏洞检查

#### Redis弱密码检测
```bash
cat /etc/redis/redis.conf 2>/dev/null | grep -P '(?<=requirepass )(test|123456|admin|root|12345678|111111|p@ssw0rd|test|qwerty|zxcvbnm|123123|12344321|123qwe|password|1qaz|000000|666666|888888)'
```
- **解释**: 检查Redis配置文件中的密码设置是否为常见的弱密码。
- **用法**: 识别并修复Redis配置中的弱密码，防止未经授权的访问。

#### JDWP调试检测
```bash
if ps aux | grep -P '(?:runjdwp|agentlib:jdwp)' | egrep -v 'grep' >/dev/null 2>&1; then
  echo "存在JDWP调试高风险进程\n $(ps aux | grep -P '(?:runjdwp|agentlib:jdwp)' | egrep -v 'grep') "
fi
```
- **解释**: 检查系统中是否有启用了JDWP（Java Debug Wire Protocol）调试的进程，这可能会带来高风险。
- **用法**: 识别并终止不必要的JDWP调试进程，防止潜在的安全漏洞。

#### Python http.server 列目录检测
```bash
ps aux | grep -P http.server | egrep -v 'grep'
```
- **解释**: 检查系统中是否有使用`http.server`模块启动的Python简单HTTP服务器，这可能会暴露目录结构。
- **用法**: 识别并终止不必要的Python HTTP服务器，防止目录泄露。

### Docker信息检测

#### Docker运行的镜像
```bash
docker ps
```
- **解释**: 使用`docker ps`命令列出当前运行的Docker容器及其相关信息。
- **用法**: 检查当前正在运行的Docker容器，识别可能存在的异常或未授权容器。

#### 检测CAP_SYS_ADMIN权限
```bash
if command -v capsh >/dev/null 2>&1; then
  cap_sys_adminNum=$(capsh --print | grep cap_sys_admin | wc -l)
  if [ $cap_sys_adminNum -gt 0 ]; then
    echo "存在CAP_SYS_ADMIN权限！"
  fi
else
  echo "未发现capsh命令！"
fi
```
- **解释**: 使用`capsh --print`命令查看当前进程的能力（capabilities），并检查是否存在CAP_SYS_ADMIN权限。
- **用法**: 识别当前进程是否具有CAP_SYS_ADMIN权限，这是一个高危权限，可能带来安全风险。

#### 检测CAP_DAC_READ_SEARCH权限
```bash
if command -v capsh >/dev/null 2>&1; then
  cap_dac_read_searchNum=$(capsh --print | grep cap_dac_read_search | wc -l)
  if [ $cap_dac_read_searchNum -gt 0 ]; then
    echo "存在CAP_DAC_READ_SEARCH！"
  fi
else
  echo "未发现capsh命令！"
fi
```
- **解释**: 使用`capsh --print`命令查看当前进程的能力（capabilities），并检查是否存在CAP_DAC_READ_SEARCH权限。
- **用法**: 识别当前进程是否具有CAP_DAC_READ_SEARCH权限，这个权限允许进程绕过文件读取权限检查，可能带来安全风险。
