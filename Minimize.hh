// Performs heuristic minimization wherein equivalent states are merged, but
// with no consideration for weight pushing or other manipulations of the
// automaton to achieve true minimization.  This function stands in for
// Fsa::minimize, which calls determinize(transpose(determinize(transpose()))),
// and seems to fail frequently.

#ifndef _PERMUTE_MINIMIZE_HH
#define _PERMUTE_MINIMIZE_HH

#include <Fsa/Automaton.hh>

namespace Permute {

  Fsa::ConstAutomatonRef minimize (Fsa::ConstAutomatonRef);

}

#endif//_PERMUTE_MINIMIZE_HH
