#include <map>
#include <Core/StringUtilities.hh>
#include <Core/Utility.hh>
#include <Fsa/Static.hh>
#include "nGram.hh"

namespace Permute {
  // Maps strings to states in a given automaton.  Provides helper methods to
  // add arcs and final weights to states indexed by their names.
  class StateMap {
  private:
    Fsa::StaticAutomaton * fsa_;
    std::map <std::string, Fsa::StateRef> map_;
  public:
    typedef std::map <std::string, Fsa::StateRef>::const_iterator iterator;
    StateMap (Fsa::StaticAutomaton * fsa) :
      fsa_ (fsa),
      map_ ()
    {}
    // Returns the named state if it already exists, or creates a new state and
    // adds it to the map with the given name, then returns it.
    Fsa::StateRef operator [] (const std::string & name) {
      std::map <std::string, Fsa::StateRef>::iterator existing =
	map_.find (name);
      if (existing == map_.end ()) {
	Fsa::StateRef state (fsa_ -> newState ());
	map_.insert (std::make_pair (name, state));
	return state;
      } else {
	return existing -> second;
      }
    }
    iterator begin () const {
      return map_.begin ();
    }
    iterator end () const {
      return map_.end ();
    }
    // Creates an arc from the state named by source to the target with the
    // given label and weight.  If such an arc already exists, collects the
    // weight without creating a new arc.
    void addArc (const std::string & source,
		 const Fsa::StateId & target,
		 const Fsa::Weight & weight,
		 const Fsa::LabelId & label) {
      Fsa::StateRef s = operator [] (source);
      for (Fsa::State::iterator arc = s -> begin (); arc != s -> end (); ++ arc) {
	if (arc -> target () == target && arc -> input () == label) {
	  arc -> weight_ = fsa_ -> semiring () -> collect (arc -> weight (), weight);
	  return;
	}
      }
      s -> newArc (target, weight, label);
    }
    // Adds a final weight to the named state or collects the given weight with
    // an existing one.
    void addFinal (const std::string & state,
		   const Fsa::Weight & weight) {
      Fsa::StateRef s = operator [] (state);
      if (s -> isFinal ()) {
	s -> weight_ = fsa_ -> semiring () -> collect (s -> weight_, weight);
      } else {
	s -> setFinal (weight);
      }
    }
  };

  /**********************************************************************/

  // Counts the number of arcs leaving each state with count 1, and adds an
  // epsilon arc to the backoff state (one fewer history tokens) with weight
  // determined by that count, or weight 1 if there are no such arcs.
  void smooth (StateMap & map) {
    for (StateMap::iterator state = map.begin (); state != map.end (); ++state) {
      if (state -> first != "") {
	int hapax = 0;
	Fsa::StateRef backoff = map [state -> first.substr (state -> first.find (' ') + 1)];
	for (Fsa::State::const_iterator arc = state -> second -> begin ();
	     arc != state -> second -> end (); ++ arc) {
	  if (int (arc -> weight ()) == 1) {
	    ++ hapax;
	  }
	}
	map.addArc (state -> first, backoff -> id (), Fsa::Weight (hapax || 1), Fsa::Epsilon);
      }
    }
    return;
  }

  /**********************************************************************/

  // Replaces the counts on the arcs in the wrapped automaton with negative logs
  // of normalized counts.
  class NormalizeCountAutomaton : public Fsa::SlaveAutomaton {
  public:
    NormalizeCountAutomaton (Fsa::ConstAutomatonRef fsa) :
      Fsa::SlaveAutomaton (fsa)
    {}
    virtual Fsa::ConstSemiringRef semiring () const {
      return Fsa::LogSemiring;
    }
    virtual Fsa::ConstStateRef getState (Fsa::StateId s) const {
      Fsa::State * slaveState =
	new Fsa::State (* fsa_ -> getState (s));
      slaveState -> setId (s);
      double count = 0.0;
      for (Fsa::State::const_iterator arc = slaveState -> begin ();
	   arc != slaveState -> end (); ++arc) {
	count += int (arc -> weight ());
      }
      if (slaveState -> isFinal ()) {
	count += int (slaveState -> weight_);
      }
      double denominator = log (count);
      for (Fsa::State::iterator arc = slaveState -> begin ();
	   arc != slaveState -> end (); ++arc) {
	arc -> weight_ =
	  Fsa::Weight (denominator -
		       log (int (arc -> weight ())));
      }
      if (slaveState -> isFinal ()) {
	slaveState -> weight_ =
	  Fsa::Weight (denominator -
		       log (int (slaveState -> weight_)));
      }
      return Fsa::ConstStateRef (slaveState);
    }
    virtual std::string describe () const {
      return "NormalizeCountAutomaton(" + fsa_ -> describe () + ")";
    }
  };

  /**********************************************************************/
  
  // Constructs an n-gram model from the counts on each line of the input.  Each
  // line must consist of a count and an n-gram, all white space delimited.
  // Smooths the counts based on the number of singletons at each state.
  Fsa::ConstAutomatonRef ngram (std::istream & in) {
    Fsa::StaticAutomaton * fsa =
      new Fsa::StaticAutomaton (Fsa::TypeAcceptor);
    fsa -> setSemiring (Fsa::CountSemiring);
    Fsa::StaticAlphabet * alphabet =
      new Fsa::StaticAlphabet ();
    fsa -> setInputAlphabet (Fsa::ConstAlphabetRef (alphabet));

    StateMap map (fsa);

    fsa -> setInitialStateId (map ["<s> "] -> id ());
    
    int count;
    std::string line;
    std::vector <std::string> tokens;

    while (in >> count) {
      Core::wsgetline (in, line);
      Core::StringTokenizer tokenizer (line);
      for (Core::StringTokenizer::iterator token = tokenizer.begin (); token != tokenizer.end (); ++token) {
	tokens.push_back (* token);
      }
      
      if (tokens.back () == "</s>") {
	std::string state = "";
	map.addFinal (state, Fsa::Weight (count));
	for (std::vector <std::string>::reverse_iterator token = ++ tokens.rbegin (); token != tokens.rend (); ++ token) {
	  state = * token + " " + state;
	  map.addFinal (state, Fsa::Weight (count));
	}
      } 

      std::string source, target = "";
      std::vector <std::string>::iterator token = tokens.begin ();
      if (* token == "<s>") {
	target = * token ++ + " ";
      }
      for (; token != tokens.end (); ++ token) {
	source = target;
	target += * token + " ";
	if (token == -- tokens.end ()) {
	  target = target.substr (tokens.front ().length () + 1);
	}
	if (* token != "</s>") {
	  map.addArc (source, map [target] -> id (), Fsa::Weight (count), alphabet -> addSymbol (* token));
	}
      }

      tokens.clear ();
    }

    smooth (map);

    return Fsa::ConstAutomatonRef (new NormalizeCountAutomaton (Fsa::ConstAutomatonRef (fsa)));
  }
}
