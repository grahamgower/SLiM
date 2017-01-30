//
//  genome.h
//  SLiM
//
//  Created by Ben Haller on 12/13/14.
//  Copyright (c) 2014-2016 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/slim/
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
 
 The class Genome represents a particular genome, defined as a vector of mutations.  Each individual in the simulation has a genome,
 which determines that individual's fitness (from the fitness effects of all of the mutations possessed).
 
 */

#ifndef __SLiM__genome__
#define __SLiM__genome__


#include "mutation.h"
#include "slim_global.h"
#include "eidos_value.h"
#include "chromosome.h"
#include "mutation_run.h"

#include <vector>
#include <string.h>


extern EidosObjectClass *gSLiM_Genome_Class;


// Genome now keeps a C array of MutationRun objects, and those objects actually hold the mutations of the Genome.  This design
// allows multiple Genome objects to share the same runs of mutations, for speed in copying runs during offspring generation.
// The maximum number of mutation runs is determined at compile time; the actual number of runs kept by a simulation can
// be anywhere between 1 and that maximum, as determined by the dynamics of the simulation.
// NOTE: presently only a value of 1 is supported by SLiM's code; this is a work in progress!
#define SLIM_GENOME_MUTRUN_MAX	1


class Genome : public EidosObjectElement
{
	// This class has a restricted copying policy; see below
	
	EidosValue_SP self_value_;									// cached EidosValue object for speed
	
private:
	
	GenomeType genome_type_ = GenomeType::kAutosome;			// SEX ONLY: the type of chromosome represented by this genome
	
	int32_t run_count_;											// number of runs being used; 0 for a null genome, otherwise >= 1
	MutationRun_SP runs_[SLIM_GENOME_MUTRUN_MAX];				// runs of mutations; the first run_count_ entries should never be nullptr
	
	slim_usertag_t tag_value_;									// a user-defined tag value
	
#ifdef DEBUG
	static bool s_log_copy_and_assign_;							// true if logging is disabled (see below)
#endif
	
	// Bulk operation optimization; see WillModifyRunForBulkOperation().  The idea is to keep track of changes to MutationRun
	// objects in a bulk operation, and short-circuit the operation for all Genomes with the same initial MutationRun (since
	// the bulk operation will produce the same product MutationRun given the same initial MutationRun).
	static int64_t s_bulk_operation_id;
	static std::map<MutationRun*, MutationRun*> s_bulk_operation_runs;
	
public:
	
	//
	//	This class should not be copied, in general, but the default copy constructor and assignment operator cannot be entirely
	//	disabled, because we want to keep instances of this class inside STL containers.  We therefore override the default copy
	//	constructor and the default assignment operator to log whenever they are called.  This is intended to reduce the risk of
	//	unintentional copying.  Logging can be disabled by bracketing with LogGenomeCopyAndAssign() when appropriate, or by
	//	using copy_from_genome(), which is the preferred way to intentionally copy a Genome.
	//
	Genome(const Genome &p_original);
	Genome& operator= (const Genome &p_original);
#ifdef DEBUG
	static bool LogGenomeCopyAndAssign(bool p_log);		// returns the old value; save and restore that value!
#endif
	
	Genome(void);											// default constructor; gives a non-null genome of type GenomeType::kAutosome
	Genome(MutationRun *p_run);								// supply a custom mutation run
	Genome(GenomeType p_genome_type_, bool p_is_null);		// a constructor for parent/child genomes, particularly in the SEX ONLY case
	Genome(enum GenomeType p_genome_type_, bool p_is_null, MutationRun *p_run);		// SEX ONLY case with a supplied mutation run
	~Genome(void);
	
	void NullGenomeAccessError(void) const __attribute__((__noreturn__)) __attribute__((cold));		// prints an error message, a stacktrace, and exits; called only for DEBUG
	
	inline bool IsNull(void) const									// returns true if the genome is a null (placeholder) genome, false otherwise
	{
		return (run_count_ == 0);
	}
	
