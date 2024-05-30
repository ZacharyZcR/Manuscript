要使用 Python 创建一个新的虚拟环境，您可以遵循以下步骤：

1. 打开命令行界面：首先，您需要打开命令行界面。这可以是 Windows 的命令提示符、PowerShell，或者是 macOS/Linux 的终端。
2. 导航到项目目录：使用 `cd` 命令导航到您想要创建虚拟环境的目录。例如：

```Bash
cd path/to/your/project
```

1. 创建虚拟环境：使用 `python -m venv` 命令创建一个新的虚拟环境。您需要指定一个目录来存放虚拟环境的文件。例如：

```Plaintext
python -m venv venv
```

1. 这里，`venv` 是虚拟环境的目录名。您可以根据需要选择任何名称。
2. 激活虚拟环境：创建虚拟环境后，您需要激活它。激活命令取决于您的操作系统。
   1. 在 Windows 上，运行：
   2. ```Plaintext
      .\venv\Scripts\activate
      ```

   3. 在 macOS/Linux 上，运行：
   4. ```Bash
      source venv/bin/activate
      ```
3. 使用虚拟环境：一旦虚拟环境被激活，您的命令行提示将会更改，显示虚拟环境的名称。现在，您可以在这个隔离的环境中安装和使用 Python 包，而不会影响到其他项目或您的全局 Python 安装。
4. 退出虚拟环境：当您完成工作并想要退出虚拟环境时，只需在命令行中运行：

```Plaintext
deactivate
```

确保您的系统中已安装了 Python，并且 `python` 命令指向您想要使用的 Python 版本。如果您有多个 Python 版本，可能需要使用 `python3` 或类似的命令来指定正确的版本。