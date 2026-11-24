# AES-128 em C++

Trabalho de Projeto e Análise de Algoritmos (CTC04), Tema 1.10, Instituto de Matemática e Computação da UNIFEI.

Implementação do AES-128 feita do zero em C++, sem biblioteca externa no núcleo criptográfico. O projeto tem a versão padrão (as quatro transformações de cada rodada), uma versão otimizada com T-Tables e um benchmark que compara as duas com o OpenSSL.

## Equipe

| Integrante | Responsabilidade |
|---|---|
| Integrante 1 - Karol Luiza | Transformações básicas: SubBytes, ShiftRows, MixColumns e AddRoundKey |
| Integrante 2 - Rafaella Gonzales | Expansão de chave, funções inversas, cifração/decifração e T-Tables |
| Integrante 3 - Gabriel Furlan | Validação com vetores do NIST, benchmark e documentação |

## Estrutura do repositório

```
src/
  aes_transformacoes.h / .cpp   quatro transformações da rodada e S-Box
  aes_cifrador.h / .cpp         expandirChave, inversas, cifrarBloco, decifrarBloco
  aes_ttables.cpp               cifrarBlocoTTables (versão com T-Tables)
tests/
  teste_transformacoes.cpp      confere a Rodada 1 do exemplo do Apêndice B do FIPS-197
  teste_cifrador.cpp            confere expansão de chave, cifração e decifração (Apêndices B e C.1)
main.cpp                        programa principal
benchmark.cpp                   medição de desempenho
```

## Decisões de projeto

- Todo bloco de 128 bits é um `unsigned char estado[16]` em ordem de colunas (column-major): o byte `estado[4*c + r]` é a linha `r` da coluna `c`.
- As 11 subchaves ficam num único vetor de 176 bytes. A subchave da rodada `r` começa em `chavesRodada + 16 * r`.
- A S-Box só é acessada por `obterValorSBox()`. A S-Box inversa e as tabelas T0..T3 são montadas quando o programa carrega, então não entram na medição do benchmark.
- `entrada` e `saida` das funções de bloco podem apontar para o mesmo buffer.

## Requisitos

- Linux (testado em Ubuntu)
- `g++` com suporte a C++11 ou superior
- Para o benchmark: OpenSSL com cabeçalhos de desenvolvimento

```
sudo apt update
sudo apt install g++ libssl-dev
```

## Compilar e rodar os testes

Todos os comandos partem da raiz do repositório.

Transformações básicas (Integrante 1):
```
g++ -O2 -o teste_transformacoes src/aes_transformacoes.cpp tests/teste_transformacoes.cpp
./teste_transformacoes
```

Cifrador completo, versão padrão e T-Tables (Integrante 2):
```
g++ -O2 -o teste_cifrador src/aes_transformacoes.cpp src/aes_cifrador.cpp src/aes_ttables.cpp tests/teste_cifrador.cpp
./teste_cifrador
```

A saída esperada termina com `Tudo certo`. Cada linha mostra `PASSOU` ou `FALHOU` para uma verificação.

## Validação

Os testes usam os vetores oficiais do NIST FIPS-197:

- Apêndice A.1: última subchave gerada pela expansão da chave `2b7e1516 28aed2a6 abf71588 09cf4f3c`
- Apêndice B: texto claro `3243f6a8 885a308d 313198a2 e0370734` com a mesma chave, resultado `3925841d 02dc09fb dc118597 196a0b32`
- Apêndice C.1: chave `000102...0f`, texto claro `00112233...ff`, resultado `69c4e0d8 6a7b0430 d8cdb780 70b4c55a`

A versão padrão e a versão com T-Tables precisam dar exatamente o mesmo texto cifrado, e `decifrarBloco(cifrarBloco(M))` precisa devolver `M`.

## Benchmark

> Preencher quando o `benchmark.cpp` estiver fechado (Integrante 3): comando exato de compilação, tamanhos de buffer usados, modo de operação (ECB ou CBC) e quantas repetições por medição.

Comando de compilação previsto, linkando o OpenSSL:
```
g++ -O2 -o benchmark src/aes_transformacoes.cpp src/aes_cifrador.cpp src/aes_ttables.cpp benchmark.cpp -lcrypto
./benchmark
```

O throughput é calculado como tamanho em MB dividido pelo tempo em segundos, com `std::chrono`. Os volumes testados são 1 MB, 10 MB, 50 MB e 100 MB, comparando AES padrão, AES com T-Tables e OpenSSL.

### Resultados

> Colar aqui a tabela de MB/s por tamanho de entrada e o gráfico, com o speedup das T-Tables sobre a versão padrão.

| Tamanho | Padrão (MB/s) | T-Tables (MB/s) | OpenSSL (MB/s) | Speedup T-Tables |
|---|---|---|---|---|
| 1 MB | | | | |
| 10 MB | | | | |
| 50 MB | | | | |
| 100 MB | | | | |

## Ambiente usado nas medições

> Preencher: processador, versão do g++, versão do OpenSSL, sistema operacional.

## Observações

Este código é um trabalho acadêmico. Ele serve para estudar o algoritmo e não deve ser usado para proteger dados reais: não há proteção contra ataques de canal lateral (as tabelas indexadas por dado secreto são justamente um exemplo clássico) e os modos de operação do benchmark existem só para medir velocidade.
