// A StartEndCell is a BestCell that requires that its paths begin at a state in
// the epsilon closure of the start state and end at a final state,
// incorporating corresponding weights.

#ifndef _PERMUTE_START_END_CELL_HH
#define _PERMUTE_START_END_CELL_HH

#include <Fsa/Automaton.hh>
#include "BestCell.hh"

namespace Permute {
  class StartEndCell : public BestCell {
  private:
    Fsa::ConstAutomatonRef fsa_;
    ConstCellRef epsilonClosure_;
  public:
    // The first parameter is the automaton whose initial and final states must
    // be matched.  The second parameter must contain the epsilon closure of the
    // given automaton.
    StartEndCell (Fsa::ConstAutomatonRef, ConstCellRef);
    virtual bool add (ConstPathRef);
  };
}

#endif//_PERMUTE_START_END_CELL_HH
