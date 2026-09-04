#!/usr/bin/python3
import os
import sys
import time

# Lê a query string enviada pelo GET (ex: ?type=error ou ?type=timeout)
query = os.environ.get("QUERY_STRING", "")

if "type=timeout" in query:
    # 1. SIMULAÇÃO DE LOOP INFINITO / SCRIPT TRAVADO (Sera atingido pelo Timeout 504)
    # O script entra em loop e nao responde nada. O Ceifador deve enviar SIGKILL.
    while True:
        time.sleep(1)

elif "type=error" in query:
    # 2. SIMULAÇÃO DE ERRO NO PYTHON (Gera status 500 Internal Server Error)
    # Força uma divisão por zero que lança uma exceção e encerra sem imprimir HTML valido
    resultado = 10 / 0

else:
    # 3. COMPORTAMENTO NORMAL (Sucesso 200 OK)
    print("Content-Type: text/html\r\n\r\n")
    print("<html><body><h1>CGI Executado com Sucesso!</h1></body></html>")