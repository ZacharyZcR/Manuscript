package main

import (
	"context"
	"crypto/tls"
	"fmt"
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
