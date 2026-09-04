#!/usr/bin/env python3
"""
test.py — Bateria de testes do Webserv (jhuliaah/Webserver) baseada na régua de
avaliação (evaluation scale) da 42.

Calibrado com o repositório e o config_files/default/config.conf reais:
    server { listen 8080; ... }   -> site principal
    server { listen 8081; ... }   -> segundo site (mesmo root)

    location /            GET POST DELETE   autoindex off
    location /uploads     GET POST DELETE   autoindex on   (tem index.html!)
    location /readonly    GET               autoindex on
    location /error_pages GET               autoindex on   (sem index -> lista)
    location /go-home     return 302 /
    location /cgi-bin     GET POST          cgi_ext .py, cgi_path /usr/bin/python3

    error_page 403/404/405/413/500 -> www/error_pages/*.html
    client_max_body_size 2k (2048 bytes)

Só usa a biblioteca padrão (socket, http.client) -> roda em qualquer máquina
com python3, sem precisar instalar nada.

COMO USAR
---------
1. Suba o servidor com o config default:
       ./webserv config_files/default/config.conf
2. Rode todos os testes:
       python3 test.py
   ou só uma categoria:
       python3 test.py --only basic,config,cgi,ports,robustness
3. Se você mudar o config (outras rotas/portas), ajuste a seção CONFIG abaixo.

O QUE ESTE SCRIPT NÃO SUBSTITUI
--------------------------------
- siege (disponibilidade >99.5%, memory leak, hanging connections)
- a parte de leitura de código com o corrigidor (poll único, errno não usado
  após read/write, remoção de client em erro de socket, fork só p/ CGI etc.)
"""

import socket
import http.client
import sys
import time
import os
import argparse

# ============================== CONFIG ====================================
HOST = "127.0.0.1"
PORT = 8080
EXTRA_PORTS = [8081]                 # segundo "server {}" do config.conf

STATIC_ROUTE = "/"                   # autoindex off, serve index.html (148KB)
NOT_FOUND_ROUTE = "/nao-existe-123"

AUTOINDEX_ROUTE = "/error_pages/"    # autoindex on, SEM index.html -> lista de fato
INDEX_PRIORITY_ROUTE = "/uploads/"   # autoindex on, MAS tem index.html -> deve servir ele

REDIRECT_ROUTE = "/go-home"          # return 302 /

UPLOAD_DIR_ROUTE = "/uploads"        # local dos uploads
DELETE_ALLOWED_DIR = "/uploads"

READONLY_ROUTE = "/readonly"         # allow_methods GET (sem POST/DELETE)

CGI_SCRIPT = "/cgi-bin/test.py"                     # eco de method/query/body
CGI_ERROR_SCRIPT = "/cgi-bin/test_errors.py"        # ?type=error / ?type=timeout
CGI_TIMEOUT_LIMIT = 30               # visto no código: Server.cpp checkTimeouts() > 30s
CGI_TIMEOUT_WAIT = 40                # margem de segurança pro teste esperar

MAX_BODY_SIZE = 2048                 # client_max_body_size 2k

SOCK_TIMEOUT = 6
# ============================================================================

PASS = "\033[92mPASS\033[0m"
FAIL = "\033[91mFAIL\033[0m"
WARN = "\033[93mWARN\033[0m"
SKIP = "\033[90mSKIP\033[0m"

results = {"pass": 0, "fail": 0, "warn": 0, "skip": 0}


def report(status, name, detail=""):
    results[status] += 1
    tag = {"pass": PASS, "fail": FAIL, "warn": WARN, "skip": SKIP}[status]
    line = f"[{tag}] {name}"
    if detail:
        line += f"  -> {detail}"
    print(line)


# ------------------------------------------------------------------
# Helpers de baixo nível: ler uma resposta HTTP crua até o fim de
# verdade (por Content-Length, chunked ou fechamento de conexão),
# em vez de um único recv() que trunca corpos grandes.
# ------------------------------------------------------------------

