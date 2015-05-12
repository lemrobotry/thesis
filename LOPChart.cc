#include "Chart.hh"
#include "LOPChart.hh"
#include "Log.hh"

namespace Permute {

  LOPChart::LOPChart (Permutation & pi, int window) :
    pi_ (pi),
    n_ (pi.size ()),
    window_ (window),
    cells_ (n_ + Chart::index (0, n_, n_) + 1)
  {}

  int LOPChart::index (int i, int j) const {
    return n_ + Chart::index (i, j, n_);
  }

  LOPCell & LOPChart::cell (int i, int j) {
    return const_cast <LOPCell &>
      (static_cast <const LOPChart *> (this) -> cell (i, j));
  }

  const LOPCell & LOPChart::cell (int i, int j) const {
    return cells_ [index (i, j)];
  }

  void LOPChart::permute (const ParseControllerRef & controller, ScorerRef & scorer) {
    // Initializes the scorer.
    scorer -> compute (controller);
    
    for (int span = 2; span <= n_; ++ span) {
      for (int begin = 0; begin <= n_ - span; ++ begin) {
	int end = begin + span;
	// Initializes the (begin, end) cell.
	LOPCell & cell = this -> cell (begin, end);
	cell.score = Core::Type <double>::min;
	// Iterates over middle positions.
	ParseController::iterator end_it = controller -> end (begin, end);
	for (ParseController::iterator middle = controller -> begin (begin, end);
	     middle != end_it;
	     ++ middle) {
	  const LOPCell & left = this -> cell (begin, middle);
	  const LOPCell & right = this -> cell (middle, end);
	  cell.max_equals (middle,
			   false,
			   left.score + right.score + scorer -> score (begin, middle, end));
	  if (! getWindow () || getWindow () >= span) {
	    cell.max_equals (middle,
			     true,
			     left.score + right.score + scorer -> score (end, middle, begin));
	  }
	}
      }
    }
  }

  ConstPathRef LOPChart::getBestPath () const {
    return path (0, n_);
  }

  ConstPathRef LOPChart::path (int i, int j) const {
    if (j - i == 1) {
      return Path::arc (pi_ [i], 0, 0, 0.0);
    } else {
      const LOPCell & cell = this -> cell (i, j);
      ConstPathRef left = this -> path (i, cell.midpoint);
      ConstPathRef right = this -> path (cell.midpoint, j);
      double delta = cell.score - left -> getScore () - right -> getScore ();
      if (cell.swap) {
	return Path::connect (right, left, delta, true);
      } else {
	return Path::connect (left, right, delta, false);
      }
    }
  }

  int LOPChart::getWindow () const {
    return window_;
  }

  int LOPChart::getLength () const {
    return n_;
  }

  ////////////////////////////////////////////////////////////////////////////////

  NormalLOPChart::NormalLOPChart (Permutation & pi, int window) :
    pi_ (pi),
    n_ (pi.size ()),
    window_ (window),
    cells_ (2 * (n_ + Chart::index (0, n_, n_) + 1))
  {}

  int NormalLOPChart::index (int i, int j, Path::Type type) const {
    return 2 * (n_ + Chart::index (i, j, n_)) + type - 1;
  }

  const ConstPathRef & NormalLOPChart::cell (int i, int j, Path::Type type) const {
    return cells_ [index (i, j, type)];
  }

  ConstPathRef & NormalLOPChart::cell (int i, int j, Path::Type type) {
    return const_cast <ConstPathRef &>
      (static_cast <const NormalLOPChart *> (this) -> cell (i, j, type));
  }

  const ConstPathRef & NormalLOPChart::betterPath (int i, int j) const {
    const ConstPathRef & keep = cell (i, j, Path::KEEP);
    const ConstPathRef & swap = cell (i, j, Path::SWAP);
    return (swap -> getScore () > keep -> getScore ()) ? swap : keep;
  }

