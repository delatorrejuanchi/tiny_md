#ifndef CORE_H
#define CORE_H

extern int N; // número de particulas (debe ser un 4m^3 para el cristal inicial)

void init_pos(double* rxyz, const double rho);
void init_vel(double* vxyz, double* temp, double* ekin);
void forces(const double* rxyz, double* fxyz, double* epot, double* pres,
            const double* temp, const double rho, const double V, const double L);
void velocity_verlet(double* rxyz, double* vxyz, double* fxyz, double* epot,
                     double* ekin, double* pres, double* temp, const double rho,
                     const double V, const double L);

#endif
