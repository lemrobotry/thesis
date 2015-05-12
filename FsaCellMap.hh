#ifndef _PERMUTE_FSA_CELL_MAP_HH
#define _PERMUTE_FSA_CELL_MAP_HH

#include <utility>
#include <Fsa/Automaton.hh>
#include "CellMap.hh"

namespace Permute {
  // Maps labels to cells containing all arcs in the given automaton with
  // matching input label, as well as their epsilon closures.
  class FsaCellMap : public CellMap {
  protected:
    Fsa::ConstAutomatonRef fsa_;
    ConstCellRef closure_;
  public:
    FsaCellMap (Fsa::ConstAutomatonRef);
    virtual ConstCellRef operator () (size_t, Fsa::LabelId);
    ConstCellRef epsilonClosure () const;
    static std::pair <CellMapRef, CellRef> mapFsa (Fsa::ConstAutomatonRef);
  };

  // Maps labels to cells containing arcs from state 0 to state 0 with the
  // weight of the best arc in the given automaton with matching input label.
  // This reduces a potentially complex automaton into a single-state unigram
  // automaton that can be used for generating A* estimates.
  class UnigramFsaCellMap : public FsaCellMap {
  public:
    UnigramFsaCellMap (Fsa::ConstAutomatonRef);
    virtual ConstCellRef operator () (size_t, Fsa::LabelId);
    static std::pair <CellMapRef, CellRef> mapFsa (Fsa::ConstAutomatonRef);
  };
}

#endif//_PERMUTE_FSA_CELL_MAP_HH

