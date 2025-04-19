#!/bin/bash
# file_check.sh - 系统可疑文件和目录检测工具
# 版本: 1.0

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # 无颜色

# 变量定义
SCAN_DIR="/"
EXCLUDE_DIRS=("/proc" "/sys" "/dev" "/run" "/mnt" "/media" "/tmp/systemd-private*" "/var/lib/docker" "/var/lib/lxc")
MAX_DEPTH=7
LARGE_FILE_SIZE=100000 # 100MB in KB
RECENT_FILES_DAYS=3
OUTPUT_DIR="$(pwd)/security_scan_$(date +%Y%m%d_%H%M%S)"
LOG_FILE="$OUTPUT_DIR/file_check.log"
SUMMARY_FILE="$OUTPUT_DIR/summary.txt"
FOUND_COUNT=0
VERBOSE=0
SCAN_ALL=0

# 横幅
show_banner() {
    clear
    echo -e "${CYAN}====================================================${NC}"
    echo -e "${WHITE}           系统可疑文件检测工具 v1.0                  ${NC}"
    echo -e "${CYAN}====================================================${NC}"
    echo -e "${GREEN}[+] 开始扫描时间: $(date '+%Y-%m-%d %H:%M:%S')${NC}\n"
}

# 显示帮助信息
show_help() {
    echo -e "使用说明: $0 [选项]"
    echo -e "选项:"
    echo -e "  -d, --directory DIR    指定要扫描的目录 (默认: /)"
    echo -e "  -o, --output DIR       指定输出目录 (默认: 当前目录)"
    echo -e "  -m, --max-depth N      最大递归深度 (默认: 7)"
    echo -e "  -a, --all              扫描所有文件 (默认: 排除一些系统目录)"
    echo -e "  -v, --verbose          显示详细输出"
    echo -e "  -h, --help             显示帮助信息"
    echo -e "\n示例:"
    echo -e "  $0 -d /home -o /tmp/scan_results -v     # 扫描/home目录并输出详细结果"
    echo -e "  $0 -a -m 10                            # 扫描所有文件，递归深度为10"
}

# 解析命令行参数
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -d|--directory)
                SCAN_DIR="$2"
                shift 2
                ;;
            -o|--output)
                OUTPUT_DIR="$2"
                LOG_FILE="$OUTPUT_DIR/file_check.log"
                SUMMARY_FILE="$OUTPUT_DIR/summary.txt"
                shift 2
                ;;
            -m|--max-depth)
                MAX_DEPTH="$2"
                shift 2
                ;;
            -a|--all)
                SCAN_ALL=1
                shift
                ;;
            -v|--verbose)
                VERBOSE=1
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                echo -e "${RED}[-] 未知参数: $1${NC}"
                show_help
                exit 1
                ;;
        esac
    done
}

# 初始化
initialize() {
    # 检查是否为root用户
    if [[ $EUID -ne 0 ]]; then
        echo -e "${YELLOW}[!] 警告: 脚本未以root权限运行，某些文件可能无法检查${NC}"
        echo -e "${YELLOW}[!] 建议使用sudo或root用户运行此脚本${NC}"
        sleep 2
    fi

    # 创建输出目录
    mkdir -p "$OUTPUT_DIR"
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}[-] 无法创建输出目录: $OUTPUT_DIR${NC}"
        exit 1
    fi

    # 初始化日志和摘要文件
    echo "系统可疑文件检测报告 - $(date)" > "$LOG_FILE"
    echo "系统可疑文件检测报告摘要 - $(date)" > "$SUMMARY_FILE"
    echo "===============================================" >> "$SUMMARY_FILE"

    # 记录系统信息
    echo -e "${BLUE}[*] 记录系统信息...${NC}"
    {
        echo "## 系统信息"
        echo "扫描时间: $(date)"
        echo "主机名: $(hostname)"
        echo "内核版本: $(uname -r)"
        echo "操作系统: $(grep -E "^PRETTY_NAME=" /etc/os-release 2>/dev/null | cut -d= -f2- | tr -d '"' || echo "未知")"
        echo "==============================================="
    } >> "$LOG_FILE"

    if [[ $VERBOSE -eq 1 ]]; then
        echo -e "${GREEN}[+] 输出目录: $OUTPUT_DIR${NC}"
        echo -e "${GREEN}[+] 日志文件: $LOG_FILE${NC}"
        echo -e "${GREEN}[+] 摘要文件: $SUMMARY_FILE${NC}"
    fi
}

