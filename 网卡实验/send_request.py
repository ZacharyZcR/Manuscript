import socket
import ssl
import http.client
import time

def send_request():
    while True:
        try:
            # 创建一个绑定到指定端口的socket
            local_port = 12345
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.bind(('', local_port))

            # 包装socket以支持SSL
            context = ssl.create_default_context()
            conn = context.wrap_socket(sock, server_hostname="www.baidu.com")

            # 连接到目标服务器
            conn.connect(("www.baidu.com", 443))

            # 创建HTTP连接对象
            http_conn = http.client.HTTPSConnection("www.baidu.com", context=context, sock=conn)

            # 发送HTTP请求
            http_conn.request("GET", "/")
            response = http_conn.getresponse()

            # 输出响应状态码
            print(f'状态码: {response.status}')
            # 输出响应内容长度
            content = response.read()
            print(f'内容长度: {len(content)}')

            # 关闭连接
            http_conn.close()
        except Exception as e:
            print(f'请求失败: {e}')

        # 等待 5 秒
        time.sleep(5)

if __name__ == '__main__':
    send_request()
