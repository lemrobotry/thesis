#ifndef _PERMUTE_FSA_HH
#define _PERMUTE_FSA_HH

#include <Fsa/Best.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Project.hh>

namespace Permute {

  class BestOutput :
    public std::binary_function <Fsa::ConstAutomatonRef, Fsa::ConstAutomatonRef, Fsa::ConstAutomatonRef>
  {
  public:
    Fsa::ConstAutomatonRef operator () (Fsa::ConstAutomatonRef a, Fsa::ConstAutomatonRef b) const {
      return Fsa::projectOutput (Fsa::best (Fsa::composeMatching (a, b)));
    }
  };

}

#endif//_PERMUTE_FSA_HH
