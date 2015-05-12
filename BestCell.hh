// A BestCell keeps track only of the best path covering its span, in contrast
// to FullCell, which stores the best path covering the span with each start and
// end state.

#ifndef _PERMUTE_BEST_CELL_HH
#define _PERMUTE_BEST_CELL_HH

#include "Cell.hh"

namespace Permute {
  class BestCell : public CellImpl {
  public:
    BestCell ();
    virtual bool add (ConstPathRef);
  };
}

#endif//_PERMUTE_BEST_CELL_HH
