/*
 * MaterialProperties.cpp
 *
 *  Created on: 16 Aug 2017
 *      Author: roger
 */

#include "MaterialProperties.h"

#include <iomanip>
#include <stdarg.h>
#include <vector>
#include <iostream>
#include <string>
#include <map>
#include "RandomNG.h"
using namespace std;

MaterialProperties::MaterialProperties(double p1, double p2, double p3, double p4, double p5, double p6, double p7,
		double p8)
{
	A = p1;
	sigma_T = p2;
	sigma_I = p3;
	sigma_R = p4;
	dEdx = p5;
	X0 = p6;
	density = p7;
	conductivity = p8;
	extra = new map<string, double>; // deleted by destructor
	Update();
}

void MaterialProperties::Update()
{
	dEdxGeVpermetre = dEdx * density / 10.0;
	lambda = A * 1.E-6 / ((sigma_T + sigma_R) * density * 1.E-28 * 6.022E23);
}

MaterialProperties* MaterialProperties::EnergyScale(double E)
{
	MaterialProperties* p = new MaterialProperties(A, sigma_T, sigma_I, sigma_R, dEdx, X0, density, conductivity);
	// adjust dEdx and cross sections with new energy
	p->Update();
	return p;
}

MaterialProperties::MaterialProperties(const MaterialProperties& a)
{ //copy constructor
	density = a.density;
	A = a.A;
	sigma_T = a.sigma_T;
	sigma_I = a.sigma_I;
	sigma_R = a.sigma_R;
	dEdx = a.dEdx;
	X0 = a.X0;
	conductivity = a.conductivity;
	extra = a.extra;

}

MaterialProperties& MaterialProperties::operator=(const MaterialProperties& a)
{ // copy assignment
	if (this != &a)
	{ // beware of self-assignment
		density = a.density;
		A = a.A;
		sigma_T = a.sigma_T;
		sigma_I = a.sigma_I;
		sigma_R = a.sigma_R;
		dEdx = a.dEdx;
		X0 = a.X0;
		conductivity = a.conductivity;
		extra = a.extra;
	}
	return *this;
}

ostream& operator<<(ostream& o, MaterialProperties& d)
{
	o << setiosflags(ios::fixed) << setw(5) << setprecision(1) << d.A << setw(7) << setprecision(3) << d.sigma_T
			<< setw(7) << d.sigma_I << setw(9) << setprecision(5) << d.sigma_R << setw(6) << setprecision(3) << d.dEdx
			<< setw(7) << d.density << scientific << setw(13) << setprecision(3) << d.conductivity << " " << fixed
			<< setw(7) << setprecision(1) << d.X0;
	if (d.extra && d.extra->size() > 0)
	{
		for (map<string, double>::iterator i = d.extra->begin(); i != d.extra->end(); i++)
			o << " " << i->first << " " << i->second;
	}
	return o;
}
//
void MaterialProperties::SetExtra(string prop, double val)
{
	/**
	 * Avoids the user having to mess with brackets and contents-of
	 */
	(*extra)[prop] = val;
}

double MaterialProperties::GetExtra(string prop)
{
	return (*extra)[prop];
}

MaterialProperties::~MaterialProperties()
{
	if (extra)
		delete extra;
}
