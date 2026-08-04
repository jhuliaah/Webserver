# Webserver — TODO (estado real em 2026-08-04)

> Gerado a partir do código atual (`make`, `git log`, `git status`, leitura dos
> `.cpp`), não apenas dos docs antigos. `PROJECT_STATUS.md`/`ROADMAP.md` estão
> desatualizados (27/07) — várias coisas mudaram desde então (ConfigParser,
> Tokenizer, ConfigTypes entraram; `Config.cpp` sumiu).

## ✅ RESOLVIDO — build voltou a compilar, Config religado ao ConfigParser

- [x] **`Config.cpp` reescrito** (`srcs/config/Config.cpp`, novo arquivo,
  adicionado ao Makefile). `Config()`/`Config(path)` agora chamam
  `ConfigParser::parse()` de verdade e convertem o resultado para as classes
  `ServerConfig`/`LocationConfig` que o resto do servidor usa.
  `makeConfig(int argc, char* argv[])` decide entre `argv[1]` e o default
  `config_files/default/config.conf`. O overload de mock
  (`makeConfig(std::string)`) foi removido — já estava marcado "APAGAR
  DEPOIS" no próprio header — e `main.cpp` agora chama o `makeConfig(argc,
  argv)` real em vez de `"MOCK_BASIC"`.
- [x] **Colisão de nomes corrigida**: `ConfigTypes.hpp` tinha `struct
  ServerConfig`/`struct location` com o mesmo nome das classes
  `ServerConfig`/`LocationConfig` já usadas pelo `Router`/`Server` — não dava
  para incluir os dois mundos no mesmo arquivo. Renomeados para
  `ParsedServer`/`ParsedLocation` (só no lado do parser).
- [x] **Bugs do `ConfigParser` corrigidos** (necessários para ele conseguir
  ler o `config_files/default/config.conf` real, não só compilar):
  - faltava tratar `root`/`index` no nível do `server{}` (o parser jogava
    "Unknown token" nesses casos)
  - nomes de diretiva de `location{}` não batiam com o arquivo real:
    trocado `methods`→`allow_methods`, `cgi_extension`→`cgi_path`/`cgi_ext`
  - `location <path> {` — o path era lido tarde demais (`loc.path =
    tokens[i]` pegava `"}"`, não o path)
  - `index <valor>;` usava `==` em vez de `=` (não atribuía nada)
  - `return 302 /;` só consumia um token e quebrava no `;expected`
  - `error_page ...;` não incrementava `i` depois do `;` → loop infinito
  - Testado: parseia os dois `server{}` do config default (6 + 1 locations,
    error_pages, cgi_path/cgi_ext) e rejeita configs malformados/inexistentes
    com uma `ServerException` clara em vez de travar.
- [ ] **Pendência menor descoberta nesse processo**: `client_max_body_size
  2k;` vira `2` (o `k`/`m` de sufixo não é convertido, `atol("2k")==2`) —
  já estava listado como parte da Fase 2 (`ConfigParser`), não bloqueia mais
  nada, só fica registrado aqui para não esquecer.
- [ ] `Router::classify` vai precisar do `_cgi_path`/`_cgi_extension` que já
  chegam populados agora — bom próximo passo lógico depois deste fix.

## ✅ O que já roda (antes da regressão do build)

- Socket + epoll loop, accept/read/write, timeout de 60s — igual ao status antigo.
- CGI: fork/execve/pipes não-bloqueantes funcionam fim a fim para
  `cgi-bin/script1.py` fixo, com timeout de 5s (SIGKILL + 504) e reap de
  zumbis em `removeClient`/timeout.
- `HttpRequest` agora é uma classe real com getters/setters/`getHeader()`
  case-sensitive (ainda não populada por um parser de request real).
- `ConfigParser::parse()` + `Tokenizer` — implementação nova (211 linhas),
  compila sozinha; ainda não foi testada contra `config_files/` nem plugada
  em `Config`.
- `Router.hpp`/`ConfigTypes.hpp` reorganizados, mas `Router::matchLoc`/
  `classify` continuam só declarados (stub vazio em `Router.cpp`).

## ⚠️ Mudanças não commitadas (working tree)

`git status` mostra estas modificações ainda soltas, não commitadas:
`includes/{CgiHandler,Client,HttpRequest,IRequestHandler,Server}.hpp`,
`srcs/HttpRequest.cpp`, `srcs/handlers/CgiHandler.cpp`, `srcs/server/Server.cpp`.
- [ ] Revisar e commitar (ou descartar) essas mudanças — é o trabalho de CGI
  parcial (POST stdin, env builder) descrito acima.

## Fase 0 — Fundação (continua pendente)

- [ ] Corrigir `Client::readData()` para respeitar `Content-Length` (hoje só
  olha o primeiro `\r\n\r\n`, quebra qualquer POST com body)
- [ ] Corrigir `Client::writeData()` para partial write / `EWOULDBLOCK`
  (hoje faz um `send()` e sempre fecha)
- [ ] Trocar `CREATE_FAKE_PORTS()`/ports mockados (`8080/8085/9005` em
  `127.0.0.1`) em `Server::initServer()` por bind real a partir de `_config`
  (depende do bloqueador acima estar resolvido)
- [ ] Corrigir `Server::~Server()` para fechar/deletar `_clients` (hoje só
  fecha sockets — leak de clientes)

## Fase 1 — Parsing HTTP real

- [ ] Implementar parsing real de request (method/URI/headers/body) e
  alimentar o `HttpRequest` já pronto — hoje `Client::readData()` nem
  instancia um `HttpRequest`
- [ ] 400 para request line/headers malformados

## Fase 2 — Config (parser religado, falta validação semântica)

- [x] Tokenizer + `ConfigParser::parse()` escritos e compilando
- [x] `ConfigParser::parse()` testado contra `config_files/default/config.conf`
      — parseia os 2 `server{}`, locations, error_pages, cgi_path/cgi_ext
- [x] `Config()`/`Config(path)` religados ao `ConfigParser` (ver seção
      "RESOLVIDO" no topo); `main.cpp` já chama `makeConfig(argc, argv)` real
- [ ] Testar contra os 7 fixtures de `config_files/invalid/*.conf` — hoje o
      parser só rejeita erro de sintaxe (token inesperado/faltando), não faz
      validação semântica (porta > 65535, IP inválido, `server_name`
      duplicado, `root`+`alias` juntos etc. — isso é a Fase 2 tarefa 6 do
      roadmap antigo, ainda não implementada)
- [ ] Sufixo de tamanho em `client_max_body_size` (`2k`/`2m`) não é
      convertido — vira só `2` bytes

## Fase 3 — Roteamento & arquivos estáticos

- [ ] `Router::matchLoc`/`Router::classify` — ainda só declarados, `.cpp` é
      ctor/dtor vazio
- [ ] `StaticHandler::handle()` — ainda não existe override, classe
      permanece abstrata (só ctor/dtor em `StaticHandler.cpp`)
- [ ] `HttpResponse` — sem serializer; `Client::_buildStaticResponse()`
      continua servindo sempre `./www/index.html` hardcoded, ignorando
      método/URI
- [ ] Páginas de erro customizadas por config

## Fase 4 — Limites & uploads

- [ ] `client_max_body_size` (413)
- [ ] Upload/DELETE de arquivo

## Fase 5 — CGI (parcialmente pronto)

- [x] Spawn do processo, env builder básico, leitura assíncrona via epoll
- [x] Timeout de CGI (5s → SIGKILL + 504) e reap de zumbis
- [ ] Escrita do body do POST no stdin do CGI (`handleCgiWrite` existe em
      `Server.cpp` mas não commitado/verificar se está fiado ao fluxo real)
- [ ] `CgiHandler::parseCgiOutput()` — ainda `TODO`, saída crua do script é
      jogada direto como resposta em vez de virar `HttpResponse`
- [ ] Script CGI hardcoded (`cgi-bin/script1.py`) em vez de vir do
      `Router`/config
- [ ] Disparo do CGI ainda não passa pelo `Router` — `Server::serverLoop`
      tem um comentário dizendo que falta essa decisão

## Fase 6 — Polimento

- [ ] Keep-alive vs close-per-request (decisão ainda não tomada)
- [ ] Teste de carga / malformado (siege, slowloris, pipelining)
- [ ] Limpar código morto: `includes/WebServer.hpp` (órfão),
      `srcs/CgiRequest.cpp` (não compila, fora do Makefile),
      `RouteConfig.hpp/.cpp` (não referenciado em lugar nenhum agora)

---

*Próximo passo imediato: resolver o bloqueador do build (`Config.cpp`)
antes de continuar qualquer outra fase — nada roda até lá.*