def recv_until(sock, marker, timeout):
    sock.settimeout(timeout)
    data = b""
    while marker not in data:
        chunk = sock.recv(4096)
        if not chunk:
            break
        data += chunk
    return data


def recv_full_response(sock, timeout=SOCK_TIMEOUT):
    """Lê uma resposta HTTP completa (headers + body) respeitando
    Content-Length ou Transfer-Encoding: chunked. Retorna (headers_str, body_bytes)
    ou (None, None) se a conexão fechar sem nada."""
    sock.settimeout(timeout)
    data = recv_until(sock, b"\r\n\r\n", timeout)
    if b"\r\n\r\n" not in data:
        return None, None
    header_end = data.index(b"\r\n\r\n") + 4
    headers_raw = data[:header_end].decode(errors="replace")
    body = data[header_end:]

    content_length = None
    is_chunked = False
    for line in headers_raw.split("\r\n")[1:]:
        if ":" not in line:
            continue
        k, v = line.split(":", 1)
        k = k.strip().lower()
        v = v.strip()
        if k == "content-length":
            content_length = int(v)
        elif k == "transfer-encoding" and "chunked" in v.lower():
            is_chunked = True

    if is_chunked:
        while b"0\r\n\r\n" not in body:
            chunk = sock.recv(65536)
            if not chunk:
                break
            body += chunk
    elif content_length is not None:
        while len(body) < content_length:
            chunk = sock.recv(65536)
            if not chunk:
                break
            body += chunk
    else:
        try:
            while True:
                chunk = sock.recv(65536)
                if not chunk:
                    break
                body += chunk
        except socket.timeout:
            pass

    return headers_raw, body


def raw_request(host, port, raw_bytes, timeout=SOCK_TIMEOUT):
    """Manda bytes crus e devolve (headers, body) via recv_full_response.
    Nunca lança em conexões fechadas/timeout — devolve (None, None), o que já
    é informação útil (o servidor não deveria travar nem cair)."""
    try:
        with socket.create_connection((host, port), timeout=timeout) as s:
            s.sendall(raw_bytes)
            return recv_full_response(s, timeout)
    except (ConnectionRefusedError, ConnectionResetError, OSError, socket.timeout):
        return None, None


def http_request(method, path, host=HOST, port=PORT, body=None, headers=None, timeout=SOCK_TIMEOUT):
    headers = headers or {}
    conn = http.client.HTTPConnection(host, port, timeout=timeout)
    try:
        conn.request(method, path, body=body, headers=headers)
        resp = conn.getresponse()
        data = resp.read()
        return resp.status, dict(resp.getheaders()), data
    finally:
        conn.close()


def server_is_alive(host=HOST, port=PORT):
    try:
        http_request("GET", "/", host, port, timeout=3)
        return True
    except Exception:
        return False


# ============================== BASIC CHECKS ================================

