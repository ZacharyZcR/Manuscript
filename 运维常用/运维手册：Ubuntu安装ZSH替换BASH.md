在Ubuntu中设置Zsh（Z Shell）作为默认shell，而不是默认的Bash（Bourne Again Shell），可以通过以下步骤来完成：

1. 安装Zsh：首先确保Zsh已经安装在您的系统上。您可以通过运行以下命令来安装Zsh（如果尚未安装）：

```Bash
sudo apt update
sudo apt install zsh
```

1. 更改默认Shell：安装Zsh后，您可以使用`chsh`（change shell）命令来更改默认shell。要做到这一点，运行：

```Bash
chsh -s $(which zsh)
```

1. 这会将您的默认shell更改为Zsh。
2. 重新登录或重启：为了使更改生效，您需要注销并重新登录，或者重启您的系统。更改默认shell通常不会立即生效，直到您重新登录或重启。
3. 配置Zsh：如果您是第一次使用Zsh，您可能会看到一个配置向导来帮助您设置Zsh的初始配置。您可以选择一个预配置的环境或者手动配置。
4. （可选）安装Oh My Zsh：许多用户喜欢使用Oh My Zsh，这是一个用于管理Zsh配置的开源框架，它提供了大量的主题和插件来自定义您的shell体验。要安装Oh My Zsh，可以运行：

```Bash
sh -c "$(wget https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)"
```

请确保在执行这些步骤之前，您有足够的权限（可能需要使用sudo）。如果您在更改默认shell或配置Zsh时遇到任何问题，请查看相应的文档或搜索特定的错误消息。