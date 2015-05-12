#include "Cell.hh"

namespace Permute {

  // Connects each pair of paths in the left and right cells such that the left
  // path's end state matches the right path's start state, and the right path
  // is of the given type.
  void Cell::build (CellRef parent, ConstCellRef left, ConstCellRef right, double score, bool swap, Path::Type type) {
    for (PathIterator leftPath = left -> begin (); leftPath != left -> end (); ++ leftPath) {
      Fsa::StateId middle = (* leftPath) -> getEnd ();
      for (PathIterator rightPath = right -> begin (middle, type);
	   rightPath != right -> end ();
	   rightPath = right -> next (rightPath, middle, type)) {
	parent -> add (Path::connect (* leftPath, * rightPath, score, swap));
      }
    }
  }

  // Iterates over all paths in the given cell and returns the one with the
  // highest score.
  ConstPathRef Cell::getBestPath (ConstCellRef cell) {
    ConstPathRef best = Path::nullPath ();
    for (PathIterator path = cell -> begin (); path != cell -> end (); ++path) {
      if ((* path) -> getScore () > best -> getScore ()) {
	best = (* path);
      }
    }
    return best;
  }

  /**********************************************************************/

  CellImpl::CellImpl () :
    paths_ ()
  {}

  bool CellImpl::empty () const {
    return paths_.empty ();
  }

  size_t CellImpl::size () const {
    return paths_.size ();
  }

  void CellImpl::clear () {
    paths_.clear ();
  }

  Cell::PathIterator CellImpl::begin () const {
    return paths_.begin ();
  }

  Cell::PathIterator CellImpl::end () const {
    return paths_.end ();
  }

  Cell::PathIterator CellImpl::begin (Fsa::StateId start, Path::Type type) const {
    return paths_.lower_bound (start, type);
  }

  Cell::PathIterator CellImpl::next (PathIterator i, Fsa::StateId start, Path::Type type) const {
    return paths_.next (i, start, type);
  }

  Cell::PathIterator CellImpl::find (Fsa::StateId start, Fsa::StateId end, Path::Type type) const {
    return paths_.find (start, end, type);
  }

  /**********************************************************************/

  CellDecorator::CellDecorator (CellRef decorated) :
    decorated_ (decorated)
  {}

  bool CellDecorator::empty () const {
    return decorated_ -> empty ();
  }

  size_t CellDecorator::size () const {
    return decorated_ -> size ();
  }
  
  void CellDecorator::clear () {
    decorated_ -> clear ();
  }
  Cell::PathIterator CellDecorator::begin () const {
    return decorated_ -> begin ();
  }
  Cell::PathIterator CellDecorator::end () const {
    return decorated_ -> end ();
  }
  Cell::PathIterator CellDecorator::begin (Fsa::StateId start, Path::Type type) const {
    return decorated_ -> begin (start, type);
  }
  Cell::PathIterator CellDecorator::next (PathIterator i, Fsa::StateId start, Path::Type type) const {
    return decorated_ -> next (i, start, type);
  }
  Cell::PathIterator CellDecorator::find (Fsa::StateId start, Fsa::StateId end, Path::Type type) const {
    return decorated_ -> find (start, end, type);
  }
  bool CellDecorator::add (ConstPathRef path) {
    return decorated_ -> add (path);
  }
}
