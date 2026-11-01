#![allow(non_snake_case)]

struct Poli {
    nVars: usize,
    vCoef: Vec<f64>,
    vOff: Vec<usize>,
    vIdVar: Vec<usize>,
    vExp: Vec<i32>,
}

fn gradSec(p: &Poli, vX: &[f64]) -> Vec<f64> {
    let mut vGrad = vec![0.0; p.nVars];
    let mut vPre = Vec::new();
    for m in 0..p.vCoef.len() {
        let (ini, fin) = (p.vOff[m], p.vOff[m + 1]);
        vPre.clear();
        let mut acum = p.vCoef[m];
        for t in ini..fin {
            let x = vX[p.vIdVar[t]];
            let base = x.powi(p.vExp[t] - 1);
            vPre.push(acum * p.vExp[t] as f64 * base);
            acum *= base * x;
        }
        let mut suf = 1.0;
        for t in (ini..fin).rev() {
            vGrad[p.vIdVar[t]] += vPre[t - ini] * suf;
            suf *= vX[p.vIdVar[t]].powi(p.vExp[t]);
        }
    }
    vGrad
}

fn main() {
    let p = Poli {
        nVars: 3,
        vCoef: vec![3.0, 4.0, 2.0, 5.0],
        vOff: vec![0, 2, 4, 5, 5],
        vIdVar: vec![0, 1, 1, 2, 0],
        vExp: vec![1, 1, 1, 1, 1],
    };
    let vGrad = gradSec(&p, &[1.0, 2.0, 3.0]);
    println!("grad = {:?}", vGrad);
}
