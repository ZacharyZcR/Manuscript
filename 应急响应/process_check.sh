#!/bin/bash
# 跨平台系统进程监控脚本 (v2.0)
# 支持Linux、macOS和Windows(通过WSL或Git Bash)

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # 无颜色

# 检测操作系统类型
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="Linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macOS"
    elif [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* ]]; then
        OS="Windows"
    else
        OS="Unknown"
    fi
    echo $OS
}

# 清屏
clear

# 获取操作系统类型
OS=$(detect_os)

# 标题与系统信息
echo -e "${CYAN}====================================================${NC}"
echo -e "${WHITE}               系统进程安全监控脚本 v2.0           ${NC}"
echo -e "${CYAN}====================================================${NC}"
echo -e "${GREEN}[+] 执行时间: $(date '+%Y-%m-%d %H:%M:%S')${NC}"
echo -e "${GREEN}[+] 操作系统: ${OS}${NC}"

# 检查必要的命令
check_commands() {
    local missing_commands=0

    case $OS in
        Linux)
            for cmd in ps grep awk sort head find; do
                if ! command -v $cmd &> /dev/null; then
                    echo -e "${RED}[-] 缺少必要命令: $cmd${NC}"
                    missing_commands=1
                fi
            done
            ;;
        macOS)
            for cmd in ps grep awk sort head find; do
                if ! command -v $cmd &> /dev/null; then
                    echo -e "${RED}[-] 缺少必要命令: $cmd${NC}"
                    missing_commands=1
                fi
            done
            ;;
        Windows)
            for cmd in ps grep awk sort head find; do
                if ! command -v $cmd &> /dev/null; then
                    echo -e "${RED}[-] 缺少必要命令: $cmd${NC}"
                    missing_commands=1
                fi
            done
            if ! command -v wmic &> /dev/null && ! command -v powershell &> /dev/null; then
                echo -e "${YELLOW}[!] Windows系统下可能需要额外安装工具以获取全部功能${NC}"
            fi
            ;;
    esac

    if [ $missing_commands -eq 1 ]; then
        echo -e "${RED}[-] 一些必要的命令不可用，脚本可能无法正常运行${NC}"
    else
        echo -e "${GREEN}[+] 所有必要命令可用${NC}"
    fi
}

# 显示系统负载
show_system_load() {
    echo -e "\n${BLUE}[*] 系统负载信息:${NC}"
    case $OS in
        Linux)
            echo -e "  ${CYAN}CPU 核心数: $(grep -c processor /proc/cpuinfo)${NC}"
            echo -e "  ${CYAN}内存总量: $(free -h | grep Mem | awk '{print $2}')${NC}"
            echo -e "  ${CYAN}已用内存: $(free -h | grep Mem | awk '{print $3}') ($(free | grep Mem | awk '{printf "%.1f%%", $3/$2*100}'))${NC}"
            echo -e "  ${CYAN}系统负载: $(uptime | sed -E 's/.*load average: (.*)/\1/')${NC}"
            ;;
        macOS)
            echo -e "  ${CYAN}CPU 核心数: $(sysctl -n hw.ncpu)${NC}"
            echo -e "  ${CYAN}内存总量: $(sysctl -n hw.memsize | awk '{printf "%.1f GB", $1/1024/1024/1024}')${NC}"
            echo -e "  ${CYAN}系统负载: $(uptime | sed -E 's/.*load averages: (.*)/\1/')${NC}"
            ;;
        Windows)
            if command -v wmic &> /dev/null; then
                echo -e "  ${CYAN}CPU 核心数: $(wmic cpu get NumberOfCores | grep -v NumberOfCores | tr -d '\r\n ')${NC}"
                echo -e "  ${CYAN}内存总量: $(wmic computersystem get TotalPhysicalMemory | grep -v TotalPhysicalMemory | awk '{printf "%.1f GB", $1/1024/1024/1024}')${NC}"
            elif command -v powershell &> /dev/null; then
                echo -e "  ${CYAN}CPU 核心数: $(powershell -command "Get-WmiObject -Class Win32_Processor | Measure-Object -Property NumberOfCores -Sum | Select-Object -ExpandProperty Sum")${NC}"
                echo -e "  ${CYAN}内存总量: $(powershell -command "[math]::Round((Get-WmiObject -Class Win32_ComputerSystem).TotalPhysicalMemory/1GB, 2)" | tr -d '\r\n') GB${NC}"
            else
                echo -e "  ${YELLOW}[!] 无法获取完整系统信息，Windows需要wmic或powershell支持${NC}"
            fi
            ;;
    esac
}

