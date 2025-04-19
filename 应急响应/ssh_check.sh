#!/bin/bash
# SSH配置安全检测脚本 v1.0
# 支持Linux、macOS和BSD系统

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # 无颜色

# 清屏
clear

# 标题
echo -e "${CYAN}====================================================${NC}"
echo -e "${WHITE}          SSH配置安全性检测工具 v1.0              ${NC}"
echo -e "${CYAN}====================================================${NC}"
echo -e "${GREEN}[+] 执行时间: $(date '+%Y-%m-%d %H:%M:%S')${NC}\n"

# 检测操作系统类型
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="Linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macOS"
    elif [[ "$OSTYPE" == "freebsd"* ]]; then
        OS="FreeBSD"
    elif [[ "$OSTYPE" == "openbsd"* ]]; then
        OS="OpenBSD"
    else
        OS="Unknown"
    fi
    echo $OS
}

OS=$(detect_os)
echo -e "${GREEN}[+] 检测到操作系统: ${OS}${NC}\n"

# 查找SSH配置文件
find_ssh_config() {
    local config_paths=(
        "/etc/ssh/sshd_config"
        "/usr/local/etc/ssh/sshd_config"
        "/usr/local/etc/sshd_config"
        "/etc/sshd_config"
    )

    for path in "${config_paths[@]}"; do
        if [ -f "$path" ]; then
            echo "$path"
            return 0
        fi
    done

    echo ""
    return 1
}

SSH_CONFIG=$(find_ssh_config)

if [ -z "$SSH_CONFIG" ]; then
    echo -e "${RED}[-] 无法找到SSH配置文件。请确认SSH是否已安装或手动指定配置文件路径。${NC}"
    echo -e "${YELLOW}[!] 用法: $0 [SSH配置文件路径]${NC}"
    exit 1
elif [ ! -r "$SSH_CONFIG" ]; then
    echo -e "${RED}[-] 无法读取SSH配置文件: ${SSH_CONFIG}${NC}"
    echo -e "${YELLOW}[!] 请尝试使用root权限或sudo运行此脚本${NC}"
    exit 1
else
    echo -e "${GREEN}[+] 找到SSH配置文件: ${SSH_CONFIG}${NC}"
fi

# 检查SSH服务器版本
check_ssh_version() {
    if command -v ssh -V &> /dev/null; then
        echo -e "${BLUE}[*] SSH服务器版本:${NC}"
        ssh -V 2>&1 | awk '{print "  "$0}'
    elif command -v sshd -V &> /dev/null; then
        echo -e "${BLUE}[*] SSH服务器版本:${NC}"
        sshd -V 2>&1 | awk '{print "  "$0}'
    else
        echo -e "${YELLOW}[!] 无法检测SSH版本，命令不可用${NC}"
    fi
}

