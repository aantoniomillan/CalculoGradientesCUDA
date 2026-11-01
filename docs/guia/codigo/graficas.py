import sys
import polars as pl
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def main():
    df = pl.read_csv(sys.argv[1] if len(sys.argv) > 1 else "tiempos.csv")
    dfMed = df.group_by(["variante", "hilos"]).agg(
        pl.col("tiempo").median().alias("mediana"), pl.col("tiempo").std().alias("desv")
    )
    tSec = dfMed.filter((pl.col("variante") == "secuencial") & (pl.col("hilos") == 1))["mediana"][0]
    dfMed = dfMed.with_columns(
        (tSec / pl.col("mediana")).alias("speedup"),
        (tSec / pl.col("mediana") / pl.col("hilos")).alias("eficiencia"),
    ).sort(["variante", "hilos"])
    print(dfMed)
    fig, ax = plt.subplots(figsize=(6, 4))
    for variante in dfMed["variante"].unique().sort():
        dfV = dfMed.filter(pl.col("variante") == variante)
        ax.plot(dfV["hilos"], dfV["speedup"], marker="o", label=variante)
    ax.set_xlabel("Hilos")
    ax.set_ylabel("Speedup frente a secuencial")
    ax.legend()
    fig.tight_layout()
    fig.savefig("speedup.png", dpi=150)
    print("Escrito speedup.png")


if __name__ == "__main__":
    main()
