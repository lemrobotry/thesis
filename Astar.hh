// NOTE: Not in use.
//
// Provides an alternative to Chart's permute method which performs A* search
// using a priority queue rather than pruning.

#ifndef _PERMUTE_A_STAR_HH
#define _PERMUTE_A_STAR_HH

#include "Chart.hh"
#include "Outside.hh"

namespace Permute {

  class Astar {
  public:
    static void permute (ChartRef, OutsideRef, ParseControllerRef, ScorerRef);
  };

}

#endif//_PERMUTE_A_STAR_HH
