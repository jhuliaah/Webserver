# WNOHANG e CGI no Webserv (42)

## O que é o WNOHANG?

`WNOHANG` é uma flag (opção) que você passa para a chamada de sistema `waitpid()`. Ela faz com que o `waitpid` **não bloqueie** o processo pai esperando o filho terminar — se o filho ainda não terminou, `waitpid` retorna imediatamente com valor `0` (em vez de ficar parado até o filho morrer).

Sem `WNOHANG`:
```c
waitpid(pid, &status, 0); // bloqueia até o filho terminar
```

Com `WNOHANG`:
```c
int ret = waitpid(pid, &status, WNOHANG);
// ret == 0  -> filho ainda rodando
// ret == pid -> filho terminou, status contém o código de saída
// ret == -1 -> erro
```

## Por que isso importa no webserv

No webserv, o servidor é tipicamente **single-threaded** e baseado em um loop de eventos com `poll`/`select`/`epoll` (ou `kqueue`). Tudo precisa ser não-bloqueante, porque enquanto você está lidando com um CGI, o servidor ainda precisa continuar atendendo outras conexões (outros clientes, outras requisições).

Quando você executa um script CGI, o fluxo é mais ou menos:

1. `fork()` cria um processo filho.
2. O filho faz `execve()` para rodar o script CGI (PHP, Python, etc.).
3. O pai (o servidor) precisa, em algum momento, saber quando esse filho terminou para:
   - ler a saída do CGI (via pipe)
   - pegar o status de saída
   - evitar processos **zumbis** (processos terminados que ainda ocupam uma entrada na tabela de processos até serem "colhidos" com `wait`/`waitpid`)

Se você chamasse `waitpid(pid, &status, 0)` (sem `WNOHANG`), e o CGI demorasse para terminar (ou travasse), **todo o servidor congelaria** esperando — nenhuma outra requisição seria processada.

Por isso usa-se `WNOHANG`: a cada iteração do loop principal (ou a cada "tick"), o servidor faz algo como:

```c
int ret = waitpid(cgi_pid, &status, WNOHANG);
if (ret == 0) {
    // ainda rodando, continua o loop, talvez verifique timeout
} else if (ret == cgi_pid) {
    // terminou! processa a saída, monta a resposta HTTP
} else {
    // erro
}
```

### Resumindo o uso típico

- Evitar bloquear o event loop esperando o CGI terminar.
- Permitir implementar **timeout** para CGIs que demoram demais ou travam (você verifica `WNOHANG` repetidamente e, se passar muito tempo sem o filho terminar, você manda `kill()` no PID).
- Evitar processos zumbis, "colhendo" (reaping) os filhos terminados sem travar o servidor.

É basicamente a peça que faz o gerenciamento de CGI ser compatível com a arquitetura assíncrona/não-bloqueante exigida pelo projeto.

---

## Preciso ficar checando o valor de `ret` periodicamente até ele ser preenchido?

Sim, exatamente essa é a ideia. Você não fica "esperando" de forma bloqueante — em vez disso, a cada volta do seu loop principal (o `poll`/`select`/`epoll`), você dá uma "espiada" rápida:

```c
int ret = waitpid(cgi_pid, &status, WNOHANG);
if (ret == 0) {
    // ainda não terminou, segue o loop normalmente
} else if (ret == cgi_pid) {
    // terminou agora! hora de processar
}
```

Como o loop já está rodando continuamente (cuidando de outros clientes, sockets, etc.), essa checagem é praticamente "de graça" — você só insere essa chamada em algum lugar do loop para os CGIs que estão pendentes.

### Algumas dicas práticas

**1. Combine com timeout**

Já que você está fazendo polling, é fácil também guardar o timestamp de quando o CGI começou. Em cada checagem, se `ret == 0` mas já passou muito tempo (ex: 5 segundos), você mata o processo:

```c
if (ret == 0) {
    if (time(NULL) - cgi_start_time > TIMEOUT) {
        kill(cgi_pid, SIGKILL);
        waitpid(cgi_pid, &status, 0); // agora pode bloquear, já que tá morto
        // trata como erro (502/504)
    }
}
```

**2. Use o pipe como sinal principal, não o `waitpid`**

Uma abordagem bem comum (e mais "orientada a eventos") é: você coloca o `fd` de leitura do pipe (stdout do CGI) no seu `poll()`. Quando o CGI termina e fecha o pipe (o `execve` encerra, o kernel fecha os fds dele), o `poll` te avisa que o fd está pronto para leitura e, depois, que chegou EOF (`read` retorna 0). **Esse é o gatilho real** de "o CGI terminou de produzir saída".

