// A FullCell stores the best path covering its span for every pair of start and
// end states.

#ifndef _PERMUTE_FULL_CELL_HH
#define _PERMUTE_FULL_CELL_HH

#include "Cell.hh"

namespace Permute {
  class FullCell : public CellImpl {
  public:
    FullCell ();
    virtual bool add (ConstPathRef);
  };
}

#endif//_PERMUTE_FULL_CELL_HH
