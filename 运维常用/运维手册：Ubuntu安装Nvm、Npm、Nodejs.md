在Ubuntu上安装NVM（Node Version Manager），然后使用NVM安装NPM和Node.js，可以按照以下步骤进行操作：

### 1. 安装NVM

首先，打开终端并运行以下命令来下载并安装NVM：

```bash
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.3/install.sh | bash
```

然后，加载NVM到当前终端会话中：

```bash
export NVM_DIR="$([ -z "${XDG_CONFIG_HOME-}" ] && printf %s "${HOME}/.nvm" || printf %s "${XDG_CONFIG_HOME}/nvm")"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"
```

你也可以将上述命令添加到你的终端配置文件中（例如`~/.bashrc`, `~/.zshrc`），这样每次打开终端时NVM都会自动加载。

### 2. 使用NVM安装Node.js和NPM

安装最新的LTS（长期支持）版本的Node.js和NPM：

```bash
nvm install --lts
```

你也可以安装特定版本的Node.js，例如：

```bash
nvm install 14.17.0
```

安装完成后，使用以下命令来查看安装的Node.js和NPM版本：

```bash
node -v
npm -v
```

### 3. 切换Node.js版本

如果你安装了多个版本的Node.js，可以使用以下命令来切换版本：

```bash
nvm use <version>
```

例如：

```bash
nvm use 14.17.0
```

### 4. 设置默认版本

你可以将一个特定版本设置为默认版本，这样每次打开终端时都会自动使用该版本：

```bash
nvm alias default <version>
```

例如：

```bash
nvm alias default 14.17.0
```

这样，你就成功在Ubuntu上安装了NVM，并使用NVM安装和管理了Node.js和NPM。