Nesse esquema, o `waitpid(WNOHANG)` serve mais para:
- confirmar que o processo de fato morreu e pegar o exit status,
- evitar zumbis,

...e você pode chamá-lo só **depois** de detectar EOF no pipe, em vez de checar a cada iteração do loop. Isso simplifica bastante: você não precisa ficar "pollando" o `waitpid` toda hora — o pipe já te avisa via `poll`.

### Resumo

- Se você só usa `waitpid(WNOHANG)`: sim, checa periodicamente dentro do loop até `ret != 0`.
- Se você combina com `poll` no fd do pipe (mais elegante e mais alinhado com a arquitetura do webserv): o `poll` te avisa quando o CGI terminou de escrever, e aí você chama `waitpid` (com ou sem `WNOHANG`, já que nesse ponto o processo provavelmente já terminou ou está terminando) só uma vez para colher o status.

A segunda abordagem é geralmente preferida porque evita "busy waiting" e se encaixa melhor na arquitetura de I/O não-bloqueante que o projeto exige.

---

## Mas como eu, que cuido só da parte de CGI, vou mexer/atualizar o poll feito pelo meu colega?

Essa é uma questão bem comum em projetos em grupo desse tipo, e a resposta é: você não "mexe" no poll do seu colega diretamente — você **expõe uma interface** que o módulo de CGI oferece, e o loop principal (que seu colega mantém) **consome** essa interface.

A ideia central é inversão de controle: o seu código não controla o loop, mas o loop "pergunta" ao seu módulo o que ele precisa.

### Como estruturar isso na prática

**1. Seu módulo expõe os fds que precisam ser monitorados**

Cada CGI ativo tem (tipicamente) até 3 fds relevantes:
- fd de escrita do pipe (para enviar o body da requisição ao CGI, se for POST)
- fd de leitura do pipe (para ler a saída/stdout do CGI)

Seu módulo de CGI mantém uma lista/estrutura desses CGIs ativos, cada um com:
```cpp
struct CgiSession {
    pid_t   pid;
    int     fd_read;   // stdout do CGI -> servidor
    int     fd_write;  // servidor -> stdin do CGI (se houver body)
    time_t  start_time;
    // ... buffers, estado, etc.
};
```

**2. O loop principal "pergunta" quais fds adicionar**

Você cria uma função (ou método) que retorna os fds que precisam entrar no `poll()`:

```cpp
std::vector<pollfd> Cgi::getPollFds();
```

Seu colega, no loop dele, faz algo como:
```cpp
std::vector<pollfd> cgi_fds = cgiManager.getPollFds();
// adiciona esses fds ao array principal de pollfds
```

**3. Quando o `poll()` retorna, o loop avisa seu módulo quais fds estão prontos**

Algo como:
```cpp
cgiManager.handleReadyFd(fd, revents);
```

Dentro disso, **você** trata: ler do pipe, escrever no pipe, detectar EOF, chamar `waitpid`, montar a resposta etc. Tudo isso é interno ao seu módulo.

**4. Defina um "contrato" claro de entrada/saída**

Combine com seu colega algo do tipo:
- Você recebe: a requisição já parseada (método, path, headers, body) + talvez o socket fd do cliente.
- Você retorna/produz: os dados da resposta CGI (já formatados como resposta HTTP, ou pelo menos os bytes a serem enviados), ou um status (ainda processando / pronto / erro/timeout).
- O loop dele apenas chama suas funções no momento certo e te dá os fds prontos.

### Resumindo a divisão de responsabilidades

| Quem | Responsabilidade |
|---|---|
| Loop principal (colega) | `poll()`, gerenciar todos os fds (sockets de cliente + fds do CGI que você expõe), chamar suas funções quando algo estiver pronto |
| Seu módulo CGI | `fork`/`execve`, criar pipes, ler/escrever nesses pipes, `waitpid`, montar resposta, gerenciar timeout |

Assim, você nunca precisa editar o código do `poll` dele — só negociar a **interface** (assinatura de funções/structs) entre os dois módulos. Isso é literalmente o motivo de existirem interfaces/headers bem definidos em projetos C++ em grupo: cada um implementa seu `.cpp`, mas o `.hpp` é o contrato combinado entre vocês.
</file_text>