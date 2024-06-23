安装 Emscripten（emcc）可以按照以下步骤进行：

### 一、安装依赖

在安装 Emscripten 之前，您需要确保系统已经安装了以下依赖项：

- Python 3
- Git
- CMake

您可以使用包管理器来安装这些依赖项。例如，在 Debian/Ubuntu 系统上，可以使用以下命令：

```sh
sudo apt update
sudo apt install python3 git cmake
```

在 macOS 上，可以使用 Homebrew：

```sh
brew install python3 git cmake
```

### 二、安装 Emscripten SDK

Emscripten SDK 包含了所有必要的工具和库。以下是安装 Emscripten SDK 的步骤：

1. **下载并安装 Emscripten SDK**：

   打开终端，执行以下命令来下载并安装 Emscripten SDK：

   ```sh
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ```

2. **获取最新的 SDK**：

   执行以下命令来获取最新的 Emscripten SDK：

   ```sh
   ./emsdk install latest
   ./emsdk activate latest
   ```

3. **设置环境变量**：

   每次使用 Emscripten 工具链之前，需要设置环境变量。可以通过以下命令来设置：

   ```sh
   source ./emsdk_env.sh
   ```

   为了每次打开终端时自动设置环境变量，可以将上述命令添加到您的 `~/.bashrc` 或 `~/.zshrc` 文件中（取决于您使用的 shell）。

   ```sh
   echo 'source /path/to/emsdk/emsdk_env.sh' >> ~/.bashrc
   # or if using zsh
   echo 'source /path/to/emsdk/emsdk_env.sh' >> ~/.zshrc
   ```

### 三、验证安装

完成以上步骤后，可以通过以下命令来验证 Emscripten 是否安装成功：

```sh
emcc -v
```

如果安装成功，您将看到 Emscripten 编译器的版本信息。

### 四、编译示例代码

您可以编写一个简单的 C 代码并使用 Emscripten 进行编译以测试安装是否成功。以下是一个简单示例：

1. 创建一个 C 文件 `hello.c`：

   ```c
   #include <stdio.h>
   
   int main() {
       printf("Hello, WebAssembly!\n");
       return 0;
   }
   ```

2. 使用 Emscripten 编译：

   ```c
   emcc hello.c -o hello.html
   ```

3. 打开生成的 `hello.html` 文件，您应该能够在浏览器控制台中看到 "Hello, WebAssembly!" 的输出。

通过以上步骤，您应该能够成功安装并使用 Emscripten。如果在安装过程中遇到任何问题，请随时告诉我，我会尽力帮助您解决。