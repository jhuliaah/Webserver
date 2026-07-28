#!/usr/bin/env python3

# 1. Preparamos o corpo da página
body = "<html><body><h1>Fala, epoll! CGI em Alta Velocidade!</h1></body></html>\n"

# 2. Imprimimos o cabeçalho HTTP completo (COM Content-Length!)
print(f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(body)}\r\n\r\n", end="")

# 3. Imprimimos o corpo
print(body, end="")