#include <queue>
#include "EpsilonClosure.hh"
#include "FullCell.hh"

namespace Permute {
  // Given a cell containing all the epsilon arcs in an automaton, computes the
  // closure of these arcs.  Uses a queue to process epsilon paths.  First,
  // enqueues each of the epsilon arcs.  Then, for each path on the queue, finds
  // all epsilon arcs that begin with the path's end state, and extends the path
  // by those arcs, enqueuing each resulting path if it is novel.  A path is
  // novel if it is not already contained in the epsilon closure cell.
  ConstCellRef epsilonClosure (const ConstCellRef & epsilonArcs) {
    Cell * epsilonClosure (new FullCell ());
    std::queue <ConstPathRef> q;
    for (Cell::PathIterator path = epsilonArcs -> begin (); path != epsilonArcs -> end (); ++path) {
      if (epsilonClosure -> add (* path)) {
	q.push (* path);
      }
    }
    while (! q.empty ()) {
      Fsa::StateId middle = q.front () -> getEnd ();
      for (Cell::PathIterator right = epsilonArcs -> begin (middle, Path::NEITHER);
	   right != epsilonArcs -> end ();
	   right = epsilonArcs -> next (right, middle, Path::NEITHER)) {
	ConstPathRef connected = Path::connect (q.front (), * right, 0.0, false);
	if (epsilonClosure -> add (connected)) {
	  q.push (connected);
	}
      }
      q.pop ();
    }
    return ConstCellRef (epsilonClosure);
  }
}
