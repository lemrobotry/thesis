#ifndef _PERMUTE_LINEAR_ORDERING_HH
#define _PERMUTE_LINEAR_ORDERING_HH

#include "BeforeScorer.hh"
#include "MetropolisHastings.hh"
#include "Permutation.hh"

namespace Permute {

  double search_insert (Permutation & pi, const BeforeCostRef & bc);
  double visit (Permutation & pi, const BeforeCostRef & bc);
  void insert (Permutation & pi, int i, int j);

  double becker_greedy (Permutation & pi, const BeforeCostRef & bc);

  double block_lsf (Permutation & pi, const BeforeCostRef & bc, int max_width);
  void insert (Permutation & pi, int i, int j, int k);

  // Implements a neighborhood of size N - 1, consisting of the permutations
  // achieved by transposing a pair of adjacent indices.
  class AdjacentTransposition : public Neighborhood {
  private:
    const BeforeCostRef & cost_;
    std::vector <double> totals_;
    double alpha_;
  public:
    AdjacentTransposition (const Permutation & permutation,
			   const BeforeCostRef & cost,
			   double alpha = 1.0);
    virtual AdjacentTransposition * clone (const Permutation & permutation) const;
    virtual void sample (Permutation & neighbor);
    virtual double score ();
  };

  double search_adjacent (Permutation & pi, const BeforeCostRef & bc);

  // 
  class QuadraticNeighborhood : public Neighborhood {
  private:
    double alpha_;
  public:
    QuadraticNeighborhood (const Permutation & permutation,
			   /* . . . , */
			   double alpha = 1.0);
    virtual QuadraticNeighborhood * clone (const Permutation & permutation) const;
    virtual void sample (Permutation & neighbor);
    virtual double score ();
  };
}

#endif//_PERMUTE_LINEAR_ORDERING_HH
