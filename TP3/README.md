# TP3 - Sistemas Operacionais - 2024/2 

## Membros

- Iago da Silva Rodrigues Alves;
- Vitor Moreira Ramos de Rezende.

## Introdução
O TP3 teve como objetivo a criação de um shell capaz de manipular sistemas de arquivos ext2 contidos em extensões do tipo `.img`, a partir da manipulação de inodes.

---

## Comandos implementados:

### Parte 1: Comando obrigatórios e suas especificações:
- **`cd <diretório>`** - Muda para o diretório especificado;
- **`ls`** - Lista os arquivos e diretórios do diretório atual;
- **`find <diretório>`** - Imprime toda a árvore de arquivos/diretórios iniciando do diretório especificado;
- **`find`** - Imprime toda a árvore de arquivos/diretórios iniciando do diretório atual;
- **`stat <diretório>`** - Pega os metadados de um arquivo/diretório contido no diretório atual;
- **`sb`** - Lê os dados do super-bloco.
---
### Parte 2: Extras:

- **`clear`** - Limpa a tela do shell;
- **`help`** - Imprime os comandos disponíveis;
- **`exit`** - Encerra o shell.
  
---

## Modo de usar:

1. **Compile**: gcc shell.c -o shell;
2. **Execute**: ./shell sistema_arquivo_ext.img
3. **Insira um dos comandos abaixo(Leve em conta as especificaçãos acima)**:
    - **`cd <diretório>`**;
    - **`ls`**;
    - **`find <diretório>`**;
    - **`find`**;
    - **`stat <diretório>`**;
    - **`sb`**;

---

## Conclusão
O trabalho permitiu:
- Compreender melhor o Fast File System (FFS);
- Compreender melhor a manipulação dos inodes;
- Compreender melhor como os arquivos e diretórios são armazenados;
- Compreender melhor os comandos implementados.
