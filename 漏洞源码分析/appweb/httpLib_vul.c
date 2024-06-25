static int authCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op) {
    HttpAuth *auth;
    cchar *username, *password;

    assert(conn);
    assert(route);

    auth = route->auth;
    if (!auth || !auth->type) {
        /* Authentication not required */
        return HTTP_ROUTE_OK;
    }
    if (!httpIsAuthenticated(conn)) {
        httpGetCredentials(conn, &username, &password);
        if (!httpLogin(conn, username, password)) {
            if (!conn->tx->finalized) {
                if (auth && auth->type) {
                    (auth->type->askLogin)(conn);
                } else {
                    httpError(conn, HTTP_CODE_UNAUTHORIZED, "Access Denied, login required");
                }
                /* Request has been denied and a response generated. So OK to accept this route. */
            }
            return HTTP_ROUTE_OK;
        }
    }
    if (!httpCanUser(conn, NULL)) {
        httpTrace(conn, "auth.check", "error", "msg:'Access denied, user is not authorized for access'");
        if (!conn->tx->finalized) {
            httpError(conn, HTTP_CODE_FORBIDDEN, "Access denied. User is not authorized for access.");
            /* Request has been denied and a response generated. So OK to accept this route. */
        }
    }
    /* OK to accept route. This does not mean the request was authenticated - an error may have been already generated */
    return HTTP_ROUTE_OK;
}

PUBLIC bool httpGetCredentials(HttpConn *conn, cchar **username, cchar **password) {
    HttpAuth *auth;

    assert(username);
    assert(password);
    *username = *password = NULL;

    auth = conn->rx->route->auth;
    if (!auth || !auth->type) {
        return 0;
    }
    if (auth->type) {
        if (conn->authType && !smatch(conn->authType, auth->type->name)) {
            if (!(smatch(auth->type->name, "form") && conn->rx->flags & HTTP_POST)) {
                /* If a posted form authentication, ignore any basic|digest details in request */
                return 0;
            }
        }
        if (auth->type->parseAuth && (auth->type->parseAuth)(conn, username, password) < 0) {
            return 0;
        }
    } else {
        *username = httpGetParam(conn, "username", 0);
        *password = httpGetParam(conn, "password", 0);
    }
    return 1;
}

PUBLIC bool httpLogin(HttpConn *conn, cchar *username, cchar *password) {
    HttpRx *rx;
    HttpAuth *auth;
    HttpSession *session;
    HttpVerifyUser verifyUser;

    rx = conn->rx;
    auth = rx->route->auth;
    if (!username || !*username) {
        httpTrace(conn, "auth.login.error", "error", "msg:'missing username'");
        return 0;
    }
    if (!auth->store) {
        mprLog("error http auth", 0, "No AuthStore defined");
        return 0;
    }
    if ((verifyUser = auth->verifyUser) == 0) {
        if (!auth->parent || (verifyUser = auth->parent->verifyUser) == 0) {
            verifyUser = auth->store->verifyUser;
        }
    }
    if (!verifyUser) {
        mprLog("error http auth", 0, "No user verification routine defined on route %s", rx->route->pattern);
        return 0;
    }
    if (auth->username && *auth->username) {
        /* If using auto-login, replace the username */
        username = auth->username;
        password = 0;
    }
    if (!(verifyUser)(conn, username, password)) {
        return 0;
    }
    if (!(auth->flags & HTTP_AUTH_NO_SESSION) && !auth->store->noSession) {
        if ((session = httpCreateSession(conn)) == 0) {
            /* Too many sessions */
            return 0;
        }
        httpSetSessionVar(conn, HTTP_SESSION_USERNAME, username);
        httpSetSessionVar(conn, HTTP_SESSION_IP, conn->ip);
    }
    rx->authenticated = 1;
    rx->authenticateProbed = 1;
    conn->username = sclone(username);
    conn->encoded = 0;
    return 1;
}

