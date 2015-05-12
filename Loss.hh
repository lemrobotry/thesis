#ifndef _PERMUTE_LOSS_HH
#define _PERMUTE_LOSS_HH

#include "AdjacentLoss.hh"
#include "BeforeScorer.hh"
#include "BleuScore.hh"

namespace Permute {

  class Loss {
    friend std::ostream & operator << (std::ostream &, const Loss &);
  private:
    BleuScore bleu_;
    double adjacent_;
    double tau_;
  public:
    Loss ();
    void add (const Permutation & ref, const Permutation & cand);
  };

}

#endif//_PERMUTE_LOSS_HH
