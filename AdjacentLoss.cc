#include "AdjacentLoss.hh"
#include "Chart.hh"

namespace Permute {

  AdjacentLoss::AdjacentLoss (const Permutation & target) :
    target_ (target),
    succ_ (target.size ())
  {
    for (Permutation::const_iterator it = target_.begin (),
	   end = -- target_.end (); it != end; ) {
      succ_ [* it] = * (++ it);
    }
    succ_ [target_.back ()] = std::numeric_limits <size_t>::max ();
  }

  double AdjacentLoss::score (const Permutation & pi) const {
    double loss = 0.0;
    for (Permutation::const_iterator it = pi.begin (),
	   end = -- pi.end (); it != end; ) {
      loss += (succ_ [* it] != * (++ it));
    }
    return loss;
  }

  /**********************************************************************/

  template <class Value>
  Value AdjacentItem <Value>::keep () const {
    if (isLeaf ()) {
      return leaf ();
    } else {
      return keepNewRight () + keepSameRight ();
    }
  }

  template <class Value>
  Value AdjacentItem <Value>::newRight () const {
    if (isLeaf ()) {
      return Value::Zero;
    } else {
      return keepNewRight () + swap ();
    }
  }

  template <class Value>
  Value AdjacentItem <Value>::any () const {
    if (isLeaf ()) {
      return leaf ();
    } else {
      return keep () + swap ();
    }
  }

  template <class Value>
  std::ostream & operator << (std::ostream & out, const AdjacentItem <Value> & item) {
    if (item.isLeaf ()) {
      out << "Leaf: " << item.leaf ();
    } else {
      out << "Keep+R: " << item.keepSameRight () << ", "
	  << "Keep-R: " << item.keepNewRight () << ", "
	  << "Swap: " << item.swap ();
    }
    return out;
  }

  // Explicit template instantiation.
  template std::ostream & operator << (std::ostream & out, const AdjacentItem <Expectation> & item);

  /**********************************************************************/

  AdjacentGradientChart::AdjacentGradientChart (const Permutation & pi) :
    pi_ (pi),
    n_ (pi_.size ()),
    inside_ (index (0, n_) + 1),
    outside_ (index (0, n_) + 1)
  {}

