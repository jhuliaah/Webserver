# Comandos para a defesa do Webserv (jhuliaah/Webserver)

Baseado no `config_files/default/config.conf`:

| Rota | Métodos | Observação |
|---|---|---|
| `/` | GET POST DELETE | autoindex off, serve `index.html` |
| `/uploads` | GET POST DELETE | autoindex on, mas tem `index.html` (index tem prioridade) |
| `/readonly` | GET | autoindex on |
| `/error_pages` | GET | autoindex on, sem index -> lista de verdade |
| `/go-home` | — | `return 302 /` |
| `/cgi-bin` | GET POST | `.py`, `cgi_path /usr/bin/python3` |

Portas: `8080` e `8081` (dois `server{}` no mesmo arquivo).
`client_max_body_size 2k` = 2048 bytes.

Rode tudo a partir da raiz do repo, com o servidor já de pé:

```bash
./webserv config_files/default/config.conf
```

---

## 1. Compilação (Regras gerais / Makefile)

```bash
make            # deve compilar limpo, sem warnings
make clean
make fclean
make re
```

Checar as flags exigidas:

```bash
grep -n "CFLAGS" Makefile
# esperado: -std=c++98 -Wall -Wextra -Werror
```

---

## 2. Check the code and ask questions

Sem comando de terminal — é leitura de código com o corrigidor. Pontos-chave pra apontar direto no editor:

```bash
grep -n "epoll_wait\|poll(\|select(" srcs/server/Server.cpp     # onde fica o único poll/epoll
grep -n "EPOLLIN\|EPOLLOUT" srcs/server/Server.cpp               # confirma leitura+escrita no mesmo evento
grep -n "errno" srcs/server/Client.cpp srcs/server/Server.cpp     # confirmar que errno NUNCA é checado após read/write
grep -rn "recv(\|send(\|read(\|write(" srcs/ | grep -v "\.hpp"    # toda I/O de socket, pra mostrar que passa por poll
```

---

## 3. Configuration

```bash
# múltiplos sites em portas diferentes
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8081/

# error page customizada (403/404/405/413/500) — troque o 404.html e reteste
curl -i http://127.0.0.1:8080/nao-existe

# limite de body (client_max_body_size 2k = 2048 bytes)
curl -i -X POST -H "Content-Type: plain/text" \
  --data "$(python3 -c 'print("a"*1000)')" http://127.0.0.1:8080/uploads/pequeno.txt   # < limite, 2xx

curl -i -X POST -H "Content-Type: plain/text" \
  --data "$(python3 -c 'print("a"*5000)')" http://127.0.0.1:8080/uploads/grande.txt    # > limite, 413

# rotas apontando pra diretórios diferentes
curl -i http://127.0.0.1:8080/readonly/
curl -i http://127.0.0.1:8080/uploads/

# arquivo default ao pedir um diretório
curl -i http://127.0.0.1:8080/uploads/           # serve uploads/index.html
curl -i http://127.0.0.1:8080/error_pages/        # sem index -> autoindex lista

# lista de métodos aceitos por rota
curl -i -X DELETE http://127.0.0.1:8080/readonly/index.html   # 405 (readonly só permite GET)
curl -i -X DELETE http://127.0.0.1:8080/uploads/pequeno.txt   # 204 (uploads permite DELETE)
```

---

## 4. Basic checks

```bash
# GET / POST / DELETE
curl -i http://127.0.0.1:8080/
curl -i -X POST --data "conteudo" http://127.0.0.1:8080/uploads/teste.txt
curl -i -X DELETE http://127.0.0.1:8080/uploads/teste.txt

# método desconhecido não pode derrubar o servidor
printf 'FOOBAR / HTTP/1.1\r\nHost: localhost\r\n\r\n' | nc -w2 127.0.0.1 8080

# upload + retrieve
curl -i -X POST --data "arquivo de teste" http://127.0.0.1:8080/uploads/upload_demo.txt
curl -i http://127.0.0.1:8080/uploads/upload_demo.txt

# telnet cru pra mostrar o corrigidor
telnet 127.0.0.1 8080
# depois digitar:
# GET / HTTP/1.1
# Host: localhost
# (linha em branco)
```