  // @bug Does not produce the same results as Chart::permute, which also
  // performs normal-form parsing.  This may simply be due to ties and ordering,
  // or it may be due to a logical error. 
  void NormalLOPChart::permute (const ParseControllerRef & controller, ScorerRef & scorer) {
    // Initializes the scorer.
    scorer -> compute (controller);
    // Initializes the width-1 paths.
    for (int i = 0; i < n_; ++ i) {
      cell (i, i + 1, Path::SWAP) = cell (i, i + 1, Path::KEEP) = Path::arc (pi_ [i], 0, 0, 0.0);
    }
    for (int span = 2; span <= n_; ++ span) {
      for (int begin = 0; begin <= n_ - span; ++ begin) {
	int end = begin + span;
	// Initializes the (begin, end) cell.
	ConstPathRef & keep_path = this -> cell (begin, end, Path::KEEP);
	double keep_path_score = Core::Type <double>::min;
	ConstPathRef & swap_path = this -> cell (begin, end, Path::SWAP);
	double swap_path_score = Core::Type <double>::min;
	// Iterates over middle positions.
	for (ParseController::iterator middle = controller -> begin (begin, end);
	     middle != controller -> end (begin, end);
	     ++ middle) {
	  // Uses the better left child.
	  ConstPathRef left = this -> betterPath (begin, middle);
	  ConstPathRef right = this -> cell (middle, end, Path::SWAP);
	  ConstPathRef keep = Path::connect (left, right, scorer -> score (begin, middle, end), false);
	  if (keep -> getScore () >  keep_path_score) {
	    keep_path = keep;
	    keep_path_score = keep_path -> getScore ();
	  }
	  if (! getWindow () || getWindow () >= span) {
	    // Uses the better right child.
	    right = this -> betterPath (middle, end);
	    left = this -> cell (begin, middle, Path::KEEP);
	    ConstPathRef swap = Path::connect (right, left, scorer -> score (end, middle, begin), true);
	    if (swap -> getScore () > swap_path_score) {
	      swap_path = swap;
	      swap_path_score = swap_path -> getScore ();
	    }
	  }
	}
      }
    }
  }
    
  const ConstPathRef & NormalLOPChart::getBestPath () const {
    return betterPath (0, n_);
  }

  int NormalLOPChart::getWindow () const {
    return window_;
  }

  int NormalLOPChart::getLength () const {
    return n_;
  }

  ////////////////////////////////////////////////////////////////////////////////

  QuadraticNormalLOPChart::QuadraticNormalLOPChart (Permutation & pi, int width, bool left) :
    pi_ (pi),
    n_ (pi.size ()),
    w_ (width),
    left_ (left),
    cells_ (n_ + Chart::index (0, n_, n_) + 1)
  {}

  int QuadraticNormalLOPChart::getLength () const {
    return n_;
  }

  int QuadraticNormalLOPChart::index (int i, int j) const {
    return n_ + Chart::index (i, j, n_);
  }

  QuadraticNormalLOPCell & QuadraticNormalLOPChart::cell (int i, int j) {
    return const_cast <QuadraticNormalLOPCell &>
      (static_cast <const QuadraticNormalLOPChart *> (this) -> cell (i, j));
  }

  const QuadraticNormalLOPCell & QuadraticNormalLOPChart::cell (int i, int j) const {
    return cells_ [index (i, j)];
  }

  void QuadraticNormalLOPChart::permute (const ParseControllerRef & controller, ScorerRef & scorer) {
    // Initializes the scorer.
    scorer -> compute (controller);

    // Leaf cells should work properly without any initialization.

    // Compute the rest of the cells.
    for (int span = 2; span <= n_; ++ span) {
      for (int begin = 0; begin <= n_ - span; ++ begin) {
	int end = begin + span;
	// Initializes the (begin, end) cell.
	QuadraticNormalLOPCell & cell = this -> cell (begin, end);
	cell.white ().score = cell.black ().score = cell.red ().score = Core::Type <double>::min;
	// Iterates over middle positions.
	ParseController::iterator end_it = controller -> end (begin, end);
	for (ParseController::iterator middle = controller -> begin (begin, end);
	     middle != end_it;
	     ++ middle) {
	  const QuadraticNormalLOPCell & left = this -> cell (begin, middle);
	  const QuadraticNormalLOPCell & right = this -> cell (middle, end);
	  if (end - middle <= w_) {
	    // (middle, end) is narrow:
	    cell.white ().max_equals (middle, false,
				      left.any ().score
				      + right.non_white ().score
				      + scorer -> score (begin, middle, end));
	    cell.black ().max_equals (middle, true,
				      left.any ().score
				      + right.non_black ().score
				      + scorer -> score (end, middle, begin));
	  } else if (middle - begin <= w_ || (left_ && begin == 0)) {
	    // @bug The above condition is redundant.  If the ParseController
	    // allows this midpoint, it must be close to begin or begin must be
	    // zero.

	    // (begin, middle) is narrow or begin == 0
	    cell.red ().max_equals (middle, false,
				    left.non_white ().score
				    + right.non_white ().score
				    + scorer -> score (begin, middle, end));
	    cell.red ().max_equals (middle, true,
				    left.non_black ().score
				    + right.non_black ().score
				    + scorer -> score (end, middle, begin));
	  }
	}
	// Changes cell from {white, black, red} to {non_black, non_white, any}.
	cell.finish ();
      }
    }
  }

