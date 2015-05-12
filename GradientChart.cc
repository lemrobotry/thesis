#include "Chart.hh"
#include "GradientChart.hh"
#include "Log.hh"

namespace Permute {

  GradientChart::GradientChart (const Permutation & pi) :
    pi_ (pi),
    n_ (pi_.size ()),
    inside_ (index (0, n_, Path::SWAP) + 1, Log::Zero),
    outside_ (inside_)
  {}

  void GradientChart::parse (const ParseControllerRef & controller,
			     GradientScorer & scorer,
			     PV & pv) {
    scorer.compute (controller);
    // Computes insides.
    for (int i = 0; i < n_; ++ i) {
      inside (i, i + 1, Path::KEEP) = 0.0;
      inside (i, i + 1, Path::SWAP) = 0.0;
    }
    for (int span = 2; span <= n_; ++ span) {
      for (int begin = 0, end = begin + span; end <= n_; ++ begin, ++ end) {
	double & keep_inside (inside (begin, end, Path::KEEP));
	double & swap_inside (inside (begin, end, Path::SWAP));
	for (ParseController::iterator middle = controller -> begin (begin, end),
	       middle_end = controller -> end (begin, end);
	     middle != middle_end;
	     ++ middle) {
	  Log::increase (keep_inside,
			 total (begin, middle) +
			 inside (middle, end, Path::SWAP) +
			 scorer.score (begin, middle, end));
	  Log::increase (swap_inside,
			 inside (begin, middle, Path::KEEP) +
			 total (middle, end) +
			 scorer.score (end, middle, begin));
	}
      }
    }
    // Computes outsides.
    outside (0, n_, Path::SWAP) = 0.0;
    outside (0, n_, Path::KEEP) = 0.0;
    for (int span = n_; span >= 2; -- span) {
      for (int begin = 0, end = begin + span; end <= n_; ++ begin, ++ end) {
	double keep_outside = outside (begin, end, Path::KEEP);
	double swap_outside = outside (begin, end, Path::SWAP);
	for (ParseController::iterator middle = controller -> begin (begin, end),
	       middle_end = controller -> end (begin, end);
	     middle != middle_end;
	     ++ middle) {
	  // Increases the gradient of (i,j) before (j,k) by outside(i,k,KEEP) *
	  // inside(i,j,ANY) * inside(j,k,SWAP) * score(i,j,k).
	  scorer.gradient (begin, middle, end) =
	    - exp (keep_outside +
		   total (begin, middle) +
		   inside (middle, end, Path::SWAP) +
		   scorer.score (begin, middle, end) -
		   Z ());
	  // Three addends of KEEP -> ANY SWAP.
	  Log::increase (outside (begin, middle, Path::KEEP),
			 keep_outside +
			 inside (middle, end, Path::SWAP) +
			 scorer.score (begin, middle, end));
	  Log::increase (outside (begin, middle, Path::SWAP),
			 keep_outside +
			 inside (middle, end, Path::SWAP) +
			 scorer.score (begin, middle, end));
	  Log::increase (outside (middle, end, Path::SWAP),
			 keep_outside +
			 total (begin, middle) +
			 scorer.score (begin, middle, end));
	  // Increases the gradient of (j,k) before (i,j) by outside(i,k,SWAP) *
	  // inside(i,j,KEEP) * inside(j,k,ANY) * score(k,j,i).
	  scorer.gradient (end, middle, begin) =
	    - exp (swap_outside +
		   inside (begin, middle, Path::KEEP) +
		   total (middle, end) +
		   scorer.score (end, middle, begin) -
		   Z ());
	  // Three addends of SWAP -> KEEP ANY.
	  Log::increase (outside (begin, middle, Path::KEEP),
			 swap_outside +
			 total (middle, end) +
			 scorer.score (end, middle, begin));
	  Log::increase (outside (middle, end, Path::KEEP),
			 swap_outside +
			 inside (begin, middle, Path::KEEP) +
			 scorer.score (end, middle, begin));
	  Log::increase (outside (middle, end, Path::SWAP),
			 swap_outside +
			 inside (begin, middle, Path::KEEP) +
			 scorer.score (end, middle, begin));
	}
      }
    }
    // Zeroes out the parameters.
    for (PV::iterator it = pv.begin (); it != pv.end (); ++ it) {
      it -> second = 0.0;
    }
    // Sets the gradients of the parameters.
    scorer.finish (controller);
  }

