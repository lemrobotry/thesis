#ifndef _PERMUTE_DEPENDENCY_ORDER_HH
#define _PERMUTE_DEPENDENCY_ORDER_HH

#include "Permutation.hh"
#include "PV.hh"

namespace Permute {
  // Reorders the source permutation using the given parameters, but respecting
  // the dependency parse implied by the given parents.
  void dependencyOrder (BeforeCostRef bc,
			Permutation & source,
			const std::vector <int> & parents);
}

#endif//_PERMUTE_DEPENDENCY_ORDER_HH