  ConstPathRef QuadraticNormalLOPChart::getBestPath () const {
    return path (0, n_, 2);
  }

  ConstPathRef QuadraticNormalLOPChart::path (int i, int j, int color) const {
    if (j - i == 1) {
      return Path::arc (pi_ [i], 0, 0, 0.0);
    } else {
      const QuadraticNormalLOPCell & cell = this -> cell (i, j);
      switch (color) {
      case 0:
	return path (i, j, cell.non_black ());
      case 1:
	return path (i, j, cell.non_white ());
      case 2:
	return path (i, j, cell.any ());
      }
    }
  }

  ConstPathRef QuadraticNormalLOPChart::path (int i, int j, const LOPCell & cell) const {
    ConstPathRef left, right;
    if (j - cell.midpoint <= w_) {
      left = path (i, cell.midpoint, 2);
      right = path (cell.midpoint, j, cell.swap ? 0 : 1);
    } else {
      if (cell.swap) {
	left = path (i, cell.midpoint, 0);
	right = path (cell.midpoint, j, 0);
      } else {
	left = path (i, cell.midpoint, 1);
	right = path (cell.midpoint, j, 1);
      }
    }
    double delta = cell.score - left -> getScore () - right -> getScore ();
    if (cell.swap) {
      return Path::connect (right, left, delta, true);
    } else {
      return Path::connect (left, right, delta, false);
    }
  }

  ////////////////////////////////////////////////////////////////////////////////

  LOPkBestChart::LOPkBestChart (Permutation & pi) :
    pi_ (pi),
    n_ (pi.size ()),
    cells_ (n_ + Chart::index (0, n_, n_) + 1)
  {}

  int LOPkBestChart::index (int i, int j) const {
    return n_ + Chart::index (i, j, n_);
  }

  NormVertices & LOPkBestChart::cell (int i, int j) {
    return cells_ [index (i, j)];
  }

  void LOPkBestChart::permute (const ParseControllerRef & controller, ScorerRef & scorer) {
    // Initializes the scorer.
    scorer -> compute (controller);
    // Initializes the leaves.
    for (int i = 0; i < n_; ++ i) {
      LOP::Derivation arc (Path::arc (pi_ [i], 0, 0, 0.0));
      cell (i, i + 1).non_black ().derivations_.push_back (arc);
      cell (i, i + 1).non_white ().derivations_.push_back (arc);
      cell (i, i + 1).any ().derivations_.push_back (arc);
    }
    // Initializes the rest of the chart.
    for (int span = 2; span <= n_; ++ span) {
      for (int begin = 0; begin <= n_ - span; ++ begin) {
	int end = begin + span;
	NormVertices & vertices = cell (begin, end);
	// Iterates over middle positions.
	ParseController::iterator end_it = controller -> end (begin, end);
	for (ParseController::iterator middle = controller -> begin (begin, end);
	     middle != end_it;
	     ++ middle) {
	  NormVertices & left = cell (begin, middle);
	  NormVertices & right = cell (middle, end);
	  LOP::Derivation white (& left.any (), & right.non_white (),
				 scorer -> score (begin, middle, end),
				 LOP::Derivation::KEEP);
	  vertices.non_black ().insert (white);
	  vertices.any ().insert (white);
	  LOP::Derivation black (& left.any (), & right.non_black (),
				 scorer -> score (end, middle, begin),
				 LOP::Derivation::SWAP);
	  vertices.non_white ().insert (black);
	  vertices.any ().insert (black);
	}
	// Computes the Vertex's best path, so the first Derivation can always
	// compute its score.
	// @bug It would be less error-prone if the Derivation did this
	// automatically.
	vertices.non_black ().lazyKthBest (0);
	vertices.non_white ().lazyKthBest (0);
	vertices.any ().lazyKthBest (0);
      }
    }
  }

  // Allows getBestPaths to call std::transform.
  struct DerivationToPath {
    const ConstPathRef & operator () (const LOP::Derivation & d) const {
      return d.path ();
    }
  };

