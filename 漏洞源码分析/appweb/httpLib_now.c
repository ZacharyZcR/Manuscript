static int authCondition(HttpStream *stream, HttpRoute *route, HttpRouteOp *op)
{
    HttpAuth    *auth;
    cchar       *username, *password;

    assert(stream);
    assert(route);

    auth = route->auth;
    if (!auth || !auth->type || !(auth->type->flags & HTTP_AUTH_TYPE_CONDITION)) {
        /* Authentication not required or not using authCondition */
        return HTTP_ROUTE_OK;
    }
    if (!httpIsAuthenticated(stream)) {
        if (!httpGetCredentials(stream, &username, &password) || !httpLogin(stream, username, password)) {
            if (!stream->tx->finalized) {
                if (auth && auth->type) {
                    (auth->type->askLogin)(stream);
                } else {
                    httpError(stream, HTTP_CODE_UNAUTHORIZED, "Access Denied. Login required");
                }
            }
            /*
                Request has been denied and a response generated. So OK to accept this route.
             */
            return HTTP_ROUTE_OK;
        }
    }
    if (!httpCanUser(stream, NULL)) {
        httpLog(stream->trace, "rx.auth.check", "error", "msg:Access denied. User is not authorized for access.");
        if (!stream->tx->finalized) {
            httpError(stream, HTTP_CODE_FORBIDDEN, "Access denied. User is not authorized for access.");
            /* Request has been denied and a response generated. So OK to accept this route. */
        }
    }
    /*
        OK to accept route. This does not mean the request was authenticated - an error may have been already generated
     */
    return HTTP_ROUTE_OK;
}

PUBLIC bool httpLogin(HttpStream *stream, cchar *username, cchar *password)
{
    HttpRx          *rx;
    HttpAuth        *auth;
    HttpVerifyUser  verifyUser;

    rx = stream->rx;
    auth = rx->route->auth;
    if (!username || !*username) {
        httpLog(stream->trace, "auth.login.error", "error", "msg:missing username");
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

    } else if (!username || !password) {
        return 0;
    }
    if (!(verifyUser)(stream, username, password)) {
        return 0;
    }
    if (!(auth->flags & HTTP_AUTH_NO_SESSION) && !auth->store->noSession) {
        if (httpCreateSession(stream) == 0) {
            /* Too many sessions */
            return 0;
        }
        httpSetSessionVar(stream, HTTP_SESSION_USERNAME, username);
        httpSetSessionVar(stream, HTTP_SESSION_IP, stream->ip);
    }
    rx->authenticated = 1;
    rx->authenticateProbed = 1;
    stream->username = sclone(username);
    stream->encoded = 0;
    return 1;
}

PUBLIC bool httpGetCredentials(HttpStream *stream, cchar **username, cchar **password)
{
    HttpAuth    *auth;
    HttpRx      *rx;

    assert(username);
    assert(password);

    rx = stream->rx;

    *username = *password = NULL;

    auth = rx->route->auth;
    if (!auth || !auth->type || !(auth->type->flags & HTTP_AUTH_TYPE_CONDITION)) {
        return 0;
    }
    if (auth->type) {
        if (rx->authType && !smatch(rx->authType, auth->type->name)) {
            return 0;
        }
        if (auth->type->parseAuth && (auth->type->parseAuth)(stream, username, password) < 0) {
            return 0;
        }
    } else {
        *username = httpGetParam(stream, "username", 0);
        *password = httpGetParam(stream, "password", 0);
    }
    return 1;
}
