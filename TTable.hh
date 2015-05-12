#ifndef _PERMUTE_TTABLE_HH
#define _PERMUTE_TTABLE_HH

#include <PhraseDictionaryTree.h>

#include <Fsa/Alphabet.hh>

#include "Permutation.hh"

namespace Permute {
  class WeightModel {
  public:
    virtual Fsa::Weight weight (const std::string &) const = 0;
    virtual Fsa::Weight weight (const std::vector <float> &) const = 0;
  };

  class TTableWeights : public WeightModel {
  private:
    std::vector <double> weightt_;
  public:
    TTableWeights (std::vector <double> weightt) :
      weightt_ (weightt)
    {}
    virtual Fsa::Weight weight (const std::string & line) const;
    virtual Fsa::Weight weight (const std::vector <float> & v) const;
  };

  // Extract the set of phrases that occur in the given permutation from the
  // given phrase dictionary, thresholded by the last parameter, and construct
  // a transducer accepting those phrases using the given weight model.
  Fsa::ConstAutomatonRef ttable (const Permutation &,
				 Fsa::ConstAlphabetRef,
				 const WeightModel &,
				 const PhraseDictionaryTree &,
				 double);
}

#endif//_PERMUTE_TTABLE_HH
