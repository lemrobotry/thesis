// Returns a new automaton that contains the portion of the given automaton that
// matches symbols in the given permutation.

#ifndef _PERMUTE_LIMIT_HH
#define _PERMUTE_LIMIT_HH

#include <Fsa/Automaton.hh>
#include "Permutation.hh"

namespace Permute {
  Fsa::ConstAutomatonRef limit (Fsa::ConstAutomatonRef,
				const Permutation &);
}

#endif//_PERMUTE_LIMIT_HH
