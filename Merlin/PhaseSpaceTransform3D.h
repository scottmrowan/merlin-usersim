/*
 * Merlin++: C++ Class Library for Charged Particle Accelerator Simulations
 * Copyright (c) 2001-2018 The Merlin++ developers
 * This file is covered by the terms the GNU GPL version 2, or (at your option) any later version, see the file COPYING
 * This file is derived from software bearing the copyright notice in merlin4_copyright.txt
 */

#ifndef PSvectorTransform3D_h
#define PSvectorTransform3D_h 1

#include "merlin_config.h"
#include "Transform3D.h"
#include "PhaseSpaceHeaders.h"

/**
*	Utility class for performing an arbitrary 3D coordinate
*	transformation on PSvector objects. The transformation
*	assumes that the particle(s) are fully relativistic.
*	Since small angle approximations are assumed, large
*	rotations about the x and y axis may lead to significant
*	errors.
*/

class PhaseSpaceTransform3D
{
public:

	PhaseSpaceTransform3D (const Transform3D& tfrm);

	Particle& Apply (Particle& p) const;
	ParticleArray& Apply (ParticleArray& pv) const;
	Particle& operator () (Particle& p) const;

private:

	Transform3D T;
	bool bNoRot;
};

inline Particle& PhaseSpaceTransform3D::operator () (Particle& p) const
{
	return Apply(p);
}

#endif
