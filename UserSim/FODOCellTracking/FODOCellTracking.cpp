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
#include "MADInterface.h"

#include "PSvector.h"
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

	// LATTICE INPUT OPTION BEGIN

//	// OPTION 1 !!! FODO lattice periodic
//	for (int n=1;n<(ncell+1);++n) {
//		latticeConstructor.AppendComponent(new Quadrupole("QF",lquad,0.0098*brho), n==1 ? 0 : 0.15*lcell-ldipole);
//		latticeConstructor.AppendComponent(new SectorBend("MB",ldipole,h,brho*h), 0.15*lcell-lquad);
//		latticeConstructor.AppendComponent(new SectorBend("MB",ldipole,h,brho*h), 0.2*lcell-ldipole);
//		latticeConstructor.AppendComponent(new Quadrupole("QD",lquad,-0.0098*brho), 0.15*lcell-ldipole);
//		latticeConstructor.AppendComponent(new SectorBend("MB",ldipole,h,brho*h), 0.15*lcell-lquad);
//		latticeConstructor.AppendComponent(new SectorBend("MB",ldipole,h,brho*h), 0.2*lcell-ldipole);
//	}
//	latticeConstructor.AppendDrift(0.15*lcell-ldipole);
//
//	AcceleratorModel* lattice = latticeConstructor.GetModel();

	// OPTION 2 !!! FODO lattice periodic
	// CHOOSE ONE
	// - MERLINFodo.lattice.txt is "stable" lattice from Andy
	// - twiss.out is MAD equiv of above
	// - twiss.7.0tev.b1_new.tfs is LHC optics

//	string paths[] = {"../inputdata/MERLINFodo.lattice.txt", "inputdata/MERLINFodo.lattice.txt",
//					  "UserSim/inputdata/MERLINFodo.lattice.txt"};
	// OR

//	string paths[] = {"../inputdata/twiss.out", "inputdata/twiss.out",
//					  "UserSim/inputdata/twiss.out"};
	// OR

	string paths[] = {"../inputdata/twiss.7.0tev.b1_new.tfs", "inputdata/twiss.7.0tev.b1_new.tfs",
					  "UserSim/inputdata/twiss.7.0tev.b1_new.tfs"};

	string lattice_path;
	for(size_t i = 0; i < 3; i++)
	{
		ifstream test_file;
		test_file.open(paths[i].c_str());
		if(test_file)
		{
			lattice_path = paths[i];
			break;
		}
	}
	cout << "Lattice " << lattice_path << endl;

	MADInterface madi(lattice_path, beamenergy);
	madi.TreatTypeAsDrift("RFCAVITY");

//	ofstream madlog("mad.log");
//	madi.SetLogFile(madlog);
//	madi.SetLoggingOn();

	AcceleratorModel* lattice = madi.ConstructModel();

	// LATTICE INPUT OPTION END

	lattice->Output(cout);

	cout << "Lattice complete" << endl;


	// Calculate the closed orbit

	ClosedOrbit theClosedOrbit(lattice,beamenergy);
	Particle co(0);
	theClosedOrbit.FindClosedOrbit(co);

	cout << "Closed orbit complete" << endl;


	// Calculate the lattice functions

	LatticeFunctionTable latticeFunctions(lattice,beamenergy);
	latticeFunctions.Calculate();

	cout << "Closed orbit complete" << endl;

	// Generate a file with the results.
	// First column is the s position in the beamline.
	// ofstream latticeFunctionLog("LatticeFunctions.dat");
	// latticeFunctions.PrintTable(latticeFunctionLog);

	Particle particle(0);
	ParticleBunch* bunch = new ParticleBunch(beamenergy);

	particle.x() = 0.001;
	particle.y() = 0.002;
	bunch->AddParticle(particle);

	ParticleTracker tracker(lattice->GetBeamline(), bunch);

	// ofstream trackingLog("Tracking.dat");

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
		//tracked.Output(trackingLog,false);
	}

	delete bunch;
	delete lattice;

	cout<<"Finished!"<<endl;

	return 0;
}

