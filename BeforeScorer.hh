#ifndef _PERMUTE_BEFORE_SCORER_HH
#define _PERMUTE_BEFORE_SCORER_HH

#include <map>
#include <Fsa/Vector.hh>
#include "Permutation.hh"
#include "Scorer.hh"

namespace Permute {

  // BeforeCostInterface acts as a matrix of linear ordering scores.  The size
  // method returns the dimension n of the n x n matrix.  The cost(i,j) method
  // returns the (i,j)th entry in the matrix.  The index(i,j) method is a
  // convenience method for subclasses that returns the position in an array of
  // size n*n of the (i,j)th entry.
  class BeforeCostInterface : public Core::ReferenceCounted {
  private:
    size_t n_;
  public:
    BeforeCostInterface (size_t);
    virtual size_t size () const;
    virtual double cost (int, int) const = 0;
    virtual double score (const Permutation &) const;
    virtual size_t index (int, int) const;
    virtual const std::string & name () const = 0;
  };

  typedef Core::Ref <BeforeCostInterface> BeforeCostRef;

  /**********************************************************************/

  // BeforeCost implements the BeforeCostInterface using a vector of doubles.
  // It adds a setCost method and a normalize method.  The latter makes zero an
  // admissible heuristic for the LOP by setting max(B[i,j], B[j,i]) to zero.
  class BeforeCost : public BeforeCostInterface {
  private:
    Fsa::Vector <double> matrix_;
    std::string name_;
  public:
    BeforeCost (size_t, const std::string & name);
    virtual double cost (int, int) const;
    virtual const std::string & name () const;
    void setCost (int, int, double);
    double normalize ();
  };

  BeforeCost * readLOLIB (std::istream &);

  /**********************************************************************/
  
  // BeforeScorer implements the Scorer interface with LOP costs.  The method
  // score(i,j,k) returns the total LOP cost of putting (i,j) before (j,k) if i
  // < k, or the opposite if k < i.  Caches results to achieve constant time
  // look-up.
  class BeforeScorer : public Scorer {
  private:
    BeforeCostRef cost_;
    const Permutation & permutation_;
    int n_;
    std::vector <int> index_;
    std::vector <double> keep_;
    std::vector <double> swap_;
  public:
    BeforeScorer (const BeforeCostRef & cost, const Permutation & pi);

    virtual double score (int, int, int) const;
    virtual double score (const Permutation &) const;
    virtual void compute (const ParseControllerRef &);
    double compute (const ParseControllerRef &, int, int, int);

    int size () const { return n_; }
    double cost (int, int) const;
    int index (int, int, int) const;
    static int binomial (int);
  };

  /**********************************************************************/

  BeforeCostRef tauCost (const Permutation & target);
  ScorerRef tauScorer (const Permutation & target, const Permutation & pi);
}

#endif//_PERMUTE_BEFORE_SCORER_HH
