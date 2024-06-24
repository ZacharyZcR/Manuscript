下面是翻译成中文的表格：

| 命令                     | 描述                                          |
| ------------------------ | --------------------------------------------- |
| ?                        | 帮助菜单                                      |
| background               | 将当前会话放入后台                            |
| bg                       | background的别名                              |
| bgkill                   | 终止一个后台Meterpreter脚本                   |
| bglist                   | 列出正在运行的后台脚本                        |
| bgrun                    | 以后台线程方式执行一个Meterpreter脚本         |
| channel                  | 显示或控制活动通道                            |
| close                    | 关闭一个通道                                  |
| detach                   | 分离Meterpreter会话（用于http/https）         |
| disable_unicode_encoding | 禁用Unicode字符串编码                         |
| enable_unicode_encoding  | 启用Unicode字符串编码                         |
| exit                     | 终止Meterpreter会话                           |
| get_timeouts             | 获取当前会话超时值                            |
| guid                     | 获取会话GUID                                  |
| help                     | 帮助菜单                                      |
| info                     | 显示一个Post模块的信息                        |
| irb                      | 在当前会话上打开一个交互式Ruby shell          |
| load                     | 加载一个或多个Meterpreter扩展                 |
| machine_id               | 获取连接到会话的机器的MSF ID                  |
| migrate                  | 将服务器迁移到另一个进程                      |
| pivot                    | 管理枢纽监听器                                |
| pry                      | 在当前会话上打开Pry调试器                     |
| quit                     | 终止Meterpreter会话                           |
| read                     | 从一个通道读取数据                            |
| resource                 | 运行存储在文件中的命令                        |
| run                      | 执行一个Meterpreter脚本或Post模块             |
| secure                   | 在会话上（重新）协商TLV数据包加密             |
| sessions                 | 快速切换到另一个会话                          |
| set_timeouts             | 设置当前会话超时值                            |
| sleep                    | 强制Meterpreter安静一段时间，然后重新建立会话 |
| ssl_verify               | 修改SSL证书验证设置                           |
| transport                | 管理传输机制                                  |
| use                      | 已弃用的"load"命令别名                        |
| uuid                     | 获取当前会话的UUID                            |
| write                    | 向一个通道写入数据                            |

下面是翻译成中文的表格：

### Stdapi: 文件系统命令

| 命令       | 描述                           |
| ---------- | ------------------------------ |
| cat        | 将文件内容读取到屏幕上         |
| cd         | 更改目录                       |
| checksum   | 获取文件的校验和               |
| cp         | 复制源文件到目标位置           |
| del        | 删除指定的文件                 |
| dir        | 列出文件（ls的别名）           |
| download   | 下载文件或目录                 |
| edit       | 编辑文件                       |
| getlwd     | 打印本地工作目录（lpwd的别名） |
| getwd      | 打印工作目录                   |
| lcat       | 将本地文件内容读取到屏幕上     |
| lcd        | 更改本地工作目录               |
| ldir       | 列出本地文件（lls的别名）      |
| lls        | 列出本地文件                   |
| lmkdir     | 在本地机器上创建新目录         |
| lpwd       | 打印本地工作目录               |
| ls         | 列出文件                       |
| mkdir      | 创建目录                       |
| mv         | 移动源文件到目标位置           |
| pwd        | 打印工作目录                   |
| rm         | 删除指定的文件                 |
| rmdir      | 删除目录                       |
| search     | 搜索文件                       |
| show_mount | 列出所有挂载点/逻辑驱动器      |
| upload     | 上传文件或目录                 |

下面是翻译成中文的表格：

### Stdapi: 用户界面命令

| 命令          | 描述                                 |
| ------------- | ------------------------------------ |
| enumdesktops  | 列出所有可访问的桌面和窗口站点       |
| getdesktop    | 获取当前Meterpreter桌面              |
| idletime      | 返回远程用户的空闲时间（以秒为单位） |
| keyboard_send | 发送按键                             |
| keyevent      | 发送按键事件                         |
| keyscan_dump  | 导出按键缓冲区                       |
| keyscan_start | 开始捕获按键                         |
| keyscan_stop  | 停止捕获按键                         |
| mouse         | 发送鼠标事件                         |
| screenshare   | 实时查看远程用户桌面                 |
| screenshot    | 捕获交互式桌面的屏幕截图             |
| setdesktop    | 更改Meterpreter当前桌面              |
| uictl         | 控制一些用户界面组件                 |

下面是翻译成中文的表格：

### Stdapi: 摄像头命令

| 命令          | 描述                    |
| ------------- | ----------------------- |
| record_mic    | 录制默认麦克风的音频X秒 |
| webcam_chat   | 开始视频聊天            |
| webcam_list   | 列出摄像头              |
| webcam_snap   | 从指定的摄像头拍摄快照  |
| webcam_stream | 播放指定摄像头的视频流  |

### Stdapi: 音频输出命令

| 命令 | 描述                                |
| ---- | ----------------------------------- |
| play | 在目标系统上播放波形音频文件 (.wav) |

### Priv: 提权命令

| 命令      | 描述                     |
| --------- | ------------------------ |
| getsystem | 尝试将权限提升到本地系统 |

### Priv: 密码数据库命令

| 命令     | 描述                |
| -------- | ------------------- |
| hashdump | 转储SAM数据库的内容 |

### Priv: 时间篡改命令

| 命令      | 描述               |
| --------- | ------------------ |
| timestomp | 操作文件的MACE属性 |