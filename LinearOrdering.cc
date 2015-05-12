#include <algorithm>
#include <numeric>

#include "LinearOrdering.hh"
#include "Log.hh"
#include "ParseController.hh"

namespace Permute {

  // Implements greedy search of the insert neighborhood.
  double search_insert (Permutation & pi, const BeforeCostRef & bc) {
    int max_i;
    int max_r;
    double max_delta = 0.0;
    for (int i = 0; i < pi.size (); ++ i) {
      double delta = 0.0;
      // Looks to the left of i.
      for (int r = i - 1; r >= 0; -- r) {
	delta -= bc -> cost (pi [r], pi [i]);
	delta += bc -> cost (pi [i], pi [r]);
	if (delta > max_delta) {
	  max_i = i;
	  max_r = r;
	  max_delta = delta;
	}
      }
      delta = 0.0;
      // Looks to the right of i.
      for (int r = i + 1; r < pi.size (); ++ r) {
	delta -= bc -> cost (pi [i], pi [r]);
	delta += bc -> cost (pi [r], pi [i]);
	if (delta > max_delta) {
	  max_i = i;
	  max_r = r;
	  max_delta = delta;
	}
      }
    }
    // Returns the new permutation if it is better.
    if (max_delta > 0.0) {
      insert (pi, max_i, max_r);
    }
    return max_delta;
  }
      

  // Implements the LS_f search described in Schiavinotto & Stützle (2004).  This
  // search efficiently computes the best permutation in the neighborhood of
  // insert moves reachable from the current permutation, by considering moves in
  // an order that allows changes in cost to be accumulated using simple swap
  // moves.
  //
  // function visit (\pi)
  //   for i = 0 .. n - 1 do
  //     \bar{r} = \arg \max_{r != i} f(insert(\pi, i, r))
  //     \pi' = insert(\pi, i, \bar{r})
  //     if f(\pi') > f(\pi) then
  //       return \pi'
  //     end if
  //   end for
  //   return \pi
  //
  // Returns whether a better permutation was found.
  double visit (Permutation & pi, const BeforeCostRef & bc) {
    for (int i = 0; i < pi.size (); ++ i) {
      int max_r = i;
      double max_delta = 0.0;
      double delta = 0.0;
      // Looks to the left of i.
      for (int r = i - 1; r >= 0; -- r) {
	delta -= bc -> cost (pi [r], pi [i]);
	delta += bc -> cost (pi [i], pi [r]);
	if (delta > max_delta) {
	  max_r = r;
	  max_delta = delta;
	}
      }
      delta = 0.0;
      // Looks to the right of i.
      for (int r = i + 1; r < pi.size (); ++ r) {
	delta -= bc -> cost (pi [i], pi [r]);
	delta += bc -> cost (pi [r], pi [i]);
	if (delta > max_delta) {
	  max_r = r;
	  max_delta = delta;
	}
      }
      // Returns the new permutation if it is better.
      if (max_delta > 0.0) {
	insert (pi, i, max_r);
	return max_delta;
      }
    }
    return 0.0;
  }

  // Moves pi[i] to position j and shifts everything between.
  void insert (Permutation & pi, int i, int j) {
    for (int d = (i < j) ? 1 : -1; i != j; i += d) {
      std::swap (pi [i], pi [i + d]);
    }
  }

  class Ratio {
  public:
    int i;
    double first;
    double last;
    bool used;
    Ratio (int i, double first, double last) :
      i (i),
      first (first),
      last (last),
      used (false)
    {}
    // Since some LOLIB scores are negative, the ratio is not well defined.  Uses
    // the difference instead.
    double ratio () const {
      if (used) {
	return Core::Type <double>::min;
      } else {
	return first - last;
      }
    }
  };

  class LessRatio {
  public:
    bool operator () (const Ratio & a, const Ratio & b) const {
      return a.ratio () < b.ratio ();
    }
  };

  // Implements Becker's (1967) greedy algorithm for proposing a LOP solution.
  // Chooses for pi_1 the element i that maximizes the ratio of the sum over j
  // of B[i,j] to the sum over j of B[j,i], and then recursively applies the
  // same rule to the remaining n - 1 items.  Returns the score of the
  // permutation.
  //
  // First computes the numerator and denominator for each item, while
  // simultaneously recording the maximum ratio.
  //
  // For each remaining element, subtracts the effect of the last removed item
  // from each numerator and denominator, while simultaneously recording the
  // maximum ratio.
  double becker_greedy (Permutation & pi, const BeforeCostRef & bc) {
    double score = 0;
    std::vector <Ratio> items (pi.size (), Ratio (0, 0, 0));
    for (Permutation::const_iterator i = pi.begin (); i != pi.end (); ++ i) {
      items [* i].i = * i;
      for (Permutation::const_iterator j = i + 1; j != pi.end (); ++ j) {
	items [* i].first += bc -> cost (* i, * j);
	items [* i].last += bc -> cost (* j, * i);
	items [* j].first += bc -> cost (* j, * i);
	items [* j].last += bc -> cost (* i, * j);
      }
    }
    for (Permutation::iterator pi_it = pi.begin (); pi_it != pi.end (); ++ pi_it) {
      // Finds the maximum ratio in the items:
      std::vector <Ratio>::iterator best =
	std::max_element (items.begin (), items.end (), LessRatio ());
      * pi_it = best -> i;
      score += best -> first;
      best -> used = true;
      // Updates the first and last scores of the remaining items:
      for (std::vector <Ratio>::iterator it = items.begin (); it != items.end (); ++ it) {
	if (! it -> used) {
	  it -> first -= bc -> cost (it -> i, best -> i);
	  it -> last -= bc -> cost (best -> i, it -> i);
	}
      }
    }
    return score;
  }

