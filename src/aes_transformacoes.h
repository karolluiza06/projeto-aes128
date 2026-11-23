#ifndef AES_TRANSFORMACOES_H
#define AES_TRANSFORMACOES_H

// Retorna o valor correspondente na tabela S-Box oficial do AES (0 a 255)
unsigned char obterValorSBox(int indice);

// As 4 operacoes basicas de transformacao sobre um bloco de 16 bytes
void substituirBytes(unsigned char estado[16]);
void deslocarLinhas(unsigned char estado[16]);
void misturarColunas(unsigned char estado[16]);
void adicionarChaveRodada(unsigned char estado[16], const unsigned char chaveRodada[16]);

#endif