# 显示CPU使用率前十的进程
show_top_cpu_processes() {
    echo -e "\n${YELLOW}[*] CPU使用率前十的进程:${NC}"
    echo -e "  ${WHITE}PID      用户     CPU%     内存%    命令${NC}"

    case $OS in
        Linux)
            ps aux --sort=-%cpu | head -11 | grep -v USER | \
                awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $2, $1, $3, $4, $11}'
            ;;
        macOS)
            ps aux | sort -nr -k 3 | head -10 | \
                awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $2, $1, $3, $4, $11}'
            ;;
        Windows)
            if command -v wmic &> /dev/null; then
                # 尝试使用wmic获取进程信息
                wmic process get ProcessId,Name,PercentProcessorTime,WorkingSetSize /format:csv | \
                    grep -v "^Node" | sort -t, -k4 -nr | head -10 | \
                    awk -F, '{printf "  %-8s %-8s %-8s %-8s %s\n", $2, "N/A", $4, $5/1024/1024, $3}'
            elif command -v ps &> /dev/null; then
                # 尝试使用Git Bash或WSL的ps命令
                ps aux | sort -nr -k 3 | head -10 | \
                    awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $2, $1, $3, $4, $11}'
            else
                echo -e "  ${YELLOW}[!] 无法获取CPU使用率信息${NC}"
            fi
            ;;
    esac
}

# 显示内存使用率前十的进程
show_top_memory_processes() {
    echo -e "\n${YELLOW}[*] 内存使用率前十的进程:${NC}"
    echo -e "  ${WHITE}PID      用户     CPU%     内存%    命令${NC}"

    case $OS in
        Linux)
            ps aux --sort=-%mem | head -11 | grep -v USER | \
                awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $2, $1, $3, $4, $11}'
            ;;
        macOS)
            ps aux | sort -nr -k 4 | head -10 | \
                awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $2, $1, $3, $4, $11}'
            ;;
        Windows)
            if command -v wmic &> /dev/null; then
                # 尝试使用wmic获取进程信息
                wmic process get ProcessId,Name,PercentProcessorTime,WorkingSetSize /format:csv | \
                    grep -v "^Node" | sort -t, -k5 -nr | head -10 | \
                    awk -F, '{printf "  %-8s %-8s %-8s %-8.1f %s\n", $2, "N/A", $4, $5/1024/1024/1024*100, $3}'
            elif command -v ps &> /dev/null; then
                # 尝试使用Git Bash或WSL的ps命令
                ps aux | sort -nr -k 4 | head -10 | \
                    awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $2, $1, $3, $4, $11}'
            else
                echo -e "  ${YELLOW}[!] 无法获取内存使用率信息${NC}"
            fi
            ;;
    esac
}

# 显示父进程为1的进程
show_parent_processes() {
    echo -e "\n${MAGENTA}[*] 父进程为1的关键进程:${NC}"
    echo -e "  ${WHITE}PID      PPID     用户     命令${NC}"

    case $OS in
        Linux)
            init_processes=$(ps -eo pid,ppid,user,cmd --sort=start_time | awk '$2 == 1 && !/(systemd|init|upstart)/ {print}')
            if [ -n "$init_processes" ]; then
                echo "$init_processes" | awk '{printf "  %-8s %-8s %-8s %s\n", $1, $2, $3, $4}'
            else
                echo -e "  ${GREEN}未发现异常的父进程为1的进程${NC}"
            fi
            ;;
        macOS)
            init_processes=$(ps -eo pid,ppid,user,comm | awk '$2 == 1 && !/(launchd|kernel)/ {print}')
            if [ -n "$init_processes" ]; then
                echo "$init_processes" | awk '{printf "  %-8s %-8s %-8s %s\n", $1, $2, $3, $4}'
            else
                echo -e "  ${GREEN}未发现异常的父进程为1的进程${NC}"
            fi
            ;;
        Windows)
            echo -e "  ${YELLOW}[!] 此功能在Windows系统下不完全适用${NC}"
            if command -v ps &> /dev/null; then
                # 尝试查找PPID为1或其他低值的进程
                init_processes=$(ps -eo pid,ppid,user,comm 2>/dev/null | awk '$2 < 5 && $2 > 0 {print}')
                if [ -n "$init_processes" ]; then
                    echo -e "  ${WHITE}以下是PPID较低的进程:${NC}"
                    echo "$init_processes" | awk '{printf "  %-8s %-8s %-8s %s\n", $1, $2, $3, $4}'
                fi
            fi
            ;;
    esac
}