# 构建排除目录的参数字符串
build_exclude_params() {
    local exclude_params=""

    if [[ $SCAN_ALL -eq 0 ]]; then
        for dir in "${EXCLUDE_DIRS[@]}"; do
            exclude_params="$exclude_params -path $dir -prune -o "
        done
    fi

    echo "$exclude_params"
}

# 记录到日志
log_finding() {
    local section="$1"
    local message="$2"
    local severity="$3" # high, medium, low

    echo "[$section] $message" >> "$LOG_FILE"

    # 在终端显示
    case "$severity" in
        high)
            echo -e "  ${RED}[!] $message${NC}"
            ;;
        medium)
            echo -e "  ${YELLOW}[!] $message${NC}"
            ;;
        low)
            echo -e "  ${BLUE}[i] $message${NC}"
            ;;
        *)
            echo -e "  [*] $message"
            ;;
    esac

    # 增加计数
    FOUND_COUNT=$((FOUND_COUNT + 1))
}

# 添加到摘要
add_to_summary() {
    local section="$1"
    local count="$2"
    local details="$3"

    echo "## $section: $count 个发现" >> "$SUMMARY_FILE"
    if [[ -n "$details" ]]; then
        echo "$details" >> "$SUMMARY_FILE"
    fi
    echo "-----------------------------------------------" >> "$SUMMARY_FILE"
}

# 检查SUID和SGID文件
check_suid_sgid() {
    echo -e "\n${CYAN}[*] 检查SUID和SGID文件...${NC}"

    local exclude_params=$(build_exclude_params)
    local count=0
    local suspicious_count=0
    local suspicious_list=""
    local normal_suid=(
        "/usr/bin/sudo" "/usr/bin/su" "/usr/bin/passwd" "/usr/bin/gpasswd"
        "/usr/bin/chsh" "/usr/bin/chfn" "/usr/bin/newgrp" "/usr/bin/pkexec"
        "/usr/lib/policykit-1/polkit-agent-helper-1" "/usr/lib/dbus-1.0/dbus-daemon-launch-helper"
        "/usr/lib/openssh/ssh-keysign" "/usr/bin/at" "/usr/bin/crontab" "/usr/bin/wall"
        "/bin/mount" "/bin/umount" "/bin/su" "/bin/ping" "/usr/bin/traceroute"
    )

    echo -e "${WHITE}查找SUID文件:${NC}"
    while IFS= read -r file; do
        if [[ -n "$file" ]]; then
            count=$((count + 1))

            # 检查是否为常见的SUID文件
            local is_normal=0
            for normal in "${normal_suid[@]}"; do
                if [[ "$file" == "$normal" ]]; then
                    is_normal=1
                    break
                fi
            done

            if [[ $is_normal -eq 0 ]]; then
                suspicious_count=$((suspicious_count + 1))
                suspicious_list="${suspicious_list}${file}\n"
                log_finding "SUID" "可疑的SUID文件: $file" "high"
            elif [[ $VERBOSE -eq 1 ]]; then
                log_finding "SUID" "常见的SUID文件: $file" "low"
            fi
        fi
    done < <(eval find "$SCAN_DIR" $exclude_params -type f -perm -4000 -print 2>/dev/null)

    echo -e "${WHITE}查找SGID文件:${NC}"
    while IFS= read -r file; do
        if [[ -n "$file" ]]; then
            count=$((count + 1))
            log_finding "SGID" "SGID文件: $file" "medium"
            suspicious_list="${suspicious_list}${file}\n"
        fi
    done < <(eval find "$SCAN_DIR" $exclude_params -type f -perm -2000 -print 2>/dev/null)

    echo -e "${GREEN}[+] 找到 $count 个SUID/SGID文件，其中 $suspicious_count 个可疑${NC}"
    add_to_summary "SUID/SGID文件" "$count" "可疑文件数: $suspicious_count\n$(echo -e $suspicious_list | sort | head -20)"
}

