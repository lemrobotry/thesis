#include <Fsa/Static.hh>

#include "Distortion.hh"

namespace Permute {

  Fsa::ConstAutomatonRef distortion (const Permutation & p, double d) {
    // For now, assumes (possibly erroneously) that the tokens in the sentence
    // are unique.
    Fsa::StaticAutomaton * fsa = new Fsa::StaticAutomaton (Fsa::TypeAcceptor);
    fsa -> setSemiring (Fsa::LogSemiring);
    fsa -> setInputAlphabet (p.alphabet ());

    Fsa::State * initial (new Fsa::State (0));
    fsa -> setState (initial);
    fsa -> setInitialStateId (0);

    // Constructs a bigram model where i -> j with weight d * |j - i - 1|.
    for (Permutation::const_iterator i = p.begin (); i != p.end (); ++ i) {
      // Creates a state numbered according to position i.
      Fsa::State * state (new Fsa::State (* i + 1));
      // NOTE: no penalty for ending the sentence anywhere.
      state -> setFinal (Fsa::LogSemiring -> one ());
      fsa -> setState (state);
      // NOTE: penalty for starting the sentence anywhere but zero.
      initial -> newArc (state -> id (), Fsa::Weight (d * (* i)), p.label (* i));
      
      for (Permutation::const_iterator j = p.begin (); j != p.end (); ++ j) {
	if (j == i) continue;
	Fsa::Weight w (d * ::abs (* j - * i - 1));
	state -> newArc (* j + 1, w, p.label (* j));
      }
    }

    return Fsa::ConstAutomatonRef (fsa);
  }
  
}
