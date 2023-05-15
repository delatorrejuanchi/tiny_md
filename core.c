#include "core.h"
#include "parameters.h"

#include <math.h>
#include <stdlib.h> // rand()

#define ECUT (4.0 * (pow(RCUT, -12) - pow(RCUT, -6)))

void init_pos(double* restrict rxyz, const double rho)
{
    // inicialización de las posiciones de los átomos en un cristal FCC

    double a = cbrt(4.0 / rho);
    int nucells = ceil(cbrt((double)N / 4.0));
    int idx = 0;

    for (int i = 0; i < nucells; i++) {
        for (int j = 0; j < nucells; j++) {
            for (int k = 0; k < nucells; k++) {
                rxyz[idx + 0] = i * a; // x
                rxyz[idx + 1] = j * a; // y
                rxyz[idx + 2] = k * a; // z
                                       // del mismo átomo
                rxyz[idx + 3] = (i + 0.5) * a;
                rxyz[idx + 4] = (j + 0.5) * a;
                rxyz[idx + 5] = k * a;

                rxyz[idx + 6] = (i + 0.5) * a;
                rxyz[idx + 7] = j * a;
                rxyz[idx + 8] = (k + 0.5) * a;

                rxyz[idx + 9] = i * a;
                rxyz[idx + 10] = (j + 0.5) * a;
                rxyz[idx + 11] = (k + 0.5) * a;

                idx += 12;
            }
        }
    }
}


void init_vel(double* restrict vxyz, double* restrict temp, double* restrict ekin)
{
    // inicialización de velocidades aleatorias

    double sf, sumv2 = 0.0;
    double sumv[3] = { 0.0, 0.0, 0.0 };

    for (int i = 0; i < 3 * N; i += 3) {
        for (int j = 0; j < 3; j++) {
            vxyz[i + j] = rand() / (double)RAND_MAX - 0.5;
            sumv[j] += vxyz[i + j];
            sumv2 += vxyz[i + j] * vxyz[i + j];
        }
    }

    for (int j = 0; j < 3; j++)
        sumv[j] /= (double)N;

    *temp = sumv2 / (3.0 * N);
    *ekin = 0.5 * sumv2;
    sf = sqrt(T0 / *temp);

    for (int i = 0; i < 3 * N; i += 3) { // elimina la velocidad del centro de masa
        for (int j = 0; j < 3; j++) { // y ajusta la temperatura
            vxyz[i + j] = sf * (vxyz[i + j] - sumv[j]);
        }
    }
}


static double minimum_image(double cordi, const double cell_length, const double cell_length_r)
{
    return cordi - cell_length * round(cordi * cell_length_r);
}


void forces(const double* restrict rxyz, double* restrict fxyz, double* restrict epot, double* restrict pres,
            const double* restrict temp, const double rho, const double V, const double L)
{
    // calcula las fuerzas LJ (12-6)

    // gets optimized to __builtin_memset
    for (int i = 0; i < 3 * N; i++) {
        fxyz[i] = 0.0;
    }

    double rcut2 = RCUT * RCUT;
    const double L_r = 1.0 / L;

    double rij2;
    double ri[3], rj[3], rij[3];
    double _epot = 0.0;
    double pres_vir = 0.0;

    for (int i = 0; i < 3 * (N - 1); i += 3) {

        for (int k = 0; k < 3; k++)
            ri[k] = rxyz[i + k];

        for (int j = i + 3; j < 3 * N; j += 3) {

            for (int k = 0; k < 3; k++)
                rj[k] = rxyz[j + k];

            // distancia mínima entre r_i y r_j
            for (int k = 0; k < 3; k++)
                rij[k] = ri[k] - rj[k];

            for (int k = 0; k < 3; k++)
                rij[k] = minimum_image(rij[k], L, L_r);

            rij2 = 0.0;
            for (int k = 0; k < 3; k++)
                rij2 += rij[k] * rij[k];

            if (rij2 <= rcut2) {
                double r2inv = 1.0 / rij2;
                double r6inv = r2inv * r2inv * r2inv;

                double fr = 24.0 * r2inv * r6inv * (2.0 * r6inv - 1.0);

                for (int k = 0; k < 3; k++) {
                    fxyz[i + k] += rij[k] * fr;
                    fxyz[j + k] -= rij[k] * fr;
                }

                _epot += 4.0 * r6inv * (r6inv - 1.0) - ECUT;
                pres_vir += fr * rij2;
            }
        }
    }

    *epot = _epot;
    pres_vir /= (3.0 * V);
    *pres = *temp * rho + pres_vir;
}

static double pbc(double cordi, const double cell_length, const double cell_length_r)
{
    return cordi - cell_length * floor(cordi * cell_length_r);
}


void velocity_verlet(double* restrict rxyz, double* restrict vxyz, double* restrict fxyz, double* restrict epot,
                     double* restrict ekin, double* restrict pres, double* restrict temp, const double rho,
                     const double V, const double L)
{
    const double L_r = 1.0 / L;
    const double DT_2 = 0.5 * DT;

    for (int i = 0; i < 3 * N; i++) {
        vxyz[i] += fxyz[i] * DT_2;
    }

    for (int i = 0; i < 3 * N; i++) {
        rxyz[i] += vxyz[i] * DT;
    }

    for (int i = 0; i < 3 * N; i++) {
        rxyz[i] = pbc(rxyz[i], L, L_r);
    }

    forces(rxyz, fxyz, epot, pres, temp, rho, V, L); // actualizo fuerzas

    double sumv2 = 0.0;
    for (int i = 0; i < 3 * N; i++) { // actualizo velocidades
        vxyz[i] += 0.5 * fxyz[i] * DT;
        sumv2 += vxyz[i] * vxyz[i];
    }

    *ekin = 0.5 * sumv2;
    *temp = sumv2 / (3.0 * N);
}
