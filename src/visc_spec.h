/*  Copyright 2018 Giuseppe Bilotta, Alexis Hérault, Robert A.
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
 * Viscosity-related definitions and auxiliary functions
 */

#ifndef _VISC_SPEC_H
#define _VISC_SPEC_H

#include "simflags.h"

#include "average.h"

//! Rheology of the fluid(s)
/*! For the time being, we only support NEWTONIAN, but
 * this will be extended to include things such as temperature dependency
 * and generalized Newtonian rheologies.
 */
/** @defpsubsection{rheology, RHEOLOGY}
 * @inpsection{physics}
 * @default{Newtonian}
 * @values{Inviscid, Newtonian}
 * TLT_RHEOLOGY
 */
enum RheologyType {
	INVISCID, ///< No (laminar) viscosity
	NEWTONIAN, ///< Viscosity independent of strain rate
};

//! Name of the rheology type
#ifndef GPUSPH_MAIN
extern
#endif
const char* RheologyTypeName[NEWTONIAN+1]
#ifdef GPUSPH_MAIN
= {
	"Inviscid",
	"Newtonian",
}
#endif
;

//! Turbulence model
/** @defpsubsection{turbulence, TURBULENCE}
 * @inpsection{physics}
 * @default{disable}
 * @values{disable, artificial viscosity, k-epsilon, SPS model (LES)}
 * TLT_TURBULENCE
 */
/*!
 * While strictly speaking not a turbulence model, artificial viscosity is considered
 * among the turbulence models, since its behavior can be assimilated to it (i.e.
 * an addition to the viscous model, rather than an alternative to it), even though
 * it's normally only used for inviscid flow.
 */
enum TurbulenceModel {
	LAMINAR_FLOW, ///< No turbulence
	ARTIFICIAL, ///< Artificial viscosity
	SPS, ///< Sub-particle scale turbulence model
	KEPSILON, ///< k-epsilon turbulence model
	INVALID_TURBULENCE
};

//! Name of the turbulence model model
#ifndef GPUSPH_MAIN
extern
#endif
const char* TurbulenceName[INVALID_TURBULENCE+1]
#ifdef GPUSPH_MAIN
= {
	"Pure laminar flow",
	"Artificial viscosity",
	"Sub-particle scale",
	"k-epsilon",
	"(invalid)"
}
#endif
;

//! Kind of viscosity used within the simulation
/*! This can be either KINEMATIC or DYNAMIC, depending on whether
 * the preference is to work in terms of the kinematic viscosity ν,
 * or in terms of the dynamic viscosity µ = ρν
 */
/** @defpsubsection{viscosityType, VISCOSITY_TYPE}
 * @inpsection{physics}
 * @default{kinematic}
 * @values{kinematic, dynamic}
 * TLT_VISCOSITY_TYPE
 */
enum ComputationalViscosityType {
	KINEMATIC, ///< Kinematic viscosity (SI units: m²/s)
	DYNAMIC, ///< Dynamic viscosity (SI units: Pa s)
};

//! Name of the viscous model
#ifndef GPUSPH_MAIN
extern
#endif
const char* ComputationalViscosityName[DYNAMIC+1]
#ifdef GPUSPH_MAIN
= {
	"Kinematic",
	"Dynamic",
}
#endif
;

//! Supported viscous models
/*! Currently only MORRIS is available, with plans to add Monaghan's and
 * Español & Revenga too
 */
/** @defpsubsection{viscousModel, VISCOUS_MODEL}
 * @inpsection{physics}
 * @default{Morris}
 * @values{Morris}
 * TLT_VISCOUS_MODEL
 */
enum ViscousModel {
	MORRIS, ///< Morris et al., JCP 1997
};
//! Name of the viscous model
#ifndef GPUSPH_MAIN
extern
#endif
const char* ViscousModelName[MORRIS+1]
#ifdef GPUSPH_MAIN
= {
	"Morris 1997",
}
#endif
;



