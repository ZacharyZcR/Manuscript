#!/bin/bash
# 优化的Zsh安装和配置脚本

# 测试Google连接
echo "测试与Google的连接..."
if curl -s --connect-timeout 5 https://www.google.com > /dev/null; then
  echo "成功连接到Google，继续执行脚本..."
else
  echo "无法连接到Google，脚本停止执行。"
  exit 1
fi

# 更新软件包并安装zsh
echo "更新软件包列表并安装必要软件..."
sudo apt update && sudo apt install -y zsh git curl || {
  echo "安装必要软件失败，脚本停止执行。"
  exit 1
}

# 安装Oh My Zsh（非交互式）
echo "安装Oh My Zsh..."
RUNZSH=no KEEP_ZSHRC=yes sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" || {
  echo "安装Oh My Zsh失败，脚本停止执行。"
  exit 1
}

# 安装zsh-autosuggestions插件
echo "安装zsh-autosuggestions插件..."
git clone https://github.com/zsh-users/zsh-autosuggestions ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions || {
  echo "安装zsh-autosuggestions失败，脚本停止执行。"
  exit 1
}

# 安装zsh-syntax-highlighting插件
echo "安装zsh-syntax-highlighting插件..."
git clone https://github.com/zsh-users/zsh-syntax-highlighting.git ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting || {
  echo "安装zsh-syntax-highlighting失败，脚本停止执行。"
  exit 1
}

# 安装Powerlevel10k主题
echo "安装Powerlevel10k主题..."
git clone --depth=1 https://github.com/romkatv/powerlevel10k.git ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/themes/powerlevel10k || {
  echo "安装Powerlevel10k主题失败，脚本停止执行。"
  exit 1
}

# 更新.zshrc配置
echo "更新.zshrc配置..."
echo "source ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh" >> ~/.zshrc
echo "source ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting/zsh-syntax-highlighting.zsh" >> ~/.zshrc
sed -i 's/plugins=(git)/plugins=(git zsh-autosuggestions zsh-syntax-highlighting)/' ~/.zshrc || {
  echo "更新插件配置失败，脚本停止执行。"
  exit 1
}
sed -i 's/ZSH_THEME=".*"/ZSH_THEME="powerlevel10k\/powerlevel10k"/' ~/.zshrc || {
  echo "更新主题配置失败，脚本停止执行。"
  exit 1
}

# 设置默认shell为zsh
echo "设置默认shell为zsh..."
chsh -s $(which zsh) || {
  echo "设置默认shell失败，脚本停止执行。"
  exit 1
}

# 提示用户重新启动终端
echo "安装完成，请重新启动终端以应用更改。"