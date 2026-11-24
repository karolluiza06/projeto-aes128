#ifndef AES_CIFRADOR_H
#define AES_CIFRADOR_H

#include <cstdint>

// Expande a chave de 16 bytes em 11 subchaves (176 bytes).
// Subchave da rodada r comeca em chavesRodada + 16 * r
void expandirChave(const unsigned char chaveInicial[16], unsigned char chavesRodada[176]);

// Operacoes inversas (usadas na decifracao)
void substituirBytesInverso(unsigned char estado[16]);
void deslocarLinhasInverso(unsigned char estado[16]);
void misturarColunasInverso(unsigned char estado[16]);

// Versao padrao: 10 rodadas usando as 4 transformacoes do Integrante 1
void cifrarBloco(const unsigned char entrada[16], unsigned char saida[16],
                 const unsigned char chavesRodada[176]);
void decifrarBloco(const unsigned char entrada[16], unsigned char saida[16],
                   const unsigned char chavesRodada[176]);

// Versao acelerada com T-Tables (SubBytes + ShiftRows + MixColumns fundidos)
void cifrarBlocoTTables(const unsigned char entrada[16], unsigned char saida[16],
                        const unsigned char chavesRodada[176]);

#endif
