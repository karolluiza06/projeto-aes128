#include <iostream>
#include <iomanip>
#include "../src/aes_transformacoes.h"

bool blocosIguais(const unsigned char blocoA[16], const unsigned char blocoB[16]) {
    for (int indice = 0; indice < 16; indice++) {
        if (blocoA[indice] != blocoB[indice]) {
            return false;
        }
    }
    return true;
}

void imprimirBloco(const char* titulo, const unsigned char bloco[16]) {
    std::cout << titulo << ": ";
    for (int indice = 0; indice < 16; indice++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bloco[indice]) << " ";
    }
    std::cout << std::dec << std::endl;
}

void validarEtapa(const char* nomeEtapa, const unsigned char obtido[16], const unsigned char esperado[16]) {
    imprimirBloco(nomeEtapa, obtido);
    if (blocosIguais(obtido, esperado)) {
        std::cout << "PASSOU\n\n";
    } else {
        std::cout << "FALHOU\n\n";
    }
}

int main() {
    // Valores oficiais do exemplo passo a passo do NIST FIPS-197 (Apendice B)
    unsigned char estado[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
    };

    const unsigned char chaveInicial[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };

    const unsigned char chaveRodada1[16] = {
        0xa0, 0xfa, 0xfe, 0x17, 0x88, 0x54, 0x2c, 0xb1,
        0x23, 0xa3, 0x39, 0x39, 0x2a, 0x6c, 0x76, 0x05
    };

    // Saidas esperadas oficialmente apos cada etapa da Rodada 1
    const unsigned char esperadoAposChave0[16] = {
        0x19, 0x3d, 0xe3, 0xbe, 0xa0, 0xf4, 0xe2, 0x2b,
        0x9a, 0xc6, 0x8d, 0x2a, 0xe9, 0xf8, 0x48, 0x08
    };

    const unsigned char esperadoAposSubBytes[16] = {
        0xd4, 0x27, 0x11, 0xae, 0xe0, 0xbf, 0x98, 0xf1,
        0xb8, 0xb4, 0x5d, 0xe5, 0x1e, 0x41, 0x52, 0x30
    };

    const unsigned char esperadoAposShiftRows[16] = {
        0xd4, 0xbf, 0x5d, 0x30, 0xe0, 0xb4, 0x52, 0xae,
        0xb8, 0x41, 0x11, 0xf1, 0x1e, 0x27, 0x98, 0xe5
    };

    const unsigned char esperadoAposMixColumns[16] = {
        0x04, 0x66, 0x81, 0xe5, 0xe0, 0xcb, 0x19, 0x9a,
        0x48, 0xf8, 0xd3, 0x7a, 0x28, 0x06, 0x26, 0x4c
    };

    const unsigned char esperadoFimRodada1[16] = {
        0xa4, 0x9c, 0x7f, 0xf2, 0x68, 0x9f, 0x35, 0x2b,
        0x6b, 0x5b, 0xea, 0x43, 0x02, 0x6a, 0x50, 0x49
    };

    adicionarChaveRodada(estado, chaveInicial);
    validarEtapa("1. Apos adicionarChaveRodada (0)", estado, esperadoAposChave0);

    substituirBytes(estado);
    validarEtapa("2. Apos substituirBytes ", estado, esperadoAposSubBytes);

    deslocarLinhas(estado);
    validarEtapa("3. Apos deslocarLinhas ", estado, esperadoAposShiftRows);

    misturarColunas(estado);
    validarEtapa("4. Apos misturarColunas ", estado, esperadoAposMixColumns);

    adicionarChaveRodada(estado, chaveRodada1);
    validarEtapa("5. Fim da Rodada 1 ", estado, esperadoFimRodada1);

    return 0;
}