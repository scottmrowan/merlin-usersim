//============================================================================
// Name        : UserSim.cpp
// Author      : <ENTER NAME>
// Version     : <ENTER VERSION>
// Copyright   : <COPYRIGHT>
// Description : Merlin++ User Simulation Script
//============================================================================

#include <iostream>
#include <fstream>
#include <cmath>

#include "PhysicalUnits.h"
#include "PhysicalConstants.h"
#include "NumericalConstants.h"

#include "AcceleratorModel.h"
#include "AcceleratorModelConstructor.h"
#include "AcceleratorComponent.h"
#include "ClosedOrbit.h"
#include "Drift.h"
#include "StandardMultipoles.h"
#include "SectorBend.h"
#include "LatticeFunctions.h"

#include "Particle.h"
#include "ParticleBunch.h"
#include "ParticleTracker.h"

using namespace std;
using namespace PhysicalUnits;
using namespace PhysicalConstants;
using namespace ParticleTracking;

#define circum 1000.0
#define ncell 20.0
#define lcell 50.0
#define lquad 0
#define ldipole 0
#define beamenergy 20*GeV

int main() {
	AcceleratorModelConstructor latticeConstructor;
	latticeConstructor.NewModel();

	double brho = beamenergy/eV/SpeedOfLight;
	double h = (2*pi/(4*ncell));

	//confirm equiv to MAD input
	double knl = 0.0098*brho;
	cout << knl << endl;

	//FODO lattice periodic
	for (int n=1;n<(ncell+1);++n) {
		latticeConstructor.AppendComponent(new Quadrupole("QF",lquad,0.0098*brho), n==1 ? 0 : 0.15*lcell-ldipole);
		latticeConstructor.AppendComponent(new SectorBend("MB",ldipole,h,brho*h), 0.15*lcell-lquad);
		latticeConstructor.AppendComponent(new SectorBend("MB",ldipole,h,brho*h), 0.2*lcell-ldipole);
		latticeConstructor.AppendComponent(new Quadrupole("QD",lquad,-0.0098*brho), 0.15*lcell-ldipole);
		latticeConstructor.AppendComponent(new SectorBend("MB",ldipole,h,brho*h), 0.15*lcell-lquad);
		latticeConstructor.AppendComponent(new SectorBend("MB",ldipole,h,brho*h), 0.2*lcell-ldipole);
	}
	latticeConstructor.AppendDrift(0.15*lcell-ldipole);
	AcceleratorModel* lattice = latticeConstructor.GetModel();

	ClosedOrbit theClosedOrbit(lattice,beamenergy);
	Particle co(0);
	theClosedOrbit.FindClosedOrbit(co);

	lattice->Output(cout);

	LatticeFunctionTable latticeFunctions(lattice,beamenergy);
	latticeFunctions.Calculate();


	// We add functions to give us the dispersion.
	// Dx  = beta(1,6,3)/beta(6,6,3)
	// Dpx = beta(2,6,3)/beta(6,6,3)
	// etc.
//	latticeFunctions.AddFunction(1,6,3);
//	latticeFunctions.AddFunction(2,6,3);
//	latticeFunctions.AddFunction(3,6,3);
//	latticeFunctions.AddFunction(4,6,3);
//	latticeFunctions.AddFunction(6,6,3);

	// Calculate the lattice functions.

	// Generate a file with the results.
	// First column is the s position in the beamline.
	ofstream latticeFunctionLog("LatticeFunctions.dat");
//	latticeFunctions.PrintTable(latticeFunctionLog);

	Particle particle(0);
	ParticleBunch* bunch = new ParticleBunch(beamenergy);

	particle.x() = 0.001;
	particle.y() = 0.002;
	bunch->AddParticle(particle);

	ParticleTracker tracker(lattice->GetBeamline(), bunch);

	ofstream trackingLog("Tracking.dat");

	for(int turn=0; turn<200; turn++)
	{
		if(turn==0)
		{
			tracker.Run();
		}
		else
		{
			tracker.Continue();
		}
		ParticleBunch& tracked = tracker.GetTrackedBunch();
		tracked.Output(trackingLog,false);
	}

	delete bunch;
	delete lattice;

	cout<<"Finished!"<<endl;

	return 0;
}

