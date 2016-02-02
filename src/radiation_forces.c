/**
 * @file    radiation_forces.c
 * @brief   Add radiation forces
 * @author  Dan Tamayo <tamayo.daniel@gmail.com>
 * 
 * @section     LICENSE
 * Copyright (c) 2015 Dan Tamayo, Hanno Rein
 *
 * This file is part of reboundx.
 *
 * reboundx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * reboundx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rebound.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "reboundx.h"

void rebx_radiation_forces(struct reb_simulation* const sim, struct rebx_effect* const rad){
    const struct rebx_params_radiation_forces* const params = rad->paramsPtr;
    const double c = params->c;
    const int source_index = params->source_index;
    struct reb_particle* particles = sim->particles;
    const struct reb_particle source = particles[source_index];
    const double mu = sim->G*source.m;

    const int _N_real = sim->N - sim->N_var;
#pragma omp parallel for
    for (int i=0;i<_N_real;i++){
        if(i == source_index){
            continue;
        }
        const struct reb_particle p = sim->particles[i];
        double* betaPtr = rebx_search_param(&p, RAD_BETA);
        if(betaPtr == NULL) continue; // only particles with Q_pr set feel radiation forces
        const double dx = p.x - source.x; 
        const double dy = p.y - source.y;
        const double dz = p.z - source.z;
        const double dr = sqrt(dx*dx + dy*dy + dz*dz); // distance to star
        
        const double dvx = p.vx - source.vx;
        const double dvy = p.vy - source.vy;
        const double dvz = p.vz - source.vz;
        const double rdot = (dx*dvx + dy*dvy + dz*dvz)/dr; // radial velocity
        const double a_rad = *betaPtr*mu/(dr*dr);

        // Equation (5) of Burns, Lamy & Soter (1979)

		particles[source_index].ax += a_rad*((1.-rdot/c)*dx/dr - dvx/c);
		particles[source_index].ay += a_rad*((1.-rdot/c)*dy/dr - dvy/c);
		particles[source_index].az += a_rad*((1.-rdot/c)*dz/dr - dvz/c);
	}
}
