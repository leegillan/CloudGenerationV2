#include "Nucleation.h"
#include <iostream>

Nucleation::Nucleation()
{
	//atmospheric variables
	scaleHeight = 7.99e3f;
	temperature = 273.0f + 15.0f; //Change temp for hotter/colder locations
	pressure = 101325.0f;
	density = 1.225f;

	//Nucleation variables
	sigma = 72.75e-3;
	dalton = 1.66053906660e-27;
	m1 = 18.02 * dalton; //m1 = 2.99e-23f * 1e-3f  Converted into SI
	v1 = 2.99e-23 * 1e-6; // converted into SI
	kb = 1.38e-23;
}

void Nucleation::CalcAtmosphere()
{
	int altitude[25]; //array of values for the altitude, from 0 to 25km

	for (int i = 0; i < 25; i++)
	{
		altitude[i] = i * 1000;

		//compute temperature of atmosphere
		double common = 1.0f - (((gamma - 1.0f) / gamma) * (altitude[i] / scaleHeight));
		double temp0 = temperature * common;

		//(1 - ((gamma - 1) / gamma) * (altitude[i] / scaleHeight));

		//compute density and pressure
		double press0 = pressure * glm::pow(common, (gamma / (gamma - 1.0f)));
		double dens0 = density * glm::pow(common, (1.0f / (gamma - 1.0f)));

		//set results
		dens.push_back(dens0);
		temp.push_back(temp0);
		press.push_back(press0);
	}

	CalcWaterPressure();
}

void Nucleation::CalcWaterPressure()
{
	double superSat[25];
	double pressH20;

	double temp = 293.f;

	//std::cout << press[0];

	for (int i = 0; i < 25; i++)
	{
		superSat[i] = i;
		pressH20 = superSat[i] * CalcPressureSat(temp);

		waterPress.push_back(pressH20);
		nucleation.push_back(CalcNucleation(temp, pressH20));

		//Add console dump
	}
}

void Nucleation::CalcShaderValues()
{
	double pressH20;
	double temp = 293.f;

	for (int i = 0; i < 100; i++)
	{
		pressH20 = ((i / 10) * 10000) * CalcPressureSat(temp);

		shaderNucleation.push_back(CalcNucleation(temp, pressH20));
	}
}

double Nucleation::CalcPressureSat(double t)
{
	t -= 273;	//convert to Celsius

	return 610.78f * glm::exp((17.27 * t) / (237.3 + t));
}
//calculates nucleation
double Nucleation::CalcNucleation(double t, double waterPress)
{
	double N = waterPress / (kb * t);
	double S = waterPress / CalcPressureSat(t);

	double inside = -(16.0f * glm::pi<double>() / 3.0f)
		* (glm::pow(v1, 2.0) * glm::pow(sigma, 3.0));

	double j = glm::sqrt(2.0 * sigma / (glm::pi<double>() * m1)) * (v1 * glm::pow(N, 2.0) / S) * glm::exp(inside);

	return j;
}