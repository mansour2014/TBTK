/* Copyright 2018 Kristofer Björnson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file MatsubaraSusceptibility.cpp
 *
 *  @author Kristofer Björnson
 */

#include "TBTK/PropertyExtractor/MatsubaraSusceptibility.h"
#include "TBTK/Functions.h"
#include "TBTK/Streams.h"
#include "TBTK/Timer.h"

#include <cmath>

using namespace std;

namespace TBTK{
namespace PropertyExtractor{

MatsubaraSusceptibility::MatsubaraSusceptibility(
	Solver::MatsubaraSusceptibility &solver
){
	this->solver = &solver;
//	energyType = EnergyType::Real;
}

/*void MatsubaraSusceptibility::setEnergyWindow(
	double lowerBound,
	double upperBound,
	int resolution
){
	PropertyExtractor::setEnergyWindow(
		lowerBound,
		upperBound,
		resolution
	);

	energyType = EnergyType::Real;
}

void MatsubaraSusceptibility::setEnergyWindow(
	int lowerFermionicMatsubaraEnergyIndex,
	int upperFermionicMatsubaraEnergyIndex,
	int lowerBosonicMatsubaraEnergyIndex,
	int upperBosonicMatsubaraEnergyIndex
){
	TBTKAssert(
		abs(lowerFermionicMatsubaraEnergyIndex%2) == 1,
		"PropertyExtractor::MatsubaraSusceptibility::setEnergyWindow()",
		"'lowerFermionicMatsubaraEnergyIndex="
		<< lowerFermionicMatsubaraEnergyIndex << "' must be odd.",
		""
	);
	TBTKAssert(
		abs(upperFermionicMatsubaraEnergyIndex%2) == 1,
		"PropertyExtractor::MatsubaraSusceptibility::setEnergyWindow()",
		"'upperFermionicMatsubaraEnergyIndex="
		<< upperFermionicMatsubaraEnergyIndex << "' must be odd.",
		""
	);
	TBTKAssert(
		abs(lowerBosonicMatsubaraEnergyIndex%2) == 0,
		"PropertyExtractor::MatsubaraSusceptibility::setEnergyWindow()",
		"'lowerBosonicMatsubaraEnergyIndex="
		<< lowerBosonicMatsubaraEnergyIndex << "' must be odd.",
		""
	);
	TBTKAssert(
		abs(upperBosonicMatsubaraEnergyIndex%2) == 0,
		"PropertyExtractor::MatsubaraSusceptibility::setEnergyWindow()",
		"'upperBosoonicMatsubaraEnergyIndex="
		<< upperBosonicMatsubaraEnergyIndex << "' must be odd.",
		""
	);

	energyType = EnergyType::Matsubara;
	this->lowerFermionicMatsubaraEnergyIndex
		= lowerFermionicMatsubaraEnergyIndex;
	this->upperFermionicMatsubaraEnergyIndex
		= upperFermionicMatsubaraEnergyIndex;
	this->lowerBosonicMatsubaraEnergyIndex
		= lowerBosonicMatsubaraEnergyIndex;
	this->upperBosonicMatsubaraEnergyIndex
		= upperBosonicMatsubaraEnergyIndex;
}*/

Property::Susceptibility MatsubaraSusceptibility::calculateSusceptibility(
	vector<Index> patterns
){
	//Calculate allIndices.
	IndexTree allIndices;
	for(unsigned int n = 0; n < patterns.size(); n++){
		const Index &pattern = *(patterns.begin() + n);

		vector<Index> indices = pattern.split();
		TBTKAssert(
			indices.size() == 5,
			"PropertyExtractor::MatsubaraSusceptibility::calculateSusceptibility()",
			"Invalid pattern '" << pattern.toString() << "'.",
			"The Index must be a compound Index with five"
			<< " component Indices, but the number of components"
			<< " are " << indices.size() << "."
		);
		for(unsigned int n = 2; n < indices.size(); n++){
			TBTKAssert(
				indices[n].getSize() == indices[1].getSize(),
				"PropertyExtractor::MatsubaraSusceptibility::calculateSusceptibility()",
				"Currently the last four Indices has to have"
				" the same number of subindices. The clash"
				<< " appeared between '"
				<< indices[1].toString() << "' and '"
				<< indices[n].toString() << "'",
				"Contact the developer if support for more"
				<< " general Indices is needed."
			);
			//TODO
			//For general Indices to be supported, a more general
			//way for finding all possible kIndices is required.
			//The current method relies on knowing the number of
			//subindices to append after the kIndex.
		}

		Index kIndexPattern = indices[0];

		//TODO
		//This is the restricting assumption.
		Index kIndexPatternExtended = kIndexPattern;
		for(unsigned int n = 0; n < indices[1].getSize(); n++)
			kIndexPatternExtended.push_back(IDX_ALL);

		IndexTree kIndexTree = generateIndexTree(
			{kIndexPatternExtended},
			solver->getModel().getHoppingAmplitudeSet(),
			false,
			false
		);
		for(
			IndexTree::ConstIterator iteratorK
				= kIndexTree.cbegin();
			iteratorK != kIndexTree.cend();
			++iteratorK
		){
			Index kIndex = *iteratorK;
			kIndex = kIndex.getSubIndex(
				0,
				kIndexPattern.getSize()-1
			);

			Index intraBlockIndices[4];
			for(unsigned int n = 0; n < 4; n++){
				intraBlockIndices[n] = Index(
					kIndex,
					indices[n+1]
				);
			}

			IndexTree intraBlockIndicesTree[4];
			for(unsigned int n = 0; n < 4; n++){
				intraBlockIndicesTree[n] = generateIndexTree(
					{intraBlockIndices[n]},
					solver->getModel(
					).getHoppingAmplitudeSet(),
					false,
					false
				);
			}

			for(
				IndexTree::ConstIterator iterator0
					= intraBlockIndicesTree[0].cbegin();
				iterator0 != intraBlockIndicesTree[0].cend();
				++iterator0
			){
				Index index0 = *iterator0;
				index0 = index0.getSubIndex(
					kIndex.getSize(),
					index0.getSize()-1
				);
				for(
					IndexTree::ConstIterator iterator1
						= intraBlockIndicesTree[1].cbegin();
					iterator1
						!= intraBlockIndicesTree[1].cend();
					++iterator1
				){
					Index index1 = *iterator1;
					index1 = index1.getSubIndex(
						kIndex.getSize(),
						index1.getSize()-1
					);
					for(
						IndexTree::ConstIterator iterator2
							= intraBlockIndicesTree[2].cbegin();
						iterator2
							!= intraBlockIndicesTree[2].cend();
						++iterator2
					){
						Index index2 = *iterator2;
						index2 = index2.getSubIndex(
							kIndex.getSize(),
							index2.getSize()-1
						);
						for(
							IndexTree::ConstIterator iterator3
								= intraBlockIndicesTree[3].cbegin();
							iterator3
								!= intraBlockIndicesTree[3].cend();
							++iterator3
						){
							Index index3 = *iterator3;
							index3 = index3.getSubIndex(
								kIndex.getSize(),
								index3.getSize()-1
							);
							allIndices.add({
								kIndex,
								index0,
								index1,
								index2,
								index3
							});
						}
					}
				}
			}
		}
	}
	allIndices.generateLinearMap();

	//Calculate memoryLayout.
	IndexTree memoryLayout;
	for(unsigned int n = 0; n < patterns.size(); n++){
		const Index &pattern = *(patterns.begin() + n);

		vector<Index> indices = pattern.split();
		TBTKAssert(
			indices.size() == 5,
			"PropertyExtractor::MatsubaraSusceptibility::calculateSusceptibility()",
			"Invalid pattern '" << pattern.toString() << "'.",
			"The Index must be a compound Index with five"
			<< " component Indices, but the number of components"
			<< " are " << indices.size() << "."
		);
		for(unsigned int n = 2; n < indices.size(); n++){
			TBTKAssert(
				indices[n].getSize() == indices[1].getSize(),
				"PropertyExtractor::MatsubaraSusceptibility::calculateSusceptibility()",
				"Currently the last four Indices has to have"
				" the same number of subindices. The clash"
				<< " appeared between '"
				<< indices[1].toString() << "' and '"
				<< indices[n].toString() << "'",
				"Contact the developer if support for more"
				<< " general Indices is needed."
			);
			//TODO
			//For general Indices to be supported, a more general
			//way for finding all possible kIndices is required.
			//The current method relies on knowing the number of
			//subindices to append after the kIndex.
		}

		Index kIndexPattern = indices[0];

		//TODO
		//This is the restricting assumption.
		Index kIndexPatternExtended = kIndexPattern;
		for(unsigned int n = 0; n < indices[1].getSize(); n++)
			kIndexPatternExtended.push_back(IDX_ALL);

		IndexTree kIndexTree = generateIndexTree(
			{kIndexPatternExtended},
			solver->getModel().getHoppingAmplitudeSet(),
			true,
			false
		);
		for(
			IndexTree::ConstIterator iteratorK
				= kIndexTree.cbegin();
			iteratorK != kIndexTree.cend();
			++iteratorK
		){
			//Index that prepends the intraBlockIndices.
			Index kIndex = *iteratorK;
			kIndex = kIndex.getSubIndex(
				0,
				kIndexPattern.getSize()-1
			);

			Index intraBlockIndices[4];
			for(unsigned int n = 0; n < 4; n++){
				intraBlockIndices[n] = Index(
					kIndex,
					indices[n+1]
				);
			}

			IndexTree intraBlockIndicesTree[4];
			for(unsigned int n = 0; n < 4; n++){
				intraBlockIndicesTree[n] = generateIndexTree(
					{intraBlockIndices[n]},
					solver->getModel(
					).getHoppingAmplitudeSet(),
					true,
					false
				);
			}

			for(
				IndexTree::ConstIterator iterator0
					= intraBlockIndicesTree[0].cbegin();
				iterator0 != intraBlockIndicesTree[0].cend();
				++iterator0
			){
				Index index0 = *iterator0;
				index0 = index0.getSubIndex(
					kIndex.getSize(),
					index0.getSize()-1
				);
				for(
					IndexTree::ConstIterator iterator1
						= intraBlockIndicesTree[1].cbegin();
					iterator1
						!= intraBlockIndicesTree[1].cend();
					++iterator1
				){
					Index index1 = *iterator1;
					index1 = index1.getSubIndex(
						kIndex.getSize(),
						index1.getSize()-1
					);
					for(
						IndexTree::ConstIterator iterator2
							= intraBlockIndicesTree[2].cbegin();
						iterator2
							!= intraBlockIndicesTree[2].cend();
						++iterator2
					){
						Index index2 = *iterator2;
						index2 = index2.getSubIndex(
							kIndex.getSize(),
							index2.getSize()-1
						);
						for(
							IndexTree::ConstIterator iterator3
								= intraBlockIndicesTree[3].cbegin();
							iterator3
								!= intraBlockIndicesTree[3].cend();
							++iterator3
						){
							Index index3 = *iterator3;
							index3 = index3.getSubIndex(
								kIndex.getSize(),
								index3.getSize()-1
							);
							memoryLayout.add({
								kIndex,
								index0,
								index1,
								index2,
								index3
							});
						}
					}
				}
			}
		}
	}
	memoryLayout.generateLinearMap();

	switch(getEnergyType()){
	case EnergyType::Real:
	{
		TBTKExit(
			"PropertyExtractor::MatsubaraSusceptibility::calculateSusceptibility()",
			"Real energies not yet supported.",
			""
		);
	}
	case EnergyType::Matsubara:
	{
		TBTKAssert(
			getLowerBosonicMatsubaraEnergyIndex()
			<= getUpperBosonicMatsubaraEnergyIndex(),
			"PropertyExtractor::MatsubaraSusceptibility::calculateSusceptibility()",
			"'lowerBosonicMatsubaraEnergyIndex="
			<< getLowerBosonicMatsubaraEnergyIndex() << "' must be"
			<< " less or equal to"
			<< " 'upperBosonicMatsubaraEnergyIndex='"
			<< getUpperBosonicMatsubaraEnergyIndex() << "'.",
			"This should never happen, contact the developer."
		);

		Property::Susceptibility susceptibility(
			memoryLayout,
			getLowerBosonicMatsubaraEnergyIndex(),
			getUpperBosonicMatsubaraEnergyIndex(),
			solver->getGreensFunction(
			).getFundamentalMatsubaraEnergy()
		);

		calculate(
			calculateSusceptibilityCallback,
			allIndices,
			memoryLayout,
			susceptibility
		);

		return susceptibility;
	}
	default:
		TBTKExit(
			"PropertyExtractor::MatsubaraSusceptibility::MatsubaraSusceptibility()",
			"Uknown EnergyType.",
			"This should never happen, contact the developer."
		);
	}
}

void MatsubaraSusceptibility::calculateSusceptibilityCallback(
	PropertyExtractor *cb_this,
	void *susceptibility,
	const Index &index,
	int offset
){
	MatsubaraSusceptibility *propertyExtractor
		= (MatsubaraSusceptibility*)cb_this;

	vector<complex<double>> s
		= propertyExtractor->solver->calculateSusceptibility(
			index,
			propertyExtractor->getLowerBosonicMatsubaraEnergyIndex(),
			propertyExtractor->getUpperBosonicMatsubaraEnergyIndex()
		);

	for(unsigned int e = 0; e < s.size(); e++)
		((complex<double>*)susceptibility)[offset + e] += s[e];
}

};	//End of namespace PropertyExtractor
};	//End of namespace TBTK
