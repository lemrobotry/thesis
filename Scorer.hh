#ifndef _PERMUTE_SCORER_HH
#define _PERMUTE_SCORER_HH

#include <Core/ReferenceCounting.hh>
#include "Permutation.hh"

namespace Permute {

  // Forward declares the ParseController class.
  class ParseController;
  typedef Core::Ref <const ParseController> ParseControllerRef;
  
  // Computes scores for ordering subsequences of permutation: score(i, j, k) is
  // the score for everything in (i, j) preceding everything in (j, k), score(k,
  // j, i) is the opposite, and score(pi) computes the total score of a given
  // permutation.
  class Scorer : public Core::ReferenceCounted {
  public:
    Scorer () : Core::ReferenceCounted () {}
    virtual double score (int, int, int) const;
    virtual double score (const Permutation &) const;
    virtual void compute (const ParseControllerRef &) {};
  };

  typedef Core::Ref <Scorer> ScorerRef;
}

#endif//_PERMUTE_SCORER_HH
