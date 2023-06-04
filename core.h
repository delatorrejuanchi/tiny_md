#ifndef CORE_H
#define CORE_H

void init_pos(float* restrict rxyz, const float rho);
void init_vel(float* restrict vxyz, float* restrict temp, float* restrict ekin);
void forces(const float* restrict rxyz, float* restrict fxyz, float* restrict private_fxyz, float* restrict epot, float* restrict pres,
            const float* restrict temp, const float rho, const float V, const float L);
void velocity_verlet(float* restrict rxyz, float* restrict vxyz, float* restrict fxyz, float* restrict private_fxyz, float* restrict epot,
                     float* restrict ekin, float* restrict pres, float* restrict temp, const float rho,
                     const float V, const float L);

#endif