	// This should be called before modifying the run at a given index.  It will replicate the run to produce a single-referenced copy
	// if necessary, thus guaranteeting that the run can be modified legally.  If the run is already single-referenced, it is a no-op.
	void WillModifyRun(int p_run_index);
	
	// This is an alternate version of WillModifyRun().  It labels the upcoming modification as being the result of a bulk operation
	// being applied across multiple genomes, such that identical input genomes will produce identical output genomes, such as adding
	// the same mutation to all target genomes.  It returns T if the caller needs to actually perform the operation on this genome,
	// or F if this call performed the run for the caller (because the operation had already been performed on an identical genome).
	// The goal is that genomes that share the same mutation run should continue to share the same mutation run after being processed
	// by a bulk operation using this method.  A bit strange, but potentially important for efficiency.  Note that this method knows
	// about the operation being performed; it just plays around with MutationRun pointers, recognizing when they are identical.  The
	// first call for a new operation ID will always return T, and the caller will then perform the operation; subsequent calls for
	// genomes with the same starting MutationRun will substitute the same final MutationRun and return F.
	static void BulkOperationStart(int64_t p_operation_id);
	bool WillModifyRunForBulkOperation(int p_run_index, int64_t p_operation_id);
	static void BulkOperationEnd(int64_t p_operation_id);
	
	GenomeType Type(void) const										// returns the type of the genome: automosomal, X chromosome, or Y chromosome
	{
		return genome_type_;
	}
	
	void RemoveFixedMutations(slim_refcount_t p_fixed_count, int64_t p_operation_id);		// Remove all mutations with a refcount of p_fixed_count, indicating that they have fixed
	
