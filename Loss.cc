#include "Loss.hh"

namespace Permute {

  Loss::Loss () :
    bleu_ (),
    adjacent_ (0.0),
    tau_ (0.0)
  {}

  void Loss::add (const Permutation & ref, const Permutation & cand) {
    bleu_ += computeBleuScore (ref, cand);
    AdjacentLoss adj (ref);
    adjacent_ += adj.score (cand);
    tau_ += tauScorer (ref, cand) -> score (cand);
  }

  std::ostream & operator << (std::ostream & out, const Loss & loss) {
    return out << "BLEU = " << loss.bleu_ << std::endl
	       << "Adjacent = " << loss.adjacent_ << std::endl
	       << "Tau = " << loss.tau_;
  }
  
}
