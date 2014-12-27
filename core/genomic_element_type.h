//
//  genomic_element_type.h
//  SLiM
//
//  Created by Ben Haller on 12/13/14.
//  Copyright (c) 2014 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/software/
//

//	This file is part of SLiM.
//
//	SLiM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//	SLiM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with SLiM.  If not, see <http://www.gnu.org/licenses/>.

/*
 
 The class GenomicElementType represents a possible type of genomic element, defined by the types of mutations the element undergoes,
 and the relative fractions of each of those mutation types.  Exons and introns might be represented by different genomic element types,
 for example, and might have different types of mutations (exons undergo adaptive mutations while introns do not, perhaps).  At present,
 these mutational dynamics are the only defining characteristics of genomic elements.
 
 */

#ifndef __SLiM__genomic_element_type__
#define __SLiM__genomic_element_type__


#include <vector>
#include <iostream>

#include "g_rng.h"
#include "mutation_type.h"


class GenomicElementType
{
private:
	
	//
	//	This class has its copy constructor and assignment operator disabled, to prevent accidental copying.
	//
	GenomicElementType(const GenomicElementType&);						// disable copy constructor
	GenomicElementType &operator = (const GenomicElementType&);			// disable assignment operator
	
	gsl_ran_discrete_t *lookup_mutation_type;
	
public:
	
	int genomic_element_type_id_;				// the id by which this genomic element type is indexed in the chromosome
	
	std::vector<MutationType*> mutation_type_ptrs_;		// mutation types identifiers in this element
	std::vector<double> mutation_fractions_;	// relative fractions of each mutation type
	
	GenomicElementType(int p_genomic_element_type_id, std::vector<MutationType*> p_mutation_type_ptrs, std::vector<double> p_mutation_fractions);
	
	const MutationType *DrawMutationType() const;
};

// support stream output of GenomicElementType, for debugging
std::ostream &operator<<(std::ostream &p_outstream, const GenomicElementType &p_genomic_element_type);


#endif /* defined(__SLiM__genomic_element_type__) */




































































