/////////////////////////////////////////////////////////////////////////
//
// Merlin C++ Class Library for Charged Particle Accelerator Simulations
//  
// Class library version 3 (2004)
// 
// Copyright: see Merlin/copyright.txt
// 
/////////////////////////////////////////////////////////////////////////

#include "merlin_config.h"
#include <iterator>
#include <iomanip>
#include <typeinfo>
#include <fstream>
#include <sstream>
#include "AcceleratorModel/Apertures/CollimatorAperture.hpp"
#include "NumericalUtils/utils.h"
#include "NumericalUtils/PhysicalUnits.h"
// CollimateParticleProcess
#include "Collimators/CollimateParticleProcess.h"
// Aperture
#include "AcceleratorModel/Aperture.h"
// Spoiler
#include "AcceleratorModel/StdComponent/Spoiler.h"

using namespace std;

//extern void ScatterParticle(PSvector& p, double X0, double x, double E0);
//extern void ScatterProton(PSvector& p, double x, double E0, const TiltedAperture* tap);

namespace {

using namespace ParticleTracking;

void OutputIndexParticles(const PSvectorArray lost_p, const list<size_t>& lost_i, ostream& os)
{
    PSvectorArray::const_iterator p = lost_p.begin();
    list<size_t>::const_iterator ip = lost_i.begin();

    while(p!=lost_p.end())
    {
        os<<std::setw(12)<<right<<*ip;
        os<<*p;
        ++p;
        ++ip;
    }
}

} // end anonymous namespace

