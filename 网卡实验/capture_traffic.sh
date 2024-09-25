#!/bin/bash

# 提示用户输入PID
read -p "请输入要捕获流量的程序PID: " SCRIPT_PID

# 确认PID是否有效
if ! ps -p $SCRIPT_PID > /dev/null; then
    echo "无效的PID: $SCRIPT_PID"
    exit 1
fi

# 使用lsof获取程序的网络端口
PORTS=$(lsof -i -n -P | grep $SCRIPT_PID | grep -Eo 'TCP [^ ]+' | grep -Eo '[0-9]+->|->[^ ]+' | grep -Eo '[0-9]+' | sort | uniq)

if [ -z "$PORTS" ]; then
    echo "未找到程序的网络端口"
    exit 1
fi

echo "捕获端口: $PORTS"

# 构建tcpdump过滤表达式
FILTER=""
for PORT in $PORTS; do
    if [ -z "$FILTER" ]; then
        FILTER="port $PORT"
    else
        FILTER="$FILTER or port $PORT"
    fi
done

# 使用tcpdump捕获流量
sudo tcpdump -i en0 $FILTER -w capture.pcap &

# 等待一段时间后停止tcpdump
# 这里等待时间根据需求调整，假设捕获 1 分钟的流量
sleep 60
sudo pkill tcpdump

echo "流量捕获完成，保存为capture.pcap文件"
