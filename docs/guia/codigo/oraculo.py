import random
import sys
import sympy as sp


def generaFuncion(nVars, nMon, kMax, eMax, semilla):
    rng = random.Random(semilla)
    vNombres = [f"x{i}" for i in range(nVars)]
    vTerm = []
    for _ in range(nMon):
        coef = rng.randint(1, 9)
        vIds = rng.sample(range(nVars), rng.randint(1, kMax))
        txt = str(coef)
        for v in vIds:
            e = rng.randint(1, eMax)
            txt += vNombres[v] + (f"^{e}" if e > 1 else "")
        vTerm.append(txt)
    vTerm.append(str(rng.randint(0, 9)))
    vPunto = [rng.choice([-2, -1, 1, 2, 3]) for _ in range(nVars)]
    return vNombres, " + ".join(vTerm), vPunto


def oraculo(vNombres, txt, vPunto):
    vSim = sp.symbols(vNombres)
    tabla = dict(zip(vNombres, vSim))
    expr = 0
    for term in txt.split(" + "):
        expr += sp.sympify(term.replace("^", "**").replace("x", "*x").lstrip("*"), locals=tabla)
    sust = dict(zip(vSim, vPunto))
    return [sp.diff(expr, s).subs(sust) for s in vSim]


def main():
    nVars, nMon, semilla = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
    vNombres, txt, vPunto = generaFuncion(nVars, nMon, 4, 3, semilla)
    vGrad = oraculo(vNombres, txt, vPunto)
    with open("funcion.txt", "w") as f:
        f.write(" ".join(vNombres) + "\n" + txt + "\n" + " ".join(map(str, vPunto)) + "\n")
    with open("ref.txt", "w") as f:
        f.write(" ".join(str(g) for g in vGrad) + "\n")
    print("Escritos funcion.txt y ref.txt con", nVars, "variables y", nMon, "monomios")


if __name__ == "__main__":
    main()
