#include "core.h"
#include "parameters.h"

#include <math.h>
#include <omp.h>
#include <stdlib.h> // rand()
#include <omp.h>

#define ECUT (4.0 * (powf(RCUT, -12) - powf(RCUT, -6)))

void init_pos(float* restrict rxyz, const float rho)
{
    // inicialización de las posiciones de los átomos en un cristal FCC

    float a = cbrtf(4.0 / rho);
    int nucells = ceilf(cbrtf((float)N / 4.0));
    int idx = 0;

    for (int i = 0; i < nucells; i++) {
        for (int j = 0; j < nucells; j++) {
            for (int k = 0; k < nucells; k++) {
                rxyz[idx + 0] = i * a; // x
                rxyz[N + idx + 0] = j * a; // y
                rxyz[2 * N + idx + 0] = k * a; // z
                                               // del mismo átomo
                rxyz[idx + 1] = (i + 0.5) * a;
                rxyz[N + idx + 1] = (j + 0.5) * a;
                rxyz[2 * N + idx + 1] = k * a;

                rxyz[idx + 2] = (i + 0.5) * a;
                rxyz[N + idx + 2] = j * a;
                rxyz[2 * N + idx + 2] = (k + 0.5) * a;

                rxyz[idx + 3] = i * a;
                rxyz[N + idx + 3] = (j + 0.5) * a;
                rxyz[2 * N + idx + 3] = (k + 0.5) * a;

                idx += 4;
            }
        }
    }
}


void init_vel(float* restrict vxyz, float* restrict temp, float* restrict ekin)
{
    float sf, sumv2 = 0.0;
    float sumv[3] = { 0.0, 0.0, 0.0 };

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 3; j++) {
            vxyz[j * N + i] = rand() / (float)RAND_MAX - 0.5;
            sumv[j] += vxyz[j * N + i];
            sumv2 += vxyz[j * N + i] * vxyz[j * N + i];
        }
    }

    for (int j = 0; j < 3; j++)
        sumv[j] /= (float)N;

    *temp = sumv2 / (3.0 * N);
    *ekin = 0.5 * sumv2;
    sf = sqrtf(T0 / *temp);

    for (int i = 0; i < N; i++) { // elimina la velocidad del centro de masa
        for (int j = 0; j < 3; j++) { // y ajusta la temperatura
            vxyz[j * N + i] = sf * (vxyz[j * N + i] - sumv[j]);
        }
    }
}


static float minimum_image(float cordi, const float cell_length, const float cell_length_r)
{
    return cordi - cell_length * roundf(cordi * cell_length_r);
}


void forces(const float* restrict rxyz, float* restrict fxyz, float* restrict epot, float* restrict pres,
            const float* restrict temp, const float rho, const float V, const float L)
{
    for (int i = 0; i < 3 * N; i++) {
        fxyz[i] = 0.0;
    }

    const float L_r = 1.0 / L;
    float _epot = 0.0;
    float pres_vir = 0.0;

    #pragma omp parallel reduction(+ : _epot, pres_vir, fxyz[:3 * N])
    {
        float ri[4 * N];

        #pragma omp for nowait
        for (int i = 0; i < N - 1; i++) {
            for (int j = i + 1; j < N; j++) {
                ri[j] = minimum_image(rxyz[i + 0] - rxyz[j], L, L_r);
                ri[j + N] = minimum_image(rxyz[i + N] - rxyz[j + N], L, L_r);
                ri[j + 2 * N] = minimum_image(rxyz[i + 2 * N] - rxyz[j + 2 * N], L, L_r);
                ri[j + 3 * N] = ri[j] * ri[j] + ri[j + N] * ri[j + N] + ri[j + 2 * N] * ri[j + 2 * N];
            }

            for (int j = i + 1; j < N; j++) {
                if (ri[j + 3 * N] <= RCUT2) {
                    float r2inv = 1.0 / ri[j + 3 * N];
                    float r6inv = r2inv * r2inv * r2inv;

                    float fr = 24.0 * r2inv * r6inv * (2.0 * r6inv - 1.0);

                    fxyz[i + 0] += fr * ri[j];
                    fxyz[i + N] += fr * ri[j + N];
                    fxyz[i + 2 * N] += fr * ri[j + 2 * N];

                    fxyz[j] -= fr * ri[j];
                    fxyz[j + N] -= fr * ri[j + N];
                    fxyz[j + 2 * N] -= fr * ri[j + 2 * N];

                    _epot += 4.0 * r6inv * (r6inv - 1.0) - ECUT;
                    pres_vir += fr * ri[j + 3 * N];
                }
            }
        }
    }

    *epot = _epot;
    pres_vir /= (3.0 * V);
    *pres = *temp * rho + pres_vir;
}

static float pbc(float cordi, const float cell_length, const float cell_length_r)
{
    return cordi - cell_length * floorf(cordi * cell_length_r);
}


void velocity_verlet(float* restrict rxyz, float* restrict vxyz, float* restrict fxyz, float* restrict epot,
                     float* restrict ekin, float* restrict pres, float* restrict temp, const float rho,
                     const float V, const float L)
{
    const float L_r = 1.0 / L;
    const float DT_2 = 0.5 * DT;

    for (int i = 0; i < 3 * N; i++) {
        vxyz[i] += fxyz[i] * DT_2;
    }

    for (int i = 0; i < 3 * N; i++) {
        rxyz[i] += vxyz[i] * DT;
    }

    for (int i = 0; i < 3 * N; i++) {
        rxyz[i] = pbc(rxyz[i], L, L_r);
    }

    forces(rxyz, fxyz, epot, pres, temp, rho, V, L);

    float sumv2 = 0.0;
    for (int i = 0; i < 3 * N; i++) {
        vxyz[i] += 0.5 * fxyz[i] * DT;
        sumv2 += vxyz[i] * vxyz[i];
    }

    *ekin = 0.5 * sumv2;
    *temp = sumv2 / (3.0 * N);
}
