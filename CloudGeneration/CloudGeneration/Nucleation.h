#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Nucleation
{
public:
	Nucleation();

	void CalcAtmosphere();
	double CalcPressureSat(double t);
	void CalcWaterPressure();
	double CalcNucleation(double t, double waterPress);

	std::vector<double> GetNucleation() { return nucleation; }

private:
	const float gamma = 7.0f / 5.0f;	//thermodynamics constant
	double scaleHeight;					//scale height of atmosphere
	double temperature;					//temperature at sea level (15C) in Kelvin
	double pressure;						//pressure at sea level in pascals (Pa)
	double density;						//rHo density at sea level (kg/m^3)

	//nucleation variables
	double sigma;
	double dalton;
	double m1;
	double v1;
	double kb;

	std::vector<float> dens;			//density results
	std::vector<float> temp;			//temp results
	std::vector<float> press;			//pressure results
	std::vector<double> waterPress;		//water pressure results
	std::vector<double> nucleation;		//nucleation results

};
