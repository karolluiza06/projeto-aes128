import csv
import matplotlib.pyplot as plt
import numpy as np

tamanhos = []
padrao = []
ttables = []
openssl = []
speedups = []

with open("resultados_benchmark.csv", newline="", encoding="utf-8") as f:
    reader = csv.DictReader(f)
    for row in reader:
        tamanhos.append(f"{row['Tamanho_MB']} MB")
        padrao.append(float(row["Padrao_MBs"]))
        ttables.append(float(row["TTables_MBs"]))
        openssl.append(float(row["OpenSSL_MBs"]))
        speedups.append(float(row["Speedup_TTables"]))

x = np.arange(len(tamanhos))

# --- Grafico 1: AES Padrao vs T-Tables (escala linear) ---
fig, ax = plt.subplots(figsize=(8, 4.8))
largura = 0.35
b1 = ax.bar(x - largura / 2, padrao, largura, label="AES-128 Padrão", color="#4C78A8")
b2 = ax.bar(x + largura / 2, ttables, largura, label="AES-128 T-Tables", color="#F58518")

ax.set_ylabel("Throughput (MB/s)")
ax.set_xlabel("Tamanho da Entrada")
ax.set_title("Ganho de Desempenho: AES-128 Padrão vs. T-Tables")
ax.set_xticks(x)
ax.set_xticklabels(tamanhos)
ax.legend()
ax.grid(axis="y", linestyle="--", alpha=0.5)

for barra in b1:
    h = barra.get_height()
    ax.annotate(f"{h:.1f}", xy=(barra.get_x() + barra.get_width() / 2, h),
                xytext=(0, 3), textcoords="offset points", ha="center", va="bottom", fontsize=9)

for barra, sp in zip(b2, speedups):
    h = barra.get_height()
    ax.annotate(f"{h:.1f}\n({sp:.2f}x)", xy=(barra.get_x() + barra.get_width() / 2, h),
                xytext=(0, 3), textcoords="offset points", ha="center", va="bottom", fontsize=9, fontweight="bold")

plt.tight_layout()
plt.savefig("grafico_padrao_vs_ttables.png", dpi=300)
plt.close()

# --- Grafico 2: Comparacao Tripla (escala logaritmica) ---
fig, ax = plt.subplots(figsize=(8.5, 5))
largura = 0.26
b1 = ax.bar(x - largura, padrao, largura, label="AES-128 Padrão (C++)", color="#4C78A8")
b2 = ax.bar(x, ttables, largura, label="AES-128 T-Tables (C++)", color="#F58518")
b3 = ax.bar(x + largura, openssl, largura, label="OpenSSL (EVP / AES-NI)", color="#54A24B")

ax.set_yscale("log")
ax.set_ylabel("Throughput (MB/s) — Escala Logarítmica")
ax.set_xlabel("Tamanho da Entrada")
ax.set_title("Comparação Tripla de Throughput: Padrão × T-Tables × OpenSSL")
ax.set_xticks(x)
ax.set_xticklabels(tamanhos)
ax.legend()
ax.grid(axis="y", which="both", linestyle="--", alpha=0.4)

for grupo in (b1, b2, b3):
    for barra in grupo:
        h = barra.get_height()
        ax.annotate(f"{h:.0f}" if h >= 100 else f"{h:.1f}",
                    xy=(barra.get_x() + barra.get_width() / 2, h),
                    xytext=(0, 3), textcoords="offset points", ha="center", va="bottom", fontsize=8)

plt.tight_layout()
plt.savefig("grafico_comparacao_tripla.png", dpi=300)
plt.close()

print("Graficos salvos: grafico_padrao_vs_ttables.png e grafico_comparacao_tripla.png")