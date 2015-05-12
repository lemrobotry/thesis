#include <Fsa/Dfs.hh>
#include <Fsa/Hash.hh>
#include <Fsa/Static.hh>
#include <Fsa/Sort.hh>
#include "Minimize.hh"

namespace Permute {

  // Defines arc precedence by first target state, then input, then output, then
  // weight.
  struct byTargetAndArc : public std::binary_function <Fsa::Arc, Fsa::Arc, bool> {
    bool operator () (const Fsa::Arc & a, const Fsa::Arc & b) const {
      if (a.target () < b.target ()) {
	return true;
      } else if (a.target () == b.target ()) {
	if (a.input () < b.input ()) {
	  return true;
	} else if (a.input () == b.input ()) {
	  if (a.output () < b.output ()) {
	    return true;
	  } else if (a.output () == b.output ()) {
	    return (a.weight () < b.weight ());
	  }
	}
      }
      return false;
    }
  };

  // Sorts the arcs of each state in the given automaton using byTargetAndArc.
  class SortByTargetAndArcAutomaton : public Fsa::ModifyAutomaton {
  public:
    SortByTargetAndArcAutomaton (Fsa::ConstAutomatonRef f) :
      Fsa::ModifyAutomaton (f)
    {}
    virtual void modifyState (Fsa::State * sp) const {
      sp -> sort (byTargetAndArc ());
    }
    virtual std::string describe () const {
      return "sort(" + fsa_ -> describe () + ")";
    }
  };

  Fsa::ConstAutomatonRef sort (Fsa::ConstAutomatonRef f) {
    return Fsa::ConstAutomatonRef (new SortByTargetAndArcAutomaton (f));
  }

  // Evaluates state equivalence by comparing both final weights and arc
  // vectors.  Finality is compared using both the isFinal flag and the final
  // weight.  Arc equivalence uses the arc != operator.
  struct StatesEquivalent {
    // Two states are equivalent if they have the same arcs and final
    // weight.
    bool operator () (Fsa::ConstStateRef sp1, Fsa::ConstStateRef sp2) const {
      return (finalEquiv (sp1, sp2) && arcsEquiv (sp1, sp2));
    }
    bool finalEquiv (Fsa::ConstStateRef sp1, Fsa::ConstStateRef sp2) const {
      if (sp1 -> isFinal ()) {
	if (sp2 -> isFinal () && sp1 -> weight_ == sp2 -> weight_) {
	  // Both final with same weight.
	  return true;
	} else {
	  // 1 is final but 2 is not or weights differ.
	  return false;
	}
      } else if (sp2 -> isFinal ()) {
	// 2 is final but 1 is not.
	return false;
      } else {
	// Neither final.
	return true;
      }
    }
    bool arcsEquiv (Fsa::ConstStateRef sp1, Fsa::ConstStateRef sp2) const {
      Fsa::State::const_iterator a1, a2;
      for (a1 = sp1 -> begin (), a2 = sp2 -> begin ();
	   a1 != sp1 -> end () && a2 != sp2 -> end ();
	   ++ a1, ++ a2) {
	// Any different arcs are fatal.
	if ((* a1) != (* a2)) {
	  return false;
	}
      }
      // Different number of arcs is also fatal.
      if (a1 == sp1 -> end () && a2 == sp2 -> end ()) {
	return true;
      } else {
	return false;
      }
    }
  };

  // Hashes a state using a function of the final weight, if applicable, and the
  // set of arcs.  Any two states with the same isFinal flag, final weight, and
  // sequence of arcs will hash the same.
  struct StateHashKey {
    u32 operator () (Fsa::ConstStateRef sp) const {
      u32 value = 0;
      if (sp -> isFinal ()) {
	value = sp -> weight_;
      }
      for (Fsa::State::const_iterator a = sp -> begin (); a != sp -> end (); ++ a) {
	value = 337 * value +
	  a -> target () +
	  5 * a -> input () +
	  17 * a -> output () +
	  65 * u32 (a -> weight ());
      }
      return value;
    }
  };

  // When discovering a new state, hashes it to see if an equivalent state
  // already exists, and if so, sets the equivalent state of this state.  When
  // finishing a state, creates a new state in a minimal automaton, but only
  // once for each set of equivalent states.  Replaces arc targets with the
  // corresponding equivalent state number.
  class MinimizeDfs : public Fsa::DfsState {
  private:
    typedef Fsa::Hash <Fsa::ConstStateRef, StateHashKey, StatesEquivalent> StateHash;
    StateHash states_;
    Fsa::Vector <Fsa::StateId> equivalent_;
    Fsa::StaticAutomaton * minimal_;
    bool smaller_;
  public:
    MinimizeDfs (Fsa::ConstAutomatonRef f) :
      Fsa::DfsState (f),
      states_ (),
      equivalent_ (),
      minimal_ (new Fsa::StaticAutomaton (f -> type ())),
      smaller_ (false)
    {
      minimal_ -> setProperties (f -> knownProperties (), f -> getProperties ());
      minimal_ -> setSemiring (f -> semiring ());
      minimal_ -> setInputAlphabet (f -> getInputAlphabet ());
      minimal_ -> setOutputAlphabet (f -> getOutputAlphabet ());
    }
    virtual void discoverState (Fsa::ConstStateRef sp) {
      // Hash the state using its old arcs and add it to the
      // equivalent_ table.
      Fsa::StateId equiv = states_.insert (sp);
      equivalent_.grow (sp -> id () + 1);
      equivalent_ [sp -> id ()] = equiv;
    }
    virtual void finishState (Fsa::StateId s) {
      // Replace the targets of all arcs with the state ids from the
      // equivalent_ table, but only if the state has not already been
      // created.
      if (minimal_ -> getState (equivalent_ [s])) {
	smaller_ = true;
      } else {
	Fsa::State * sp = new Fsa::State (* fsa_ -> getState (s));
	sp -> setId (equivalent_ [s]);
	for (Fsa::State::iterator a = sp -> begin (); a != sp -> end (); ++ a) {
	  a -> target_ = equivalent_ [a -> target ()];
	}
	minimal_ -> setState (sp);
      }
    }
    virtual void finish () {
      minimal_ -> setInitialStateId (equivalent_ [fsa_ -> initialStateId ()]);
    }
    bool smaller () const {
      return smaller_;
    }
    Fsa::ConstAutomatonRef result () const {
      return Fsa::ConstAutomatonRef (minimal_);
    }
  };

  // Repeatedly invokes MinimizeDfs until the automaton fails to grow smaller,
  // then returns the result.
  Fsa::ConstAutomatonRef minimize (Fsa::ConstAutomatonRef f) {
    f = sort (f);
    while (true) {
      MinimizeDfs minimizer (f);
      minimizer.dfs ();
      if (minimizer.smaller ()) {
	f = minimizer.result ();
      } else {
	return f;
      }
    }
  }
}