namespace ParticleTracking {

CollimateParticleProcess::CollimateParticleProcess (int priority, int mode, std::ostream* osp)
        : ParticleBunchProcess("PARTICLE COLLIMATION",priority),cmode(mode),os(osp),
        createLossFiles(false),file_prefix(""),lossThreshold(1),nstart(0),pindex(0),scatter(false),bin_size(0.1*PhysicalUnits::meter),Imperfections(false)
{}

CollimateParticleProcess::~CollimateParticleProcess ()
{
    if(pindex!=0)
        delete pindex;
}

void CollimateParticleProcess::InitialiseProcess (Bunch& bunch)
{
    ParticleBunchProcess::InitialiseProcess(bunch);
    idtbl.clear();
    if(currentBunch) {
        nstart = currentBunch->size();
        nlost = 0;
        if(pindex!=0) {
            pindex->clear();
            for(size_t n=0; n<nstart; n++)
                pindex->push_back(n);
        }
    }
}

void CollimateParticleProcess::SetCurrentComponent (AcceleratorComponent& component)
{
	active = (currentBunch!=0) && (component.GetAperture()!=0);
	if(active)
	{
		currentComponent = &component;
		s=0;
		Spoiler* aSpoiler = dynamic_cast<Spoiler*>(&component);
		is_spoiler = scatter && aSpoiler;

		if(!is_spoiler)
		{ // not a spoiler so set up for normal hard-edge collimation
			at_entr = (COLL_AT_ENTRANCE & cmode)!=0;
			at_cent = (COLL_AT_CENTER & cmode)!=0;
			at_exit = (COLL_AT_EXIT & cmode)!=0;
			SetNextS();
		}
		else
		{
			at_entr = at_cent = false; // currently scatter only at exit
			at_exit = true;
			SetNextS();
			//Xr = aSpoiler->GetMaterialRadiationLength();
			currentBunch->SetScatterConfigured(false);
			len = aSpoiler->GetLength();
			CollimatorAperture* CollimatorJaw = dynamic_cast<CollimatorAperture*>(aSpoiler->GetAperture());
			if(CollimatorJaw)
			{
				//CollimatorJaw->
				if(Imperfections)
				{
					CollimatorJaw->SetJawLength(len);
					CollimatorJaw->EnableErrors(true);
				}
			}
		}
	}
	else
	{
		s_total += component.GetLength();
		currentComponent = 0;
	}
}

void CollimateParticleProcess::DoProcess (double ds)
{
    s+=ds;

    if(fequal(s,next_s))
    {
        DoCollimation();
        SetNextS();
    }

    // If we are finished, GetNextS() will have set the process inactive.
    // In that case we can update s_total with the component length.
    if(!active)
        s_total += currentComponent->GetLength();
	//cout<<"the component name is:"<<currentComponent->GetName()<<endl;
	//cout<<"component name:"<<currentComponent->GetQualifiedName()<<endl;
	//cout << s << "\t" << s_total << "\t" << currentComponent->GetLength() << endl;
}

double CollimateParticleProcess::GetMaxAllowedStepSize () const
{
	return next_s-s;
}

void CollimateParticleProcess::IndexParticles (bool index)
{
	if(index && pindex==0)
	{
		pindex = new list<size_t>;
	}
	else if(!index && pindex!=0)
	{
		delete pindex;
		pindex=0;
	}
}

void CollimateParticleProcess::IndexParticles (list<size_t>& anIndex)
{
    if(!pindex)
        delete pindex;

    pindex=&anIndex;
}


void CollimateParticleProcess::SetLossThreshold (double losspc)
{
	lossThreshold = losspc/100.0;
}

void CollimateParticleProcess::DoCollimation ()
{
	const Aperture *ap = currentComponent->GetAperture();
	PSvectorArray lost;
	list<size_t>  lost_i;
	//cout << "Collimating" << endl;
	list<size_t>::iterator ip;
	if(pindex!=0)
	ip=pindex->begin();
	ParticleBunch* newbunch=new ParticleBunch(currentBunch->GetReferenceMomentum(),currentBunch->GetTotalCharge()/currentBunch->size());

	for(PSvectorArray::iterator p = currentBunch->begin(); p!=currentBunch->end();)
	{
		if(!ap->PointInside((*p).x(),(*p).y(),s))
		{
			// If the 'aperture' is a spoiler, then the particle is lost
			// if the DoScatter(*p) returns true (energy cut)
			// If not a spoiler, then do not scatter and directly remove the particle.
			if(!is_spoiler || DoScatter(*p)) 
			{
				lost.push_back(*p);
				/* This is slow for a STL Vector - instead we place the surviving particles into a new bunch and then swap - this is faster */
				//p=currentBunch->erase(p);
				p++;
				if(pindex!=0)
				{
					lost_i.push_back(*ip);
					ip = pindex->erase(ip);
				}
			}
			else
			{
				//Particle survives
				newbunch->AddParticle(*p);
				// need to increment iterators
				p++;
				if(pindex!=0)
				{
					ip++;
				}
			}
		}
		else
		{
			//Not interacting with the collimator: "Inside" the aperture; particle lives
			newbunch->AddParticle(*p);
			p++;
			if(pindex!=0)
			{
				ip++;
			}
		}
	}

	currentBunch->clear();
	currentBunch->swap(*newbunch);
	delete newbunch;

	nlost+=lost.size();
	//cout << currentComponent->GetQualifiedName() << "\t" << nlost << "\t" << nstart << "\t" << currentBunch->size() << endl;
	//cout << "The number of particles lost is: " << nlost << endl;
	DoOutput(lost,lost_i);

	if(double(nlost)/double(nstart)>=lossThreshold)
	{
		throw ExcessiveParticleLoss(currentComponent->GetQualifiedName(),lossThreshold,nlost,nstart);
	}
}


void CollimateParticleProcess::SetNextS ()
{
	if(at_entr)
	{
		next_s = 0;
		at_entr = false;
	}
	else if(at_cent)
	{
		next_s=currentComponent->GetLength()/2;
		at_cent=false;
	}
	else if(at_exit)
	{
		next_s=currentComponent->GetLength();
		at_exit = false;
	}
	else
	{
		active=false;
	}
}

void CollimateParticleProcess::DoOutput (const PSvectorArray& lostb, const list<size_t>& lost_i)
{
	// Create a file and dump the lost particles
	// (if there are any)
	if(!lostb.empty())
	{
//	PSvectorArray lostp = bin_lost_output(lostb);
        	if(os!=0)
		{
			double length = currentComponent->GetLength();
			//We bin in 0.1m sections of the element, this is how many bins we have
			//n is our number of bins
			int n = length / bin_size;

			bool overflow = false;
			//The element length may not be an exact multiple of the bin size, so we must take this into account.
			if ((length/(double)n) != 0.0)
			{
				//Add an aditional bin
				n++;
				overflow = true;
			}

			//Create the array - n rows, 3 cols
			double** lostp = new double*[n];
			for(int i = 0; i<n; ++i)
			{
				lostp[i] = new double[3];
			}

			//Initialize the array elements
			for(int j = 0; j<n; j++)
			{
				lostp[j][0] = bin_size*j;	//Start position
				lostp[j][1] = bin_size;		//Length
				lostp[j][2] = 0.0;		//Entries
			}

			//deal with the last bin if needed - will have a different length.
			if(overflow)
			{
				lostp[n-1][0] = bin_size*(n-1);
				lostp[n-1][1] = length - (bin_size*(n-1));
				lostp[n-1][2] = 0.0;
			}

			//cout << "Number of lost particles at " << currentComponent->GetName() << ": " << lostb.size() << endl;

			for(size_t l=0; l<lostb.size(); l++)
			{
				int x = lostb[l].ct()/bin_size;
				//Add one loss count to this bin
				lostp[x][2]++;
			}

			//Now to do the output - first loop over each bin
			for(int j = 0; j<n; j++)
			{
				//We then check if there are any lost particles in this bin
				//If there are lost particles, then we must output the count (lost count > 0)
				if(lostp[j][2] > 0.0)
				{
				//cout << lostp[j][0] << "\t" << lostp[j][1] << "\t" << lostp[j][2] << endl;
				(*os).precision(16);
				(*os) << std::setw(35)<<left<<(*currentComponent).GetQualifiedName().c_str();			//Component name - can be quite long for certain LHC magnets
				//(*os) << std::setw(24)<<left<<lostp[j][0] + currentBunch->GetReferenceTime()-length;	//Bin start position
				(*os) << std::setw(24)<<left<<lostp[j][0] + currentComponent->GetComponentLatticePosition();	//Bin start position
				(*os) << std::setw(24)<<left<<lostp[j][1];							//Bin length
				(*os) << "\t" << lostp[j][2];									//Loss count
				(*os) << endl;
				}
			}

		/*
			(*os)<<std::setw(24)<<left<<(*currentComponent).GetQualifiedName().c_str();
			(*os)<<std::setw(12)<<right<<currentComponent->GetLength();
			(*os)<<std::setw(12)<<right<<currentBunch->GetReferenceTime();
			(*os)<<std::setw(8)<<right<<lostb.size()<<endl;
		*/
		
		
		}

		if(createLossFiles)
		{
			string id = (*currentComponent).GetQualifiedName();   
			pair<IDTBL::iterator,bool> result = idtbl.insert(IDTBL::value_type(id,0));
			int n = ++(*(result.first)).second;
			ostringstream fname;

			if(!file_prefix.empty())
			{
				fname << file_prefix;
			}

			fname << id << '.' << n << ".loss";
			ofstream file(fname.str().c_str());

			if(pindex==0)
			{
				copy(lostb.begin(),lostb.end(),ostream_iterator<PSvector>(file));
			}
			else
			{
				OutputIndexParticles(lostb,lost_i,file);
			}
		}
	}
}

bool CollimateParticleProcess::DoScatter (Particle& p)
{
	//double E0=currentBunch->GetReferenceMomentum();
	const CollimatorAperture *tap = (CollimatorAperture*) currentComponent->GetAperture();

	int scatter_type = currentBunch->Scatter(p,len,tap);

//	return p.dp()<=-0.99; // return true if E below 1% cut-off
	if(scatter_type == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

ExcessiveParticleLoss::ExcessiveParticleLoss (const string& c_id, double threshold, size_t nlost, size_t nstart)
        : MerlinException()
{
	ostringstream buffer;
	buffer << "CollimateParticleProcess Exception\n";
	buffer << "particle loss threshold of " << 100*threshold << "% exceeded ";
	buffer << '(' << nlost << '/' << nstart << ") at " << c_id;
	SetMsg(buffer.str());
}

void CollimateParticleProcess::bin_lost_output(const PSvectorArray& lostb)
{

	double leng = currentComponent->GetLength();
	int n = leng / 0.1;


	bool overflow = false;
	if ((leng/(double)n) != 0.0)
	{
		n++;
		overflow = true;
	}


	//Create the array
	double** lostp = new double*[n];
	for(int i = 0; i<n; ++i)
	{
		lostp[i] = new double[3];
	}
	//fill the aray
	for(int j = 0; j<n; j++)
	{
		lostp[j][0] = 0.1*j;	//Start position
		lostp[j][1] = 0.1;	//Length
		lostp[j][2] = 0.0;	//Entries
	}
	//deal with the last bin
	if(overflow)
	{
		lostp[n-1][0] = 0.1*(n-1);
		lostp[n-1][1] = leng - (0.1*(n-1));
		lostp[n-1][2] = 0.0;
	}


	for(size_t l = 0; l < lostb.size(); l++)
	{
		int x = lostb[l].ct()/0.1;
		lostp[x][2]++;
	}

	for(int j = 0; j<n; j++)
	{
		cout << lostp[j][0] << "\t" << lostp[j][1] << "\t" << lostp[j][2] << endl;
	}
}

void CollimateParticleProcess::SetOutputBinSize(double binsize)
{

}

double CollimateParticleProcess::GetOutputBinSize() const
{
	return bin_size;
}

void CollimateParticleProcess::EnableImperfections(bool enable)
{
	Imperfections = enable;
}
} // end namespace ParticleTracking
