import numpy as np
import polars as pl

CONFIG = {
    "monomios": 1_000,
    "dims": 1_000,
    "eMax": 100,
    "cMax": 100,
    "coefMax": 10,
    "bloque": 100_000,
    "semilla": 0,
    "fMonomios": "monomios.csv",
    "fFactores": "factores.csv",
}

LONG_NOMBRE = 5
ALFABETO = np.frombuffer(
    b"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
    dtype=np.uint8,
)


def numAntoniobeto(vIds):
    mCaracteres = np.empty((vIds.size, LONG_NOMBRE), dtype=np.uint8)
    resto = vIds.copy()
    for k in range(LONG_NOMBRE - 1, -1, -1):
        mCaracteres[:, k] = ALFABETO[resto % 62]
        resto //= 62
    return mCaracteres.view(f"S{LONG_NOMBRE}").ravel().astype(str)


def generaBloque(rng, inicio, n):
    vCuentas = rng.integers(1, CONFIG["cMax"] + 1, size=n)
    total = int(vCuentas.sum())
    vIdMono = np.repeat(np.arange(inicio, inicio + n, dtype=np.int64), vCuentas)
    vVars = rng.integers(0, CONFIG["dims"], size=total, dtype=np.int64)
    vExps = rng.integers(1, CONFIG["eMax"] + 1, size=total)

    dfFactores = (
        pl.DataFrame({"monomio": vIdMono, "idVar": vVars, "exp": vExps})
        .unique(subset=["monomio", "idVar"], keep="first", maintain_order=True)
    )
    vNombres = numAntoniobeto(dfFactores["idVar"].to_numpy())
    dfFactores = dfFactores.with_columns(pl.Series("variable", vNombres)).select(
        ["monomio", "variable", "exp"]
    )

    dfMonomios = pl.DataFrame(
        {
            "monomio": np.arange(inicio, inicio + n, dtype=np.int64),
            "coef": rng.integers(1, CONFIG["coefMax"] + 1, size=n),
        }
    )
    return dfMonomios, dfFactores


def main():
    rng = np.random.default_rng(CONFIG["semilla"])
    nMonomios = CONFIG["monomios"]
    bloque = CONFIG["bloque"]

    with open(CONFIG["fMonomios"], "wb") as fMono, open(
        CONFIG["fFactores"], "wb"
    ) as fFact:
        for inicio in range(0, nMonomios, bloque):
            n = min(bloque, nMonomios - inicio)
            dfMonomios, dfFactores = generaBloque(rng, inicio, n)
            dfMonomios.write_csv(fMono, include_header=inicio == 0)
            dfFactores.write_csv(fFact, include_header=inicio == 0)

    print("Generados", nMonomios, "monomios")


if __name__ == "__main__":
    main()
