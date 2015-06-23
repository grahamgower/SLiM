//
//  mutation_type.cpp
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


#include <iostream>
#include <gsl/gsl_randist.h>
#include <sstream>

#include "mutation_type.h"
#include "g_rng.h"
#include "slim_global.h"
#include "script_functionsignature.h"


using std::endl;
using std::string;


#ifdef SLIMGUI
MutationType::MutationType(int p_mutation_type_id, double p_dominance_coeff, char p_dfe_type, std::vector<double> p_dfe_parameters, int p_mutation_type_index) :
	mutation_type_id_(p_mutation_type_id), dominance_coeff_(static_cast<typeof(dominance_coeff_)>(p_dominance_coeff)), dfe_type_(p_dfe_type), dfe_parameters_(p_dfe_parameters), mutation_type_index_(p_mutation_type_index)
#else
MutationType::MutationType(int p_mutation_type_id, double p_dominance_coeff, char p_dfe_type, std::vector<double> p_dfe_parameters) :
	mutation_type_id_(p_mutation_type_id), dominance_coeff_(static_cast<typeof(dominance_coeff_)>(p_dominance_coeff)), dfe_type_(p_dfe_type), dfe_parameters_(p_dfe_parameters)
#endif
{
	static string possible_dfe_types = "fge";
	
	if (possible_dfe_types.find(dfe_type_) == string::npos)
		SLIM_TERMINATION << "ERROR (Initialize): invalid mutation type '" << dfe_type_ << "'" << slim_terminate();
	if (dfe_parameters_.size() == 0)
		SLIM_TERMINATION << "ERROR (Initialize): invalid mutation type parameters" << slim_terminate();
}

MutationType::~MutationType(void)
{
	if (self_symbol_)
		delete self_symbol_;
}

double MutationType::DrawSelectionCoefficient() const
{
	switch (dfe_type_)
	{
		case 'f': return dfe_parameters_[0];
		case 'g': return gsl_ran_gamma(g_rng, dfe_parameters_[1], dfe_parameters_[0] / dfe_parameters_[1]);
		case 'e': return gsl_ran_exponential(g_rng, dfe_parameters_[0]);
		default: SLIM_TERMINATION << "ERROR (DrawSelectionCoefficient): invalid DFE type" << slim_terminate(); return 0;
	}
}

std::ostream &operator<<(std::ostream &p_outstream, const MutationType &p_mutation_type)
{
	p_outstream << "MutationType{dominance_coeff_ " << p_mutation_type.dominance_coeff_ << ", dfe_type_ '" << p_mutation_type.dfe_type_ << "', dfe_parameters_ ";
	
	if (p_mutation_type.dfe_parameters_.size() == 0)
	{
		p_outstream << "*";
	}
	else
	{
		p_outstream << "<";
		
		for (int i = 0; i < p_mutation_type.dfe_parameters_.size(); ++i)
		{
			p_outstream << p_mutation_type.dfe_parameters_[i];
			
			if (i < p_mutation_type.dfe_parameters_.size() - 1)
				p_outstream << " ";
		}
		
		p_outstream << ">";
	}
	
	p_outstream << "}";
	
	return p_outstream;
}

//
// SLiMscript support
//

void MutationType::GenerateCachedSymbolTableEntry(void)
{
	std::ostringstream mut_type_stream;
	
	mut_type_stream << "m" << mutation_type_id_;
	
	self_symbol_ = new SymbolTableEntry(mut_type_stream.str(), (new ScriptValue_Object(this))->SetExternallyOwned(true)->SetInSymbolTable(true));
}

std::string MutationType::ElementType(void) const
{
	return gStr_MutationType;
}

void MutationType::Print(std::ostream &p_ostream) const
{
	p_ostream << ElementType() << "<m" << mutation_type_id_ << ">";
}

std::vector<std::string> MutationType::ReadOnlyMembers(void) const
{
	std::vector<std::string> constants = ScriptObjectElement::ReadOnlyMembers();
	
	constants.push_back(gStr_id);						// mutation_type_id_
	constants.push_back(gStr_distributionType);		// dfe_type_
	constants.push_back(gStr_distributionParams);		// dfe_parameters_
	
	return constants;
}

std::vector<std::string> MutationType::ReadWriteMembers(void) const
{
	std::vector<std::string> variables = ScriptObjectElement::ReadWriteMembers();
	
	variables.push_back(gStr_dominanceCoeff);		// dominance_coeff_
	
	return variables;
}

