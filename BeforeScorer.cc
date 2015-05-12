#include "BeforeScorer.hh"
#include "ParseController.hh"
#include <Core/Utility.hh>

namespace Permute {

  BeforeCostInterface::BeforeCostInterface (size_t n) :
    Core::ReferenceCounted (),
    n_ (n)
  {}

  size_t BeforeCostInterface::size () const {
    return n_;
  }

  size_t BeforeCostInterface::index (int i, int j) const {
    return n_ * i + j;
  }

  double BeforeCostInterface::score (const Permutation & pi) const {
    double s = 0.0;
    for (Permutation::const_iterator i = pi.begin (); i != -- pi.end (); ++ i) {
      for (Permutation::const_iterator j = i + 1; j != pi.end (); ++ j) {
	s += cost ((* i), (* j));
      }
    }
    return s;
  }

  /**********************************************************************/

  /**
   * The first line of the file contains a name.
   * The next token is the size, and the remaining tokens contain the
   * scores.
   */
  // @bug Check whether input is successful and do something intelligent if it
  // fails.
  BeforeCost * readLOLIB (std::istream & in) {
    std::string name;
    int size;
    double cost;
    getline (in, name);
    in >> size;
    BeforeCost * bc = new BeforeCost (size, name);
    for (int i = 0; i < size; ++ i) {
      for (int j = 0; j < size; ++ j) {
	in >> cost;
	bc -> setCost (i, j, cost);
      }
    }
    return bc;
  }

  /**********************************************************************/
  
  BeforeCost::BeforeCost (size_t n, const std::string & name) :
    BeforeCostInterface (n),
    matrix_ (n * n, 0.0),
    name_ (name)
  {}

  double BeforeCost::cost (int i, int j) const {
    return matrix_ [index (i, j)];
  }

  const std::string & BeforeCost::name () const {
    return name_;
  }

  void BeforeCost::setCost (int i, int j, double cost) {
    size_t position = index (i, j);
    matrix_.grow (position, 0.0);
    matrix_ [position] = cost;
  }

  // Makes zero an admissible heuristic for this LOP.  Returns the amount that
  // should be added to each normalized LOP score to arrive at the original LOP
  // score.
  double BeforeCost::normalize () {
    double excess = 0;
    for (int i = 0; i < size (); ++ i) {
      for (int j = i + 1; j < size (); ++ j) {
	double m = std::max (cost (i, j), cost (j, i));
	setCost (i, j, cost (i, j) - m);
	setCost (j, i, cost (j, i) - m);
	excess += m;
      }
    }
    return excess;
  }

  /**********************************************************************/

  BeforeScorer::BeforeScorer (const BeforeCostRef & cost, const Permutation & pi) :
    Scorer (),
    cost_ (cost),
    permutation_ (pi),
    n_ (pi.size ()),
    index_ (n_, 0),
    keep_ (n_ * (n_ * n_ - 1) / 6, -1e500),
    swap_ (keep_)
  {
    for (int i = 0; i < n_ - 1; ++ i) {
      index_ [i + 1] = index_ [i] + binomial (n_ - i);
    }
  }

  double BeforeScorer::score (int i, int j, int k) const {
    if (i == j || j == k) {
      return 0.0;
    } else if (i < k) {
      return keep_ [index (i, j, k)];
    } else {
      return swap_ [index (k, j, i)];
    }
  }

  double BeforeScorer::cost (int i, int k) const {
    return cost_ -> cost (permutation_ [i], permutation_ [k]);
  }

  int BeforeScorer::index (int i, int j, int k) const {
    return index_ [i]
      + (j - i - 1) * (2 * n_ - i - j) / 2 // (j-i-1)(n_-i) - binom(j-i)
      + (k - j - 1);
  }

  int BeforeScorer::binomial (int n) {
    return (n * (n - 1) / 2);
  }

  double BeforeScorer::score (const Permutation & pi) const {
    return cost_ -> score (pi);
  }

  void BeforeScorer::compute (const ParseControllerRef & controller) {
    for (int span = 2; span <= n_; ++ span) {
      for (int i = 0; i <= n_ - span; ++ i) {
	int k = i + span;
	for (ParseController::iterator j = controller -> begin (i, k);
	     j != controller -> end (i, k); ++ j) {
	  int index = this -> index (i, j, k);
 	  keep_ [index] = controller -> grammar (* this, i, j, k);
 	  swap_ [index] = controller -> grammar (* this, k, j, i);
	}
      }
    }
  }

  double BeforeScorer::compute (const ParseControllerRef & controller,
				int i, int j, int k) {
    if (i < k) {
      return keep_ [index (i, j, k)] = controller -> grammar (* this, i, j, k);
    } else {
      return swap_ [index (k, j, i)] = controller -> grammar (* this, i, j, k);
    }
  }

  /**********************************************************************/

  BeforeCostRef tauCost (const Permutation & target) {
    BeforeCost * bc (new BeforeCost (target.size (), "tauScorer"));
    for (Permutation::const_iterator i = target.begin (); i != -- target.end (); ++ i) {
      for (Permutation::const_iterator j = i + 1; j != target.end (); ++ j) {
	if (* i < * j) {
	  bc -> setCost (* i, * j, 1.0);
	} else {
	  bc -> setCost (* j, * i, -1.0);
	}
      }
    }
    return BeforeCostRef (bc);
  }

  ScorerRef tauScorer (const Permutation & target, const Permutation & pi) {
    return ScorerRef (new BeforeScorer (tauCost (target), pi));
  }
}