# 检查配置文件权限
check_file_permissions() {
    echo -e "\n${BLUE}[*] 配置文件权限:${NC}"

    local perm=$(stat -c "%a" "$SSH_CONFIG" 2>/dev/null || stat -f "%Lp" "$SSH_CONFIG" 2>/dev/null)
    local owner=$(stat -c "%U" "$SSH_CONFIG" 2>/dev/null || stat -f "%Su" "$SSH_CONFIG" 2>/dev/null)

    echo -e "  文件: ${SSH_CONFIG}"
    echo -e "  权限: ${perm}"
    echo -e "  所有者: ${owner}"

    if [[ "$perm" == "600" || "$perm" == "644" ]] && [[ "$owner" == "root" ]]; then
        echo -e "  ${GREEN}[✓] 文件权限配置正确${NC}"
    else
        echo -e "  ${RED}[✗] 文件权限配置不安全${NC}"
        echo -e "  ${YELLOW}[!] 建议: 将权限设置为600或644，且所有者为root${NC}"
        echo -e "  ${YELLOW}[!] 执行: chmod 600 ${SSH_CONFIG} && chown root ${SSH_CONFIG}${NC}"
    fi

    # 检查密钥文件权限
    echo -e "\n${BLUE}[*] SSH密钥文件权限:${NC}"

    local host_keys=$(grep -l "BEGIN .* HOST KEY" /etc/ssh/*.pub 2>/dev/null)
    if [ -n "$host_keys" ]; then
        for key in $host_keys; do
            private_key=${key%.pub}
            if [ -f "$private_key" ]; then
                local key_perm=$(stat -c "%a" "$private_key" 2>/dev/null || stat -f "%Lp" "$private_key" 2>/dev/null)
                local key_owner=$(stat -c "%U" "$private_key" 2>/dev/null || stat -f "%Su" "$private_key" 2>/dev/null)

                echo -e "  密钥: ${private_key}"
                echo -e "  权限: ${key_perm}"
                echo -e "  所有者: ${key_owner}"

                if [[ "$key_perm" == "600" || "$key_perm" == "400" ]] && [[ "$key_owner" == "root" ]]; then
                    echo -e "  ${GREEN}[✓] 密钥权限配置正确${NC}"
                else
                    echo -e "  ${RED}[✗] 密钥权限配置不安全${NC}"
                    echo -e "  ${YELLOW}[!] 建议: 将权限设置为600或400，且所有者为root${NC}"
                    echo -e "  ${YELLOW}[!] 执行: chmod 600 ${private_key} && chown root ${private_key}${NC}"
                fi
            fi
        done
    else
        echo -e "  ${YELLOW}[!] 未找到SSH主机密钥文件${NC}"
    fi
}

# 核心安全配置检查
check_security_config() {
    echo -e "\n${BLUE}[*] 核心安全配置检查:${NC}"

    # 创建函数检查配置项
    check_config() {
        local param="$1"
        local recommended="$2"
        local recommendation="$3"
        local severity="$4"  # high, medium, low

        # 转义特殊字符以用于grep
        local grep_param=$(echo "$param" | sed 's/[\/&]/\\&/g')

        # 获取参数当前值（排除注释行）
        local current_value=$(grep -E "^[[:space:]]*$grep_param[[:space:]]+" "$SSH_CONFIG" | awk '{print $2}')

        # 获取包含参数的行（用于显示）
        local config_line=$(grep -E "^[[:space:]]*$grep_param[[:space:]]+" "$SSH_CONFIG")

        # 如果找不到，检查是否被注释掉了
        if [ -z "$current_value" ]; then
            local commented=$(grep -E "^[[:space:]]*#[[:space:]]*$grep_param[[:space:]]+" "$SSH_CONFIG" | head -1)
            if [ -n "$commented" ]; then
                echo -e "  ${YELLOW}[$param]${NC} 已注释: ${commented}"
                echo -e "  ${YELLOW}[!] 建议: $recommendation${NC}"
            else
                echo -e "  ${YELLOW}[$param]${NC} 未设置 (使用默认值)"
                echo -e "  ${YELLOW}[!] 建议: $recommendation${NC}"
            fi
        else
            if [ "$current_value" == "$recommended" ]; then
                echo -e "  ${GREEN}[✓] [$param]${NC} 设置正确: $config_line"
            else
                case "$severity" in
                    "high")
                        echo -e "  ${RED}[✗] [$param]${NC} 当前值: $config_line"
                        ;;
                    "medium")
                        echo -e "  ${YELLOW}[!] [$param]${NC} 当前值: $config_line"
                        ;;
                    "low")
                        echo -e "  ${BLUE}[i] [$param]${NC} 当前值: $config_line"
                        ;;
                esac
                echo -e "  ${YELLOW}[!] 建议: $recommendation${NC}"
            fi
        fi
    }

    # 检查核心安全参数
    check_config "Protocol" "2" "使用 'Protocol 2' 以禁用不安全的SSH协议版本1" "high"
    check_config "PermitRootLogin" "no" "使用 'PermitRootLogin no' 禁止root直接登录" "high"
    check_config "PasswordAuthentication" "no" "使用 'PasswordAuthentication no' 禁用密码认证，强制使用密钥认证" "medium"
    check_config "PubkeyAuthentication" "yes" "使用 'PubkeyAuthentication yes' 启用公钥认证" "high"
    check_config "PermitEmptyPasswords" "no" "使用 'PermitEmptyPasswords no' 禁止空密码" "high"
    check_config "MaxAuthTries" "4" "使用 'MaxAuthTries 4' 或更低值限制认证尝试次数" "medium"
    check_config "X11Forwarding" "no" "使用 'X11Forwarding no' 禁用X11转发" "low"
    check_config "UsePAM" "yes" "使用 'UsePAM yes' 启用PAM认证" "medium"
    check_config "TCPKeepAlive" "no" "使用 'TCPKeepAlive no' 提高安全性" "low"
    check_config "ClientAliveInterval" "300" "使用 'ClientAliveInterval 300' 设置5分钟无活动超时" "medium"
    check_config "ClientAliveCountMax" "2" "使用 'ClientAliveCountMax 2' 限制无响应次数" "medium"
    check_config "LoginGraceTime" "60" "使用 'LoginGraceTime 60' 限制登录时间" "low"
    check_config "StrictModes" "yes" "使用 'StrictModes yes' 检查权限" "medium"
    check_config "IgnoreRhosts" "yes" "使用 'IgnoreRhosts yes' 禁用不安全的.rhosts文件" "medium"
    check_config "HostbasedAuthentication" "no" "使用 'HostbasedAuthentication no' 禁用基于主机的认证" "medium"
    check_config "LogLevel" "VERBOSE" "使用 'LogLevel VERBOSE' 提高日志详细度" "low"
}

# 检查密码和加密算法配置
check_crypto_config() {
    echo -e "\n${BLUE}[*] 密码和加密算法配置:${NC}"

    # 推荐的安全配置
    local recommended_ciphers="chacha20-poly1305@openssh.com,aes256-gcm@openssh.com,aes128-gcm@openssh.com,aes256-ctr,aes192-ctr,aes128-ctr"
    local recommended_macs="hmac-sha2-512-etm@openssh.com,hmac-sha2-256-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-512,hmac-sha2-256"
    local recommended_kex="curve25519-sha256@libssh.org,diffie-hellman-group-exchange-sha256,diffie-hellman-group16-sha512,diffie-hellman-group18-sha512"

    # 检查Ciphers配置
    local current_ciphers=$(grep -E "^[[:space:]]*Ciphers[[:space:]]+" "$SSH_CONFIG" | cut -d ' ' -f 2-)
    if [ -z "$current_ciphers" ]; then
        echo -e "  ${YELLOW}[!] Ciphers 未配置，使用默认值${NC}"
        echo -e "  ${YELLOW}[!] 建议: 添加以下配置${NC}"
        echo -e "  ${YELLOW}[!] Ciphers ${recommended_ciphers}${NC}"
    else
        echo -e "  ${BLUE}[i] 当前Ciphers配置:${NC}"
        echo -e "  ${current_ciphers}"

        # 检查是否包含不安全的密码
        if [[ "$current_ciphers" == *"3des"* || "$current_ciphers" == *"blowfish"* || "$current_ciphers" == *"arcfour"* || "$current_ciphers" == *"cbc"* ]]; then
            echo -e "  ${RED}[✗] 包含不安全的加密算法${NC}"
            echo -e "  ${YELLOW}[!] 建议: 使用更安全的加密算法${NC}"
            echo -e "  ${YELLOW}[!] Ciphers ${recommended_ciphers}${NC}"
        fi
    fi

    # 检查MACs配置
    local current_macs=$(grep -E "^[[:space:]]*MACs[[:space:]]+" "$SSH_CONFIG" | cut -d ' ' -f 2-)
    if [ -z "$current_macs" ]; then
        echo -e "  ${YELLOW}[!] MACs 未配置，使用默认值${NC}"
        echo -e "  ${YELLOW}[!] 建议: 添加以下配置${NC}"
        echo -e "  ${YELLOW}[!] MACs ${recommended_macs}${NC}"
    else
        echo -e "  ${BLUE}[i] 当前MACs配置:${NC}"
        echo -e "  ${current_macs}"

        # 检查是否包含不安全的MAC算法
        if [[ "$current_macs" == *"md5"* || "$current_macs" == *"96"* ]]; then
            echo -e "  ${RED}[✗] 包含不安全的MAC算法${NC}"
            echo -e "  ${YELLOW}[!] 建议: 使用更安全的MAC算法${NC}"
            echo -e "  ${YELLOW}[!] MACs ${recommended_macs}${NC}"
        fi
    fi

    # 检查KexAlgorithms配置
    local current_kex=$(grep -E "^[[:space:]]*KexAlgorithms[[:space:]]+" "$SSH_CONFIG" | cut -d ' ' -f 2-)
    if [ -z "$current_kex" ]; then
        echo -e "  ${YELLOW}[!] KexAlgorithms 未配置，使用默认值${NC}"
        echo -e "  ${YELLOW}[!] 建议: 添加以下配置${NC}"
        echo -e "  ${YELLOW}[!] KexAlgorithms ${recommended_kex}${NC}"
    else
        echo -e "  ${BLUE}[i] 当前KexAlgorithms配置:${NC}"
        echo -e "  ${current_kex}"

        # 检查是否包含不安全的密钥交换算法
        if [[ "$current_kex" == *"sha1"* || "$current_kex" == *"group1"* || "$current_kex" == *"group14-sha1"* ]]; then
            echo -e "  ${RED}[✗] 包含不安全的密钥交换算法${NC}"
            echo -e "  ${YELLOW}[!] 建议: 使用更安全的密钥交换算法${NC}"
            echo -e "  ${YELLOW}[!] KexAlgorithms ${recommended_kex}${NC}"
        fi
    fi
}

# 检查访问控制配置
check_access_control() {
    echo -e "\n${BLUE}[*] 访问控制配置:${NC}"

    # 检查AllowUsers和AllowGroups
    local allow_users=$(grep -E "^[[:space:]]*AllowUsers[[:space:]]+" "$SSH_CONFIG")
    local allow_groups=$(grep -E "^[[:space:]]*AllowGroups[[:space:]]+" "$SSH_CONFIG")

    if [ -z "$allow_users" ] && [ -z "$allow_groups" ]; then
        echo -e "  ${YELLOW}[!] 未配置用户/组访问限制${NC}"
        echo -e "  ${YELLOW}[!] 建议: 使用AllowUsers或AllowGroups限制可访问SSH的用户${NC}"
    else
        if [ -n "$allow_users" ]; then
            echo -e "  ${GREEN}[✓] 已配置用户访问控制:${NC} ${allow_users}"
        fi
        if [ -n "$allow_groups" ]; then
            echo -e "  ${GREEN}[✓] 已配置组访问控制:${NC} ${allow_groups}"
        fi
    fi

    # 检查DenyUsers和DenyGroups
    local deny_users=$(grep -E "^[[:space:]]*DenyUsers[[:space:]]+" "$SSH_CONFIG")
    local deny_groups=$(grep -E "^[[:space:]]*DenyGroups[[:space:]]+" "$SSH_CONFIG")

    if [ -n "$deny_users" ]; then
        echo -e "  ${BLUE}[i] 已配置拒绝用户:${NC} ${deny_users}"
    fi
    if [ -n "$deny_groups" ]; then
        echo -e "  ${BLUE}[i] 已配置拒绝组:${NC} ${deny_groups}"
    fi

    # 检查Banner配置
    local banner=$(grep -E "^[[:space:]]*Banner[[:space:]]+" "$SSH_CONFIG")
    if [ -z "$banner" ]; then
        echo -e "  ${YELLOW}[!] 未配置登录Banner${NC}"
        echo -e "  ${YELLOW}[!] 建议: 设置Banner显示法律警告信息${NC}"
        echo -e "  ${YELLOW}[!] Banner /etc/issue.net${NC}"
    else
        echo -e "  ${GREEN}[✓] 已配置Banner:${NC} ${banner}"

        # 检查Banner文件是否存在
        local banner_file=$(echo "$banner" | awk '{print $2}')
        if [ ! -f "$banner_file" ]; then
            echo -e "  ${RED}[✗] Banner文件不存在:${NC} ${banner_file}"
            echo -e "  ${YELLOW}[!] 建议: 创建Banner文件并添加适当的警告信息${NC}"
        fi
    fi

    # 检查Port配置
    local port=$(grep -E "^[[:space:]]*Port[[:space:]]+" "$SSH_CONFIG" | awk '{print $2}')
    if [ -z "$port" ]; then
        echo -e "  ${BLUE}[i] 未指定端口，使用默认22端口${NC}"
    elif [ "$port" -eq 22 ]; then
        echo -e "  ${YELLOW}[!] 使用默认22端口${NC}"
        echo -e "  ${YELLOW}[!] 建议: 考虑使用非标准端口减少自动扫描威胁${NC}"
    else
        echo -e "  ${GREEN}[✓] 使用非标准端口:${NC} ${port}"
    fi
}

# 高级安全配置检查
check_advanced_config() {
    echo -e "\n${BLUE}[*] 高级安全配置检查:${NC}"

    # 检查MaxSessions
    local max_sessions=$(grep -E "^[[:space:]]*MaxSessions[[:space:]]+" "$SSH_CONFIG" | awk '{print $2}')
    if [ -z "$max_sessions" ]; then
        echo -e "  ${YELLOW}[!] MaxSessions 未配置，使用默认值${NC}"
        echo -e "  ${YELLOW}[!] 建议: 设置 MaxSessions 10 限制每个连接的最大会话数${NC}"
    elif [ "$max_sessions" -gt 10 ]; then
        echo -e "  ${YELLOW}[!] MaxSessions 值较高:${NC} ${max_sessions}"
        echo -e "  ${YELLOW}[!] 建议: 考虑减少到10或更低${NC}"
    else
        echo -e "  ${GREEN}[✓] MaxSessions 配置合理:${NC} ${max_sessions}"
    fi

    # 检查MaxStartups
    local max_startups=$(grep -E "^[[:space:]]*MaxStartups[[:space:]]+" "$SSH_CONFIG")
    if [ -z "$max_startups" ]; then
        echo -e "  ${YELLOW}[!] MaxStartups 未配置，使用默认值${NC}"
        echo -e "  ${YELLOW}[!] 建议: 设置 MaxStartups 10:30:100 防止暴力攻击${NC}"
    else
        echo -e "  ${GREEN}[✓] 已配置MaxStartups:${NC} ${max_startups}"
    fi

    # 检查是否启用了压缩
    local compression=$(grep -E "^[[:space:]]*Compression[[:space:]]+" "$SSH_CONFIG" | awk '{print $2}')
    if [ "$compression" == "yes" ]; then
        echo -e "  ${YELLOW}[!] 已启用压缩，可能存在DOS攻击风险${NC}"
        echo -e "  ${YELLOW}[!] 建议: 设置 Compression no 或 Compression delayed${NC}"
    elif [ "$compression" == "delayed" ]; then
        echo -e "  ${GREEN}[✓] 使用延迟压缩:${NC} Compression ${compression}"
    elif [ -z "$compression" ]; then
        echo -e "  ${YELLOW}[!] Compression 未配置，使用默认值${NC}"
        echo -e "  ${YELLOW}[!] 建议: 设置 Compression no 或 Compression delayed${NC}"
    else
        echo -e "  ${GREEN}[✓] 已禁用压缩:${NC} Compression ${compression}"
    fi

    # 检查是否使用AllowTcpForwarding
    local tcp_forwarding=$(grep -E "^[[:space:]]*AllowTcpForwarding[[:space:]]+" "$SSH_CONFIG" | awk '{print $2}')
    if [ -z "$tcp_forwarding" ]; then
        echo -e "  ${YELLOW}[!] AllowTcpForwarding 未配置，使用默认值(yes)${NC}"
        echo -e "  ${YELLOW}[!] 建议: 如无需求，设置 AllowTcpForwarding no${NC}"
    elif [ "$tcp_forwarding" == "yes" ]; then
        echo -e "  ${YELLOW}[!] 已启用TCP转发，可能增加安全风险${NC}"
        echo -e "  ${YELLOW}[!] 建议: 如无需求，设置 AllowTcpForwarding no${NC}"
    else
        echo -e "  ${GREEN}[✓] 已禁用TCP转发:${NC} AllowTcpForwarding ${tcp_forwarding}"
    fi

    # 检查是否使用GatewayPorts
    local gateway_ports=$(grep -E "^[[:space:]]*GatewayPorts[[:space:]]+" "$SSH_CONFIG" | awk '{print $2}')
    if [ -z "$gateway_ports" ]; then
        echo -e "  ${GREEN}[✓] GatewayPorts 未配置，使用默认值(no)${NC}"
    elif [ "$gateway_ports" == "yes" ]; then
        echo -e "  ${RED}[✗] 已启用GatewayPorts，存在安全风险${NC}"
        echo -e "  ${YELLOW}[!] 建议: 设置 GatewayPorts no${NC}"
    else
        echo -e "  ${GREEN}[✓] 已禁用GatewayPorts:${NC} GatewayPorts ${gateway_ports}"
    fi

    # 检查是否使用PermitUserEnvironment
    local permit_user_env=$(grep -E "^[[:space:]]*PermitUserEnvironment[[:space:]]+" "$SSH_CONFIG" | awk '{print $2}')
    if [ -z "$permit_user_env" ]; then
        echo -e "  ${GREEN}[✓] PermitUserEnvironment 未配置，使用默认值(no)${NC}"
    elif [ "$permit_user_env" == "yes" ]; then
        echo -e "  ${RED}[✗] 已启用PermitUserEnvironment，存在安全风险${NC}"
        echo -e "  ${YELLOW}[!] 建议: 设置 PermitUserEnvironment no${NC}"
    else
        echo -e "  ${GREEN}[✓] 已禁用PermitUserEnvironment:${NC} PermitUserEnvironment ${permit_user_env}"
    fi
}

# 提供安全配置模板
provide_secure_template() {
    echo -e "\n${BLUE}[*] 安全配置建议模板:${NC}"
    echo -e "  ${YELLOW}以下是推荐的安全配置模板，可以根据实际需求调整:${NC}"

    cat << EOT
  # 基本安全设置
  Protocol 2
  PermitRootLogin no
  PasswordAuthentication no
  PubkeyAuthentication yes
  PermitEmptyPasswords no
  MaxAuthTries 4
  LoginGraceTime 60

  # 登录限制
  AllowUsers youruser1 youruser2
  # AllowGroups sshusers

  # 加密和安全算法
  Ciphers chacha20-poly1305@openssh.com,aes256-gcm@openssh.com,aes128-gcm@openssh.com,aes256-ctr,aes192-ctr,aes128-ctr
  MACs hmac-sha2-512-etm@openssh.com,hmac-sha2-256-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-512,hmac-sha2-256
  KexAlgorithms curve25519-sha256@libssh.org,diffie-hellman-group-exchange-sha256,diffie-hellman-group16-sha512

  # 其他安全设置
  X11Forwarding no
  UsePAM yes
  TCPKeepAlive no
  ClientAliveInterval 300
  ClientAliveCountMax 2
  StrictModes yes
  IgnoreRhosts yes
  HostbasedAuthentication no

  # 高级设置
  MaxSessions 10
  MaxStartups 10:30:100
  Compression no
  AllowTcpForwarding no
  GatewayPorts no
  PermitUserEnvironment no

  # 日志设置
  LogLevel VERBOSE

  # 警告Banner
  Banner /etc/issue.net
EOT
}

# 主函数
main() {
    check_ssh_version
    check_file_permissions
    check_security_config
    check_crypto_config
    check_access_control
    check_advanced_config
    provide_secure_template

    # 总结
    echo -e "\n${CYAN}====================================================${NC}"
    echo -e "${GREEN}[+] SSH配置安全检测完成${NC}"
    echo -e "${CYAN}====================================================${NC}"

    # 检测后的建议
    echo -e "\n${BLUE}[*] 后续建议:${NC}"
    echo -e "  ${WHITE}1. 根据上述检测结果修改SSH配置${NC}"
    echo -e "  ${WHITE}2. 修改配置后重启SSH服务生效:${NC}"
    echo -e "     ${CYAN}sudo systemctl restart sshd${NC} (Linux系统)"
    echo -e "     ${CYAN}sudo service sshd restart${NC} (FreeBSD系统)"
    echo -e "     ${CYAN}sudo launchctl unload /System/Library/LaunchDaemons/ssh.plist${NC} (macOS系统)"
    echo -e "     ${CYAN}sudo launchctl load /System/Library/LaunchDaemons/ssh.plist${NC} (macOS系统)"
    echo -e "  ${WHITE}3. 修改配置前请保留一个活动的SSH会话，以防配置错误导致无法连接${NC}"
    echo -e "  ${WHITE}4. 定期使用此脚本检查SSH配置安全性${NC}"
}

# 执行主函数
main

exit 0