	// This counts up the total MutationRun references, using their usage counts, as a checkback
	inline void TallyGenomeReferences(slim_refcount_t *p_genome_tally, slim_refcount_t *p_mutrun_tally, int64_t p_operation_id)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		if (runs_[0]->operation_id_ != p_operation_id)
		{
			(*p_genome_tally) += runs_[0]->use_count();
			(*p_mutrun_tally)++;
			runs_[0]->operation_id_ = p_operation_id;
		}
	}
	
	// This tallies up individual Mutation references, using MutationRun usage counts for speed
	void TallyMutationReferences(int64_t p_operation_id);
	
	inline Mutation *const & operator[] (int p_index) const			// [] returns a reference to a pointer to Mutation; this is the const-pointer variant
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		return (*runs_[0].get())[p_index];
	}
	
	inline Mutation *& operator[] (int p_index)						// [] returns a reference to a pointer to Mutation; this is the non-const-pointer variant
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		return (*runs_[0].get())[p_index];
	}
	
	inline int size(void) const
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		return runs_[0]->size();
	}
	
	inline void clear(void)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		if (runs_[0]->size() != 0)
		{
			// If the MutationRun is private to us, we can just empty it out, otherwise we replace it with a new empty one
			if (runs_[0]->use_count() == 1)
				runs_[0]->clear();
			else
				runs_[0] = MutationRun_SP(MutationRun::NewMutationRun());
		}
	}
	
	inline void set_to_run(MutationRun *p_run)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		// This is used by Population::ClearParentalGenomes() to clear out all references to MutationRun objects
		// in the parental generation, so that the refcounts of MutationRuns reflect their usage count in the
		// child generation, so those refcounts can be used for fast tallying of mutations.
		runs_[0] = MutationRun_SP(p_run);
	}
	
	inline bool contains_mutation(const Mutation *p_mutation)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		return runs_[0]->contains_mutation(p_mutation);
	}
	
	inline void pop_back(void)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		runs_[0]->pop_back();
	}
	
	inline void emplace_back(Mutation *p_mutation)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		runs_[0]->emplace_back(p_mutation);
	}
	
	inline void emplace_back_bulk(Mutation **p_mutation_ptr, long p_copy_count)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		runs_[0]->emplace_back_bulk(p_mutation_ptr, p_copy_count);
	}
	
	inline void insert_sorted_mutation(Mutation *p_mutation)
	{
		runs_[0]->insert_sorted_mutation(p_mutation);
	}
	
	inline void insert_sorted_mutation_if_unique(Mutation *p_mutation)
	{
		runs_[0]->insert_sorted_mutation_if_unique(p_mutation);
	}
	
	bool _enforce_stack_policy_for_addition(slim_position_t p_position, MutationType *p_mut_type_ptr, MutationStackPolicy p_policy);
	
	inline bool enforce_stack_policy_for_addition(slim_position_t p_position, MutationType *p_mut_type_ptr)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		MutationStackPolicy policy = p_mut_type_ptr->stack_policy_;
		
		if (policy == MutationStackPolicy::kStack)
		{
			// If mutations are allowed to stack (the default), then we have no work to do and the new mutation is always added
			return true;
		}
		else
		{
			// Otherwise, a relatively complicated check is needed, so we call out to a non-inline function
			return _enforce_stack_policy_for_addition(p_position, p_mut_type_ptr, policy);
		}
	}
	
	inline void copy_from_genome(const Genome &p_source_genome)
	{
		if (p_source_genome.IsNull())
		{
			// p_original is a null genome, so make ourselves null too, if we aren't already
			if (run_count_)
			{
				runs_[0].reset();
				run_count_ = 0;
			}
		}
		else
		{
#ifdef DEBUG
			if (run_count_ == 0)
				NullGenomeAccessError();
#endif
			runs_[0] = p_source_genome.runs_[0];
		}
		
		// and copy other state
		genome_type_ = p_source_genome.genome_type_;
	}
	
	inline Mutation *const *begin_pointer_const(void) const
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		return runs_[0]->begin_pointer_const();
	}
	
	inline Mutation *const *end_pointer_const(void) const
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		return runs_[0]->end_pointer_const();
	}
	
	inline Mutation **begin_pointer(void)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		return runs_[0]->begin_pointer();
	}
	
	inline Mutation **end_pointer(void)
	{
#ifdef DEBUG
		if (run_count_ == 0)
			NullGenomeAccessError();
#endif
		return runs_[0]->end_pointer();
	}
	
	// print the sample represented by genomes, using SLiM's own format
	static void PrintGenomes_slim(std::ostream &p_out, std::vector<Genome *> &genomes, slim_objectid_t p_source_subpop_id);
	
	// print the sample represented by genomes, using "ms" format
	static void PrintGenomes_ms(std::ostream &p_out, std::vector<Genome *> &genomes, const Chromosome &p_chromosome);
	
	// print the sample represented by genomes, using "vcf" format
	static void PrintGenomes_vcf(std::ostream &p_out, std::vector<Genome *> &genomes, bool p_output_multiallelics);
	
	
	//
	// Eidos support
	//
	void GenerateCachedEidosValue(void);
	inline EidosValue_SP CachedEidosValue(void) { if (!self_value_) GenerateCachedEidosValue(); return self_value_; };
	
	virtual const EidosObjectClass *Class(void) const;
	virtual void Print(std::ostream &p_ostream) const;
	
	virtual EidosValue_SP GetProperty(EidosGlobalStringID p_property_id);
	virtual void SetProperty(EidosGlobalStringID p_property_id, const EidosValue &p_value);
	virtual EidosValue_SP ExecuteInstanceMethod(EidosGlobalStringID p_method_id, const EidosValue_SP *const p_arguments, int p_argument_count, EidosInterpreter &p_interpreter);
	
	// Accelerated property access; see class EidosObjectElement for comments on this mechanism
	virtual eidos_logical_t GetProperty_Accelerated_Logical(EidosGlobalStringID p_property_id);
	virtual int64_t GetProperty_Accelerated_Int(EidosGlobalStringID p_property_id);
	
	// Accelerated property writing; see class EidosObjectElement for comments on this mechanism
	virtual void SetProperty_Accelerated_Int(EidosGlobalStringID p_property_id, int64_t p_value);
	
	friend class Genome_Class;
};


#endif /* defined(__SLiM__genome__) */




































































