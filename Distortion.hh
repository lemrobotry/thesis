// Constructs a standard unlexicalized distortion model automaton, which is
// essentially a bigram model that applies cost d |j - i - 1|, where d is the
// parameter, to moving from word i to word j.

#ifndef _PERMUTE_DISTORTION_HH
#define _PERMUTE_DISTORTION_HH

#include "Permutation.hh"

namespace Permute {

  Fsa::ConstAutomatonRef distortion (const Permutation &, double);
  
}

#endif//_PERMUTE_DISTORTION_HH
