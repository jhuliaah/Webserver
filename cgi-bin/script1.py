#!/usr/bin/env python3
import sys

# O sys.stdin.read() vai "chupar" o tubo até o C++ fazer close(stdin_fd)
body_recebido = sys.stdin.read()

html = f"<html><body><h1>Sucesso absoluto! C++ disse: '{body_recebido}'</h1></body></html>\n"

print(f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(html)}\r\n\r\n{html}", end="")