  void LOPkBestChart::getBestPaths (std::vector <ConstPathRef> & v, int k) {
    LOP::Vertex & vertex = cell (0, n_).any ();
    vertex.lazyKthBest (k - 1);
    std::transform (vertex.derivations_.begin (), vertex.derivations_.end (),
		    std::back_inserter (v),
		    DerivationToPath ());
  }

  ////////////////////////////////////////////////////////////////////////////////

  QNormLOPkBestChart::QNormLOPkBestChart (Permutation & pi, int width) :
    pi_ (pi),
    n_ (pi.size ()),
    w_ (width),
    cells_ (n_ + Chart::index (0, n_, n_) + 1)
  {}
  
  int QNormLOPkBestChart::index (int i, int j) const {
    return n_ + Chart::index (i, j, n_);
  }
  
  NormVertices & QNormLOPkBestChart::cell (int i, int j) {
    return cells_ [index (i, j)];
  }

  void QNormLOPkBestChart::permute (const ParseControllerRef & controller, ScorerRef & scorer) {
    // Initializes the scorer.
    scorer -> compute (controller);
    // Initializes the leaves.
    for (int i = 0; i < n_; ++ i) {
      LOP::Derivation arc (Path::arc (pi_ [i], 0, 0, 0.0));
      cell (i, i + 1).non_black ().derivations_.push_back (arc);
      cell (i, i + 1).non_white ().derivations_.push_back (arc);
      cell (i, i + 1).any ().derivations_.push_back (arc);
    }
    // Initializes the rest of the chart.
    for (int span = 2; span <= n_; ++ span) {
      for (int begin = 0; begin <= n_ - span; ++ begin) {
	int end = begin + span;
	NormVertices & vertices = cell (begin, end);
	// Iterates over middle positions.
	ParseController::iterator end_it = controller -> end (begin, end);
	for (ParseController::iterator middle = controller -> begin (begin, end);
	     middle != end_it;
	     ++ middle) {
	  NormVertices & left = this -> cell (begin, middle);
	  NormVertices & right = this -> cell (middle, end);
	  if (end - middle <= w_) {
	    // (middle, end) is narrow:
	    LOP::Derivation white (& left.any (), & right.non_white (),
				   scorer -> score (begin, middle, end),
				   LOP::Derivation::KEEP);
	    vertices.non_black ().insert (white);
	    vertices.any ().insert (white);
	    LOP::Derivation black (& left.any (), & right.non_black (),
				   scorer -> score (end, middle, begin),
				   LOP::Derivation::SWAP);
	    vertices.non_white ().insert (black);
	    vertices.any ().insert (black);
	  } else {
	    // (begin, middle) is narrow or begin == 0
	    LOP::Derivation red1 (& left.non_white (), & right.non_white (),
				  scorer -> score (begin, middle, end),
				  LOP::Derivation::KEEP);
	    vertices.non_black ().insert (red1);
	    vertices.non_white ().insert (red1);
	    vertices.any ().insert (red1);
	    LOP::Derivation red2 (& left.non_black (), & right.non_black (),
				  scorer -> score (end, middle, begin),
				  LOP::Derivation::SWAP);
	    vertices.non_black ().insert (red2);
	    vertices.non_white ().insert (red2);
	    vertices.any ().insert (red2);
	  }
	}
	vertices.non_black ().lazyKthBest (0);
	vertices.non_white ().lazyKthBest (0);
	vertices.any ().lazyKthBest (0);
      }
    }
  }
  
  void QNormLOPkBestChart::getBestPaths (std::vector <ConstPathRef> & v, int k) {
    LOP::Vertex & vertex = cell (0, n_).any ();
    vertex.lazyKthBest (k - 1);
    std::transform (vertex.derivations_.begin (), vertex.derivations_.end (),
		    std::back_inserter (v),
		    DerivationToPath ());
  }

  const ConstPathRef & QNormLOPkBestChart::samplePath (double threshold, double alpha) {
    LOP::Vertex & vertex = cell (0, n_).any ();
    double total = Log::Zero;
    for (int i = 0; vertex.lazyKthBest (i); ++ i) {
      const ConstPathRef & path = vertex.derivations_ [i].path ();
      total = Log::add (total, alpha * path -> getScore ());
      if (total > threshold) {
	return path;
      }
    }
    // @bug Something went wrong.  Throw an exception?
    return vertex.derivations_.back ().path ();
  }
}
