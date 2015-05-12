#ifndef _PERMUTE_CELL_MAP_HH
#define _PERMUTE_CELL_MAP_HH

#include <Core/ReferenceCounting.hh>
#include <Fsa/Types.hh>
#include "Cell.hh"

namespace Permute {
  class CellMap;
  typedef Core::Ref <CellMap> CellMapRef;
  
  // Maps labels from an Fsa::Alphabet to cells.  Provides static methods for
  // caching the results and for generating cells in the absence of an
  // automaton.
  class CellMap : public Core::ReferenceCounted {
  public:
    virtual ConstCellRef operator () (size_t, Fsa::LabelId) = 0;
    static CellMapRef cache (CellMapRef);
    static CellMapRef trivial ();
  };
}

#endif//_PERMUTE_CELL_MAP_HH
