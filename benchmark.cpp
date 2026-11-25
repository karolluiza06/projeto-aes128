#include <openssl/evp.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

#include "src/aes_cifrador.h"

namespace {

using Relogio = std::chrono::steady_clock;

void cifrarBufferPadrao(const unsigned char* entrada,
                        unsigned char* saida,
                        std::size_t tamanho,
                        unsigned char* chavesRodada) {
    for (std::size_t i = 0; i < tamanho; i += 16) {
        cifrarBloco(const_cast<unsigned char*>(entrada + i), saida + i, chavesRodada);
    }
}

void cifrarBufferTTables(const unsigned char* entrada,
                         unsigned char* saida,
                         std::size_t tamanho,
                         unsigned char* chavesRodada) {
    for (std::size_t i = 0; i < tamanho; i += 16) {
        cifrarBlocoTTables(const_cast<unsigned char*>(entrada + i), saida + i, chavesRodada);
    }
}

void cifrarBufferOpenSSL(EVP_CIPHER_CTX* ctx,
                         const unsigned char* chave,
                         const unsigned char* entrada,
                         unsigned char* saida,
                         std::size_t tamanho) {
    int len = 0;
    int finalLen = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, chave, nullptr) != 1) {
        throw std::runtime_error("Falha em EVP_EncryptInit_ex");
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (EVP_EncryptUpdate(ctx, saida, &len, entrada, static_cast<int>(tamanho)) != 1) {
        throw std::runtime_error("Falha em EVP_EncryptUpdate");
    }
    if (EVP_EncryptFinal_ex(ctx, saida + len, &finalLen) != 1) {
        throw std::runtime_error("Falha em EVP_EncryptFinal_ex");
    }
}

struct ResultadoLinha {
    std::size_t tamanhoMB;
    double padraoMBs;
    double ttablesMBs;
    double opensslMBs;
    double speedupTTables;
};

}  // namespace

