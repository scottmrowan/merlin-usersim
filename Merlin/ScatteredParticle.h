/*
 * Merlin++: C++ Class Library for Charged Particle Accelerator Simulations
 * Copyright (c) 2001-2018 The Merlin++ developers
 * This file is covered by the terms the GNU GPL version 2, or (at your option) any later version, see the file COPYING
 * This file is derived from software bearing the copyright notice in merlin4_copyright.txt
 */

#ifndef MERLIN_SCATTEREDPARTICLE_H_
#define MERLIN_SCATTEREDPARTICLE_H_

#include "Particle.h"

class ScatteredParticle : public Particle {
public:
	ScatteredParticle();
	virtual ~ScatteredParticle();
};

#endif /* MERLIN_SCATTEREDPARTICLE_H_ */
