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


void init_vel(float* restrict vxyz, float* restrict temp, float* restrict ekin)
{
    float sumv2 = 0.0;
    float sumx = 0.0, sumy = 0.0, sumz = 0.0;

    for (int i = 0; i < 3 * N; i += 3) {
        vxyz[i + 0] = rand() / (float)RAND_MAX - 0.5;
        vxyz[i + 1] = rand() / (float)RAND_MAX - 0.5;
        vxyz[i + 2] = rand() / (float)RAND_MAX - 0.5;

        sumx += vxyz[i + 0];
        sumy += vxyz[i + 1];
        sumz += vxyz[i + 2];

        sumv2 += vxyz[i + 0] * vxyz[i + 0] + vxyz[i + 1] * vxyz[i + 1] + vxyz[i + 2] * vxyz[i + 2];
    }

    sumx /= (float)N;
    sumy /= (float)N;
    sumz /= (float)N;

    float sf = sqrtf(T0 / *temp);
    for (int i = 0; i < 3 * N; i += 3) {
        vxyz[i + 0] = sf * (vxyz[i + 0] - sumx);
        vxyz[i + 1] = sf * (vxyz[i + 1] - sumy);
        vxyz[i + 2] = sf * (vxyz[i + 2] - sumz);
    }

    *temp = sumv2 / (3.0 * N);
    *ekin = 0.5 * sumv2;
}


static float minimum_image(float cordi, const float cell_length, const float cell_length_r)
{
    return cordi - cell_length * roundf(cordi * cell_length_r);
}


void forces(const float* restrict rxyz, float* restrict fxyz, float* restrict private_fxyz, float* restrict epot, float* restrict pres,
            const float* restrict temp, const float rho, const float V, const float L)
{
    for (int i = 0; i < 3 * N; i++) {
        fxyz[i] = 0.0;
    }

    for (int i = 0; i < N_THREADS * 3 * N; i++) {
        private_fxyz[i] = 0.0;
    }

    const float L_r = 1.0 / L;
    float ri[3 * N];
    float _epot = 0.0;
    float pres_vir = 0.0;

    #pragma omp parallel for reduction(+ : _epot, pres_vir) private(ri)
    for (int i = 0; i < 3 * (N - 1); i += 3) {
        int thread_id = omp_get_thread_num();
        float* restrict thread_fxyz = private_fxyz + thread_id * 3 * N;

        for (int j = i + 3; j < 3 * N; j += 3) {
            ri[j + 0] = minimum_image(rxyz[i + 0] - rxyz[j + 0], L, L_r);
            ri[j + 1] = minimum_image(rxyz[i + 1] - rxyz[j + 1], L, L_r);
            ri[j + 2] = minimum_image(rxyz[i + 2] - rxyz[j + 2], L, L_r);
        }

        for (int j = i + 3; j < 3 * N; j += 3) {
            float rij2 = ri[j] * ri[j] + ri[j + 1] * ri[j + 1] + ri[j + 2] * ri[j + 2];

            if (rij2 <= RCUT2) {
                float r2inv = 1.0 / rij2;
                float r6inv = r2inv * r2inv * r2inv;

                float fr = 24.0 * r2inv * r6inv * (2.0 * r6inv - 1.0);

                thread_fxyz[i + 0] += fr * ri[j + 0];
                thread_fxyz[i + 1] += fr * ri[j + 1];
                thread_fxyz[i + 2] += fr * ri[j + 2];

                thread_fxyz[j + 0] -= fr * ri[j + 0];
                thread_fxyz[j + 1] -= fr * ri[j + 1];
                thread_fxyz[j + 2] -= fr * ri[j + 2];

                _epot += (4.0 * r6inv * (r6inv - 1.0) - ECUT);
                pres_vir += fr * rij2;
            }
        }
    }

    for (int t = 0; t < N_THREADS; t++) {
        float* thread_fxyz = private_fxyz + t * 3 * N;
        for (int i = 0; i < 3 * N; i++) {
            fxyz[i] += thread_fxyz[i];
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


void velocity_verlet(float* restrict rxyz, float* restrict vxyz, float* restrict fxyz, float* restrict private_fxyz, float* restrict epot,
                     float* restrict ekin, float* restrict pres, float* restrict temp, const float rho,
                     const float V, const float L)
{
    const float L_r = 1.0 / L;
    const float DT_2 = 0.5 * DT;

    for (int i = 0; i < 3 * N; i++) {
        vxyz[i] += fxyz[i] * DT_2;
        rxyz[i] += vxyz[i] * DT;
        rxyz[i] = pbc(rxyz[i], L, L_r);
    }

    forces(rxyz, fxyz, private_fxyz, epot, pres, temp, rho, V, L);

    float sumv2 = 0.0;
    for (int i = 0; i < 3 * N; i++) {
        vxyz[i] += 0.5 * fxyz[i] * DT;
        sumv2 += vxyz[i] * vxyz[i];
    }

    *ekin = 0.5 * sumv2;
    *temp = sumv2 / (3.0 * N);
}
