/*
    Session controller
 */
#include "esp.h"

static void login() { 
    if (getSessionVar("id")) {
        render("Logged in");

    } else if (smatch(param("username"), "admin") && smatch(param("password"), "secret")) {
        render("Valid Login");
        setSessionVar("id", "admin");

    } else if (smatch(getMethod(), "POST")) {
        render("Invalid login, please retry.");

    } else {
        createSession();
        securityToken();
        render("Please Login");
    }
}

ESP_EXPORT int esp_controller_app_session(HttpRoute *route, MprModule *module) {
    espAction(route, "session/login", login);
    return 0;
}
