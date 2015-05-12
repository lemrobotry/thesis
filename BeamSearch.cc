#include <utility>
#include <Core/PriorityQueue.hh>
#include <Fsa/Static.hh>
#include "BeamSearch.hh"

namespace Permute {
  struct Predecessor {
    Fsa::StateId state;
    Fsa::Arc arc;
    float score;
    Predecessor () {}
    Predecessor (Fsa::StateId state, const Fsa::Arc & arc, float score) :
      state (state), arc (arc), score (score) {}
  };
  typedef std::map <Fsa::StateId, Predecessor> PredecessorMap;

  typedef std::pair <Fsa::StateId, float> StateWeightPair;
  struct StateWeightState {
    Fsa::StateId operator () (const StateWeightPair & swp) const {
      return swp.first;
    }
  };
  struct StateWeightLess {
    bool operator () (const StateWeightPair & left, const StateWeightPair & right) const {
      return left.second < right.second;
    }
  };
  
  typedef Core::TracedPriorityQueue <StateWeightPair,
				     Fsa::StateId,
				     StateWeightState,
				     StateWeightLess> PriorityQueue;

  bool enqueue (PriorityQueue *, int, PredecessorMap &, const StateWeightPair &);
				     
  // Builds a priority queue with width fixed to the given parameter.  Initially
  // populates the queue with the given lattice's initial state.  Then, iterates
  // over the state/weight pairs in the queue, in priority order, extending each
  // entry in the queue by an additional arc (does not distinguish epsilon
  // arcs), and putting the extension into the next queue.  Accumulates the best
  // path found.
  //
  // BUG: It appears that current is not used inside the loop once its contents
  // are extracted into the ordered vector.  Why do we need a separate next
  // queue?
  //
  // Constructs an automaton from the predecessors of the best final state and
  // returns it, along with its score.
  std::pair <Fsa::ConstAutomatonRef, float> beamSearch (Fsa::ConstAutomatonRef lattice, int width) {
    PredecessorMap predecessor;
    std::vector <StateWeightPair> ordered;

    StateWeightPair best (Fsa::InvalidStateId, Core::Type <float>::min);

    PriorityQueue * current = new PriorityQueue (width),
      * next = new PriorityQueue (width);

    current -> insert (std::make_pair (lattice -> initialStateId (), 0.0f));

    while (! current -> empty ()) {
      ordered.clear ();
      while (! current -> empty ()) {
	ordered.push_back (current -> top ());
	current -> pop ();
      }
      for (std::vector <StateWeightPair>::reverse_iterator s = ordered.rbegin ();
	   s != ordered.rend (); ++ s) {
	Fsa::ConstStateRef state = lattice -> getState (s -> first);
	for (Fsa::State::const_iterator a = state -> begin (); a != state -> end (); ++ a) {
	  StateWeightPair t (a -> target (), s -> second - float (a -> weight ()));
	  PredecessorMap::iterator pred = predecessor.find (t.first);
	  if (pred != predecessor.end ()) {
	    if (t.second > pred -> second.score) {
	      pred -> second = Predecessor (s -> first, * a, t.second);
	      if (next -> contains (t.first)) {
		next -> update (t);
	      }
	    }
	  } else if (enqueue (next, width, predecessor, t)) {
	    predecessor [t.first] = Predecessor (s -> first, * a, t.second);
	  }
	}
	if (state -> isFinal ()) {
	  float weight = s -> second - float (state -> weight_);
	  if (weight > best.second) {
	    best = std::make_pair (s -> first, weight);
	  }
	}
      }
      // Swap current and next.
      PriorityQueue * temp = current;
      current = next;
      next = temp;
    }

    delete current;
    delete next;

    Fsa::StaticAutomaton * bestPath = new Fsa::StaticAutomaton (lattice -> type ());
    bestPath -> setProperties (Fsa::PropertyLinear | Fsa::PropertyAcyclic);
    bestPath -> setInputAlphabet (lattice -> getInputAlphabet ());
    if (lattice -> type () == Fsa::TypeTransducer) {
      bestPath -> setOutputAlphabet (lattice -> getOutputAlphabet ());
    }
    bestPath -> setSemiring (lattice -> semiring ());

    Fsa::ConstStateRef latticeState = lattice -> getState (best.first);
    Fsa::StateRef successor (bestPath -> newState (latticeState -> tags (), latticeState -> weight_));

    for (PredecessorMap::const_iterator sa = predecessor.find (best.first);
	 sa != predecessor.end (); sa = predecessor.find (sa -> second.state)) {

      latticeState = lattice -> getState (sa -> second.state);
      Fsa::StateRef state (bestPath -> newState (latticeState -> tags ()));
      state -> unsetFinal ();
      
      Fsa::Arc * arc = state -> newArc ();
      * arc = sa -> second.arc;
      arc -> target_ = successor -> id ();

      successor = state;
    }

    bestPath -> setInitialStateId (successor -> id ());
    return std::make_pair (Fsa::ConstAutomatonRef (bestPath), best.second);
  }

  // If the given priority queue has not yet reached the given width, always
  // inserts the given state/weight pair.  Otherwise, if the given pair has a
  // better score than the pair at the top of the queue (the worst pair in the
  // queue), replaces the top with the given pair (which leads to a heap
  // update).  Returns true in either case.  Otherwise, does nothing and returns
  // false---the given pair is not among the best partial paths.
  bool enqueue (PriorityQueue * q, int width, PredecessorMap & p, const StateWeightPair & pair) {
    if (q -> size () < width) {
      q -> insert (pair);
      return true;
    } else if (pair.second > q -> top ().second) {
      p.erase (q -> top ().first);
      q -> changeTop (pair);
      return true;
    } else {
      return false;
    }
  }
}
