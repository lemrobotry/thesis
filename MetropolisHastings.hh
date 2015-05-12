#ifndef _PERMUTE_METROPOLIS_HASTINGS_HH
#define _PERMUTE_METROPOLIS_HASTINGS_HH

#include "Permutation.hh"

namespace Permute {

  // Defines an abstract interface compatible with the Metropolis-Hastings
  // algorithm.  A neighborhood defines the set of permutations adjacent to a
  // given permutation in some implicit graph, and provides methods for
  // computing the total unnormalized score of all adjacent permutations, as
  // well as sampling a single adjacent permutation in proportion to its score.
  class Neighborhood {
  protected:
    Permutation permutation_;
  public:
    Neighborhood (const Permutation & permutation) :
      permutation_ (permutation)
    {}
    virtual ~ Neighborhood () {}
    const Permutation & permutation () const {
      return permutation_;
    }
    // Generates a new Neighborhood of the same type centered about the given
    // permutation.
    virtual Neighborhood * clone (const Permutation & permutation) const = 0;
    // Samples a neighbor according to P(neighbor)/P(Neighborhood) and copies it
    // into the argument.
    virtual void sample (Permutation & neighbor) = 0;
    // Returns the total score of the entire Neighborhood.
    virtual double score () = 0;
  };

  // Manages Markov-chain Monte Carlo permutation sampling using the
  // Neighborhood class.
  class MetropolisHastings {
  private:
    Neighborhood * neighborhood_;
  public:
    MetropolisHastings (Neighborhood * neighborhood) :
      neighborhood_ (neighborhood)
    {}
    ~ MetropolisHastings () {
      delete neighborhood_;
    }
    // Generates a sample permutation and copies it into the argument.
    void sample (Permutation & sample);
  };

}

#endif//_PERMUTE_METROPOLIS_HASTINGS_HH
