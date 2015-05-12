#ifndef _PERMUTE_BLOCK_SEARCH_HH
#define _PERMUTE_BLOCK_SEARCH_HH

#include "Permutation.hh"
#include "Chart.hh"
#include "BeforeScorer.hh"

namespace Permute {
  // Block LS_f is the local search method with the best performance on the
  // XLOLIB benchmarks.  This method adapts Block LS_f to score an A + B model
  // instead of simply a B model.
  double blockSearch (Permutation & pi,
		      const BeforeCostRef & bc,
		      int max_width,
		      Fsa::ConstAutomatonRef fsa,
		      CellMapRef cellMap,
		      CellRef topCell,
		      ConstCellRef epsilonClosure);
}

#endif//_PERMUTE_BLOCK_SEARCH_HH