ScriptValue *MutationType::GetValueForMember(const std::string &p_member_name)
{
	// constants
	if (p_member_name.compare(gStr_id) == 0)
		return new ScriptValue_Int(mutation_type_id_);
	if (p_member_name.compare(gStr_distributionType) == 0)
		return new ScriptValue_String(std::string(1, dfe_type_));
	if (p_member_name.compare(gStr_distributionParams) == 0)
		return new ScriptValue_Float(dfe_parameters_);
	
	// variables
	if (p_member_name.compare(gStr_dominanceCoeff) == 0)
		return new ScriptValue_Float(dominance_coeff_);
	
	return ScriptObjectElement::GetValueForMember(p_member_name);
}

void MutationType::SetValueForMember(const std::string &p_member_name, ScriptValue *p_value)
{
	if (p_member_name.compare(gStr_dominanceCoeff) == 0)
	{
		TypeCheckValue(__func__, p_member_name, p_value, kScriptValueMaskInt | kScriptValueMaskFloat);
		
		double value = p_value->FloatAtIndex(0);
		
		dominance_coeff_ = static_cast<typeof(dominance_coeff_)>(value);	// float, at present, but I don't want to hard-code that
		return;
	}
	
	return ScriptObjectElement::SetValueForMember(p_member_name, p_value);
}

std::vector<std::string> MutationType::Methods(void) const
{
	std::vector<std::string> methods = ScriptObjectElement::Methods();
	
	methods.push_back(gStr_changeDistribution);
	
	return methods;
}

const FunctionSignature *MutationType::SignatureForMethod(std::string const &p_method_name) const
{
	static FunctionSignature *changeDistributionSig = nullptr;
	
	if (!changeDistributionSig)
	{
		changeDistributionSig = (new FunctionSignature(gStr_changeDistribution, FunctionIdentifier::kNoFunction, kScriptValueMaskNULL))->SetInstanceMethod()->AddString_S()->AddEllipsis();
	}
	
	if (p_method_name.compare(gStr_changeDistribution) == 0)
		return changeDistributionSig;
	else
		return ScriptObjectElement::SignatureForMethod(p_method_name);
}

ScriptValue *MutationType::ExecuteMethod(std::string const &p_method_name, std::vector<ScriptValue*> const &p_arguments, ScriptInterpreter &p_interpreter)
{
	int num_arguments = (int)p_arguments.size();
	ScriptValue *arg0_value = ((num_arguments >= 1) ? p_arguments[0] : nullptr);
	
	//
	//	*********************	- (void)changeDistribution(string$ distributionType, ...)
	//
#pragma mark -changeDistribution()
	
	if (p_method_name.compare(gStr_changeDistribution) == 0)
	{
		string dfe_type_string = arg0_value->StringAtIndex(0);
		int expected_dfe_param_count = 0;
		std::vector<double> dfe_parameters;
		
		if (dfe_type_string.compare("f") == 0)
			expected_dfe_param_count = 1;
		else if (dfe_type_string.compare("g") == 0)
			expected_dfe_param_count = 2;
		else if (dfe_type_string.compare("e") == 0)
			expected_dfe_param_count = 1;
		else
			SLIM_TERMINATION << "ERROR (MutationType::ExecuteMethod): changeDistribution() distributionType \"" << dfe_type_string << "must be \"f\", \"g\", or \"e\"." << slim_terminate();
		
		char dfe_type = dfe_type_string[0];
		
		if (num_arguments != 1 + expected_dfe_param_count)
			SLIM_TERMINATION << "ERROR (MutationType::ExecuteMethod): changeDistribution() distributionType \"" << dfe_type << "\" requires exactly " << expected_dfe_param_count << " DFE parameter" << (expected_dfe_param_count == 1 ? "" : "s") << "." << slim_terminate();
		
		for (int dfe_param_index = 0; dfe_param_index < expected_dfe_param_count; ++dfe_param_index)
			dfe_parameters.push_back(p_arguments[3 + dfe_param_index]->FloatAtIndex(0));
		
		// Everything seems to be in order, so replace our distribution info with the new info
		dfe_type_ = dfe_type;
		dfe_parameters_ = dfe_parameters;
		
		return ScriptValue_NULL::Static_ScriptValue_NULL_Invisible();
	}
	
	
	else
		return ScriptObjectElement::ExecuteMethod(p_method_name, p_arguments, p_interpreter);
}
































































