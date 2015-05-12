#include <Fsa/Basic.hh>
#include <Fsa/Cache.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Static.hh>
#include "Limit.hh"

namespace Permute {
  // Constructs a one-state unweighted automaton matching exactly the set of
  // symbols in the given permutation.  Composes this unweighted automaton with
  // the given automaton, trims, and caches the result.
  //
  // @bug Adds multiple arcs matching symbols that appear in the permutation
  // multiple times.  This leads to redundant arcs in the composition, which may
  // harm efficiency.
  Fsa::ConstAutomatonRef limit (Fsa::ConstAutomatonRef fsa,
				const Permutation & permutation) {
    Fsa::StaticAutomaton * limiter = new Fsa::StaticAutomaton (Fsa::TypeAcceptor);
    limiter -> setSemiring (fsa -> semiring ());
    limiter -> setInputAlphabet (fsa -> getInputAlphabet ());

    Fsa::State * only = limiter -> newState (Fsa::StateTagFinal,
					     limiter -> semiring () -> one ());
    limiter -> setInitialStateId (only -> id ());

    for (Permutation::const_iterator index = permutation.begin ();
	 index != permutation.end (); ++ index) {
      only -> newArc (only -> id (),
		      limiter -> semiring () -> one (),
		      permutation.label (* index));
    }

    return Fsa::cache (Fsa::trim (Fsa::composeMatching (Fsa::ConstAutomatonRef (limiter), fsa, false))); 
  }
}
