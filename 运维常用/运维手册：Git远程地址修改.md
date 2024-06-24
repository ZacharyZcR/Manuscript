当你的远程仓库地址变更后，需要更新你本地仓库的远程地址。你可以通过以下步骤来修改本地 Git 的远程分支地址：

1. **打开终端**：打开你的命令行工具。
2. **导航到你的项目目录**：使用 `cd` 命令切换到你的项目所在目录。

```Bash
cd /path/to/your/project
```

1. **查看当前的远程仓库**：运行 `git remote -v` 命令查看当前的远程仓库配置。这会显示当前配置的远程仓库的名字和对应的 URL。

```Bash
git remote -v
```

1. **修改远程仓库的** **URL**：使用 `git remote set-url` 命令来修改远程仓库的 URL。这里假设你的远程仓库名为 `origin`，新的仓库地址为 `new-url`。

```Bash
git remote set-url origin new-url
```

1. **验证更改**：再次使用 `git remote -v` 命令确认远程仓库的 URL 已经更新。

```Bash
git remote -v
```

通过这些步骤，你的本地仓库将会链接到新的远程仓库地址。接下来你可以正常使用 `git push`、`git pull` 等命令与远程仓库交互。如果你有多个远程仓库或特殊配置，也可以按照相同的方法修改对应的设置。