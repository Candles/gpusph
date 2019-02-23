/*  Copyright 2013 Alexis Herault, Giuseppe Bilotta, Robert A.
 	Dalrymple, Eugenio Rustico, Ciro Del Negro

	Conservatoire National des Arts et Metiers, Paris, France

	Istituto Nazionale di Geofisica e Vulcanologia,
    Sezione di Catania, Catania, Italy

    Universita di Catania, Catania, Italy

    Johns Hopkins University, Baltimore, MD

	This file is part of GPUSPH.

    GPUSPH is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUSPH is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUSPH.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \file
 * Particle system-related definitions
 *
 *
 * TODO split non-particle stuff into different header(s)
 */

#ifndef _PARTICLEDEFINE_H
#define	_PARTICLEDEFINE_H

#include "vector_math.h"
#include "cuda_call.h"

#include "common_types.h"

#include "visc_spec.h"

//! Smoothing kernels
enum KernelType {
	CUBICSPLINE = 1,
	QUADRATIC,
	WENDLAND,
	GAUSSIAN,
	INVALID_KERNEL
} ;

//! Names of the smoothing kernels
#ifndef GPUSPH_MAIN
extern
#endif
const char* KernelName[INVALID_KERNEL+1]
#ifdef GPUSPH_MAIN
= {
	"(null)",
	"Cubic spline",
	"Quadratic",
	"Wendland",
	"Gaussian",
	"(invalid)"
}
#endif
;

DEFINE_OPTION_RANGE(KernelType, KernelName, CUBICSPLINE, GAUSSIAN);

//! SPH formulations
enum SPHFormulation {
	SPH_F1 = 1,
	SPH_F2,
	SPH_GRENIER,
	SPH_INVALID
} ;

//! Names of the SPH formulations
#ifndef GPUSPH_MAIN
extern
#endif
const char* SPHFormulationName[SPH_INVALID+1]
#ifdef GPUSPH_MAIN
= {
	"(null)",
	"F1",
	"F2",
	"Grenier",
	"(invalid)"
}
#endif
;

DEFINE_OPTION_RANGE(SPHFormulation, SPHFormulationName, SPH_F1, SPH_GRENIER);

//! Density diffusion models
enum DensityDiffusionType {
	DENSITY_DIFFUSION_NONE,
	FERRARI,
	COLAGROSSI,
	BREZZI,
	INVALID_DENSITY_DIFFUSION
} ;

//! Name of the density diffusion models
#ifndef GPUSPH_MAIN
extern
#endif
const char* DensityDiffusionName[INVALID_DENSITY_DIFFUSION+1]
#ifdef GPUSPH_MAIN
= {
	"none",
	"Ferrari",
	"Colagrossi",
	"Brezzi",
	"(invalid)"
}
#endif
;

DEFINE_OPTION_RANGE(DensityDiffusionType, DensityDiffusionName, DENSITY_DIFFUSION_NONE, BREZZI);


//! Physical boundary models
enum BoundaryType {
	LJ_BOUNDARY,
	MK_BOUNDARY,
	SA_BOUNDARY,
	DYN_BOUNDARY,
	INVALID_BOUNDARY
};

//! Names of the physical boundary models
#ifndef GPUSPH_MAIN
extern
#endif
const char* BoundaryName[INVALID_BOUNDARY+1]
#ifdef GPUSPH_MAIN
= {
	"Lennard-Jones",
	"Monaghan-Kajtar",
	"Semi Analytical.",
	"Dynamic",
	"(invalid)"
}
#endif
;

DEFINE_OPTION_RANGE(BoundaryType, BoundaryName, LJ_BOUNDARY, DYN_BOUNDARY);

//! Epsilon for MLS determinant
#define EPSDETMLS				0.05f
//! Mininum number of neighbors for MLS correction
#define MINCORRNEIBSMLS			4

//! Boundary periodicity
enum Periodicity {
	PERIODIC_NONE = 0,
	PERIODIC_X   = 1,
	PERIODIC_Y   = PERIODIC_X << 1,
	PERIODIC_XY  = PERIODIC_X | PERIODIC_Y,
	PERIODIC_Z   = PERIODIC_Y << 1,
	PERIODIC_XZ  = PERIODIC_X | PERIODIC_Z,
	PERIODIC_YZ  = PERIODIC_Y | PERIODIC_Z,
	PERIODIC_XYZ = PERIODIC_X | PERIODIC_Y | PERIODIC_Z,
};

//! String representation for the boundary periodicity
#ifndef GPUSPH_MAIN
extern
#endif
const char* PeriodicityName[PERIODIC_XYZ+1]
#ifdef GPUSPH_MAIN
= {
	"none",
	"X",
	"Y",
	"X and Y",
	"Z",
	"X and Z",
	"Y and Z",
	"X, Y and Z",
}
#endif
;

DEFINE_OPTION_RANGE(Periodicity, PeriodicityName, PERIODIC_NONE, PERIODIC_XYZ);

//! Density filters
//! we define FIRST and INVALID filters to make iterating over all filters easier
enum FilterType {
	FIRST_FILTER = 0,
	SHEPARD_FILTER = FIRST_FILTER,
	MLS_FILTER,
	INVALID_FILTER
};

//! Name of the density filters
#ifndef GPUSPH_MAIN
extern
#endif
const char *FilterName[INVALID_FILTER+1]
#ifdef GPUSPH_MAIN
= {
	"Shepard",
	"MLS",
	"(invalid)"
}
#endif
;

//! Post-processing engines
enum PostProcessType {
	FIRST_POSTPROC = 0,
	VORTICITY = FIRST_POSTPROC,
	TESTPOINTS,
	SURFACE_DETECTION,
	FLUX_COMPUTATION,
	CALC_PRIVATE,
	INVALID_POSTPROC
};

//! Name of the post-processing engines
#ifndef GPUSPH_MAIN
extern
#endif
const char *PostProcessName[INVALID_POSTPROC+1]
#ifdef GPUSPH_MAIN
= {
	"Vorticity",
	"Testpoints",
	"Surface detection",
	"Flux computation",
	"Private",
	"(invalid)"
}
#endif
;

//! SPS model storage flags
enum SPSKernelSimFlags {
	SPSK_STORE_TAU = 1,
	SPSK_STORE_TURBVISC = (SPSK_STORE_TAU << 1)
};

//! Maximum number of planes allowed
#define MAX_PLANES			8
//! Maximum number of fluid types allowed
#define MAX_FLUID_TYPES		4
//! Maximum number of rigid bodies allowed
#define	MAX_BODIES			16

/*! Maximum number of elements in a linear texture.
 * CUDA linear textures have a limit of 2^27 to the number of elements they can hold.
 * This effectively imposes an upper limit on the number of particles that we can use
 * per GPU, due to our use of textures for caching.
 * TODO FIXME actually use this
 */
#define MAX_CUDA_LINEAR_TEXTURE_ELEMENTS (1U << 27)

//! 24-bit integer multiplication on 1.x was faster than simple integer multiplication,
//! so wrap the multiplication in a macro.
#if (__COMPUTE__ >= 20)
	#define INTMUL(x,y) (x)*(y)
#else
	#define INTMUL(x,y) __mul24(x,y)
#endif

#endif
