#include "Chart.hh"
#include "FullCell.hh"

namespace Permute {
  // Clears the scorer and calls the Chart's beforePermute method.  Iterates
  // over the spans in order by width.  At each span, populates the
  // corresponding cell with a call to Cell::build for every split of the span
  // that the controller allows.  Makes a second call to Cell::build reversing
  // the child spans if the span satisfies the window constraint.
  void Chart::permute (ChartRef chart, ParseControllerRef controller, ScorerRef scorer) {
    scorer -> compute (controller);
    chart -> beforePermute (controller, scorer);
    int length = chart -> getLength ();
    for (int span = 2; span <= length; ++span) {
      for (int begin = 0; begin <= length - span; ++begin) {
	int end = begin + span;
	CellRef parent = chart -> getCell (begin, end);
	parent -> clear ();
	for (ParseController::iterator middle = controller -> begin (begin, end);
	     middle != controller -> end (begin, end); ++middle) {
	  ConstCellRef
	    left = chart -> getConstCell (begin, middle),
	    right = chart -> getConstCell (middle, end);
	  Cell::build (parent, left, right, scorer -> score (begin, middle, end), false,
		       (end - middle == 1) ? Path::NEITHER : Path::SWAP);
	  if (! chart -> getWindow () || chart -> getWindow () >= span) {
	    Cell::build (parent, right, left, scorer -> score (end, middle, begin), true,
			 (middle - begin == 1) ? Path::NEITHER : Path::KEEP);
	  }
	}
      }
    }
    chart -> afterPermute (controller, scorer);
  }

  // Arranges the cells to form a zero-indexed triangular matrix where the
  // (i,j)th entry has width i + 2 and starts at position j (therefore the
  // (i,j)th entry corresponds to the span (j, j + i + 2)).  Because the ith row
  // has only n - i - 1 entries (the 0th row, corresponding to width-2 spans,
  // has n - 1 entries), subtracts the ith triangular number from i * n to
  // compute the start position of the ith row, then adds j to compute the
  // one-dimensional index of the span.
  int Chart::index (int begin, int end, int n) {
    int row = end - begin - 2;
    return row * n - triangle (row) + begin;
  }

  // Computs the nth triangular number, which is the sum_{i = 1}^n i.
  int Chart::triangle (int n) {
    return n * (n + 1) / 2;
  }

  /**********************************************************************/

  ChartImpl::ChartImpl (Permutation & permutation, CellMapRef map, CellRef top, int window) :
    permutation_ (permutation),
    map_ (map),
    cells_ (),
    window_ (window)
  {
    for (int i = 0; i < index (0, getLength ()); ++ i) {
      cells_.push_back (CellRef (new FullCell ()));
    }
    cells_.push_back (top);
  }

  CellRef ChartImpl::getCell (int begin, int end) {
    return cells_ [index (begin, end)];
  }

  ConstCellRef ChartImpl::getConstCell (int begin, int end) {
    if (end - begin == 1) {
      size_t index = permutation_ [begin];
      return map_ -> operator () (index, permutation_.label (index));
    } else {
      return ConstCellRef (cells_ [index (begin, end)]);
    }
  }
  
  int ChartImpl::index (int begin, int end) const {
    return Chart::index (begin, end, this -> getLength ());
  }

  int ChartImpl::getLength () const {
    return permutation_.size ();
  }
  
  ConstPathRef ChartImpl::getBestPath () {
    return Cell::getBestPath (getConstCell (0, getLength ()));
  }

  int ChartImpl::getWindow () const {
    return window_;
  }

  /**********************************************************************/

  ChartDecorator::ChartDecorator (ChartRef decorated) :
    decorated_ (decorated)
  {}

  CellRef ChartDecorator::getCell (int begin, int end) {
    return decorated_ -> getCell (begin, end);
  }

  ConstCellRef ChartDecorator::getConstCell (int begin, int end) {
    return decorated_ -> getConstCell (begin, end);
  }

  int ChartDecorator::getLength () const {
    return decorated_ -> getLength ();
  }

  ConstPathRef ChartDecorator::getBestPath () {
    return decorated_ -> getBestPath ();
  }

  void ChartDecorator::beforePermute (ParseControllerRef controller, ScorerRef scorer) {
    decorated_ -> beforePermute (controller, scorer);
  }

  void ChartDecorator::afterPermute (ParseControllerRef controller, ScorerRef scorer) {
    decorated_ -> afterPermute (controller, scorer);
  }

  int ChartDecorator::getWindow () const {
    return decorated_ -> getWindow ();
  }
}
