#include <ext/hash_map>
#include <numeric>

#include <Core/Hash.hh>
#include <Core/StringUtilities.hh>
#include <Core/Utility.hh>

#include <Fsa/Static.hh>

#include "Iterator.hh"
#include "Minimize.hh"
#include "TTable.hh"

namespace Permute {

  class WeightTimesNegativeLog :
    public std::binary_function <double, float, double> {
  public:
    double operator () (double w, float p) const {
      return w * - std::log (p);
    }
  };

  Fsa::Weight TTableWeights::weight (const std::string & line) const {
    std::istringstream in (line);
    std::vector <float> v;
    std::copy (std::istream_iterator <float> (in),
	       std::istream_iterator <float> (),
	       std::back_inserter (v));
    return weight (v);
  }

  Fsa::Weight TTableWeights::weight (const std::vector <float> & v) const {
    return Fsa::Weight (std::inner_product (weightt_.begin (), weightt_.end (),
					    v.begin (), 0.0,
					    std::plus <double> (),
					    WeightTimesNegativeLog ()));
  }

  /**********************************************************************/

  class CandidateWeight :
    public std::unary_function <const StringTgtCand &, double>
  {
  private:
    const WeightModel & model_;
    double weightw_;
  public:
    CandidateWeight (const WeightModel & model, double weightw) :
      model_ (model),
      weightw_ (weightw)
    {}
    double operator () (const StringTgtCand & cand) const;
  };

  // Negates the word count to match Moses.  See, e.g., TargetPhrase.cpp:115.
  double CandidateWeight::operator () (const StringTgtCand & cand) const {
    return static_cast <double> (model_.weight (cand.second))
      + weightw_ * static_cast <double> (cand.first.size ());
  }

  /**********************************************************************/

  Fsa::ConstAutomatonRef ttable (const Permutation & p,
				 Fsa::ConstAlphabetRef alphabet,
				 const WeightModel & model,
				 const PhraseDictionaryTree & tree,
				 double weightw) {
    Fsa::ConstSemiringRef semiring = Fsa::TropicalSemiring;
    CandidateWeight candWeight (model, weightw);
    // Constructs the transducer
    Fsa::StaticAutomaton * fsa = new Fsa::StaticAutomaton (Fsa::TypeTransducer);
    fsa -> setSemiring (semiring);
    fsa -> setInputAlphabet (p.alphabet ());
    fsa -> setOutputAlphabet (alphabet);
    Fsa::StateRef initial (fsa -> newFinalState (semiring -> one ()));
    fsa -> setInitialStateId (initial -> id ());

    std::vector <StringTgtCand> candidates;

    for (Permutation::const_iterator i = p.begin (); i != p.end (); ++ i) {
      PhraseDictionaryTree::PrefixPtr ptr = tree.GetRoot ();
      for (Permutation::const_iterator j = i; j != p.end (); ++ j) {
	ptr = tree.Extend (ptr, p.alphabet () -> symbol (p.label (* j)));
	if (! ptr) {
	  if (j == i) {
	    // OOV words pass through.  Weight doesn't matter---set to semiring one.
	    Fsa::LabelId iLabel = p.label (* j),
// 	      oLabel = alphabet -> addSymbol (Core::convertToLowerCase (p.alphabet () -> symbol (iLabel)));
	      oLabel = alphabet -> index ("<OOV>");
	    initial -> newArc (initial -> id (), semiring -> one (), iLabel, oLabel);
	  }
	  break;
	} else {
	  candidates.clear ();
	  tree.GetTargetCandidates (ptr, candidates);
	  for (std::vector <StringTgtCand>::const_iterator cand = candidates.begin ();
	       cand != candidates.end (); ++ cand) {
	    Fsa::Weight w (candWeight (* cand));
	    Fsa::StateRef from = initial;
	    Permutation::const_iterator input = i, input_end = j + 1;
	    std::vector <std::string const *>::const_iterator output = cand -> first.begin (),
	      output_end = cand -> first.end ();
	    while (input != input_end || output != output_end) {
	      Fsa::LabelId iLabel = (input == input_end) ? Fsa::Epsilon : p.label (* input ++);
	      Fsa::LabelId oLabel = (output == output_end) ?
		Fsa::Epsilon :
		alphabet -> index (Core::convertToLowerCase (* * output ++));
	      Fsa::StateRef to = (input != input_end || output != output_end)
		? Fsa::StateRef (fsa -> newState ())
		: initial;
	      from -> newArc (to -> id (), w, iLabel, oLabel);
	      // use the weight only for the first generated arc
	      w = semiring -> one ();
	      from = to;
	    }
	  }
	}
      }
    }

    return minimize (Fsa::ConstAutomatonRef (fsa));
  }
}
