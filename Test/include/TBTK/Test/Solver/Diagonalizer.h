#include "TBTK/Solver/Diagonalizer.h"

#include "gtest/gtest.h"

namespace TBTK{
namespace Solver{

const double EPSILON_100 = 100*std::numeric_limits<double>::epsilon();

TEST(Diagonalizer, Constructor){
	//Not testable on its own.
}

TEST(Diagonalizer, Destructor){
	//Not testable on its own.
}

int selfConsistencyCounter;
bool selfConsistencyCallback(Diagonalizer &diagonalizer){
	selfConsistencyCounter++;
	if(selfConsistencyCounter == 10)
		return true;
	else
		return false;
}

TEST(Diagonalizer, setSelfConsistencyCallback){
	Model model;
	model.setVerbose(false);
	model << HoppingAmplitude(1, {1}, {0}) + HC;
	model.construct();

	Diagonalizer solver;
	solver.setVerbose(false);
	solver.setModel(model);
	selfConsistencyCounter = 0;
	solver.setSelfConsistencyCallback(selfConsistencyCallback);
	solver.run();
	EXPECT_EQ(selfConsistencyCounter, 10);

	selfConsistencyCounter = 0;
	solver.setSelfConsistencyCallback(selfConsistencyCallback);
	solver.setMaxIterations(5);
	solver.run();
	EXPECT_EQ(selfConsistencyCounter, 5);
}

TEST(Diagonalizer, setMaxIterations){
	//Tested through Diagonalizer::setSelfConsistencyCallback
}

TEST(Diagonalizer, run){
	//Tested through
	//Diagonalizer::setSelfConsistencyCallback
	//Diagonalizer::getEigenValues
	//Diagonalizer::getEigenVectors
	//Diagonalizer::getEigenValue
	//Diagonalizer::getAmplitude
}

TEST(Diagonalizer, getEigenValues){
	Model model;
	model.setVerbose(false);
	model << HoppingAmplitude(1, {1}, {0}) + HC;
	model.construct();

	Diagonalizer solver;
	solver.setVerbose(false);
	solver.setModel(model);
	solver.run();

	//Normal access.
	const double *eigenValues0 = solver.getEigenValues();
	EXPECT_DOUBLE_EQ(eigenValues0[0], -1);
	EXPECT_DOUBLE_EQ(eigenValues0[1], 1);

	//Access with write permissions.
	double *eigenValues1 = solver.getEigenValuesRW();
	EXPECT_DOUBLE_EQ(eigenValues1[0], -1);
	EXPECT_DOUBLE_EQ(eigenValues1[1], 1);

	//Verify that write to internal data works (note that write is
	//performed to eigenValues1, while read is from eigenValues0).
	eigenValues1[0] = 2;
	EXPECT_DOUBLE_EQ(eigenValues0[0], 2);

	///////////////////////////
	// Non-orthonormal basis //
	///////////////////////////
	//This is the same problem as above, but using the basis [1 0],
	//[1/sqrt(2) 1/sqrt(2)]. The eigenvalues are therefore the same.
	Model model1;
	model1.setVerbose(false);
	model1 << HoppingAmplitude(1/sqrt(2), {0}, {1}) + HC;
	model1 << HoppingAmplitude(1, {1}, {1});
	model1.construct();

	model1 << OverlapAmplitude(1, {0}, {0});
	model1 << OverlapAmplitude(1/sqrt(2), {0}, {1});
	model1 << OverlapAmplitude(1/sqrt(2), {1}, {0});
	model1 << OverlapAmplitude(1, {1}, {1});

	Diagonalizer solver1;
	solver1.setVerbose(false);
	solver1.setModel(model1);
	solver1.run();

	const double *eigenValues2 = solver1.getEigenValues();
	EXPECT_NEAR(eigenValues2[0], -1, EPSILON_100);
	EXPECT_NEAR(eigenValues2[1], 1, EPSILON_100);
}

TEST(Diagonalizer, getEigenValuesRW){
	//Tested in Diagonalizer::getEigenValues().
}

TEST(Diagonalizer, getEigenVectors){
	Model model;
	model.setVerbose(false);
	model << HoppingAmplitude(1, {1}, {0}) + HC;
	model.construct();

	Diagonalizer solver;
	solver.setVerbose(false);
	solver.setModel(model);
	solver.run();

	//Normal access.
	const std::complex<double> *eigenVectors0 = solver.getEigenVectors();
	EXPECT_DOUBLE_EQ(real(eigenVectors0[0]/eigenVectors0[1]), -1);
	EXPECT_DOUBLE_EQ(imag(eigenVectors0[0]/eigenVectors0[1]), 0);
	EXPECT_DOUBLE_EQ(real(eigenVectors0[2]/eigenVectors0[3]), 1);
	EXPECT_DOUBLE_EQ(imag(eigenVectors0[2]/eigenVectors0[3]), 0);

	//Access with write permissions.
	std::complex<double> *eigenVectors1 = solver.getEigenVectorsRW();
	EXPECT_DOUBLE_EQ(real(eigenVectors1[0]/eigenVectors1[1]), -1);
	EXPECT_DOUBLE_EQ(imag(eigenVectors1[0]/eigenVectors1[1]), 0);
	EXPECT_DOUBLE_EQ(real(eigenVectors1[2]/eigenVectors1[3]), 1);
	EXPECT_DOUBLE_EQ(imag(eigenVectors1[2]/eigenVectors1[3]), 0);

	//Verify that write to internal data works (note that write is
	//performed to eigenVectors1, while read is from eigenVectors0).
	eigenVectors1[0] = 2;
	EXPECT_DOUBLE_EQ(real(eigenVectors0[0]), 2);
	EXPECT_DOUBLE_EQ(imag(eigenVectors0[0]), 0);

	///////////////////////////
	// Non-orthonormal basis //
	///////////////////////////
	//This is the same problem as above, but using the basis [1 0],
	//[1/sqrt(2) 1/sqrt(2)]. Since the eigen vectors in the orthonormal
	//basis are [1/sqrt(2) -1/sqrt(2)] and [1/sqrt(2) 1/sqrt(2)], the eigen
	//vectors in this alternative basis are given by
	//a[1 0] + b[1/sqrt(2) 1/sqrt(2)], and thus [a b] are [sqrt(2) -1] and
	//[0 1] for the positive and negative eigenvalues, respectively.
	Model model1;
	model1.setVerbose(false);
	model1 << HoppingAmplitude(1/sqrt(2), {0}, {1}) + HC;
	model1 << HoppingAmplitude(1, {1}, {1});
	model1.construct();

	model1 << OverlapAmplitude(1, {0}, {0});
	model1 << OverlapAmplitude(1/sqrt(2), {0}, {1});
	model1 << OverlapAmplitude(1/sqrt(2), {1}, {0});
	model1 << OverlapAmplitude(1, {1}, {1});

	Diagonalizer solver1;
	solver1.setVerbose(false);
	solver1.setModel(model1);
	solver1.run();

	const std::complex<double> *eigenVectors2 = solver1.getEigenVectors();
	EXPECT_NEAR(real(eigenVectors2[0]), sqrt(2), EPSILON_100);
	EXPECT_NEAR(imag(eigenVectors2[0]), 0, EPSILON_100);
	EXPECT_NEAR(real(eigenVectors2[1]), -1, EPSILON_100);
	EXPECT_NEAR(imag(eigenVectors2[1]), 0, EPSILON_100);

	EXPECT_NEAR(real(eigenVectors2[2]), 0, EPSILON_100);
	EXPECT_NEAR(imag(eigenVectors2[2]), 0, EPSILON_100);
	EXPECT_NEAR(real(eigenVectors2[3]), 1, EPSILON_100);
	EXPECT_NEAR(imag(eigenVectors2[3]), 0, EPSILON_100);
}

TEST(Diagonalizer, getEigenVectorsRW){
	//Tested in Diagonalizer::getEigenVectorsRW
}

TEST(Diagonalizer, getEigenValue){
	Model model;
	model.setVerbose(false);
	model << HoppingAmplitude(1, {1}, {0}) + HC;
	model.construct();

	Diagonalizer solver;
	solver.setVerbose(false);
	solver.setModel(model);
	solver.run();

	EXPECT_DOUBLE_EQ(solver.getEigenValue(0), -1);
	EXPECT_DOUBLE_EQ(solver.getEigenValue(1), 1);
}

TEST(Diagonalizer, getAmplitude){
	Model model;
	model.setVerbose(false);
	model << HoppingAmplitude(1, {1}, {0}) + HC;
	model.construct();

	Diagonalizer solver;
	solver.setVerbose(false);
	solver.setModel(model);
	solver.run();

	EXPECT_DOUBLE_EQ(
		real(solver.getAmplitude(0, {0})/solver.getAmplitude(0, {1})), -1
	);
	EXPECT_DOUBLE_EQ(
		imag(solver.getAmplitude(0, {0})/solver.getAmplitude(0, {1})), 0
	);
	EXPECT_DOUBLE_EQ(
		real(solver.getAmplitude(1, {0})/solver.getAmplitude(1, {1})), 1
	);
	EXPECT_DOUBLE_EQ(
		imag(solver.getAmplitude(1, {0})/solver.getAmplitude(1, {1})), 0
	);
}

};	//End of namespace Solver
};	//End of namespace TBTK
