/*
    header.tst - Http response header tests
 */

const HTTP = tget('TM_HTTP') || "127.0.0.1:4100"
const URL = HTTP + "/index.html"
let http: Http = new Http

http.get(HTTP + "/index.html")
connection = http.header("Connection")
ttrue(connection == "Keep-Alive")

http.get(HTTP + "/index.html")
ttrue(http.statusMessage == "OK")
ttrue(http.contentType.indexOf("text/html") == 0)
ttrue(http.date != "")
ttrue(http.lastModified != "")

http.post(HTTP + "/index.html")
ttrue(http.status == 200)

//  Request headers
http.reset()
http.setHeader("key", "value")
http.get(HTTP + "/index.html")
ttrue(http.status == 200)
http.close()