# 检查隐藏文件和目录
check_hidden_files() {
    echo -e "\n${CYAN}[*] 检查隐藏文件和目录...${NC}"

    local exclude_params=$(build_exclude_params)
    local count=0
    local suspicious_count=0
    local suspicious_list=""
    local common_patterns=(
        ".*history" ".bash_profile" ".bashrc" ".profile" ".viminfo" ".lesshst" ".ssh"
        ".config" ".cache" ".local" ".mozilla" ".gnupg" ".dbus" ".gconf"
    )

    echo -e "${WHITE}查找异常的隐藏文件和目录:${NC}"
    while IFS= read -r file; do
        if [[ -n "$file" ]]; then
            count=$((count + 1))

            # 检查是否为常见的隐藏文件
            local is_normal=0
            local basename=$(basename "$file")
            for pattern in "${common_patterns[@]}"; do
                if [[ "$basename" =~ $pattern ]]; then
                    is_normal=1
                    break
                fi
            done

            # 检查可疑的隐藏文件
            if [[ $is_normal -eq 0 ]]; then
                # 检查是否包含可执行文件
                if [[ -f "$file" && -x "$file" ]]; then
                    suspicious_count=$((suspicious_count + 1))
                    suspicious_list="${suspicious_list}${file}\n"
                    log_finding "隐藏文件" "可疑的隐藏可执行文件: $file" "high"
                # 检查位于/tmp、/var/tmp或家目录根下的隐藏目录
                elif [[ -d "$file" && ("$file" =~ ^/tmp/\. || "$file" =~ ^/var/tmp/\. || "$file" =~ ^/home/[^/]+/\.) ]]; then
                    suspicious_count=$((suspicious_count + 1))
                    suspicious_list="${suspicious_list}${file}\n"
                    log_finding "隐藏目录" "可疑的隐藏目录: $file" "medium"
                elif [[ $VERBOSE -eq 1 ]]; then
                    log_finding "隐藏文件" "隐藏文件/目录: $file" "low"
                fi
            fi
        fi
    done < <(eval find "$SCAN_DIR" $exclude_params -name ".*" -not -name "." -not -name ".." -print 2>/dev/null | sort)

    echo -e "${GREEN}[+] 找到 $count 个隐藏文件/目录，其中 $suspicious_count 个可疑${NC}"
    add_to_summary "隐藏文件/目录" "$count" "可疑隐藏文件/目录数: $suspicious_count\n$(echo -e $suspicious_list | sort | head -20)"
}

# 检查异常大小的文件
check_large_files() {
    echo -e "\n${CYAN}[*] 检查异常大小的文件...${NC}"

    local exclude_params=$(build_exclude_params)
    local count=0
    local suspicious_list=""

    echo -e "${WHITE}查找异常大小的文件 (>$LARGE_FILE_SIZE KB):${NC}"
    while IFS= read -r file; do
        if [[ -n "$file" ]]; then
            count=$((count + 1))

            # 获取文件类型
            local file_type=$(file -b "$file" | tr -d '\n')

            # 判断是否为可疑的大文件
            if [[ "$file_type" == *"executable"* || "$file_type" == *"ELF"* || "$file" == *".log."* || "$file" == *".old"* || "$file" == *".backup"* ]]; then
                suspicious_list="${suspicious_list}${file} ($(du -h "$file" | cut -f1)): ${file_type}\n"
                log_finding "大文件" "可疑的大文件: $file ($(du -h "$file" | cut -f1)) - $file_type" "medium"
            elif [[ $VERBOSE -eq 1 ]]; then
                log_finding "大文件" "大文件: $file ($(du -h "$file" | cut -f1)) - $file_type" "low"
            fi
        fi
    done < <(eval find "$SCAN_DIR" $exclude_params -type f -size +"$LARGE_FILE_SIZE"k -print 2>/dev/null | sort)

    echo -e "${GREEN}[+] 找到 $count 个大文件${NC}"
    add_to_summary "异常大小文件" "$count" "$(echo -e $suspicious_list | sort | head -20)"
}

