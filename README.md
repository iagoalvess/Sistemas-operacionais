# Projeto de Manipulação de Processos

## Descrição

Este projeto apresenta dois programas em C para manipulação e monitoramento de processos em um sistema operacional, simulando funcionalidades comuns de um shell e do comando `top` do Linux.

## Programas

- **sh.c**: Implementação de um mini shell, permitindo a execução de comandos básicos, como `ls` e `cat`. Esse shell simula a interação com o sistema, possibilitando ao usuário executar comandos e redirecionar saídas.
- **top.c**: Implementação semelhante ao comando `top` do Linux, exibindo uma tabela dos processos em execução. Permite ao usuário visualizar informações e encerrar processos específicos.

## Executáveis

- **myshell**: Executável gerado a partir do `sh.c`. Com ele, é possível rodar comandos diretamente, como:
  - `ls` - lista arquivos e diretórios.
  - `cat sh.c` - exibe o conteúdo do arquivo `sh.c`.
  - `echo "DCC605 is cool" > x.txt` - cria ou escreve em `x.txt`.
  
- **meutop**: Executável gerado a partir do `top.c`. Este programa mostra uma tabela com os 20 principais processos e permite encerrá-los, usando a seguinte regra:
  - `<pid> <sinal>` - onde `<pid>` é o identificador do processo, e `<sinal>` representa o sinal enviado para o processo, como `1` (SIGHUP) para encerrar imediatamente.
  - É importante pontuar que por o programa limpar a tela para atualizar a lista a cada 1 segundo, quando o usuário escrever o pid e o sinal provavelmente eles ou parte deles irão desaparecer da tela, por serem limpos juntamente com a tabela, porém, isso não interfere na execução do comando.

## Rodar

Para rodar os programas, utilize os seguintes comandos:

```bash
./myshell
./meutop
