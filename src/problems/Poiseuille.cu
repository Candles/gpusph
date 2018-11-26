#include "Poiseuille.h"

#include <iostream>

#include "cudasimframework.cu"

Poiseuille::Poiseuille(GlobalData *_gdata) :
	XProblem(_gdata),

	lz(1.0),
	ly(lz),
	lx(lz),

	rho(get_option("density", 1.0f)),
	kinvisc(get_option("kinvisc", 0.1f)),
	driving_force(get_option("driving-force", 0.05f)),

	max_vel(compute_poiseuille_vel(0)),
	Re(lz*max_vel/kinvisc)
{
	// *** user parameters from command line

	// particles per depth: allow specification of the particle size in terms of
	// the number of particles in z direction
	const int ppH = get_option("ppH", 32);

	// density diffusion terms: 0 none, 1 Ferrari, 2 Molteni & Colagrossi, 3 Brezzi
	const int RHODIFF = get_option("density-diffusion", 0);

	// Allow user to set the MLS frequency at runtime. Default to 0 (no MLS).
	const int mlsIters = get_option("mls", 0);

	SETUP_FRAMEWORK(
		kernel<WENDLAND>,
		viscosity<DYNAMICVISC>,
		periodicity<PERIODIC_XY>,
		boundary<DYN_BOUNDARY>
	).select_options(
		RHODIFF == FERRARI, densitydiffusion<FERRARI>(),
		RHODIFF == BREZZI, densitydiffusion<BREZZI>(),
		RHODIFF == COLAGROSSI, densitydiffusion<COLAGROSSI>()
	);

	if (mlsIters > 0)
		addFilter(MLS_FILTER, mlsIters);

	// SPH parameters

	set_deltap(lz/ppH);
	simparams()->tend = 100;

	// Physical parameters
	physparams()->gravity = make_float3(driving_force, 0.0, 0.0);

	auto fluid_idx = add_fluid(rho);
	set_kinematic_visc(fluid_idx, kinvisc);

	printf("Reynolds number = %g\n", Re);
	printf("Max flow velocity: %g m/s\n", max_vel);

	const double hydrostatic_vel = sqrt(2*driving_force*lz);
	printf("Hydrostatic velocity : %g m/s\n", hydrostatic_vel);

	const double c0 = 20*fmax(hydrostatic_vel, max_vel);

	set_equation_of_state(0,  7.0f, c0);

	// Geometry--------------------------------------------------
	setPositioning(PP_CENTER);
	GeometryID bottom = addRect(GT_FIXED_BOUNDARY, FT_BORDER,
		Point(0,0,-(lz/2)), lx - m_deltap, ly - m_deltap);
	GeometryID fluid = addBox(GT_FLUID, FT_SOLID, Point(0,0,0),
		lx - m_deltap, ly - m_deltap, lz- 2*m_deltap );

	setPositioning(PP_CORNER);
	GeometryID top = addRect(GT_FIXED_BOUNDARY, FT_BORDER,
		Point((lx - m_deltap)/2, -(ly - m_deltap)/2, (lz/2)), lx - m_deltap, ly - m_deltap);

	setEraseOperation(top, ET_ERASE_NOTHING);
	rotate(top, 0, M_PI, 0);

	add_writer(VTKWRITER, 1.0);

	// Name of problem used for directory creation
	m_name = "Poiseuille";
}

void Poiseuille::initializeParticles(BufferList &buffer, const uint numParticle)
{
	const bool init_vel = get_option("steady-init", false);

	if (!init_vel)
		return;

	double4 *pos = buffer.getData<BUFFER_POS_GLOBAL>();
	float4 *vel = buffer.getData<BUFFER_VEL>();
	const ushort4 *pinfo = buffer.getData<BUFFER_INFO>();

	for (uint i = 0 ; i < numParticle ; i++) {
		if (FLUID(pinfo[i]))
			vel[i].x = compute_poiseuille_vel(pos[i].z);
	}
}

float Poiseuille::compute_poiseuille_vel(float pos_z)
{
	float A = driving_force/(2*kinvisc);
	float B = (lz/2)*(lz/2);
	float vel = A*(B - pos_z*pos_z);
	return vel;
}