# 检查最近修改的文件
check_recent_files() {
    echo -e "\n${CYAN}[*] 检查最近修改的文件...${NC}"

    local exclude_params=$(build_exclude_params)
    local count=0
    local suspicious_count=0
    local suspicious_list=""

    echo -e "${WHITE}查找最近 $RECENT_FILES_DAYS 天内修改的可执行文件和脚本:${NC}"
    while IFS= read -r file; do
        if [[ -n "$file" ]]; then
            count=$((count + 1))

            # 获取文件类型
            local file_type=$(file -b "$file" | tr -d '\n')

            # 判断是否为可执行文件或脚本
            if [[ "$file_type" == *"executable"* || "$file_type" == *"ELF"* || "$file_type" == *"script"* ||
                  "$file_type" == *"Python"* || "$file_type" == *"Perl"* || "$file_type" == *"shell"* ||
                  "$file" == *".py" || "$file" == *".pl" || "$file" == *".sh" || "$file" == *".php" ]]; then

                # 排除一些常见的包管理器管理的文件
                if [[ -x "$(command -v dpkg)" && -n "$(dpkg -S "$file" 2>/dev/null)" ]]; then
                    # 这个文件由Debian包管理器管理
                    continue
                elif [[ -x "$(command -v rpm)" && -n "$(rpm -qf "$file" 2>/dev/null)" ]]; then
                    # 这个文件由RPM包管理器管理
                    continue
                fi

                suspicious_count=$((suspicious_count + 1))
                suspicious_list="${suspicious_list}$(stat -c "%y - %s bytes - %n" "$file"): ${file_type}\n"
                log_finding "最近文件" "最近修改的可执行文件: $(stat -c "%y - %s bytes - %n" "$file") - $file_type" "high"
            elif [[ $VERBOSE -eq 1 ]]; then
                log_finding "最近文件" "最近修改的文件: $(stat -c "%y - %n" "$file")" "low"
            fi
        fi
    done < <(eval find "$SCAN_DIR" $exclude_params -type f -mtime -"$RECENT_FILES_DAYS" -print 2>/dev/null | sort)

    echo -e "${GREEN}[+] 找到 $count 个最近修改的文件，其中 $suspicious_count 个可疑${NC}"
    add_to_summary "最近修改的文件" "$count" "可疑最近修改文件数: $suspicious_count\n$(echo -e $suspicious_list | sort | head -20)"
}

# 检查世界可写的文件和目录
check_world_writable() {
    echo -e "\n${CYAN}[*] 检查世界可写的文件和目录...${NC}"

    local exclude_params=$(build_exclude_params)
    local count=0
    local suspicious_list=""
    local normal_world_writable=(
        "/tmp" "/var/tmp" "/var/spool/vbox" "/var/spool/samba"
    )

    echo -e "${WHITE}查找世界可写的文件和目录:${NC}"
    while IFS= read -r file; do
        if [[ -n "$file" ]]; then
            count=$((count + 1))

            # 检查是否为常见的世界可写目录
            local is_normal=0
            for normal in "${normal_world_writable[@]}"; do
                if [[ "$file" == "$normal" || "$file" == "$normal/*" ]]; then
                    is_normal=1
                    break
                fi
            done

            if [[ $is_normal -eq 0 ]]; then
                suspicious_list="${suspicious_list}${file}\n"

                if [[ -d "$file" ]]; then
                    log_finding "世界可写" "世界可写目录: $file" "high"
                else
                    # 检查是否为可执行文件
                    if [[ -x "$file" ]]; then
                        log_finding "世界可写" "世界可写可执行文件: $file" "high"
                    else
                        log_finding "世界可写" "世界可写文件: $file" "medium"
                    fi
                fi
            elif [[ $VERBOSE -eq 1 ]]; then
                log_finding "世界可写" "常见的世界可写文件/目录: $file" "low"
            fi
        fi
    done < <(eval find "$SCAN_DIR" $exclude_params -perm -0002 -type f -o -perm -0002 -type d -print 2>/dev/null | sort)

    echo -e "${GREEN}[+] 找到 $count 个世界可写的文件/目录${NC}"
    add_to_summary "世界可写文件/目录" "$count" "$(echo -e $suspicious_list | sort | head -20)"
}

