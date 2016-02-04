#include "../include/HoppingAmplitude.h"
#include <iostream>

using namespace std;

HoppingAmplitude::HoppingAmplitude(Index fromIndex, Index toIndex, complex<double> amplitude) : fromIndex(fromIndex), toIndex(toIndex){
	this->amplitude = amplitude;
	this->amplitudeCallback = NULL;
};

HoppingAmplitude::HoppingAmplitude(Index fromIndex, Index toIndex, complex<double> (*amplitudeCallback)(Index, Index)) : fromIndex(fromIndex), toIndex(toIndex){
	this->amplitudeCallback = amplitudeCallback;
};

HoppingAmplitude HoppingAmplitude::getHermitianConjugate(){
	if(amplitudeCallback)
		return HoppingAmplitude(toIndex, fromIndex, amplitudeCallback);
	else
		return HoppingAmplitude(toIndex, fromIndex, conj(amplitude));
}

void HoppingAmplitude::print(){
	cout << "From index:\t";
	for(unsigned int n = 0; n < fromIndex.indices.size(); n++){
		cout << fromIndex.indices.at(n) << " ";
	}
	cout << "\n";
	cout << "To index:\t";
	for(unsigned int n = 0; n < toIndex.indices.size(); n++){
		cout << toIndex.indices.at(n) << " ";
	}
	cout << "\n";
	cout << "Amplitude:\t" << getAmplitude() << "\n";
}
