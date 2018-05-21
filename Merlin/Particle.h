/*
 * Merlin++: C++ Class Library for Charged Particle Accelerator Simulations
 * Copyright (c) 2001-2018 The Merlin++ developers
 * This file is covered by the terms the GNU GPL version 2, or (at your option) any later version, see the file COPYING
 * This file is derived from software bearing the copyright notice in merlin4_copyright.txt
 */

#ifndef PSvector_h
#define PSvector_h 1

#include "merlin_config.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include "LinearAlgebra.h"

// Phase space coordinate indices
typedef int PhaseSpaceCoord;
#define ps_X  0
#define ps_XP 1
#define ps_Y  2
#define ps_YP 3
#define ps_CT 4
#define ps_DP 5
#define particle_id 6

#define PARTICLE_LENGTH 8

class Particle
{
public:

	Particle ()
	{}

	virtual ~Particle()
	{}

	explicit Particle (double x)
	{
		std::fill(particle,particle+PARTICLE_LENGTH,x);
	}

	//	Component accessors.
	double x ()  const
	{
		return particle[0];
	}
	double xp () const
	{
		return particle[1];
	}
	double y ()  const
	{
		return particle[2];
	}
	double yp () const
	{
		return particle[3];
	}
	double ct () const
	{
		return particle[4];
	}
	double dp () const
	{
		return particle[5];
	}
	double id () const
	{
		return particle[6];
	}
	double empty () const
	{
		return particle[7];
	}


	//	Array access.
	double operator [] (PhaseSpaceCoord coord) const
	{
		return particle[coord];
	}

	//	Component mutators.
	double& x ()
	{
		return particle[0];
	}
	double& xp ()
	{
		return particle[1];
	}
	double& y ()
	{
		return particle[2];
	}
	double& yp ()
	{
		return particle[3];
	}
	double& ct ()
	{
		return particle[4];
	}
	double& dp ()
	{
		return particle[5];
	}
	double& id ()
	{
		return particle[6];
	}
	double& empty ()
	{
		return particle[7];
	}

	/**
	*	Array access.
	*/
	double& operator [] (PhaseSpaceCoord coord)
	{
		return particle[coord];
	}

	/**
	*	Conversion to a RealVector.
	*/
	operator RealVector () const
	{
		return RealVector(particle,PARTICLE_LENGTH);
	}

	bool operator == (const Particle& new_particle) const
	{
		return memcmp(particle,new_particle.particle,PARTICLE_LENGTH*sizeof(double))==0;
	}

	bool operator != (const Particle& new_particle) const
	{
		return memcmp(particle,new_particle.particle,PARTICLE_LENGTH*sizeof(double))!=0;
	}

	/**
	*	Sets the vector to zero.
	*/
	void zero ()
	{
		std::fill(particle,particle+PARTICLE_LENGTH,0.0);
	}

	/**
	*	Arithmetic assignment
	*/
	Particle& operator += (const Particle& new_particle)
	{
		double *q=particle;
		const double *r=new_particle.particle;
		while(q!=(particle+PARTICLE_LENGTH))
		{
			*(q++) += *(r++);
		}
		return *this;
	}

	Particle& operator -= (const Particle& new_particle)
	{
		double *q=particle;
		const double *r=new_particle.particle;
		while(q!=(particle+PARTICLE_LENGTH))
		{
			*(q++) -= *(r++);
		}
		return *this;
	}

	Particle& operator *= (double x)
	{
		for(double *q = particle; q!=particle+PARTICLE_LENGTH; q++)
		{
			(*q)*=x;
		}
		return *this;
	}

	Particle& operator /= (double x)
	{
		for(double *q = particle; q!=particle+PARTICLE_LENGTH; q++)
		{
			(*q)/=x;
		}
		return *this;
	}

	/**
	* binary arithmetic operators
	*/
	Particle operator+(const Particle& rhs) const
	{
		Particle rparticle(*this);
		return rparticle+=rhs;
	}

	Particle operator-(const Particle& rhs) const
	{
		Particle rparticle(*this);
		return rparticle-=rhs;
	}

	/**
	*	Reads the vector as six floating point numbers,
	*	separated by spaces and terminated by a newline.
	*/
	friend std::ostream& operator<<(std::ostream& os, const Particle& particle);

	/**
	*	Outputs the vector in row form, space delimited with a
	*	terminating newline.
	*/
	friend std::istream& operator>>(std::istream& is, Particle& particle);

private:
	double __attribute__((aligned(16))) particle[PARTICLE_LENGTH];
};

/**
*	A linear array of particle phase space vector objects.
*/
typedef std::vector<Particle> ParticleArray;

#endif
