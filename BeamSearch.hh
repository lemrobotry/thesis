// Performs a fixed-width beam search, ignoring epsilon arcs, on a given
// automaton.  Uses an inverted priority queue (worst item at the top) to
// maintain the fixed-width best partial paths of successive lengths in the
// automaton.  Constructs an automaton equivalent to the best complete path
// (subject to search error) and returns it along with its weight.

#ifndef _PERMUTE_BEAM_SEARCH_HH
#define _PERMUTE_BEAM_SEARCH_HH

#include <utility>
#include <Fsa/Automaton.hh>

namespace Permute {
  std::pair <Fsa::ConstAutomatonRef, float> beamSearch (Fsa::ConstAutomatonRef, int);
}

#endif//_PERMUTE_BEAM_SEARCH_HH