  int GradientChart::index (int i, int j, Path::Type type) const {
    return 2 * (n_ + Chart::index (i, j, n_)) + type - 1;
  }

  const double & GradientChart::inside (int i, int j, Path::Type type) const {
    return inside_ [index (i, j, type)];
  }
  double & GradientChart::inside (int i, int j, Path::Type type) {
    return const_cast <double &>
      (static_cast <const GradientChart *> (this) -> inside (i, j, type));
  }

  const double & GradientChart::outside (int i, int j, Path::Type type) const {
    return outside_ [index (i, j, type)];
  }
  double & GradientChart::outside (int i, int j, Path::Type type) {
    return const_cast <double &>
      (static_cast <const GradientChart *> (this) -> outside (i, j, type));
  }

  double GradientChart::total (int i, int j) const {
    if (j - i > 1) {
      return Log::add (inside (i, j, Path::KEEP),
		       inside (i, j, Path::SWAP));
    } else {
      return inside (i, j, Path::KEEP);
    }
  }
  double GradientChart::Z () const {
    return total (0, n_);
  }

  /**********************************************************************/

  GradientScorer::GradientScorer (const SumBeforeCostRef & cost,
				  const Permutation & pi) :
    BeforeScorer (cost, pi),
    cost_ (cost),
    permutation_ (pi),
    gradient_ (2 * BeforeScorer::index (size () - 2, size () - 1, size ()) + 2)
  {}

  // Only adds the gradient if i < j.  The other entries in the matrix are zero,
  // so they don't have gradients.
  void GradientScorer::addGradient (int left, int right, double gradient) {
    int i = permutation_ [left],
      j = permutation_ [right];
    if (i < j) {
      cost_ -> operator () (i, j).add (gradient);
    }
  }

  // Propagates gradients from internal nodes down to individual LOP costs.
  void GradientScorer::finish (const ParseControllerRef & controller,
			       bool numerator) {
    for (int span = size (); span >= 2; -- span) {
      for (int begin = 0, end = begin + span; end <= size (); ++ begin, ++ end) {
	for (ParseController::iterator middle = controller -> begin (begin, end),
	       middle_end = controller -> end (begin, end);
	     middle != middle_end;
	     ++ middle) {
	  double g = gradient (begin, middle, end);
	  gradient (begin, middle, end - 1) += g;
	  gradient (begin + 1, middle, end) += g;
	  gradient (begin + 1, middle, end - 1) -= g;
	  // Propagates the gradient to the LOP cost B[i, k - 1].
	  addGradient (begin, end - 1, g);
	  g = gradient (end, middle, begin);
	  gradient (end - 1, middle, begin) += g;
	  gradient (end, middle, begin + 1) += g;
	  gradient (end - 1, middle, begin + 1) -= g;
	  // Propagates the gradient to the LOP cost B[k - 1, i].
	  addGradient (end - 1, begin, g);
	}
      }
    }
    if (numerator) {
      // Computes the gradient of the numerator using the current permutation.
      for (Permutation::const_iterator i = permutation_.begin ();
	   i != -- permutation_.end (); ++ i) {
	for (Permutation::const_iterator j = i + 1;
	     j != permutation_.end (); ++ j) {
	  addGradient (* i, * j, 1.0);
	}
      }
    }
  }

  const double & GradientScorer::gradient (int i, int j, int k) const {
    static double dummy = 0.0;
    if (i == j || j == k) {
      return dummy;
    } else if (i < k) {
      return gradient_ [2 * BeforeScorer::index (i, j, k)];
    } else {
      return gradient_ [2 * BeforeScorer::index (k, j, i) + 1];
    }
  }

  double & GradientScorer::gradient (int i, int j, int k) {
    return const_cast <double &>
      (static_cast <const GradientScorer *> (this) -> gradient (i, j, k));
  }
}
