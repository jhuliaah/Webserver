# Webserver — Status & Roadmap

A quick snapshot of where the project stands: what works, what's built but
broken/incomplete, what hasn't started yet, and the priority order for
what's next.

---

## ✅ Done

- **Socket wrapper** — non-blocking listening socket, `SO_REUSEADDR`
- **Server event loop** — `epoll`-based, accepts connections, dispatches
  read/write events, multi-port capable
- **Client connection handling** — per-connection buffering, read/write
  state machine, 60s inactivity timeout
- **Exception handling** — clean `ServerException`/`SocketException`
  hierarchy used throughout socket/server setup
- **Config data model** — `Config`/`ServerConfig`/`LocationConfig` classes
  match the shape of a real nginx-style config file
- **Mock config generation** — hardcoded test configs for local dev
  (temporary, to be replaced by real parsing)

## ⚠️ Done but needs fixing

- **Request read loop** — only checks for `\r\n\r\n`, ignores
  `Content-Length` entirely, so any request with a body is handled
  incorrectly
- **Response write loop** — sends once and closes; doesn't retry on
  partial writes or `EWOULDBLOCK`
- **Server startup** — binds hardcoded fake ports on `127.0.0.1` instead
  of reading the actual `Config` object it's given
- **Server shutdown** — doesn't close/free connected clients, only sockets
- **`StaticHandler`** — exists but never implements `handle()`, so it's
  still an abstract class and can't be used yet
- **`CgiHandler`** — all methods present but stubbed out (`handle()`
  always returns `false`)
- **`HttpRequest`** — data class exists, but header lookup is a stub and
  it's never actually populated from a real request
- **Config file parser** — a first attempt exists but doesn't compile;
  needs a rewrite
- **`Router`** — matching/classification methods are declared but not
  implemented yet

## ❌ Not started

- Real HTTP request parsing (method/headers/body) wired into the server
- Config-file parsing + validation (reading `.conf` files, rejecting bad
  ones)
- Routing requests to the right location/handler
- Static file serving: `GET`/`POST`/`DELETE`, directory listing
- Custom error pages (403/404/405/413/500)
- Request body size limit enforcement
- File uploads
- CGI script execution
- Keep-alive connections
- Redirects

---

## 📋 To-Do List (priority order)

Fix the foundation first, then build up through the request lifecycle:
parsing → config → routing/static files → uploads/limits → CGI → polish.

### Phase 0 — Foundation fixes
- [ ] Fix request handling to respect `Content-Length` (currently breaks
      on any request body)
- [ ] Fix response sending to handle partial writes / retry properly
- [ ] Make the server actually use the real config for ports/host instead
      of hardcoded values, and clean up client resources on shutdown

### Phase 1 — HTTP parsing
- [ ] Implement full HTTP request parsing (method, URI, headers, body)

### Phase 2 — Config
- [ ] Write a working config file parser
- [ ] Add config validation (reject duplicate/invalid directives)
- [ ] Connect the server to load real config files instead of mocks

### Phase 3 — Routing & static files
- [ ] Implement location matching and request classification (static vs
      CGI vs directory vs error)
- [ ] Implement static file serving (GET/POST/DELETE, directory listing,
      method restrictions)
- [ ] Build proper HTTP responses and connect everything together
- [ ] Serve custom error pages from config

### Phase 4 — Limits & uploads
- [ ] Enforce max request body size (413 errors)
- [ ] Implement file upload/delete handling

### Phase 5 — CGI
- [ ] Implement CGI script execution (spawn process, pass request data,
      capture output, handle timeouts)

### Phase 6 — Polish
- [ ] Decide on and implement keep-alive vs close-per-request
- [ ] Stress-test with malformed requests and concurrent load
- [ ] Clean up leftover/dead code before final submission

---

*This document reflects a point-in-time status and will be updated as work
progresses.*