# 检查已知的恶意文件模式
check_malicious_patterns() {
    echo -e "\n${CYAN}[*] 检查已知的恶意文件模式...${NC}"

    local exclude_params=$(build_exclude_params)
    local count=0
    local suspicious_list=""

    # 恶意文件名模式
    local malware_patterns=(
        "c99.php" "r57.php" "webshell.php" "backdoor.php" "cmd.php" "shell.php" "hack.php"
        "bypass.php" "defacement.php" "wso.php" "emotet" "mimikatz" "meterpreter" "netcat"
        "rootkit" "cryptolocker" "ransom" "wannacry" "ryuk"
    )

    echo -e "${WHITE}查找匹配已知恶意模式的文件:${NC}"

    # 构建grep模式
    local grep_pattern=$(printf "%s|" "${malware_patterns[@]}" | sed 's/|$//')

    while IFS= read -r file; do
        if [[ -n "$file" ]]; then
            count=$((count + 1))
            suspicious_list="${suspicious_list}${file}\n"
            log_finding "恶意模式" "可能的恶意文件: $file" "high"
        fi
    done < <(eval find "$SCAN_DIR" $exclude_params -type f -name "*" | grep -E "$grep_pattern" 2>/dev/null | sort)

    # 检查文件内容中的可疑模式
    echo -e "${WHITE}查找包含可疑内容的PHP、Python、Perl或Shell脚本:${NC}"

    local content_patterns=(
        "passthru" "shell_exec" "system" "phpinfo" "base64_decode" "chmod 777" "exec"
        "eval[ \t]*\\(" "eval[ \t]*\\\$" "FromBase64String" "PasswordDeriveBytes" "net localgroup administrators"
        "rev.shell" "reverse shell" "bind shell" "bash -i" "kali" "metasploit" "reverse_tcp" "netcat"
    )

    # 构建grep模式
    local content_grep_pattern=$(printf "%s|" "${content_patterns[@]}" | sed 's/|$//')

    # 仅在指定目录中搜索，避免扫描整个系统
    local search_dirs=(
        "/var/www" "/srv/www" "/var/html" "/tmp" "/home" "/opt"
    )

    for search_dir in "${search_dirs[@]}"; do
        if [[ -d "$search_dir" ]]; then
            while IFS= read -r file; do
                if [[ -n "$file" && -f "$file" && ! -L "$file" ]]; then
                    local file_type=$(file -b "$file" | tr -d '\n')

                    # 仅检查脚本文件和非二进制文件
                    if [[ "$file_type" == *"script"* || "$file_type" == *"text"* ||
                          "$file" == *".php" || "$file" == *".py" || "$file" == *".pl" ||
                          "$file" == *".sh" || "$file" == *".cgi" || "$file" == *".js" ]]; then

                        # 检查文件内容是否包含可疑模式
                        if grep -l -E "$content_grep_pattern" "$file" &>/dev/null; then
                            count=$((count + 1))
                            suspicious_list="${suspicious_list}${file} (包含可疑内容)\n"
                            log_finding "恶意模式" "包含可疑内容的文件: $file" "high"
                        fi
                    fi
                fi
            done < <(find "$search_dir" -type f -name "*.php" -o -name "*.py" -o -name "*.pl" -o -name "*.sh" -o -name "*.cgi" -o -name "*.js" 2>/dev/null | sort | head -1000)
        fi
    done

    echo -e "${GREEN}[+] 找到 $count 个可能的恶意文件${NC}"
    add_to_summary "疑似恶意文件" "$count" "$(echo -e $suspicious_list | sort | head -20)"
}

