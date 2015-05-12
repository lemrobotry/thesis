#include "Outside.hh"

namespace Permute {
  // The mapping from (begin, end) pairs to one-dimensional indices differs for
  // the outside estimates because width-one spans get outside estimates, too.
  // For (end - begin) == 1, Chart::index returns (begin - n), and adding n
  // allows us to start at zero, as desired.
  int index (int begin, int end, int n) {
    return n + Chart::index (begin, end, n);
  }
  
  // The inside score of a span is the score of the best path covering that span.
  double inside (ChartRef chart, int begin, int end) {
    return Cell::getBestPath (chart -> getConstCell (begin, end)) -> getScore ();
  }

  Outside::Outside (ChartRef chart) :
    chart_ (chart),
    n_ (chart -> getLength ()),
    outside_ (1 + index (0, n_, n_), Core::Type <double>::min)
  {}

  // Populates the outside_ array with outside estimates.  First, calls
  // Chart::permute to compute best paths in all spans.  Next, initializes the
  // outside_ array to double::min, except outside(0,n) = 0.0.  Then, for each
  // span, sets outside(i,j) to the max over k of
  //   outside(i,k) + max(gamma(i,j,k), bargamma(i,j,k)) + inside(j,k)
  // and
  //   outside(k,j) + max(gamma(k,i,j), bargamma(k,i,j)) + inside(k,i).
  void Outside::estimate (ParseControllerRef controller, ScorerRef scorer) {
    Chart::permute (chart_, controller, scorer);
    for (std::vector <double>::iterator i = outside_.begin (); i != outside_.end (); ++ i) {
      * i = Core::Type <double>::min;
    }
    (* this) (0, n_) = 0.0;
    for (int span = n_; span >= 2; -- span) {
      for (int i = 0; i <= n_ - span; ++ i) {
	int k = i + span;
	for (ParseController::iterator j = controller -> begin (i, k); j != controller -> end (i, k); ++ j) {
	  double alpha = outside (i, k) + std::max (scorer -> score (i, j, k), scorer -> score (k, j, i));
	  operator () (i, j) = std::max (outside (i, j), alpha + inside (chart_, j, k));
	  operator () (j, k) = std::max (outside (j, k), alpha + inside (chart_, i, j));
	}
      }
    }
  }

  // Returns a reference to the appropriate element in the outside_ array. 
  double & Outside::operator () (int begin, int end) {
    return outside_ [index (begin, end, n_)];
  }

  // Returns the value of the appropriate element in the outside_ array.
  double Outside::outside (int begin, int end) const {
    return outside_ [index (begin, end, n_)];
  }

  /**********************************************************************/

  // Wraps a chart to be used for outside estimates.
  OutsideRef outside (ChartRef chart) {
    return OutsideRef (new Outside (chart));
  }
}
