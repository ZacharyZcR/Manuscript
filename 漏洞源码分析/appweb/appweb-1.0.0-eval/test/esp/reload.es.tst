/*
    reload.tst - ESP reload tests
 */

const HTTP = tget('TM_HTTP') || "127.0.0.1:4100"
let http: Http = new Http

if (thas('ME_DEBUG') && false) {
    //  First get
    let path = new Path("../web/reload.esp")
    path.write('<html><body><% render("First", -1); %></body></html>')
    http.get(HTTP + "/reload.esp")
    ttrue(http.status == 200)
    ttrue(http.response.contains("First"))
    http.close()

    //  To ensure all file system record a different mtime for the file
    App.sleep(1100);

    //  Create a new file and do a second get
    path.write('<html><body><% render("Second", -1); %></body></html>')
    http.get(HTTP + "/reload.esp")
    ttrue(http.status == 200)
    ttrue(http.response.contains("Second"))
    http.close()

    path.remove()

} else {
    tskip("Disabled")
}
