# TP2 - Sistemas Operacionais - 2024/2 

## Membros

- Iago da Silva Rodrigues Alves;
- Vitor Moreira Ramos de Rezende.

## Introdução
O TP2 foi dividido em três partes principais:

1. **Tutorial inicial**
3. **Novas chamadas**
3. **Copy-on-Write**

---

## Implementação

### Parte 1: Adição de uma Chamada Simples
- Seguindo o tutorial, foi implementada a chamada `sys_date`:
  - Um programa simples foi criado para exibir a data no formato `dd/mm/aaaa hh:mm:ss`.

---

### Parte 2: Implementação das Chamadas `virt2real` e `num_pages`

1. **`num_pages`**:  
   - A chamada calcula o número de páginas usadas por um processo.

2. **`virt2real`**:  
   - A chamada recebe um endereço virtual e retorna o real.

---

### Parte 3: Implementação do Copy-on-Write com `forkcow`
   - Foi criada a chamada `forkcow`, semelhante a `fork`, porém, diferentemente do `fork`, o intuito dessa chamada é que o filho referencie as páginas físicas do pai, em vez de criar novas páginas imediatamente a sua criação;
   - Para tanto, criamos o `copyuvmcow`, uma modificação do `copyuvm`, e o `pagefault`, necessário para tratar o caso em que o filho tenta ecrever na página compartilhada com o pai, que deveria ser apenas de leitura;
   - Além disso, modificamos outros arquivos. 

---

## Conclusão
O trabalho permitiu:
- Entender o mapeamento de diretórios de página e tabelas para endereços reais.
- Compreender o uso de flags para restrições de acesso e tratamento de page faults.
- Aprender sobre o funcionamento de `flush` do TLB para evitar inconsistências.
- Observar o funcionamento completo de chamadas copy-on-write e sua implementação no xv6.

Este projeto foi desenvolvido e testado em um ambiente Ubuntu 22 utilizando WSL (Windows Subsystem for Linux). Para compilar o sistema, basta executar os comandos `make` e `make qemu` no terminal. Após inicializar o sistema xv6, é possível testar a implementação utilizando os comandos `forktest` para validar o funcionamento da chamada `forkcow` e `corretor` para rodar os testes automáticos disponibilizados pelo corretor.