//! A complete viscous specification includes:
// * a rheological model
// * a turbulence model
// * a computational viscosity specification
// * a viscous model (discretization approach to the viscous operator)
// * an averaging operator
// * knowledge about the presence of multiple fluids
// TODO use the TypeValue and Multiplexer from CUDASimFramework
template<
	RheologyType _rheologytype = NEWTONIAN,
	TurbulenceModel _turbmodel = LAMINAR_FLOW,
	ComputationalViscosityType _compvisc = KINEMATIC,
	ViscousModel _viscmodel = MORRIS,
	AverageOperator _avgop = ARITHMETIC,
	flag_t _simflags = ENABLE_NONE,
	// is this a constant-viscosity formulation?
	bool _is_const_visc = (
		IS_SINGLEFLUID(_simflags) &&
		(_rheologytype == NEWTONIAN) &&
		(_turbmodel != KEPSILON)
	)
>
struct FullViscSpec {
	static constexpr RheologyType rheologytype = _rheologytype;
	static constexpr TurbulenceModel turbmodel = _turbmodel;
	static constexpr ComputationalViscosityType compvisc = _compvisc;
	static constexpr ViscousModel viscmodel = _viscmodel;
	static constexpr AverageOperator avgop = _avgop;
	static constexpr flag_t simflags = _simflags;

	static constexpr bool is_const_visc = _is_const_visc;

	//! Change the turbulence model
	template<TurbulenceModel newturb>
	using with_turbmodel =
		FullViscSpec<rheologytype, newturb, compvisc, viscmodel, avgop, simflags>;

	//! Change the computational viscosity type specification
	template<ComputationalViscosityType altcompvisc>
	using with_computational_visc =
		FullViscSpec<rheologytype, turbmodel, altcompvisc, viscmodel, avgop, simflags>;

	//! Change the averaging operator
	template<AverageOperator altavgop>
	using with_avg_operator =
		FullViscSpec<rheologytype, turbmodel, compvisc, viscmodel, altavgop, simflags>;

	//! Force the assumption about constant viscosity
	/*! Sometimes we need to refer to the same viscous specification, but ignoring
	 * (or forcing) the assumption that the viscosity is constant;
	 * this type alias can be used to that effect
	 */
	template<bool is_const_visc>
	using assume_const_visc =
		FullViscSpec<rheologytype, turbmodel, compvisc, viscmodel, avgop, simflags, is_const_visc>;
};

//! Legacy viscosity type
enum LegacyViscosityType {
	ARTVISC = 1,
	KINEMATICVISC, ///< Morris formula, simplified for constant kinematic viscosity and using harmonic averaging of the density
	DYNAMICVISC, ///< Morris formula, with arithmetic averaging of the dynamic viscosity
	SPSVISC, ///< KINEMATICVISC + SPS
	KEPSVISC, ///< DYNAMICVISC + SPS
	INVALID_VISCOSITY
} ;

//! Name of the viscous model
#ifndef GPUSPH_MAIN
extern
#endif
const char* LegacyViscosityName[INVALID_VISCOSITY+1]
#ifdef GPUSPH_MAIN
= {
	"(null)",
	"Artificial",
	"Kinematic",
	"Dynamic",
	"SPS + kinematic",
	"k-e model",
	"(invalid)"
}
#endif
;

//! Convert a LegacyViscosityType to a FullViscSpec
/*! A template structure with a typedef 'type' to the corresponding FullViscSpec
 */
template<LegacyViscosityType>
struct ConvertLegacyVisc;

template<>
struct ConvertLegacyVisc<ARTVISC>
{
	/* Inviscid flow with artificial viscosity */
	using type = FullViscSpec<INVISCID, ARTIFICIAL>;
};

template<>
struct ConvertLegacyVisc<KINEMATICVISC>
{
	/* The default, except for the use of harmonic average and assumption of constant
	 * viscosity kinematic viscosity */
	using type = typename FullViscSpec<>::with_avg_operator<HARMONIC>::assume_const_visc<true>;
};

template<>
struct ConvertLegacyVisc<DYNAMICVISC>
{
	/* The default: Morris model with arithmetic mean for a laminar newtonian flow */
	using type = FullViscSpec<>; /* the default! */
};

template<>
struct ConvertLegacyVisc<SPSVISC>
{
	/* KINEMATICVISC + SPS */
	using type = typename ConvertLegacyVisc<KINEMATICVISC>::type::with_turbmodel<SPS>;
};

template<>
struct ConvertLegacyVisc<KEPSVISC>
{
	/* DYNAMICVISC + KEPSILON */
	using type = typename ConvertLegacyVisc<DYNAMICVISC>::type::with_turbmodel<KEPSILON>;
};

#endif