int main() {
    constexpr int REPETICOES = 5;
    const std::vector<std::size_t> tamanhosMB = {1, 10, 50, 100};

    unsigned char chave[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    unsigned char chavesRodada[176];
    expandirChave(chave, chavesRodada);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Erro ao alocar EVP_CIPHER_CTX do OpenSSL.\n";
        return 1;
    }

    // -------------------------------------------------------------------------
    // Validacao de sanidade: garante que Padrao, T-Tables e OpenSSL geram
    // exatamente os mesmos bytes no modo ECB sem padding.
    // -------------------------------------------------------------------------
    {
        constexpr std::size_t tamTeste = 4096;
        std::vector<unsigned char> entrada(tamTeste);
        std::vector<unsigned char> outPadrao(tamTeste);
        std::vector<unsigned char> outTTables(tamTeste);
        std::vector<unsigned char> outOpenSSL(tamTeste);

        std::mt19937 rng(12345);
        for (auto& b : entrada) {
            b = static_cast<unsigned char>(rng() & 0xFF);
        }

        cifrarBufferPadrao(entrada.data(), outPadrao.data(), tamTeste, chavesRodada);
        cifrarBufferTTables(entrada.data(), outTTables.data(), tamTeste, chavesRodada);
        cifrarBufferOpenSSL(ctx, chave, entrada.data(), outOpenSSL.data(), tamTeste);

        if (std::memcmp(outPadrao.data(), outOpenSSL.data(), tamTeste) != 0 ||
            std::memcmp(outTTables.data(), outOpenSSL.data(), tamTeste) != 0) {
            std::cerr << "ERRO: divergencia entre saidas do AES local e do OpenSSL!\n";
            EVP_CIPHER_CTX_free(ctx);
            return 1;
        }
        std::cout << "[OK] Validacao cruzada (Padrao == T-Tables == OpenSSL) em 4 KB.\n\n";
    }

    std::cout << "Executando benchmark (modo ECB sem padding, 1 warm-up + media de "
              << REPETICOES << " repeticoes)...\n\n";

    std::vector<ResultadoLinha> resultados;
    std::mt19937 rng(2026);

    for (std::size_t mb : tamanhosMB) {
        const std::size_t tamanhoBytes = mb * 1024ULL * 1024ULL;
        std::vector<unsigned char> entrada(tamanhoBytes);
        std::vector<unsigned char> saida(tamanhoBytes);

        // Preenche o buffer com bytes pseudoaleatorios
        for (std::size_t i = 0; i < tamanhoBytes; i += 4) {
            std::uint32_t v = rng();
            std::memcpy(entrada.data() + i, &v, 4);
        }

        // --- 1. AES Padrao ---
        cifrarBufferPadrao(entrada.data(), saida.data(), tamanhoBytes, chavesRodada);  // warm-up
        double tempoPadrao = 0.0;
        for (int r = 0; r < REPETICOES; ++r) {
            auto ini = Relogio::now();
            cifrarBufferPadrao(entrada.data(), saida.data(), tamanhoBytes, chavesRodada);
            auto fim = Relogio::now();
            tempoPadrao += std::chrono::duration<double>(fim - ini).count();
        }
        tempoPadrao /= REPETICOES;

        // --- 2. AES T-Tables ---
        cifrarBufferTTables(entrada.data(), saida.data(), tamanhoBytes, chavesRodada);  // warm-up
        double tempoTTables = 0.0;
        for (int r = 0; r < REPETICOES; ++r) {
            auto ini = Relogio::now();
            cifrarBufferTTables(entrada.data(), saida.data(), tamanhoBytes, chavesRodada);
            auto fim = Relogio::now();
            tempoTTables += std::chrono::duration<double>(fim - ini).count();
        }
        tempoTTables /= REPETICOES;

        // --- 3. OpenSSL (EVP AES-128-ECB) ---
        cifrarBufferOpenSSL(ctx, chave, entrada.data(), saida.data(), tamanhoBytes);  // warm-up
        double tempoOpenSSL = 0.0;
        for (int r = 0; r < REPETICOES; ++r) {
            auto ini = Relogio::now();
            cifrarBufferOpenSSL(ctx, chave, entrada.data(), saida.data(), tamanhoBytes);
            auto fim = Relogio::now();
            tempoOpenSSL += std::chrono::duration<double>(fim - ini).count();
        }
        tempoOpenSSL /= REPETICOES;

        double mbsPadrao = static_cast<double>(mb) / tempoPadrao;
        double mbsTTables = static_cast<double>(mb) / tempoTTables;
        double mbsOpenSSL = static_cast<double>(mb) / tempoOpenSSL;
        double speedup = mbsTTables / mbsPadrao;

        resultados.push_back({mb, mbsPadrao, mbsTTables, mbsOpenSSL, speedup});

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Tamanho: " << std::setw(3) << mb << " MB | "
                  << "Padrao: " << std::setw(8) << mbsPadrao << " MB/s (" << tempoPadrao * 1000.0 << " ms) | "
                  << "T-Tables: " << std::setw(8) << mbsTTables << " MB/s (" << tempoTTables * 1000.0 << " ms) | "
                  << "OpenSSL: " << std::setw(9) << mbsOpenSSL << " MB/s (" << tempoOpenSSL * 1000.0 << " ms) | "
                  << "Speedup T-Tables: " << speedup << "x\n";
    }

    EVP_CIPHER_CTX_free(ctx);

    // -------------------------------------------------------------------------
    // Imprime tabela pronta em Markdown para o README.md
    // -------------------------------------------------------------------------
    std::cout << "\n=== Tabela Markdown (pronta para o README.md) ===\n\n";
    std::cout << "| Tamanho | Padrão (MB/s) | T-Tables (MB/s) | OpenSSL (MB/s) | Speedup T-Tables |\n";
    std::cout << "|---|---|---|---|---|\n";
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& r : resultados) {
        std::cout << "| " << r.tamanhoMB << " MB | "
                  << r.padraoMBs << " | "
                  << r.ttablesMBs << " | "
                  << r.opensslMBs << " | "
                  << r.speedupTTables << "x |\n";
    }

    // -------------------------------------------------------------------------
    // Exporta CSV para gerar os graficos dos slides
    // -------------------------------------------------------------------------
    std::ofstream csv("resultados_benchmark.csv");
    if (csv.is_open()) {
        csv << "Tamanho_MB,Padrao_MBs,TTables_MBs,OpenSSL_MBs,Speedup_TTables\n";
        csv << std::fixed << std::setprecision(4);
        for (const auto& r : resultados) {
            csv << r.tamanhoMB << ","
                << r.padraoMBs << ","
                << r.ttablesMBs << ","
                << r.opensslMBs << ","
                << r.speedupTTables << "\n";
        }
        csv.close();
        std::cout << "\nArquivo 'resultados_benchmark.csv' gerado com sucesso.\n";
    } else {
        std::cerr << "\nAviso: nao foi possivel salvar 'resultados_benchmark.csv'.\n";
    }

    return 0;
}