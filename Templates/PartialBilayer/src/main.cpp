/* Basic example of diagonalization of a 2D tight-binding model with t = 1 and
 * mu = -1. Bilayer lattice with edges. First layers size is 20x20 sites, while
 * the second layer is 20x10.
 */

#include <iostream>
#include <complex>
#include "Model.h"
#include "FileWriter.h"
#include "PropertyExtractor.h"
#include "DiagonalizationSolver.h"

using namespace std;

const complex<double> i(0, 1);

int main(int argc, char **argv){
	//Lattice size
	const int SIZE_X = 20;
	const int SIZE_Y_LAYER_BOTTOM = 20;
	const int SIZE_Y_LAYER_TOP = 10;

	//Parameters
	complex<double> mu = -1.0;
	complex<double> t = 1.0;

	//Create model and set up hopping parameters
	Model model;
	//First layer
	for(int x = 0; x < SIZE_X; x++){
		for(int y = 0; y < SIZE_Y_LAYER_BOTTOM; y++){
			for(int s = 0; s < 2; s++){
				//Add hopping amplitudes corresponding to chemical potential
				model.addHA(HoppingAmplitude({0, x, y, s},	{0, x, y, s},	-mu));

				//Add hopping parameters corresponding to t
				if(x+1 < SIZE_X)
					model.addHAAndHC(HoppingAmplitude({0, x, y, s},	{0, (x+1)%SIZE_X, y, s},		-t));
				if(y+1 < SIZE_Y_LAYER_BOTTOM)
					model.addHAAndHC(HoppingAmplitude({0, x, y, s},	{0, x, (y+1)%SIZE_Y_LAYER_BOTTOM, s},	-t));
			}
		}
	}
	//Second layer
	for(int x = 0; x < SIZE_X; x++){
		for(int y = 0; y < SIZE_Y_LAYER_TOP; y++){
			for(int s = 0; s < 2; s++){
				//Add hopping amplitudes corresponding to chemical potential
				model.addHA(HoppingAmplitude({1, x, y, s},	{1, x, y, s},	-mu));

				//Add hopping amplitudes between layer 0 and 1
				model.addHAAndHC(HoppingAmplitude({0, x, y, s},	{1, x, y, s},	-t));

				//Add hopping amplitudes corresponding to t
				if(x+1 < SIZE_X)
					model.addHAAndHC(HoppingAmplitude({1, x, y, s},	{1, (x+1)%SIZE_X, y, s},		-t));
				if(y+1 < SIZE_Y_LAYER_TOP)
					model.addHAAndHC(HoppingAmplitude({1, x, y, s},	{1, x, (y+1)%SIZE_Y_LAYER_TOP, s},	-t));
			}
		}
	}

	//Setup and run DiagonalizationSolver
	DiagonalizationSolver dSolver;
	dSolver.setModel(&model);
	dSolver.run();

	//Set filename and remove any file already in the folder
	FileWriter::setFileName("TBTKResults.h5");
	FileWriter::clear();

	//Create PropertyExtractor
	PropertyExtractor pe(&dSolver);

	//Extract eigenvalues and write these to file
	double *ev = pe.getEV();
	FileWriter::writeEV(ev, model.getBasisSize());
	delete [] ev;

	return 0;
}