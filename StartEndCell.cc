#include "StartEndCell.hh"

namespace Permute {
  StartEndCell::StartEndCell (Fsa::ConstAutomatonRef fsa, ConstCellRef epsilonClosure) :
    BestCell (),
    fsa_ (fsa),
    epsilonClosure_ (epsilonClosure)
  {}

  // Adds the given path to the cell if it is complete or can be extended to
  // make it complete.
  //
  // Assumes paths have their epsilon closure computed elsewhere and requires
  // that the path's end state is final.  Incorporates the corresponding weight.
  //
  // If the path starts at the initial state, adds it directly.
  //
  // If not, looks for an arc or a path connecting the initial state to the
  // path's start.  If such a path exists, extends the path by the one with the
  // highest score and adds it.
  bool StartEndCell::add (ConstPathRef path) {
    Fsa::ConstStateRef end = fsa_ -> getState (path -> getEnd ());
    if (end -> isFinal ()) {
      double finalWeight = - float (end -> weight_);
      if (path -> getStart () == fsa_ -> initialStateId ()) {
	return BestCell::add (Path::addScore (path, finalWeight));
      } else {
	Cell::PathIterator initialArc = epsilonClosure_ -> find (fsa_ -> initialStateId (), path -> getStart (), Path::NEITHER);
	Cell::PathIterator initialPath = epsilonClosure_ -> find (fsa_ -> initialStateId (), path -> getStart (), Path::KEEP);
	if (initialArc != epsilonClosure_ -> end ()) {
	  if (initialPath == epsilonClosure_ -> end () ||
	      (* initialArc) -> getScore () > (* initialPath) -> getScore ()) {
	    initialPath = initialArc;
	  }
	}
	if (initialPath != epsilonClosure_ -> end ()) {
	  return BestCell::add (Path::connect (* initialPath, path, finalWeight, false));
	}
      }
    }
    return false;
  }
}
