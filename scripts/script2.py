import sys

def main():
    # Verifica se exatamente 2 argumentos foram passados (o índice 0 é o nome do script)
    if len(sys.argv) != 3:
        print("Erro: Você deve fornecer exatamente dois números como argumentos.", file=sys.stderr)
        sys.exit(1)

    try:
        # Tenta converter os argumentos para inteiros
        num1 = int(sys.argv[1])
        num2 = int(sys.argv[2])
        
        # Realiza a soma e imprime no stdout
        soma = num1 + num2
        print(soma)
        
    except ValueError:
        # Captura o erro caso os argumentos não sejam números inteiros válidos
        print("Erro: Os argumentos fornecidos devem ser números inteiros.", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()