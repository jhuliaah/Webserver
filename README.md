*This project was created as part of the 42 curriculum by jhualves, ratanaka.*

# Webserv

## Description

**Webserv** is a fully non-blocking HTTP/1.1 server written from scratch in **C++98**, modeled after the behavior of **NGINX**. It was built as part of the 42 School curriculum, with the goal of understanding the HTTP protocol at a low level — from parsing raw byte streams off a socket to generating spec-compliant responses — without relying on any external HTTP or networking library.

The server is configured through an NGINX-style configuration file (`server {}` / `location {}` blocks) and supports:

- **Static file serving**, including directory listing (autoindex) and configurable default index files
- **GET, POST, and DELETE** methods
- **File uploads** from clients, with a configurable upload directory and `413 Payload Too Large` enforcement
- **CGI execution** (e.g. PHP-CGI, Python), including chunked request bodies and CGI output without `Content-Length`
- **HTTP redirection** at the route level
- **Custom and default error pages**
- **Multiple `server` blocks**, each listening on its own `host:port` pair
- **Per-route configuration**: allowed methods, root directory, index file, autoindex, upload path, CGI extension mapping

All client I/O — including the listening sockets — is multiplexed through a **single `epoll()` instance**, watching for both readability and writability, with no blocking `read()`/`recv()`/`write()`/`send()` calls anywhere and no use of `errno` to steer behavior after an I/O operation, as required by the subject.

## Instructions

### Build

```bash
make        # builds the webserv binary
make clean  # removes object files
make fclean # removes object files and the binary
make re     # fclean + all
```

### Run

```bash
./webserv [path/to/config/file]
```

If no configuration file is given, a default configuration path is used.

### Example

```bash
./webserv config_files/default/config.conf
```

Once running, the server can be tested with a browser or with `curl`/`telnet` against the configured host and port(s), e.g.:

```bash
curl -v http://localhost:8080/
```

### Configuration file

The configuration file follows an NGINX-inspired syntax. A minimal example:

```nginx
server {
    listen 127.0.0.1:8080;
    server_name example;
    root www;
    index index.html;
    client_max_body_size 10M;

    error_page 404 /errors/404.html;

    location / {
        allow_methods GET POST DELETE;
        autoindex on;
    }

    location /upload {
        allow_methods POST;
        upload_dir www/uploads;
    }

    location /cgi-bin {
        allow_methods GET POST;
        cgi_path /usr/bin/python3;
        cgi_ext .py;
    }
}
```

Multiple `server` blocks may be declared to bind the process to several `interface:port` pairs and serve different content on each.

## Resources

### References

- [RFC 7230 — HTTP/1.1: Message Syntax and Routing](https://www.rfc-editor.org/rfc/rfc7230)
- [RFC 7231 — HTTP/1.1: Semantics and Content](https://www.rfc-editor.org/rfc/rfc7231)
- [RFC 3875 — The Common Gateway Interface (CGI)](https://www.rfc-editor.org/rfc/rfc3875)
- [NGINX documentation — Core / HTTP module reference](https://nginx.org/en/docs/)
- [epoll(7) — Linux manual page](https://man7.org/linux/man-pages/man7/epoll.7.html)
- Video references used while researching HTTP servers and event-driven I/O:
  - https://www.youtube.com/watch?v=YwHErWJIh6Y
  - https://www.youtube.com/watch?v=bEsRapsPAWI
  - https://www.youtube.com/watch?v=pTIZ9YjE3Pw

### AI usage

Claude (Anthropic) was used throughout the project as a learning and debugging aid, not as a code-generation shortcut. Specifically, it was used to:

- Explain C++98 OOP concepts (constructors, exceptions, class design) for team members coming from a C background, before any implementation was written
- Walk through the mechanics of non-blocking I/O and `epoll`-based event loops, `fd` lifecycle management, and why `EAGAIN` on a non-blocking socket does not mean the peer disconnected
- Review and debug CGI-related issues, including parsing raw HTTP vs. CGI/1.1 output formats, handling chunked request bodies, and preventing CGI pipe `fd` leaks (`FD_CLOEXEC`, cleanup on `removeClient()`)
- Help design and stress-test the routing layer (`Router::matchLoc`/`Router::classify`) and the config parser against a set of invalid-configuration test cases
- Diagnose concurrency bugs found during stress testing (e.g. slowloris resilience, fd leaks after abrupt client disconnects)

All AI-suggested logic was manually reviewed, discussed between teammates, and re-implemented/adapted by hand into the codebase — no code was copy-pasted wholesale from AI output, in line with the project's AI usage rules.

## Team workflow

- Work is pulled and rebuilt before starting any new task, since teammates commit in parallel
- Changes are shared as complete file contents (not partial diffs) to avoid transcription errors
- Branches: `main`, `jhulia`, `cgi_zephele`, `SOcket`
