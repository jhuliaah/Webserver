#!/usr/bin/env python3

body = "<html><body><h1>CGI Funcionando Perfeitamente!</h1></body></html>\n"
print(f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: {len(body)}\r\n\r\n{body}", end="")

# import sys

# def main():
#     if len(sys.argv) != 3:
#         print("Erro: Você deve fornecer exatamente dois números como argumentos.", file=sys.stderr)
#         sys.exit(1)

#     try:
#         num1 = int(sys.argv[1])
#         num2 = int(sys.argv[2])
        
#         soma = num1 + num2
#         print(soma)
        
#     except ValueError:
#         print("Erro: Os argumentos fornecidos devem ser números inteiros.", file=sys.stderr)
#         sys.exit(1)

# if __name__ == "__main__":
#     main()
