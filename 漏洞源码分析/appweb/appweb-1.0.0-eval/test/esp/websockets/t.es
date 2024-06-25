print("TOP")
    const HTTPS = "https://localhost:4443"
    var WS = HTTPS.replace('https', 'wss') + '/websockets/basic/ssl'
    const TIMEOUT = 5000

print("WS", WS)
    let ws = new WebSocket(WS, ['chat'], { verify: false })
    print("WS", ws)
    print(ws.readyState == WebSocket.CONNECTING)

    let closed, opened, response
    let msg = "Hello Server"
    ws.onopen = function (event) {
        opened = true
        ws.send(msg)
    }
    ws.onclose = function (event) {
        closed = true
    }
    ws.onmessage = function (event) {
        print(event.data is String)
        response = event.data
        ws.close()
    }
    ws.wait(WebSocket.OPEN, TIMEOUT)
    print(opened)

    ws.wait(WebSocket.CLOSED, TIMEOUT)
    print(ws.readyState == WebSocket.CLOSED)
    print(closed)

    //  Text == 1, last == 1, first 10 data bytes
    let info = deserialize(response)
    print(info.type == 1)
    print(info.last == 1)
    print(info.length == msg.length)
    print(info.data == "Hello Serv")