  // Grammar:
  //
  // L = Leaf
  // K+R = Keep w/ same Right endpoint
  // K-R = Keep w/ different Right endpoint
  // S = Swap
  // K = Keep {K+R, K-R}
  // A = Any {L, K, S}
  //
  // K+R -> A L
  // K-R -> A S
  // S   -> K A
  void AdjacentGradientChart::parse (const ParseControllerRef & controller,
				     ExpectationGradientScorer & scorer,
				     PV & pv) {
    scorer.compute (controller);
    // Inside pass
    for (int i = 0; i < n_; ++ i) {
      inside (i, i + 1).leaf () = Expectation::One;
    }
    for (int span = 2; span <= n_; ++ span) {
      for (int begin = 0, end = begin + span; end <= n_; ++ begin, ++ end) {
	Item & parent = inside (begin, end);
	parent.fill (Expectation::Zero);
	for (ParseController::iterator middle = controller -> begin (begin, end),
	       middle_end = controller -> end (begin, end);
	     middle != middle_end;
	     ++ middle) {
	  const Item & left = inside (begin, middle),
	    & right = inside (middle, end);
	  if (right.isLeaf ()) {
	    parent.keepSameRight () += left.keepSameRight () * right.leaf ()
	      * Expectation::p_pv (scorer.score (begin, middle, end), Log::One);
	    parent.keepSameRight () += left.newRight () * right.leaf ()
	      * Expectation::p_pv (scorer.score (begin, middle, end), Log::Zero);
	  } else {
	    parent.keepNewRight () += left.any () * right.swap ()
	      * Expectation::p_pv (scorer.score (begin, middle, end), Log::Zero);
	  }
	  parent.swap () += left.keep () * right.any ()
	    * Expectation::p_pv (scorer.score (end, middle, begin), Log::Zero);
	}
 	assert (parent.any ().expectation () <= span - 1);
      }
    }
    // Outside pass
    outside (0, n_).fill (Expectation::One);
    for (int span = n_; span >= 2; -- span) {
      for (int begin = 0, end = begin + span; end <= n_; ++ begin, ++ end) {
	const Item & parent = outside (begin, end);
	for (ParseController::iterator middle = controller -> begin (begin, end),
	       middle_end = controller -> end (begin, end);
	     middle != middle_end;
	     ++ middle) {
	  Item & leftO = outside (begin, middle),
	    & rightO = outside (middle, end);
	  const Item & leftI = inside (begin, middle),
	    & rightI = inside (middle, end);
	  Expectation e, keep, swap;
	  double keep_score = scorer.score (begin, middle, end),
	    swap_score = scorer.score (end, middle, begin);
	  if (rightI.isLeaf ()) {
	    // K+R -> A L
	    if (leftI.isLeaf ()) {
	      leftO.leaf () += parent.keepSameRight () * rightI.leaf ()
		* Expectation::p_pv (keep_score, Log::One);
	      keep = parent.keepSameRight () * rightI.leaf () * leftI.leaf ()
		* Expectation::p_pv (keep_score, Log::One);
	    } else {
	      leftO.keepSameRight () += parent.keepSameRight () * rightI.leaf ()
		* Expectation::p_pv (keep_score, Log::One);
	      e = parent.keepSameRight () * rightI.leaf ()
		* Expectation::p_pv (keep_score, Log::Zero);
	      leftO.keepNewRight () += e;
	      leftO.swap () += e;
	      keep = parent.keepSameRight () * rightI.leaf ()
		* (leftI.keepSameRight () * Expectation::p_pv (keep_score, Log::One)
		   + leftI.newRight () * Expectation::p_pv (keep_score, Log::Zero));
	    }
	    rightO.leaf () += parent.keepSameRight () * leftI.keepSameRight ()
	      * Expectation::p_pv (keep_score, Log::One);
	    rightO.leaf () += parent.keepSameRight () * leftI.newRight ()
	      * Expectation::p_pv (keep_score, Log::Zero);
	  } else {
	    // K-R -> A S
	    e = parent.keepNewRight () * rightI.swap ()
	      * Expectation::p_pv (keep_score, Log::Zero);
	    if (leftI.isLeaf ()) {
	      leftO.leaf () += e;
	    } else {
	      leftO.keepSameRight () += e;
	      leftO.keepNewRight () += e;
	      leftO.swap () += e;
	    }
	    rightO.swap () += parent.keepNewRight () * leftI.any ()
	      * Expectation::p_pv (keep_score, Log::Zero);
	    keep = e * leftI.any ();
	  }
	  // S -> K A
	  e = parent.swap () * rightI.any ()
	    * Expectation::p_pv (swap_score, Log::Zero);
	  if (leftI.isLeaf ()) {
	    leftO.leaf () += e;
	  } else {
	    leftO.keepSameRight () += e;
	    leftO.keepNewRight () += e;
	  }
	  e = parent.swap () * leftI.keep ()
	    * Expectation::p_pv (swap_score, Log::Zero);
	  if (rightI.isLeaf ()) {
	    rightO.leaf () += e;
	  } else {
	    rightO.keepSameRight () += e;
	    rightO.keepNewRight () += e;
	    rightO.swap () += e;
	  }
	  swap = e * rightI.any ();

	  scorer.gradient (begin, middle, end) = keep;
	  scorer.gradient (end, middle, begin) = swap;
	}
      }
    }
    // Zeroes out the parameters.
    for (PV::iterator it = pv.begin (); it != pv.end (); ++ it) {
      it -> second = 0.0;
    }
    // Doesn't include the "numerator" in the gradients.
    scorer.finish (controller, Z ());
  }

