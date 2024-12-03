# Windows 驱动管理工具对比分析

## SC (Service Control)

### 主要功能
1. 服务管理
   - 创建/删除服务
   - 启动/停止服务
   - 查询服务状态
   - 修改服务配置

### 常用命令
```cmd
sc create <服务名> binPath= <路径>    // 创建服务
sc start <服务名>                    // 启动服务
sc stop <服务名>                     // 停止服务
sc delete <服务名>                   // 删除服务
sc query <服务名>                    // 查询服务状态
```

### 适用场景
- 系统服务类驱动
- EDR/XDR驱动
- 防病毒驱动
- 需要手动控制的驱动程序

## PnPUtil (Plug and Play Utility)

### 主要功能
1. 驱动包管理
   - 添加/删除驱动包
   - 枚举驱动包
   - 强制安装驱动
   - 管理驱动商店

### 常用命令
```cmd
pnputil -i -a <inf文件>              // 安装驱动
pnputil -e                          // 枚举所有驱动
pnputil -f -d <oem*.inf>           // 强制删除驱动
pnputil -a <inf文件>                // 添加驱动包
```

### 适用场景
- 硬件设备驱动
- 即插即用设备
- 需要INF文件的驱动
- 驱动包管理

## DevCon (Device Console)

### 主要功能
1. 设备管理
   - 安装/移除设备
   - 启用/禁用设备
   - 更新设备驱动
   - 查询设备状态

### 常用命令
```cmd
devcon install <inf文件> <硬件ID>    // 安装设备
devcon remove <设备ID>              // 移除设备
devcon enable <设备ID>              // 启用设备
devcon disable <设备ID>             // 禁用设备
devcon status *                     // 查看所有设备状态
```

### 适用场景
- 设备级别的管理
- 硬件故障排查
- 驱动测试
- 设备状态控制

## 工具选择建议

1. 使用SC的情况
- 纯软件驱动服务
- 需要服务级别控制
- 系统服务类驱动
- 需要编程控制的场景

2. 使用PnPUtil的情况
- 驱动包的部署
- 系统驱动商店管理
- INF文件相关操作
- 批量驱动安装

3. 使用DevCon的情况
- 设备级别管理
- 硬件问题诊断
- 驱动测试验证
- 需要详细设备信息

## 实际应用场景

1. EDR驱动管理
```cmd
# 使用SC
sc create edrdrv type= kernel binPath= c:\edrdrv.sys
sc start edrdrv
```

2. USB设备驱动安装
```cmd
# 使用PnPUtil
pnputil -i -a usbdriver.inf
```

3. 设备问题排查
```cmd
# 使用DevCon
devcon status @USB\*
devcon restart USB\VID*
```

## 注意事项

1. 权限要求
- SC需要管理员权限
- PnPUtil需要管理员权限
- DevCon需要管理员权限

2. 操作风险
- SC操作可能影响系统服务
- PnPUtil可能影响驱动商店
- DevCon可能影响设备运行

3. 兼容性考虑
- 不同Windows版本命令可能略有差异
- 某些命令在特定版本可能不可用
- 需要考虑系统版本兼容性

## 最佳实践

1. 脚本编写
- 合理组合多个工具
- 添加错误处理
- 记录操作日志
- 做好备份措施

2. 故障恢复
- 准备回滚方案
- 保存原始配置
- 制定应急预案

3. 日常维护
- 定期检查服务状态
- 更新驱动版本
- 清理无用驱动包
- 监控设备状态

## 总结
- SC适合服务管理
- PnPUtil适合驱动包管理
- DevCon适合设备管理
- 根据场景选择合适工具
- 注意操作安全性