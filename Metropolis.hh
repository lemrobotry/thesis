// NOTE: Not in use.  Implements the Metropolis-Hastings algorithm for sampling
// permutations using the graph implied by the very-large scale ITG permutation
// neighborhood.

#ifndef _PERMUTE_METROPOLIS_HH
#define _PERMUTE_METROPOLIS_HH

#include "ChartFactory.hh"
#include "Parameter.hh"

namespace Permute {

  // @bug This should be adapted to use lazy evaluation similar to k-best
  // parsing rather than storing in PathLists all the different ways of
  // generating a path with the same (start, end) state pair.
  class Metropolis_Hastings {
  protected:
    double f_;
    Permutation * pi_, * pi_star_;
    ChartRef chart_, chart_star_;
    ParseControllerRef controller_;
    ScorerRef scorer_;
  public:
    Metropolis_Hastings (const Permutation &, ChartFactoryRef, ParseControllerRef, ScorerRef);
    ~Metropolis_Hastings ();
    void sample ();
    void count (FeatureCounter &);
  };
  
}

#endif//_PERMUTE_METROPOLIS_HH
