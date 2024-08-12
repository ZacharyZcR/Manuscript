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

	// 创建一个背景上下文
	ctx := context.Background()

	// 使用默认的quic配置
	quicConfig := &quic.Config{}

	session, err := quic.DialAddr(ctx, "localhost:4242", tlsConfig, quicConfig)
	if err != nil {
		log.Fatal(err)
	}

	stream, err := session.OpenStreamSync(ctx)
	if err != nil {
		log.Fatal(err)
	}

	message := "Hello from client"
	_, err = stream.Write([]byte(message))
	if err != nil {
		log.Fatal(err)
	}

	fmt.Printf("Sent: %s\n", message)

	buf := make([]byte, 1024)
	n, err := stream.Read(buf)
	if err != nil {
		log.Fatal(err)
	}

	fmt.Printf("Received: %s\n", string(buf[:n]))
}