# 检测反弹shell
detect_reverse_shells() {
    echo -e "\n${RED}[*] 检测疑似Bash反弹Shell:${NC}"

    case $OS in
        Linux|macOS)
            suspicious=$(ps aux | grep -E "bash -i >& /dev/tcp/|bash.*\|.*nc|nc.*\|.*bash|\.\/.*sh.*[0-9]+\.[0-9]+|python.*socket|perl.*socket|ruby.*socket" | grep -v grep)
            if [ -n "$suspicious" ]; then
                echo -e "  ${WHITE}用户     PID      CPU%     内存%    命令${NC}"
                echo "$suspicious" | awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $1, $2, $3, $4, $11}'
            else
                echo -e "  ${GREEN}未发现疑似反弹Shell${NC}"
            fi
            ;;
        Windows)
            echo -e "  ${YELLOW}[!] 此功能在Windows系统下局限性较大${NC}"
            if command -v ps &> /dev/null; then
                suspicious=$(ps aux 2>/dev/null | grep -E "powershell.*-e|powershell.*downloadstring|powershell.*iex|cmd.exe.*\/c" | grep -v grep)
                if [ -n "$suspicious" ]; then
                    echo -e "  ${WHITE}疑似可疑PowerShell或CMD进程:${NC}"
                    echo "$suspicious" | awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $1, $2, $3, $4, $11}'
                else
                    echo -e "  ${GREEN}未发现疑似可疑Shell进程${NC}"
                fi
            fi
            ;;
    esac
}

# SSH软连接后门检测
detect_ssh_backdoors() {
    echo -e "\n${RED}[*] 检测SSH软连接后门:${NC}"

    case $OS in
        Linux|macOS)
            ssh_paths="/usr/sbin /usr/bin /bin /sbin"
            ssh_links=$(find $ssh_paths -type l -name "ssh*" -ls 2>/dev/null)
            if [ -n "$ssh_links" ]; then
                echo -e "  ${WHITE}发现SSH相关软链接:${NC}"
                echo "$ssh_links" | awk '{printf "  %s -> %s\n", $11, $13}'
                echo -e "  ${YELLOW}[!] 请检查以上软链接是否合法${NC}"
            else
                echo -e "  ${GREEN}未发现SSH软连接后门${NC}"
            fi
            ;;
        Windows)
            echo -e "  ${YELLOW}[!] 此功能在Windows系统下不适用${NC}"
            ;;
    esac
}