# 检查异常的cron任务
check_cron_jobs() {
    echo -e "\n${CYAN}[*] 检查异常的cron任务...${NC}"

    local count=0
    local suspicious_list=""

    # 检查系统cron目录
    for cron_dir in /etc/cron.d /etc/cron.daily /etc/cron.hourly /etc/cron.monthly /etc/cron.weekly; do
        if [[ -d "$cron_dir" ]]; then
            echo -e "${WHITE}检查 $cron_dir 中的cron任务:${NC}"
            for file in "$cron_dir"/*; do
                if [[ -f "$file" && ! -L "$file" ]]; then
                    # 获取文件内容
                    local content=$(cat "$file" 2>/dev/null)
                    if [[ -n "$content" && "$content" != *"#"* ]]; then
                        count=$((count + 1))
                        suspicious_list="${suspicious_list}${file}:\n$(grep -v '^#' "$file" | grep -v '^$')\n"
                        log_finding "Cron任务" "Cron任务文件: $file" "medium"
                    fi
                fi
            done
        fi
    done

    # 检查/etc/crontab
    if [[ -f "/etc/crontab" ]]; then
        echo -e "${WHITE}检查 /etc/crontab:${NC}"
        local crontab_content=$(grep -v '^#' /etc/crontab | grep -v '^$')
        if [[ -n "$crontab_content" ]]; then
            suspicious_list="${suspicious_list}/etc/crontab:\n${crontab_content}\n"
            log_finding "Cron任务" "系统Crontab包含任务" "low"
        fi
    fi

    # 检查用户crontab
    echo -e "${WHITE}检查用户crontab:${NC}"
    for user_home in /home/*; do
        local user=$(basename "$user_home")
        if [[ -d "$user_home" && "$user" != "lost+found" ]]; then
            if command -v crontab >/dev/null 2>&1; then
                local user_crontab=$(crontab -u "$user" -l 2>/dev/null | grep -v '^#' | grep -v '^$')
                if [[ -n "$user_crontab" ]]; then
                    count=$((count + 1))
                    suspicious_list="${suspicious_list}用户 ${user} crontab:\n${user_crontab}\n"
                    log_finding "Cron任务" "用户 $user 的crontab包含任务" "medium"
                fi
            fi
        fi
    done

    # 检查root用户crontab
    if command -v crontab >/dev/null 2>&1; then
        local root_crontab=$(crontab -u root -l 2>/dev/null | grep -v '^#' | grep -v '^$')
        if [[ -n "$root_crontab" ]]; then
            count=$((count + 1))
            suspicious_list="${suspicious_list}root用户 crontab:\n${root_crontab}\n"
            log_finding "Cron任务" "root用户的crontab包含任务" "medium"
        fi
    fi

    echo -e "${GREEN}[+] 找到 $count 个cron任务${NC}"
    add_to_summary "异常Cron任务" "$count" "$(echo -e $suspicious_list | head -30)"
}

# 检查异常的系统启动脚本
check_startup_scripts() {
    echo -e "\n${CYAN}[*] 检查系统启动脚本...${NC}"

    local count=0
    local suspicious_list=""

    # 检查systemd服务
    if [[ -d "/etc/systemd/system" ]]; then
        echo -e "${WHITE}检查systemd用户服务:${NC}"
        for service in /etc/systemd/system/*.service /etc/systemd/system/*/*.service; do
            if [[ -f "$service" && ! -L "$service" ]]; then
                # 排除一些默认的服务
                if [[ "$service" == *"dbus-"* || "$service" == *"systemd-"* || "$service" == *"getty"* ]]; then
                    continue
                fi

                count=$((count + 1))
                suspicious_list="${suspicious_list}${service}:\n$(grep "ExecStart" "$service" 2>/dev/null)\n"
                log_finding "启动脚本" "Systemd服务: $service" "medium"
            fi
        done
    fi

    # 检查init.d脚本
    if [[ -d "/etc/init.d" ]]; then
        echo -e "${WHITE}检查init.d脚本:${NC}"
        for script in /etc/init.d/*; do
            if [[ -f "$script" && ! -L "$script" && -x "$script" ]]; then
                # 获取脚本类型
                local script_type=$(file -b "$script" | tr -d '\n')

                # 排除一些默认的脚本
                if [[ "$script" == *"README"* || "$script" == *"skeleton"* || "$script" == *"mountall"* ||
                      "$script" == *"halt"* || "$script" == *"killall"* || "$script" == *"single"* ]]; then
                    continue
                fi

                count=$((count + 1))
                suspicious_list="${suspicious_list}${script} (${script_type})\n"
                log_finding "启动脚本" "Init.d脚本: $script - $script_type" "medium"
            fi
        done
    fi

    # 检查rc.local文件
    if [[ -f "/etc/rc.local" ]]; then
        echo -e "${WHITE}检查rc.local文件:${NC}"
        if grep -v '^#' /etc/rc.local | grep -v '^exit 0' | grep -v '^$' >/dev/null 2>&1; then
            count=$((count + 1))
            suspicious_list="${suspicious_list}/etc/rc.local:\n$(grep -v '^#' /etc/rc.local | grep -v '^exit 0' | grep -v '^$')\n"
            log_finding "启动脚本" "rc.local包含命令" "medium"
        fi
    fi

    echo -e "${GREEN}[+] 找到 $count 个系统启动脚本${NC}"
    add_to_summary "系统启动脚本" "$count" "$(echo -e $suspicious_list | head -20)"
}

# 检查异常的网络连接
check_network_connections() {
    echo -e "\n${CYAN}[*] 检查异常的网络连接...${NC}"

    local count=0
    local suspicious_list=""

    if command -v netstat >/dev/null 2>&1; then
        echo -e "${WHITE}检查监听端口:${NC}"
        while IFS= read -r line; do
            if [[ -n "$line" ]]; then
                count=$((count + 1))
                local proto=$(echo "$line" | awk '{print $1}')
                local addr=$(echo "$line" | awk '{print $4}')
                local pid_prog=$(echo "$line" | awk '{print $7}')

                suspicious_list="${suspicious_list}${proto} ${addr} - ${pid_prog}\n"

                # 检查是否为常见端口
                if [[ "$addr" != *":22" && "$addr" != *":80" && "$addr" != *":443" &&
                      "$addr" != *":25" && "$addr" != *":53" && "$addr" != *":3306" &&
                      "$addr" != *":8080" && "$addr" != *":21" && "$addr" != *":11211" &&
                      ! "$pid_prog" =~ ^[0-9]+/(sshd|apache2|nginx|httpd|mysqld|named|postfix|dovecot)$ ]]; then
                    log_finding "网络连接" "可疑的监听连接: $line" "high"
                else
                    log_finding "网络连接" "监听连接: $line" "low"
                fi
            fi
        done < <(netstat -tlnp 2>/dev/null | grep -v "^Active" | grep -v "^Proto" | grep "LISTEN")
    elif command -v ss >/dev/null 2>&1; then
        echo -e "${WHITE}检查监听端口 (使用ss命令):${NC}"
        while IFS= read -r line; do
            if [[ -n "$line" ]]; then
                count=$((count + 1))
                suspicious_list="${suspicious_list}${line}\n"

                # 简单检查是否为常见端口
                if [[ ! "$line" =~ :(22|80|443|25|53|3306|8080|21)\  && ! "$line" =~ (sshd|apache2|nginx|httpd|mysqld|named|postfix|dovecot) ]]; then
                    log_finding "网络连接" "可疑的监听连接: $line" "high"
                else
                    log_finding "网络连接" "监听连接: $line" "low"
                fi
            fi
        done < <(ss -tlnp 2>/dev/null | grep -v "^State" | grep "LISTEN")
    else
        echo -e "${YELLOW}[!] 无法检查网络连接，netstat和ss命令不可用${NC}"
    fi

    echo -e "${GREEN}[+] 找到 $count 个监听网络连接${NC}"
    add_to_summary "网络连接" "$count" "$(echo -e $suspicious_list | head -20)"
}

# 检查可疑的用户账户
check_suspicious_accounts() {
    echo -e "\n${CYAN}[*] 检查可疑的用户账户...${NC}"

    local count=0
    local suspicious_list=""

    # 检查系统用户
    echo -e "${WHITE}检查具有登录shell的系统用户:${NC}"
    while IFS=: read -r username _ uid _ _ home shell; do
        # 检查是否为系统用户但拥有登录shell
        if [[ "$uid" -lt 1000 && "$uid" -ne 0 && "$shell" != "/usr/sbin/nologin" &&
              "$shell" != "/bin/false" && "$shell" != "/sbin/nologin" ]]; then
            count=$((count + 1))
            suspicious_list="${suspicious_list}用户: ${username}, UID: ${uid}, Shell: ${shell}, 家目录: ${home}\n"
            log_finding "用户账户" "系统用户具有登录shell: $username (UID: $uid, Shell: $shell)" "high"
        fi
    done < /etc/passwd

    # 检查具有UID 0的用户
    echo -e "${WHITE}检查具有UID 0的用户:${NC}"
    while IFS=: read -r username _ uid _ _ home shell; do
        if [[ "$uid" -eq 0 && "$username" != "root" ]]; then
            count=$((count + 1))
            suspicious_list="${suspicious_list}超级用户: ${username}, UID: ${uid}, Shell: ${shell}, 家目录: ${home}\n"
            log_finding "用户账户" "发现非root的UID 0用户: $username (Shell: $shell)" "high"
        fi
    done < /etc/passwd

    # 检查sudoers配置
    if [[ -f "/etc/sudoers" ]]; then
        echo -e "${WHITE}检查sudoers配置:${NC}"
        if grep -v '^#' /etc/sudoers | grep -v '^Defaults' | grep -v '^$' | grep NOPASSWD >/dev/null 2>&1; then
            count=$((count + 1))
            suspicious_list="${suspicious_list}sudoers包含NOPASSWD配置:\n$(grep -v '^#' /etc/sudoers | grep -v '^Defaults' | grep -v '^$' | grep NOPASSWD)\n"
            log_finding "用户账户" "sudoers包含NOPASSWD配置" "medium"
        fi
    fi

    # 检查sudoers.d目录
    if [[ -d "/etc/sudoers.d" ]]; then
        for file in /etc/sudoers.d/*; do
            if [[ -f "$file" && "$file" != "/etc/sudoers.d/README" ]]; then
                local content=$(grep -v '^#' "$file" | grep -v '^$')
                if [[ -n "$content" ]]; then
                    count=$((count + 1))
                    suspicious_list="${suspicious_list}sudoers.d文件 ${file}:\n${content}\n"
                    log_finding "用户账户" "发现sudoers.d文件: $file" "medium"
                fi
            fi
        done
    fi

    echo -e "${GREEN}[+] 找到 $count 个可疑的用户账户配置${NC}"
    add_to_summary "可疑用户账户" "$count" "$(echo -e $suspicious_list)"
}

# 检查Web服务器目录中的可疑文件
check_web_directories() {
    echo -e "\n${CYAN}[*] 检查Web服务器目录中的可疑文件...${NC}"

    local count=0
    local suspicious_list=""
    local web_dirs=(
        "/var/www" "/srv/www" "/usr/share/nginx" "/usr/local/www" "/usr/local/apache"
        "/var/www/html" "/srv/http" "/opt/lampp/htdocs"
    )

    for web_dir in "${web_dirs[@]}"; do
        if [[ -d "$web_dir" ]]; then
            echo -e "${WHITE}检查 $web_dir:${NC}"

            # 查找可执行文件和脚本
            while IFS= read -r file; do
                if [[ -n "$file" && -f "$file" && ! -L "$file" ]]; then
                    # 获取文件类型和权限
                    local file_type=$(file -b "$file" | tr -d '\n')
                    local file_perms=$(stat -c "%a" "$file" 2>/dev/null || stat -f "%Lp" "$file" 2>/dev/null)

                    # 检查是否为可疑文件
                    if [[ "$file" == *".php"* || "$file" == *".cgi"* || "$file_type" == *"PHP script"* ||
                         "$file_type" == *"Perl script"* || "$file_type" == *"Python script"* ||
                         "$file" == *"shell"* || "$file" == *"eval"* || "$file" == *"backdoor"* ||
                         "$file" == *"hack"* || "$file" == *"c99"* || "$file" == *"r57"* || "$file" == *"cmd"* ]]; then

                        count=$((count + 1))
                        suspicious_list="${suspicious_list}${file} [${file_perms}]: ${file_type}\n"

                        # 检查文件内容
                        if grep -l -E "passthru|shell_exec|system|phpinfo|base64_decode|eval\(" "$file" &>/dev/null; then
                            log_finding "Web目录" "危险的Web脚本: $file (包含可疑代码) - $file_type" "high"
                        else
                            log_finding "Web目录" "可疑的Web脚本: $file - $file_type" "medium"
                        fi
                    fi
                fi
            done < <(find "$web_dir" -type f -name "*.php" -o -name "*.cgi" -o -name "*.pl" -o -name "*.py" 2>/dev/null | head -1000)
        fi
    done

    echo -e "${GREEN}[+] 找到 $count 个Web目录中的可疑文件${NC}"
    add_to_summary "Web目录可疑文件" "$count" "$(echo -e $suspicious_list | sort | head -20)"
}

# 生成摘要报告
generate_summary() {
    echo -e "\n${CYAN}[*] 生成安全摘要报告...${NC}"

    # 添加系统信息到摘要
    {
        echo "# 系统安全检测摘要报告"
        echo "## 系统信息"
        echo "主机名: $(hostname)"
        echo "内核版本: $(uname -r)"
        echo "操作系统: $(grep -E "^PRETTY_NAME=" /etc/os-release 2>/dev/null | cut -d= -f2- | tr -d '"' || echo "未知")"
        echo "扫描时间: $(date)"
        echo "扫描目录: $SCAN_DIR"
        echo "==============================================="
    } > "$SUMMARY_FILE"

    # 添加总体统计信息
    {
        echo "# 总体发现: $FOUND_COUNT 个潜在问题"
        echo "==============================================="
    } >> "$SUMMARY_FILE"

    echo -e "${GREEN}[+] 摘要报告已生成: $SUMMARY_FILE${NC}"
    echo -e "${GREEN}[+] 详细日志: $LOG_FILE${NC}"
}

# 主函数
main() {
    show_banner
    parse_args "$@"
    initialize

    echo -e "${GREEN}[+] 开始扫描目录: $SCAN_DIR (最大深度: $MAX_DEPTH)${NC}"

    # 运行各种检查
    check_suid_sgid
    check_hidden_files
    check_large_files
    check_recent_files
    check_world_writable
    check_malicious_patterns
    check_cron_jobs
    check_startup_scripts
    check_network_connections
    check_suspicious_accounts
    check_web_directories

    # 生成摘要
    generate_summary

    # 结束信息
    echo -e "\n${CYAN}====================================================${NC}"
    echo -e "${GREEN}[+] 安全扫描完成! 发现 $FOUND_COUNT 个潜在问题${NC}"
    echo -e "${GREEN}[+] 结果保存至: $OUTPUT_DIR${NC}"
    echo -e "${CYAN}====================================================${NC}"
}

# 执行主函数
main "$@"