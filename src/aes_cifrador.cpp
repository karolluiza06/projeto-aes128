#include "aes_cifrador.h"
#include "aes_transformacoes.h"

namespace {

// xtime proprio: o multiplicarPorDois do Integrante 1 nao esta no header dele
unsigned char xtime(unsigned char valor) {
    unsigned char deslocado = static_cast<unsigned char>(valor << 1);
    if (valor & 0x80) {
        deslocado ^= 0x1b;
    }
    return deslocado;
}

// Multiplicacao geral em GF(2^8) (usada com 0x09, 0x0b, 0x0d, 0x0e)
unsigned char multiplicarGF(unsigned char a, unsigned char b) {
    unsigned char resultado = 0;
    while (b) {
        if (b & 1) {
            resultado ^= a;
        }
        a = xtime(a);
        b >>= 1;
    }
    return resultado;
}

// S-Box inversa montada a partir da S-Box direta, na carga do programa
struct SBoxInversa {
    unsigned char tabela[256];
    SBoxInversa() {
        for (int i = 0; i < 256; i++) {
            tabela[obterValorSBox(i)] = static_cast<unsigned char>(i);
        }
    }
};
const SBoxInversa sboxInversa;

const unsigned char RCON[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

} // namespace

void expandirChave(const unsigned char chaveInicial[16], unsigned char chavesRodada[176]) {
    for (int i = 0; i < 16; i++) {
        chavesRodada[i] = chaveInicial[i];
    }

    // 44 palavras de 4 bytes; as 4 primeiras sao a chave original
    for (int palavra = 4; palavra < 44; palavra++) {
        unsigned char temp[4];
        for (int j = 0; j < 4; j++) {
            temp[j] = chavesRodada[4 * (palavra - 1) + j];
        }

        if (palavra % 4 == 0) {
            // RotWord
            unsigned char primeiro = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = primeiro;

            // SubWord
            for (int j = 0; j < 4; j++) {
                temp[j] = obterValorSBox(temp[j]);
            }

            // Rcon
            temp[0] ^= RCON[palavra / 4 - 1];
        }

        for (int j = 0; j < 4; j++) {
            chavesRodada[4 * palavra + j] = chavesRodada[4 * (palavra - 4) + j] ^ temp[j];
        }
    }
}

void substituirBytesInverso(unsigned char estado[16]) {
    for (int i = 0; i < 16; i++) {
        estado[i] = sboxInversa.tabela[estado[i]];
    }
}

// Linhas 1, 2 e 3 giram 1, 2 e 3 casas para a direita
void deslocarLinhasInverso(unsigned char estado[16]) {
    unsigned char temp;

    // Linha 1: 1 casa para a direita
    temp = estado[13];
    estado[13] = estado[9];
    estado[9]  = estado[5];
    estado[5]  = estado[1];
    estado[1]  = temp;

    // Linha 2: 2 casas (troca de pares)
    temp = estado[2];
    estado[2]  = estado[10];
    estado[10] = temp;
    temp = estado[6];
    estado[6]  = estado[14];
    estado[14] = temp;

    // Linha 3: 3 casas para a direita (= 1 para a esquerda)
    temp = estado[3];
    estado[3]  = estado[7];
    estado[7]  = estado[11];
    estado[11] = estado[15];
    estado[15] = temp;
}

void misturarColunasInverso(unsigned char estado[16]) {
    for (int coluna = 0; coluna < 4; coluna++) {
        int inicio = coluna * 4;
        unsigned char a0 = estado[inicio];
        unsigned char a1 = estado[inicio + 1];
        unsigned char a2 = estado[inicio + 2];
        unsigned char a3 = estado[inicio + 3];

        estado[inicio]     = multiplicarGF(a0, 0x0e) ^ multiplicarGF(a1, 0x0b) ^ multiplicarGF(a2, 0x0d) ^ multiplicarGF(a3, 0x09);
        estado[inicio + 1] = multiplicarGF(a0, 0x09) ^ multiplicarGF(a1, 0x0e) ^ multiplicarGF(a2, 0x0b) ^ multiplicarGF(a3, 0x0d);
        estado[inicio + 2] = multiplicarGF(a0, 0x0d) ^ multiplicarGF(a1, 0x09) ^ multiplicarGF(a2, 0x0e) ^ multiplicarGF(a3, 0x0b);
        estado[inicio + 3] = multiplicarGF(a0, 0x0b) ^ multiplicarGF(a1, 0x0d) ^ multiplicarGF(a2, 0x09) ^ multiplicarGF(a3, 0x0e);
    }
}

void cifrarBloco(const unsigned char entrada[16], unsigned char saida[16],
                 const unsigned char chavesRodada[176]) {
    unsigned char estado[16];
    for (int i = 0; i < 16; i++) {
        estado[i] = entrada[i];
    }

    adicionarChaveRodada(estado, chavesRodada);

    for (int rodada = 1; rodada <= 9; rodada++) {
        substituirBytes(estado);
        deslocarLinhas(estado);
        misturarColunas(estado);
        adicionarChaveRodada(estado, chavesRodada + 16 * rodada);
    }

    // Rodada 10: sem MixColumns
    substituirBytes(estado);
    deslocarLinhas(estado);
    adicionarChaveRodada(estado, chavesRodada + 160);

    for (int i = 0; i < 16; i++) {
        saida[i] = estado[i];
    }
}

void decifrarBloco(const unsigned char entrada[16], unsigned char saida[16],
                   const unsigned char chavesRodada[176]) {
    unsigned char estado[16];
    for (int i = 0; i < 16; i++) {
        estado[i] = entrada[i];
    }

    adicionarChaveRodada(estado, chavesRodada + 160);

    for (int rodada = 9; rodada >= 1; rodada--) {
        deslocarLinhasInverso(estado);
        substituirBytesInverso(estado);
        adicionarChaveRodada(estado, chavesRodada + 16 * rodada);
        misturarColunasInverso(estado);
    }

    deslocarLinhasInverso(estado);
    substituirBytesInverso(estado);
    adicionarChaveRodada(estado, chavesRodada);

    for (int i = 0; i < 16; i++) {
        saida[i] = estado[i];
    }
}