# 查找可疑的SSH进程
detect_suspicious_ssh() {
    echo -e "\n${RED}[*] 检测可疑的SSH进程:${NC}"

    case $OS in
        Linux|macOS)
            ssh_suspicious=$(ps aux | grep -E "ssh.*-R|ssh.*-D|ssh.*-L|sshd.*-p" | grep -v grep)
            if [ -n "$ssh_suspicious" ]; then
                echo -e "  ${WHITE}用户     PID      CPU%     内存%    命令${NC}"
                echo "$ssh_suspicious" | awk '{printf "  %-8s %-8s %-8.1f %-8.1f %s\n", $1, $2, $3, $4, $11}'
                echo -e "  ${YELLOW}[!] 请确认以上SSH隧道是否已授权${NC}"
            else
                echo -e "  ${GREEN}未发现可疑的SSH隧道进程${NC}"
            fi

            # 检查未授权SSH密钥
            echo -e "\n${RED}[*] 检测可能的未授权SSH密钥:${NC}"
            for user_dir in /home/* /root; do
                if [ -d "$user_dir/.ssh" ]; then
                    user=$(basename "$user_dir")
                    auth_keys="$user_dir/.ssh/authorized_keys"
                    if [ -f "$auth_keys" ]; then
                        key_count=$(wc -l < "$auth_keys")
                        echo -e "  ${YELLOW}用户 ${user} 有 ${key_count} 个SSH密钥${NC}"
                    fi
                fi
            done
            ;;
        Windows)
            echo -e "  ${YELLOW}[!] 此功能在Windows系统下局限性较大${NC}"
            ;;
    esac
}

# 检查可疑的定时任务
check_suspicious_crons() {
    echo -e "\n${RED}[*] 检测可疑的定时任务:${NC}"

    case $OS in
        Linux)
            if [ -x "$(command -v crontab)" ]; then
                for user in $(cut -f1 -d: /etc/passwd); do
                    cron_jobs=$(crontab -u "$user" -l 2>/dev/null | grep -v "^#")
                    if [ -n "$cron_jobs" ]; then
                        echo -e "  ${YELLOW}用户 ${user} 的定时任务:${NC}"
                        echo "$cron_jobs" | awk '{printf "  %s\n", $0}'
                    fi
                done

                # 检查系统定时任务
                echo -e "  ${YELLOW}系统定时任务:${NC}"
                for cron_file in /etc/cron.d/* /etc/crontab; do
                    if [ -f "$cron_file" ]; then
                        echo -e "  ${CYAN}$cron_file:${NC}"
                        grep -v "^#" "$cron_file" | grep -v "^$" | awk '{printf "  %s\n", $0}'
                    fi
                done
            else
                echo -e "  ${YELLOW}[!] 无法检测定时任务，crontab命令不可用${NC}"
            fi
            ;;
        macOS)
            if [ -x "$(command -v crontab)" ]; then
                cron_jobs=$(crontab -l 2>/dev/null | grep -v "^#")
                if [ -n "$cron_jobs" ]; then
                    echo -e "  ${YELLOW}当前用户的定时任务:${NC}"
                    echo "$cron_jobs" | awk '{printf "  %s\n", $0}'
                else
                    echo -e "  ${GREEN}未发现用户定时任务${NC}"
                fi

                # 检查launchd任务
                echo -e "  ${YELLOW}launchd任务:${NC}"
                launchd_jobs=$(ls -la ~/Library/LaunchAgents/ /Library/LaunchAgents/ /Library/LaunchDaemons/ 2>/dev/null)
                if [ -n "$launchd_jobs" ]; then
                    echo "$launchd_jobs" | awk '{printf "  %s\n", $0}'
                else
                    echo -e "  ${GREEN}未发现launchd任务${NC}"
                fi
            else
                echo -e "  ${YELLOW}[!] 无法检测定时任务，crontab命令不可用${NC}"
            fi
            ;;
        Windows)
            echo -e "  ${YELLOW}[!] 此功能在Windows系统下局限性较大${NC}"
            if command -v schtasks &> /dev/null; then
                echo -e "  ${CYAN}Windows计划任务:${NC}"
                schtasks /query /fo LIST | grep -E "TaskName|Last Run Time|Next Run Time|Status" | awk '{printf "  %s\n", $0}'
            fi
            ;;
    esac
}

# 执行所有检测
check_commands
show_system_load
show_top_cpu_processes
show_top_memory_processes
show_parent_processes
detect_reverse_shells
detect_ssh_backdoors
detect_suspicious_ssh
check_suspicious_crons

# 总结
echo -e "\n${CYAN}====================================================${NC}"
echo -e "${GREEN}[+] 系统进程安全检测完成${NC}"
echo -e "${CYAN}====================================================${NC}"

# 脚本使用说明
echo -e "\n${BLUE}[*] 使用说明:${NC}"
echo -e "  ${WHITE}1. 此脚本设计用于检测系统中的可疑进程和安全隐患${NC}"
echo -e "  ${WHITE}2. 根据您的操作系统(${OS})自动选择适当的检测方法${NC}"
echo -e "  ${WHITE}3. 如有异常，请进一步分析相关进程和配置${NC}"
echo -e "  ${WHITE}4. 建议定期运行此脚本进行系统安全检查${NC}"