  ////////////////////////////////////////////////////////////////////////////////

  typedef std::vector <double> V1D;
  typedef std::vector <V1D> V2D;
  typedef std::vector <V2D> V3D;

  // Would it be faster to use a static vector instead of reallocating it each
  // time?
  double block_lsf (Permutation & pi, const BeforeCostRef & bc, int max_width) {
    max_width = std::min (max_width, static_cast <int> (pi.size ()) / 2);
    V3D delta (max_width);
    for (int w = 0; w < max_width; ++ w) {
      V2D & dw = delta [w];
      dw.resize (pi.size () - w - 1);
      for (int i = 0; i < dw.size (); ++ i) {
	V1D & dwi = dw [i];
	dwi.resize (pi.size () + 1, 0.0);
	int j = i + w + 1;
	for (int k = 0; k < i; ++ k) { // (k, i, j)
	  dwi [k] =
	    bc -> cost (pi [j - 1], pi [k]) -
	    bc -> cost (pi [k], pi [j - 1]);
	}
	std::partial_sum (dwi.rend () - i, dwi.rend (),
			  dwi.rend () - i);
	if (w > 0) {
	  std::transform (dwi.begin (), dwi.begin () + i,
			  delta [w - 1] [i].begin (),
			  dwi.begin (),
			  std::plus <double> ());
	}
	for (int k = j + 1; k <= pi.size (); ++ k) { // (i, j, k)
	  dwi [k] =
	    bc -> cost (pi [k - 1], pi [i]) -
	    bc -> cost (pi [i], pi [k - 1]);
	}
	std::partial_sum (dwi.begin () + j + 1, dwi.end (),
			  dwi.begin () + j + 1);
	if (w > 0) {
	  std::transform (dwi.begin () + j + 1, dwi.end (),
			  delta [w - 1] [i + 1].begin () + j + 1,
			  dwi.begin () + j + 1,
			  std::plus <double> ());
	}
 	V1D::const_iterator max_it = std::max_element (dwi.begin (), dwi.end ());
 	if (* max_it > 0.0) {
	  insert (pi, i, j, max_it - dwi.begin ());
	  return * max_it;
 	}
      }
    }
    return 0.0;
  }

  // Moves the contents of (i, j) after the contents of (j, k).
  void insert (Permutation & pi, int i, int j, int k) {
    if (k < i) {
      insert (pi, k, i, j);
    } else {
      std::rotate (pi.begin () + i, pi.begin () + j, pi.begin () + k);
    }
  }

  ////////////////////////////////////////////////////////////////////////////////

  AdjacentTransposition::AdjacentTransposition (const Permutation & permutation,
						const BeforeCostRef & cost,
						double alpha) :
    Neighborhood (permutation),
    cost_ (cost),
    totals_ (0),
    alpha_ (alpha)
  {}

  AdjacentTransposition * AdjacentTransposition::clone (const Permutation & permutation) const {
    return new AdjacentTransposition (permutation, cost_, alpha_);
  }

  void AdjacentTransposition::sample (Permutation & neighbor) {
    double r = Log::random (score ());
    int j = 0;
    for (; r > totals_ [j]; ++ j);
    neighbor = permutation_;
    std::swap (neighbor [j], neighbor [j - 1]);
  }

  // Computes the score of the current permutation, and to it adds the
  // difference from swapping each pair of adjacent indices.
  double AdjacentTransposition::score () {
    if (totals_.empty ()) {
      totals_.push_back (Log::Zero);
      double identity = cost_ -> score (permutation_);
      for (Permutation::const_iterator i = permutation_.begin (); i != permutation_.end (); ++ i) {
	Permutation::const_iterator j = i + 1;
	if (j != permutation_.end ()) {
	  double score = identity + cost_ -> cost (* j, * i) - cost_ -> cost (* i, * j);
	  totals_.push_back (Log::add (totals_.back (), alpha_ * score));
	}
      }
    }
    return totals_.back ();
  }

  ////////////////////////////////////////////////////////////////////////////////

  double search_adjacent (Permutation & pi, const BeforeCostRef & bc) {
    int max_i;
    double max_delta = 0.0;
    for (int i = 0; i < pi.size () - 1; ++ i) {
      double delta = bc -> cost (pi [i + 1], pi [i])
	- bc -> cost (pi [i], pi [i + 1]);
      if (delta > max_delta) {
	max_i = i;
	max_delta = delta;
      }
    }
    if (max_delta > 0.0) {
      std::swap (pi [max_i], pi [max_i + 1]);
    }
    return max_delta;
  }

  ////////////////////////////////////////////////////////////////////////////////

  QuadraticNeighborhood::QuadraticNeighborhood (const Permutation & permutation,
						/* . . . , */
						double alpha) :
    Neighborhood (permutation),
    /* . . . , */
    alpha_ (alpha)
  {}

  QuadraticNeighborhood * QuadraticNeighborhood::clone (const Permutation & permutation) const {
    return new QuadraticNeighborhood (permutation, /* . . . , */ alpha_);
  }

  // Generates a random number between zero and score, and reorders the given
  // permutation using the sample path for which the total score surpasses that
  // threshold.
  void QuadraticNeighborhood::sample (Permutation & neighbor) {
    //neighbor.reorder (chart_.samplePath (Log::random (score ()), alpha_));
  }

  double QuadraticNeighborhood::score () {

  }
}
