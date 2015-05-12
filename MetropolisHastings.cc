#include "Log.hh"
#include "MetropolisHastings.hh"

namespace Permute {

  // Uses the Neighborhood to generate a sample, then decides whether to accept
  // it using the ratio of Neighborhood scores:
  //
  //   A(y|x) = min(1, p(N(x)) / p(N(y)))
  void MetropolisHastings::sample (Permutation & sample) {
    neighborhood_ -> sample (sample);
    Neighborhood * sample_neighborhood = neighborhood_ -> clone (sample);

    double acceptance = Log::divide (neighborhood_ -> score (),
				     sample_neighborhood -> score ());
    if (acceptance > Log::One || Log::bernoulli (acceptance, Log::One)) {
      std::swap (neighborhood_, sample_neighborhood);
    } else {
      sample = neighborhood_ -> permutation ();
    }

    delete sample_neighborhood;
  }

}
