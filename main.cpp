#include <cstring>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#include "src/aes_cifrador.h"

namespace {

void imprimirHex(const unsigned char* dados, std::size_t tamanho) {
    std::ios_base::fmtflags flags(std::cout.flags());
    for (std::size_t i = 0; i < tamanho; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(dados[i]);
        if ((i + 1) % 4 == 0 && (i + 1) < tamanho) {
            std::cout << ' ';
        }
    }
    std::cout.flags(flags);
}

bool verificarBloco(const std::string& nomeTeste,
                    const unsigned char* obtido,
                    const unsigned char* esperado,
                    std::size_t tamanho = 16) {
    bool passou = (std::memcmp(obtido, esperado, tamanho) == 0);
    std::cout << "[" << (passou ? "PASSOU" : "FALHOU") << "] " << nomeTeste << "\n";
    if (!passou) {
        std::cout << "   Esperado: ";
        imprimirHex(esperado, tamanho);
        std::cout << "\n   Obtido:   ";
        imprimirHex(obtido, tamanho);
        std::cout << "\n";
    }
    return passou;
}

}  // namespace

int main() {
    bool tudoCerto = true;
    unsigned char chavesRodada[176];

    std::cout << "=== Validacao AES-128 (Vetores Oficiais NIST FIPS-197) ===\n\n";

    // -------------------------------------------------------------------------
    // 1. Apendice A.1 - Expansao de chave (verificando a 11a subchave / rodada 10)
    // -------------------------------------------------------------------------
    unsigned char chaveB[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    unsigned char subchave10Esperada[16] = {
        0xd0, 0x14, 0xf9, 0xa8, 0xc9, 0xee, 0x25, 0x89,
        0xe1, 0x3f, 0x0c, 0xc8, 0xb6, 0x63, 0x0c, 0xa6
    };

    expandirChave(chaveB, chavesRodada);
    tudoCerto &= verificarBloco("FIPS-197 Apendice A.1 - Expansao de Chave (Rodada 10)",
                                chavesRodada + 16 * 10, subchave10Esperada);

    // -------------------------------------------------------------------------
    // 2. Apendice B - Exemplo de Cifracao
    // -------------------------------------------------------------------------
    unsigned char claroB[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
    };
    unsigned char cifradoBEsperado[16] = {
        0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
        0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
    };

    unsigned char saidaPadrao[16];
    unsigned char saidaTTables[16];
    unsigned char decifrado[16];

    cifrarBloco(claroB, saidaPadrao, chavesRodada);
    tudoCerto &= verificarBloco("FIPS-197 Apendice B   - Cifracao Padrao",
                                saidaPadrao, cifradoBEsperado);

    cifrarBlocoTTables(claroB, saidaTTables, chavesRodada);
    tudoCerto &= verificarBloco("FIPS-197 Apendice B   - Cifracao T-Tables",
                                saidaTTables, cifradoBEsperado);

    decifrarBloco(saidaPadrao, decifrado, chavesRodada);
    tudoCerto &= verificarBloco("FIPS-197 Apendice B   - Decifracao (decifrar(cifrar(M)) == M)",
                                decifrado, claroB);

    // -------------------------------------------------------------------------
    // 3. Apendice C.1 - Vetor de Teste AES-128
    // -------------------------------------------------------------------------
    unsigned char chaveC1[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    unsigned char claroC1[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    unsigned char cifradoC1Esperado[16] = {
        0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
        0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a
    };

    expandirChave(chaveC1, chavesRodada);

    cifrarBloco(claroC1, saidaPadrao, chavesRodada);
    tudoCerto &= verificarBloco("FIPS-197 Apendice C.1 - Cifracao Padrao",
                                saidaPadrao, cifradoC1Esperado);

    cifrarBlocoTTables(claroC1, saidaTTables, chavesRodada);
    tudoCerto &= verificarBloco("FIPS-197 Apendice C.1 - Cifracao T-Tables",
                                saidaTTables, cifradoC1Esperado);

    decifrarBloco(saidaPadrao, decifrado, chavesRodada);
    tudoCerto &= verificarBloco("FIPS-197 Apendice C.1 - Decifracao (decifrar(cifrar(M)) == M)",
                                decifrado, claroC1);

    // -------------------------------------------------------------------------
    // 4. Teste de consistencia em 1.000 blocos aleatorios + operacao in-place
    // -------------------------------------------------------------------------
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    bool consistenciaOk = true;
    bool inPlaceOk = true;

    for (int i = 0; i < 1000; ++i) {
        unsigned char chaveRand[16];
        unsigned char blocoRand[16];
        for (int b = 0; b < 16; ++b) {
            chaveRand[b] = static_cast<unsigned char>(dist(rng));
            blocoRand[b] = static_cast<unsigned char>(dist(rng));
        }

        expandirChave(chaveRand, chavesRodada);
        cifrarBloco(blocoRand, saidaPadrao, chavesRodada);
        cifrarBlocoTTables(blocoRand, saidaTTables, chavesRodada);
        decifrarBloco(saidaPadrao, decifrado, chavesRodada);

        if (std::memcmp(saidaPadrao, saidaTTables, 16) != 0 ||
            std::memcmp(decifrado, blocoRand, 16) != 0) {
            consistenciaOk = false;
            break;
        }

        // Verifica entrada == saida (in-place)
        unsigned char bufferInPlace[16];
        std::memcpy(bufferInPlace, blocoRand, 16);
        cifrarBloco(bufferInPlace, bufferInPlace, chavesRodada);
        if (std::memcmp(bufferInPlace, saidaPadrao, 16) != 0) {
            inPlaceOk = false;
            break;
        }
        decifrarBloco(bufferInPlace, bufferInPlace, chavesRodada);
        if (std::memcmp(bufferInPlace, blocoRand, 16) != 0) {
            inPlaceOk = false;
            break;
        }
    }

    std::cout << "[" << (consistenciaOk ? "PASSOU" : "FALHOU")
              << "] Consistencia Padrao == T-Tables e inversibilidade (1.000 blocos aleatorios)\n";
    std::cout << "[" << (inPlaceOk ? "PASSOU" : "FALHOU")
              << "] Operacao in-place (entrada == saida)\n";

    tudoCerto &= consistenciaOk && inPlaceOk;

    std::cout << "\n";
    if (tudoCerto) {
        std::cout << "Tudo certo\n";
        return 0;
    } else {
        std::cout << "Alguma verificacao FALHOU!\n";
        return 1;
    }
}