def test_basic():
    print("\n== BASIC CHECKS (GET/POST/DELETE, unknown methods, status codes, upload) ==")

    try:
        status, hdrs, body = http_request("GET", STATIC_ROUTE)
        report("pass" if status == 200 else "fail",
               "GET na rota estática (/) retorna 200", f"status={status}, bytes={len(body)}")
    except Exception as e:
        report("fail", "GET na rota estática", str(e))

    try:
        status, hdrs, body = http_request("GET", NOT_FOUND_ROUTE)
        report("pass" if status == 404 else "fail",
               "GET em rota inexistente retorna 404", f"status={status}")
        report("pass" if body else "warn", "Página de erro 404 tem corpo", f"len={len(body)}")
    except Exception as e:
        report("fail", "GET em rota inexistente", str(e))

    # Upload com nome de arquivo na URI (o handler usa o último segmento do
    # path como nome do arquivo — postar direto na pasta sem nome de arquivo
    # colide com o próprio diretório, então sempre incluímos um filename).
    fname = f"/uploads/test_{os.urandom(4).hex()}.txt"
    payload = b"conteudo de teste do webserv - " + os.urandom(16).hex().encode()
    try:
        status, hdrs, body = http_request(
            "POST", fname, body=payload,
            headers={"Content-Type": "text/plain", "Content-Length": str(len(payload))}
        )
        if status in (200, 201, 204):
            report("pass", "POST de upload (com filename na URI) aceito", f"status={status}")
        else:
            report("fail", "POST de upload", f"status={status} (esperado 200/201/204)")
    except Exception as e:
        report("fail", "POST de upload", str(e))

    # Retrieve do arquivo recém enviado
    try:
        status, hdrs, body = http_request("GET", fname)
        if status == 200 and body == payload:
            report("pass", "GET recupera exatamente o arquivo enviado")
        elif status == 200:
            report("warn", "Arquivo recuperado mas conteúdo difere", f"status={status}")
        else:
            report("fail", "GET do arquivo enviado", f"status={status}")
    except Exception as e:
        report("fail", "GET do arquivo enviado", str(e))

    # DELETE do arquivo (rota permite DELETE)
    try:
        status, hdrs, body = http_request("DELETE", fname)
        if status in (200, 204):
            report("pass", "DELETE remove o arquivo enviado", f"status={status}")
        else:
            report("fail", "DELETE do arquivo enviado", f"status={status} (esperado 200/204)")
    except Exception as e:
        report("fail", "DELETE do arquivo enviado", str(e))

    # Confirma que sumiu
    try:
        status, _, _ = http_request("GET", fname)
        report("pass" if status == 404 else "fail",
               "Arquivo deletado não existe mais (404)", f"status={status}")
    except Exception as e:
        report("warn", "Confirmação de DELETE", str(e))

    # DELETE numa rota que não permite (readonly -> só GET)
    try:
        status, _, _ = http_request("DELETE", READONLY_ROUTE + "/index.html")
        report("pass" if status == 405 else "fail",
               "DELETE em rota sem permissão (/readonly) retorna 405", f"status={status}")
    except Exception as e:
        report("fail", "DELETE em rota restrita", str(e))

    # DELETE de um diretório -> deve ser recusado (403), nunca apagar o diretório
    try:
        status, _, _ = http_request("DELETE", "/")
        report("pass" if status == 403 else "warn",
               "DELETE na raiz (é um diretório) é recusado", f"status={status} (esperado 403)")
    except Exception as e:
        report("warn", "DELETE de diretório", str(e))

    # Método completamente desconhecido -> não deve derrubar o servidor
    hdr, body = raw_request(HOST, PORT, b"FOOBAR / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    if hdr is not None:
        report("pass", "Método HTTP desconhecido (FOOBAR) não derruba o servidor",
               hdr.splitlines()[0])
    else:
        report("warn", "Sem resposta a método desconhecido")
    report("pass" if server_is_alive() else "fail",
           "Servidor continua vivo após método desconhecido")


# ============================== CONFIGURATION ================================

def test_config():
    print("\n== CONFIGURATION (rotas, error pages, body limit, autoindex, index file) ==")

    try:
        small_body = b"x" * (MAX_BODY_SIZE // 2)
        status, _, _ = http_request(
            "POST", "/uploads/body_ok.txt", body=small_body,
            headers={"Content-Type": "text/plain", "Content-Length": str(len(small_body))}
        )
        report("pass" if status != 413 else "fail",
               "Body menor que client_max_body_size é aceito", f"status={status}")
        http_request("DELETE", "/uploads/body_ok.txt")
    except Exception as e:
        report("fail", "Teste de body dentro do limite", str(e))

    try:
        big_body = b"x" * (MAX_BODY_SIZE * 4)
        status, _, _ = http_request(
            "POST", "/uploads/body_too_big.txt", body=big_body,
            headers={"Content-Type": "text/plain", "Content-Length": str(len(big_body))}
        )
        report("pass" if status == 413 else "fail",
               "Body maior que client_max_body_size retorna 413", f"status={status}")
    except Exception as e:
        report("warn", "Teste de body acima do limite", str(e))

    report("pass" if server_is_alive() else "fail",
           "Servidor segue vivo após envio de body grande")

    try:
        conn = http.client.HTTPConnection(HOST, PORT, timeout=SOCK_TIMEOUT)
        conn.request("GET", REDIRECT_ROUTE)
        resp = conn.getresponse()
        resp.read()
        if resp.status in (301, 302, 303, 307, 308):
            report("pass", f"Rota de redirect (/go-home) retorna {resp.status}",
                   f"Location: {resp.getheader('Location')}")
        else:
            report("fail", "Rota de redirect", f"status={resp.status} (esperado 3xx)")
        conn.close()
    except Exception as e:
        report("fail", "Rota de redirect", str(e))

    try:
        status, hdrs, body = http_request("GET", AUTOINDEX_ROUTE)
        if status == 200 and b"Index of" in body:
            report("pass", "Autoindex lista diretório sem index.html")
        else:
            report("fail", "Autoindex", f"status={status}, corpo parece listagem? "
                                         f"{b'Index of' in body}")
    except Exception as e:
        report("fail", "Autoindex", str(e))

    try:
        status, hdrs, body = http_request("GET", INDEX_PRIORITY_ROUTE)
        if status == 200 and b"Index of" not in body:
            report("pass", "Diretório com index.html serve o index, não a listagem")
        else:
            report("warn", "Prioridade index vs autoindex", f"status={status}")
    except Exception as e:
        report("warn", "Prioridade index vs autoindex", str(e))

    try:
        status, _, body = http_request("GET", NOT_FOUND_ROUTE)
        report("pass" if status == 404 and body else "warn",
               "Página de erro 404 (custom, error_pages/404.html)",
               f"status={status}, len={len(body)}")
    except Exception as e:
        report("warn", "Página de erro custom", str(e))


# ============================== CGI CHECKS ===================================

def test_cgi():
    print("\n== CGI CHECKS ==")

    try:
        status, hdrs, body = http_request("GET", CGI_SCRIPT + "?nome=webserv")
        ok = status == 200 and b"GET" in body
        report("pass" if ok else "fail", "CGI via GET responde 200 e ecoa o método",
               f"status={status}, len={len(body)}")
    except Exception as e:
        report("fail", "CGI via GET", str(e))

    try:
        payload = b"campo=valor&teste=webserv"
        status, hdrs, body = http_request(
            "POST", CGI_SCRIPT, body=payload,
            headers={"Content-Type": "application/x-www-form-urlencoded",
                     "Content-Length": str(len(payload))}
        )
        ok = status == 200 and b"POST" in body and b"valor" in body
        report("pass" if ok else "fail", "CGI via POST responde 200 e ecoa o body",
               f"status={status}, len={len(body)}")
    except Exception as e:
        report("fail", "CGI via POST", str(e))

    try:
        status, hdrs, body = http_request("GET", CGI_ERROR_SCRIPT + "?type=error", timeout=10)
        report("pass" if status in (500, 502) else "warn",
               "CGI que gera erro retorna 500/502 (sem crash)", f"status={status}")
    except Exception as e:
        report("warn", "CGI de erro", str(e))

    report("pass" if server_is_alive() else "fail",
           "Servidor segue vivo após CGI de erro")

    print(f"   (aguardando o timeout real do CGI, ~{CGI_TIMEOUT_LIMIT}s, "
          f"até {CGI_TIMEOUT_WAIT}s...)")
    start = time.time()
    try:
        status, hdrs, body = http_request("GET", CGI_ERROR_SCRIPT + "?type=timeout",
                                           timeout=CGI_TIMEOUT_WAIT)
        elapsed = time.time() - start
        if status == 504:
            report("pass", f"CGI em loop infinito retorna 504 após timeout ({elapsed:.1f}s)")
        else:
            report("warn", "CGI em loop infinito", f"status={status}, elapsed={elapsed:.1f}s")
    except Exception as e:
        elapsed = time.time() - start
        report("fail", f"CGI em loop infinito não respondeu em {elapsed:.1f}s", str(e))

    report("pass" if server_is_alive() else "fail",
           "Servidor continua respondendo depois do CGI em loop infinito")


# ============================== PORT ISSUES ==================================

def test_ports():
    print("\n== PORT ISSUES (múltiplas interfaces/portas) ==")

    for p in EXTRA_PORTS:
        try:
            status, hdrs, body = http_request("GET", "/", port=p, timeout=3)
            report("pass" if status == 200 else "warn",
                   f"Servidor responde na porta {p} (segundo server{{}} do config)",
                   f"status={status}")
        except Exception as e:
            report("fail", f"Porta {p} não respondeu", str(e))

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 0)
        try:
            s.bind((HOST, PORT))
            report("warn", f"Consegui bindar na porta {PORT} — o servidor está escutando nela?")
        except OSError:
            report("pass", f"Porta {PORT} já está em uso pelo servidor")
        finally:
            s.close()
    except Exception as e:
        report("skip", "Teste de bind na porta do servidor", str(e))


# ============================== ROBUSTNESS / NO-CRASH ========================

def test_robustness():
    print("\n== ROBUSTEZ / NO-CRASH (requests malformadas, chunked, keep-alive) ==")

    malformed = [
        ("Request line vazia", b"\r\n\r\n"),
        ("Só espaços", b"   \r\n\r\n"),
        ("Sem versão HTTP", b"GET /\r\n\r\n"),
        ("Versão HTTP inválida", b"GET / HTTP/9.9\r\n\r\n"),
        ("Header sem valor", b"GET / HTTP/1.1\r\nHost:\r\n\r\n"),
        ("Content-Length negativo", b"POST /uploads/x.txt HTTP/1.1\r\nHost: localhost\r\n"
                                     b"Content-Length: -1\r\n\r\n"),
        ("Content-Length não numérico", b"POST /uploads/x.txt HTTP/1.1\r\nHost: localhost\r\n"
                                         b"Content-Length: abc\r\n\r\nxx"),
        ("Path enorme", b"GET /" + b"a" * 8000 + b" HTTP/1.1\r\nHost: localhost\r\n\r\n"),
        ("Path traversal (..)", b"GET /../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n"),
        ("Múltiplos Host headers", b"GET / HTTP/1.1\r\nHost: a\r\nHost: b\r\n\r\n"),
    ]

    for name, raw in malformed:
        hdr, body = raw_request(HOST, PORT, raw, timeout=4)
        if hdr is None:
            report("warn", f"Sem resposta / conexão fechada: {name}",
                   "aceitável se foi um close() limpo, confira que não travou o servidor")
        else:
            report("pass", f"Servidor respondeu sem cair: {name}", hdr.splitlines()[0])

    report("pass" if server_is_alive() else "fail",
           "Servidor segue vivo após bateria de requests malformadas — CRÍTICO se falhar")

    body_content = b"hello world chunked"
    part1, part2 = b"hello", b" world chunked"
    chunked_raw = (
        b"POST /uploads/chunk_test.txt HTTP/1.1\r\nHost: localhost\r\n"
        b"Transfer-Encoding: chunked\r\n\r\n"
        + format(len(part1), "x").encode() + b"\r\n" + part1 + b"\r\n"
        + format(len(part2), "x").encode() + b"\r\n" + part2 + b"\r\n"
        + b"0\r\n\r\n"
    )
    hdr, body = raw_request(HOST, PORT, chunked_raw, timeout=6)
    if hdr and hdr.startswith("HTTP/1.1 2"):
        status, _, saved = http_request("GET", "/uploads/chunk_test.txt")
        if saved == body_content:
            report("pass", "Transfer-Encoding: chunked é decodificado corretamente")
        else:
            report("fail", "Chunked salvo não bate com o conteúdo original",
                   f"esperado={body_content!r} obtido={saved!r}")
        http_request("DELETE", "/uploads/chunk_test.txt")
    elif hdr:
        report("fail", "Chunked encoding rejeitado", hdr.splitlines()[0])
    else:
        report("fail", "Sem resposta para chunked encoding")

    try:
        with socket.create_connection((HOST, PORT), timeout=SOCK_TIMEOUT) as s:
            s.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n")
            h1, b1 = recv_full_response(s, timeout=8)
            s.sendall(b"GET /readonly/ HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
            h2, b2 = recv_full_response(s, timeout=8)
            ok = (h1 and h1.startswith("HTTP/1.1 200") and
                  h2 and h2.startswith("HTTP/1.1 200"))
            if ok:
                report("pass", "Keep-Alive: 2 requests completas na mesma conexão TCP",
                       f"body1={len(b1)} bytes, body2={len(b2)} bytes")
            else:
                report("fail", "Keep-Alive", f"h1={h1.splitlines()[0] if h1 else None}, "
                                              f"h2={h2.splitlines()[0] if h2 else None}")
    except Exception as e:
        report("fail", "Teste de Keep-Alive", str(e))

    try:
        with socket.create_connection((HOST, PORT), timeout=SOCK_TIMEOUT) as s:
            s.sendall(b"GET / HTTP/1.1\r\n")
            time.sleep(1)
            s.sendall(b"Host: localhost\r\n")
            time.sleep(1)
            s.sendall(b"\r\n")
            hdr, body = recv_full_response(s, timeout=8)
            report("pass" if hdr and hdr.startswith("HTTP/1.1 200") else "warn",
                   "Request enviada em pedaços (cliente lento) é montada corretamente")
    except Exception as e:
        report("warn", "Cliente lento", str(e))

    report("pass" if server_is_alive() else "fail",
           "Servidor segue disponível após cliente lento")

    try:
        socks = []
        for _ in range(50):
            s = socket.create_connection((HOST, PORT), timeout=SOCK_TIMEOUT)
            s.sendall(b"GET /readonly/ HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
            socks.append(s)
        ok = 0
        for s in socks:
            try:
                hdr, _ = recv_full_response(s, timeout=6)
                if hdr and hdr.startswith("HTTP/1.1 200"):
                    ok += 1
            except Exception:
                pass
            finally:
                s.close()
        report("pass" if ok >= 45 else "fail",
               f"50 conexões simultâneas: {ok}/50 responderam corretamente")
    except Exception as e:
        report("fail", "Teste de 50 conexões simultâneas", str(e))

    report("pass" if server_is_alive() else "fail",
           "Servidor segue vivo após mini stress de 50 conexões — CRÍTICO se falhar")


# ============================== MAIN ==========================================

CATEGORIES = {
    "basic": test_basic,
    "config": test_config,
    "cgi": test_cgi,
    "ports": test_ports,
    "robustness": test_robustness,
}


def main():
    parser = argparse.ArgumentParser(description="Testes do webserv (jhuliaah/Webserver) baseados na régua")
    parser.add_argument("--only", type=str, default=None,
                         help="Lista separada por vírgula: basic,config,cgi,ports,robustness")
    args = parser.parse_args()

    print(f"Testando webserv em {HOST}:{PORT} ...")
    if not server_is_alive():
        print(f"[{FAIL}] Não consegui conectar em {HOST}:{PORT}. "
              f"Suba o servidor com: ./webserv config_files/default/config.conf")
        sys.exit(1)

    to_run = list(CATEGORIES.keys())
    if args.only:
        to_run = [c.strip() for c in args.only.split(",") if c.strip() in CATEGORIES]

    for cat in to_run:
        CATEGORIES[cat]()

    total = sum(results.values())
    print("\n" + "=" * 60)
    print(f"RESUMO: {results['pass']} passou | {results['fail']} falhou | "
          f"{results['warn']} avisos | {results['skip']} pulados  (total: {total})")
    print("=" * 60)
    print("Lembrete: isso NÃO substitui o siege (disponibilidade >99.5%, memory leak,\n"
          "hanging connections) nem a checagem manual de código com o corrigidor\n"
          "(poll único, errno não usado após read/write, fork só p/ CGI etc).")

    sys.exit(1 if results["fail"] > 0 else 0)


if __name__ == "__main__":
    main()
