#ifndef _PERMUTE_EPSILON_CLOSURE_HH
#define _PERMUTE_EPSILON_CLOSURE_HH

#include "Cell.hh"

namespace Permute {
  // We can extract all of the epsilon arcs from an automaton into a
  // Cell using CellMap (Fsa::Epsilon).

  // Using this Cell, we can compute the set of epsilon closures (of
  // all states) by combining it with itself until convergence.

  // Detect convergence using the return value of Cell::add.

  ConstCellRef epsilonClosure (const ConstCellRef &);
}

#endif//_PERMUTE_EPSILON_CLOSURE_HH