---

## 5. Check CGI

```bash
# GET
curl -i "http://127.0.0.1:8080/cgi-bin/test.py?nome=webserv"

# POST
curl -i -X POST -H "Content-Type: application/x-www-form-urlencoded" \
  --data "campo=valor" http://127.0.0.1:8080/cgi-bin/test.py

# CGI com erro proposital (divisão por zero em Python)
curl -i "http://127.0.0.1:8080/cgi-bin/test_errors.py?type=error"

# CGI em loop infinito -> timeout de 30s, espera 504 (não travar o servidor)
time curl -i --max-time 40 "http://127.0.0.1:8080/cgi-bin/test_errors.py?type=timeout"

# confirmar que o servidor segue vivo depois de qualquer um desses
curl -i http://127.0.0.1:8080/
```

---

## 6. Check with a browser

Sem comando — abrir `http://127.0.0.1:8080/` no navegador escolhido, F12 -> Network, e conferir:

```bash
# pra comparar manualmente os headers de resposta antes de abrir no navegador
curl -i http://127.0.0.1:8080/ | head -20

# URL incorreta
curl -i http://127.0.0.1:8080/pagina-que-nao-existe

# listar diretório
curl -i http://127.0.0.1:8080/error_pages/

# URL redirecionada
curl -i http://127.0.0.1:8080/go-home
```

---

## 7. Port issues

```bash
# 1) múltiplas interfaces/portas
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8081/

# 2) dois server{} no mesmo host:port -> deve dar erro (sem host virtual)
#    monte um config de teste com 2 blocos "listen 8080;" e rode:
./webserv config_files/test_same_port.conf
# esperado: "FATAL ERROR: ... bind() ... Address already in use", exit 1, sem crash

# 3) dois processos webserv diferentes, porta em comum
./webserv config_files/default/config.conf &     # processo 1 sobe normalmente
./webserv config_files/default/config.conf        # processo 2: deve falhar com EADDRINUSE

# confirmar que o processo 1 continua vivo depois do erro do processo 2
curl -i http://127.0.0.1:8080/
kill %1    # encerra o processo 1 em background
```

---

## 8. Siege & stress test

```bash
# instalar (se precisar)
brew install siege        # macOS
sudo apt install siege    # Linux

# disponibilidade > 99.5% numa página simples, com -b (sem espera entre requests)
siege -b -c 25 -t 30S http://127.0.0.1:8080/

# controlar taxa/segurança: -c clientes, -d delay máx antes de reconectar, -r repetições
siege -c 10 -d 3 -r 20 http://127.0.0.1:8080/

# monitorar memória do processo em paralelo, em outro terminal
ps aux | grep webserv
# ou, pra acompanhar ao longo do tempo:
watch -n 2 'ps -o pid,rss,vsz,cmd -p $(pgrep -f "./webserv")'

# verificar conexões penduradas
netstat -anp | grep 8080 | grep -c ESTABLISHED
# ou
ss -tan | grep :8080

# rodar indefinidamente sem precisar reiniciar o servidor
siege -b -c 15 http://127.0.0.1:8080/
# (Ctrl+C pra parar, servidor deve continuar de pé depois)
```


## 10. Script de teste automatizado

Além dos comandos manuais acima, `test.py` (na raiz do repo) cobre boa parte das
seções 3, 4, 5, 7 e parte da 6 automaticamente:

```bash
python3 test.py                                   # roda tudo
python3 test.py --only basic,config,cgi,ports,robustness
```

Ele **não substitui** o siege (seção 8) nem a leitura de código com o
corrigidor (seção 2) — essas duas continuam manuais.