  int AdjacentGradientChart::index (int begin, int end) const {
    return n_ + Chart::index (begin, end, n_);
  }

  const AdjacentGradientChart::Item &
  AdjacentGradientChart::inside (int begin, int end) const {
    return inside_ [index (begin, end)];
  }

  AdjacentGradientChart::Item &
  AdjacentGradientChart::inside (int begin, int end) {
    return const_cast <Item &>
      (static_cast <const AdjacentGradientChart *>
       (this) -> inside (begin, end));
  }
  
  const AdjacentGradientChart::Item &
  AdjacentGradientChart::outside (int begin, int end) const {
    return outside_ [index (begin, end)];
  }
  
  AdjacentGradientChart::Item &
  AdjacentGradientChart::outside (int begin, int end) {
    return const_cast <Item &>
      (static_cast <const AdjacentGradientChart *>
       (this) -> outside (begin, end));
  }

  /**********************************************************************/

  ExpectationGradientScorer::ExpectationGradientScorer (const SumBeforeCostRef & cost,
							const Permutation & pi) :
    BeforeScorer (cost, pi),
    cost_ (cost),
    permutation_ (pi),
    gradient_ (2 * BeforeScorer::index (size () - 2, size () - 1, size ()) + 2),
    matrix_ (size () * size ())
  {}

  void ExpectationGradientScorer::addExpectation (int left, int right, Expectation gradient) {
    expectation (permutation_ [left], permutation_ [right]) += gradient;
  }

  void ExpectationGradientScorer::finish (const ParseControllerRef & controller,
					  Expectation Z) {
    for (int span = size (); span >= 2; -- span) {
      for (int begin = 0, end = begin + span; end <= size (); ++ begin, ++ end) {
	for (ParseController::iterator middle = controller -> begin (begin, end),
	       middle_end = controller -> end (begin, end);
	     middle != middle_end;
	     ++ middle) {
	  Expectation g = gradient (begin, middle, end).sum ();
	  gradient (begin, middle, end - 1) += g;
	  gradient (begin + 1, middle, end) += g;
	  gradient (begin + 1, middle, end - 1) -= g;
	  addExpectation (begin, end - 1, g);

	  g = gradient (end, middle, begin).sum ();
	  gradient (end - 1, middle, begin) += g;
	  gradient (end, middle, begin + 1) += g;
	  gradient (end - 1, middle, begin + 1) -= g;
	  addExpectation (end - 1, begin, g);
	}
      }
    }
    // Sets the gradients from the expectations in the matrix.
    for (int i = 0; i < size () - 1; ++ i) {
      for (int j = i + 1; j < size (); ++ j) {
	Expectation e = expectation (i, j);
	cost_ -> operator () (i, j).add
	  ((e.v () / Z.p ()).toP () - Z.expectation () * (e.p () / Z.p ()).toP ());
      }
    }
  }

  const ExpectationSum & ExpectationGradientScorer::gradient (int i, int j, int k) const {
    static ExpectationSum dummy;
    if (i == j || j == k) {
      return dummy;
    } else if (i < k) {
      return gradient_ [2 * BeforeScorer::index (i, j, k)];
    } else {
      return gradient_ [2 * BeforeScorer::index (k, j, i) + 1];
    }
  }

  ExpectationSum & ExpectationGradientScorer::gradient (int i, int j, int k) {
    return const_cast <ExpectationSum &>
      (static_cast <const ExpectationGradientScorer *>
       (this) -> gradient (i, j, k));
  }

  const Expectation & ExpectationGradientScorer::expectation (int i, int j) const {
    return matrix_ [i * size () + j];
  }

  Expectation & ExpectationGradientScorer::expectation (int i, int j) {
    return const_cast <Expectation &>
      (static_cast <const ExpectationGradientScorer *>
       (this) -> expectation (i, j));
  }
}
