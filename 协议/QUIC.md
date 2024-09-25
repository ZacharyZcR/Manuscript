下面是一个简单的教程，介绍如何使用`quic-go`库编写一个QUIC协议的客户端和服务端，并将输出改为中文。

### 环境准备

1. **安装Go语言环境**：确保你的计算机上已经安装了Go语言环境。可以通过访问[Go官网](https://golang.org/)下载并安装。

2. **安装`quic-go`库**：在你的项目目录下运行以下命令来安装`quic-go`库：

   ```bash
   go get -u github.com/quic-go/quic-go
   ```

3. **生成自签名证书**：使用OpenSSL生成自签名证书和密钥：

   ```bash
   openssl req -x509 -newkey rsa:2048 -keyout server.key -out server.crt -days 365 -nodes
   ```

### 服务端代码

以下是服务端的代码示例，输出信息已更改为中文：

```go
package main

import (
	"context"
	"crypto/tls"
	"fmt"
	"github.com/quic-go/quic-go"
	"io"
	"log"
)

func main() {
	tlsConfig, err := generateTLSConfig()
	if err != nil {
		log.Fatalf("生成TLS配置失败: %v", err)
	}

	listener, err := quic.ListenAddr("localhost:4242", tlsConfig, nil)
	if err != nil {
		log.Fatalf("监听地址失败: %v", err)
	}

	fmt.Println("服务端正在监听...")

	for {
		conn, err := listener.Accept(context.Background())
		if err != nil {
			log.Printf("接受连接失败: %v", err)
			continue
		}

		go handleConnection(conn)
	}
}

func handleConnection(conn quic.Connection) {
	stream, err := conn.AcceptStream(context.Background())
	if err != nil {
		log.Printf("接受流失败: %v", err)
		return
	}

	message := make([]byte, 1024)
	n, err := stream.Read(message)
	if err != nil && err != io.EOF {
		log.Printf("读取流失败: %v", err)
		return
	}

	fmt.Printf("收到: %s\n", string(message[:n]))

	_, err = stream.Write([]byte("来自服务端的问候"))
	if err != nil {
		log.Printf("写入流失败: %v", err)
	}
}

func generateTLSConfig() (*tls.Config, error) {
	cert, err := tls.LoadX509KeyPair("server.crt", "server.key")
	if err != nil {
		return nil, fmt.Errorf("加载密钥对失败: %w", err)
	}

	return &tls.Config{
		Certificates: []tls.Certificate{cert},
		NextProtos:   []string{"quic-echo-example"},
	}, nil
}
```

### 客户端代码

以下是客户端的代码示例，输出信息已更改为中文：

```go
package main

import (
	"context"
	"crypto/tls"
	"fmt"
	"github.com/quic-go/quic-go"
	"log"
)

func main() {
	tlsConfig := &tls.Config{
		InsecureSkipVerify: true,
		NextProtos:         []string{"quic-echo-example"},
	}

	ctx := context.Background()
	quicConfig := &quic.Config{}

	session, err := quic.DialAddr(ctx, "localhost:4242", tlsConfig, quicConfig)
	if err != nil {
		log.Fatal("连接服务端失败:", err)
	}

	stream, err := session.OpenStreamSync(ctx)
	if err != nil {
		log.Fatal("打开流失败:", err)
	}

	message := "来自客户端的问候"
	_, err = stream.Write([]byte(message))
	if err != nil {
		log.Fatal("写入流失败:", err)
	}

	fmt.Printf("发送: %s\n", message)

	buf := make([]byte, 1024)
	n, err := stream.Read(buf)
	if err != nil {
		log.Fatal("读取流失败:", err)
	}

	fmt.Printf("收到: %s\n", string(buf[:n]))
}
```

### 运行程序

1. **运行服务端**：在终端中执行`go run server.go`，服务端将开始监听。

2. **运行客户端**：在另一个终端中执行`go run client.go`，客户端将连接到服务端并进行通信。

### 注意事项

- 确保`server.crt`和`server.key`文件存在于服务端代码的同一目录中。
- 在生产环境中，请使用有效的TLS证书而不是自签名证书。
- 确保你的防火墙允许UDP 4242端口的通信。

通过以上步骤，你可以成功编写并运行一个简单的QUIC客户端和服务端应用，并且所有输出信息都是中文。