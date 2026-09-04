#!/usr/bin/python3
import os
import sys

print("Content-Type: text/html\r\n\r\n")

method = os.environ.get("REQUEST_METHOD", "UNKNOWN")
query_string = os.environ.get("QUERY_STRING", "")

body = ""
if method == "POST":
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        if content_length > 0:
            body = sys.stdin.read(content_length)
        else:
            body = sys.stdin.read()
    except Exception as e:
        body = f"Erro ao ler stdin: {str(e)}"

print(f"<html><body>")
print(f"<h1>Metodo Recebido: {method}</h1>")
print(f"<p>Query String (GET): {query_string}</p>")
print(f"<p>Corpo Recebido (POST): {body}</p>")
print(f"</body></html>")