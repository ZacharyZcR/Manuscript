以下是一个bash脚本，可以一键安装zsh、Oh My Zsh、zsh-autosuggestions插件、zsh-syntax-highlighting插件以及Powerlevel10k主题，并更新.zshrc配置：

```bash
#!/bin/bash

# 更新软件包并安装zsh
sudo apt update && sudo apt install -y zsh git curl

# 安装Oh My Zsh
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

# 安装zsh-autosuggestions插件
git clone https://github.com/zsh-users/zsh-autosuggestions ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions

# 安装zsh-syntax-highlighting插件
git clone https://github.com/zsh-users/zsh-syntax-highlighting.git ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting

# 安装Powerlevel10k主题
git clone --depth=1 https://github.com/romkatv/powerlevel10k.git ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/themes/powerlevel10k

# 更新.zshrc配置
echo "source ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh" >> ~/.zshrc
echo "source ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting/zsh-syntax-highlighting.zsh" >> ~/.zshrc
sed -i 's/plugins=(git)/plugins=(git zsh-autosuggestions zsh-syntax-highlighting)/' ~/.zshrc
sed -i 's/ZSH_THEME=".*"/ZSH_THEME="powerlevel10k\/powerlevel10k"/' ~/.zshrc

# 设置默认shell为zsh
chsh -s $(which zsh)

# 提示用户重新启动终端
echo "安装完成，请重新启动终端以应用更改。"

```

将以上内容保存为一个bash脚本文件，例如`install_zsh.sh`，然后在终端中运行以下命令来执行脚本：

```bash
chmod +x install_zsh.sh
./install_zsh.sh
```

这样就可以一键安装并配置zsh和相关插件及主题了。