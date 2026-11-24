#include <string>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "../src/aes_cifrador.h"

void imprimir(const char* titulo, const unsigned char* b, int n) {
    std::cout << titulo << ": ";
    for (int i = 0; i < n; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b[i];
    }
    std::cout << std::dec << "\n";
}

int falhas = 0;

void conferir(const char* nome, const unsigned char* obtido, const unsigned char* esperado, int n) {
    bool ok = std::memcmp(obtido, esperado, n) == 0;
    std::cout << (ok ? "PASSOU  " : "FALHOU  ") << nome << "\n";
    if (!ok) {
        falhas++;
        imprimir("  obtido  ", obtido, n);
        imprimir("  esperado", esperado, n);
    }
}

void testarVetor(const char* nome, const unsigned char chave[16],
                 const unsigned char claro[16], const unsigned char cifrado[16]) {
    unsigned char chaves[176], saida[16], volta[16];
    expandirChave(chave, chaves);

    cifrarBloco(claro, saida, chaves);
    conferir((std::string(nome) + " - cifrarBloco").c_str(), saida, cifrado, 16);

    cifrarBlocoTTables(claro, saida, chaves);
    conferir((std::string(nome) + " - cifrarBlocoTTables").c_str(), saida, cifrado, 16);

    decifrarBloco(cifrado, volta, chaves);
    conferir((std::string(nome) + " - decifrarBloco").c_str(), volta, claro, 16);
}

int main() {
    // Apendice A.1: ultima subchave da chave 2b7e1516...
    const unsigned char chaveA[16] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
    const unsigned char ultimaSubchave[16] = {0xd0,0x14,0xf9,0xa8,0xc9,0xee,0x25,0x89,0xe1,0x3f,0x0c,0xc8,0xb6,0x63,0x0c,0xa6};
    unsigned char chaves[176];
    expandirChave(chaveA, chaves);
    conferir("expandirChave - subchave 10", chaves + 160, ultimaSubchave, 16);

    // Apendice B
    const unsigned char claroB[16] = {0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d,0x31,0x31,0x98,0xa2,0xe0,0x37,0x07,0x34};
    const unsigned char cifradoB[16] = {0x39,0x25,0x84,0x1d,0x02,0xdc,0x09,0xfb,0xdc,0x11,0x85,0x97,0x19,0x6a,0x0b,0x32};
    testarVetor("Apendice B", chaveA, claroB, cifradoB);

    // Apendice C.1
    const unsigned char chaveC[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    const unsigned char claroC[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    const unsigned char cifradoC[16] = {0x69,0xc4,0xe0,0xd8,0x6a,0x7b,0x04,0x30,0xd8,0xcd,0xb7,0x80,0x70,0xb4,0xc5,0x5a};
    testarVetor("Apendice C.1", chaveC, claroC, cifradoC);

    std::cout << (falhas == 0 ? "\nTudo certo\n" : "\nTem teste falhando\n");
    return falhas;
}
