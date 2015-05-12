#include <Core/Hash.hh>
#include <Core/Utility.hh>
#include <Fsa/Static.hh>
#include <Fsa/Vector.hh>
#include "SRILM.hh"

namespace Permute {

  SRILMState::SRILMState (int N) :
    n_ (0),
    N_ (N),
    known_max_ (N > 0),
    semiring_ (Fsa::TropicalSemiring)
  {}

  // Returns the current n-gram size.
  int SRILMState::n () const {
    return n_;
  }
  // Returns whether the current n-gram size is the maximum.
  bool SRILMState::full () const {
    return n_ == N_;
  }
  // Parses a line beginning with '\'.  Sets the state of the SRILM from the
  // line.  If the line is '\data\', resets n_ to 0.  If the line is '\x-grams:',
  // sets n to x.  Otherwise, does nothing.
  void SRILMState::set (const std::string & line) {
    if (line.compare (0, 6, "\\data\\") == 0) {
      n_ = 0;
    } else {
      int pos = line.find ("-grams:");
      if (pos != line.npos) {
	std::istringstream nin (line.substr (1, pos));
	nin >> n_;
      } else {
	n_ = -1;
      }
    }
  }
  // Parses a line containing an n-gram count.  If the maximum desired n-gram
  // size is known, does nothing.  Otherwise, sets N_ to max(N_, N), where N is
  // the size indicated in the given line.
  void SRILMState::count (const std::string & line) {
    if (known_max_) {
      return;
    }
    if (line.compare (0, 6, "ngram ") == 0) {
      std::istringstream nin (line.substr (6));
      int N;
      nin >> N;
      N_ = std::max (N_, N);
    }
  }
  // The state is valid if n_ is nonnegative.
  bool SRILMState::valid () const {
    return n_ >= 0;
  }
  // The state is parsing n-grams if n_ is positive.  (n_ is zero when parsing
  // the \data\ section.)
  bool SRILMState::ngram () const {
    return n_ > 0;
  }
  // Reads each line from the given stream.  If the line begins with '\', calls
  // set.  If not in an n-gram state, calls count.  Otherwise, tokenizes the
  // line, reads in its weight, each token, and the backoff weight, if present,
  // then calls process_ngram with the weights and tokens.
  void SRILMState::read (std::istream & in) {
    std::string line;
    while (valid () && Core::wsgetline (in, line) != EOF) {
      readline (line);
    }
  }
  void SRILMState::readline (const std::string & line) {
    if (line [0] == '\\') {
      set (line);
    } else if (ngram ()) {
      std::istringstream line_in (line);
      Fsa::Weight ngw = weight (line_in);
      std::vector <std::string> tokens (n ());
      for (std::vector <std::string>::iterator t = tokens.begin (); t != tokens.end (); ++ t) {
	line_in >> (* t);
      }
      Fsa::Weight bow = weight (line_in);
      process_ngram (ngw, tokens, bow);
    } else {
      count (line);
    }
  }
  // Reads a double from the given input stream and multiplies it by
  // log10toWeightFactor unless it is -99, in which case returns semiring zero.
  Fsa::Weight SRILMState::weight (std::istream & in) const {
    double f;
    if (in >> f && f != -99.0) {
      return Fsa::Weight (f * log10toWeightFactor);
    } else {
      return semiring () -> zero ();
    }
  }

  // Call our weight - ln x and their weight log_10 x.  Then since log_10 x = ln
  // x / ln 10, - ln x = log_10 x * (- ln 10).
  double SRILMState::log10toWeightFactor (- log (10.0));

  /**********************************************************************/
  
  // Maps strings consisting of token* to language model states.  Provides
  // helper functions for adding arcs, final weights, and backoff weights.
  class StateMap {
  private:
    Fsa::StaticAutomaton * fsa_;
    typedef Core::StringHashMap <Fsa::StateRef> Map;
    Map map_;
    typedef std::map <Fsa::StateId, Fsa::Weight> Backoff;
    Backoff backoff_;
  public:
    StateMap (Fsa::StaticAutomaton * fsa) :
      fsa_ (fsa),
      map_ (),
      backoff_ ()
    {}
    Fsa::StateRef operator [] (const std::string & name);
    void addArc (const std::string & source,
		 const std::string & target,
		 const Fsa::Weight & weight,
		 const Fsa::LabelId & label);
    void addFinal (const std::string & state,
		   const Fsa::Weight & weight);
    void addBackoff (const std::string & state,
		     const Fsa::Weight & weight);
    void backoff ();
  private:
    void backoff (const Map::value_type & v);
    std::string backoff_state (const std::string & name) const;
    void backoff_arcs (Fsa::StateRef s, Fsa::StateRef bs, Fsa::Weight b);
  };
  
  // Returns the state corresponding to the given name, if it exists, or
  // creates a new state, assigns it the given name, and returns it.
  Fsa::StateRef StateMap::operator [] (const std::string & name) {
    Map::const_iterator existing = map_.find (name);
    if (existing == map_.end ()) {
      Fsa::StateRef state (fsa_ -> newState ());
      map_.insert (Map::value_type (name, state));
      return state;
    } else {
      return existing -> second;
    }
  }
  // Adds an arc from the state named by source to the state named by target
  // with the given label and weight.
  void StateMap::addArc (const std::string & source,
			 const std::string & target,
			 const Fsa::Weight & weight,
			 const Fsa::LabelId & label) {
    operator [] (source) -> newArc (operator [] (target) -> id (), weight, label);
  }
  // Sets the final weight of the state named by state.
  void StateMap::addFinal (const std::string & state,
			   const Fsa::Weight & weight) {
    operator [] (state) -> setFinal (weight);
  }
  // Adds a backoff weight to the backoff_ map for the given state.
  void StateMap::addBackoff (const std::string & state,
			     const Fsa::Weight & weight) {
    backoff_.insert (Backoff::value_type (operator [] (state) -> id (), weight));
  }
  // Iterates over the states.  For each one with a backoff weight, adds arcs
  // for unseen symbols.
  void StateMap::backoff () {
    for (Map::const_iterator i = map_.begin (); i != map_.end (); ++ i) {
      backoff (* i);
    }
  }
  // Receives a pair of state name and state id.  If the id has an associated
  // backoff weight, computes and finds the backoff_state of the given name,
  // calls backoff on that state, calls backoff_arcs, then removes this state
  // from the map.
  void StateMap::backoff (const Map::value_type & v) {
    Backoff::iterator b = backoff_.find (v.second -> id ());
    if (b == backoff_.end ()) {
      return;
    } else {
      Map::const_iterator i = map_.find (backoff_state (v.first));
      if (i != map_.end ()) {
	backoff (* i);
	backoff_arcs (v.second, i -> second, b -> second);
	backoff_.erase (b);
      } else {
	std::cerr << "StateMap::backoff: " << v.first << std::endl;
      }
    }
  }
  // Removes one token from the front of the given name.
  std::string StateMap::backoff_state (const std::string & name) const {
    return name.substr (name.find (' ') + 1);
  }
  // Ensures that the first given state accepts the same set of symbols as the
  // second given state (the backoff state).  When an arc is not already
  // present, adds one with the weight of the backoff state's arc extended by
  // the given weight.  This includes the </s> symbol corresponding to final
  // weight.
  void StateMap::backoff_arcs (Fsa::StateRef s, Fsa::StateRef bs, Fsa::Weight b) {
//     std::set <Fsa::LabelId> symbols;
//     for (Fsa::State::const_iterator i = s -> begin (); i != s -> end (); ++ i) {
//       symbols.insert (i -> input_);
//     }
//     for (Fsa::State::const_iterator i = bs -> begin (); i != bs -> end (); ++ i) {
//       if (symbols.find (i -> input_) == symbols.end ()) {
// 	s -> newArc (i -> target_,
// 		     fsa_ -> semiring () -> extend (i -> weight_, b),
// 		     i -> input_);
//       }
//     }
//     if (! s -> isFinal ()) {
//       s -> setFinal (fsa_ -> semiring () -> extend (bs -> weight_, b));
//     }
    s -> newArc (bs, b, Fsa::Epsilon);
  }

  /**********************************************************************/

  // Implements process_ngram to complete SRILMState, and multiplies all weights
  // by a given lambda.
  class WeightedSRILMState : public SRILMState {
  private:
    double lambda_;
    Fsa::StaticAlphabet * alphabet_;
    StateMap & states_;
  public:
    WeightedSRILMState (Fsa::StaticAlphabet * alphabet, StateMap & states, double lambda = 1.0, int N = 0) :
      SRILMState (N),
      lambda_ (lambda),
      alphabet_ (alphabet),
      states_ (states)
    {}
    // First, checks that all the tokens in the given n-gram are valid.  Next
    // computes the prefix and suffix state names.  The prefix state contains
    // all but the last token, and the suffix state contains all tokens, or all
    // but the first if the n-gram is full.
    //
    // Adds a final state or an arc depending on the last token of the n-gram
    // (is it '</s>'?) if the given n-gram weight is not semiring zero, and adds
    // a backoff weight to the suffix if the given backoff weight is not
    // semiring zero.
    void process_ngram (Fsa::Weight ngw, const std::vector <std::string> & ngram, Fsa::Weight bow) const {
//       for (std::vector <std::string>::const_iterator token = ngram.begin (); token != ngram.end (); ++ token) {
// 	if (! valid (* token)) {
// 	  return;
// 	}
//       }
      std::ostringstream prefix, suffix;
      std::copy (ngram.begin (), -- ngram.end (),
		 std::ostream_iterator <std::string> (prefix, " "));
      std::copy (full () ? ++ ngram.begin () : ngram.begin (), ngram.end (),
		 std::ostream_iterator <std::string> (suffix, " "));
      
      if (ngw != semiring () -> zero ()) {
	if (ngram.back () == "</s>") {
	  states_.addFinal (prefix.str (), ngw);
	} else {
	  states_.addArc (prefix.str (), suffix.str (), ngw, alphabet_ -> addSymbol (ngram.back ()));
	}
      }

      if (bow != semiring () -> zero ()) {
	states_.addBackoff (suffix.str (), bow);
      }
    }
    // A symbol is valid if it is a boundary symbol ('<s>' or '</s>') or it is
    // in the alphabet.
    bool valid (const std::string & symbol) const {
      if (symbol == "<s>" || symbol == "</s>") {
	return true;
      } else {
	return (alphabet_ -> index (symbol) != Fsa::InvalidLabelId);
      }
    }
    // Multiplies the weight returned by SRILMState::weight by lambda_.
    Fsa::Weight weight (std::istream & in) const {
      Fsa::Weight w (SRILMState::weight (in));
      if (w == semiring () -> zero ()) {
	return w;
      } else {
	return Fsa::Weight (lambda_ * double (w));
      }
    }
  };

  /**********************************************************************/

  Fsa::ConstAutomatonRef srilm (std::istream & in, int order, double weight) {
    // Constructs an acceptor.
    Fsa::StaticAutomaton * fsa = new Fsa::StaticAutomaton (Fsa::TypeAcceptor);
    Fsa::StaticAlphabet * alphabet = new Fsa::StaticAlphabet;
    fsa -> setSemiring (Fsa::TropicalSemiring);
    fsa -> setInputAlphabet (Fsa::ConstAlphabetRef (alphabet));

    // Constructs n-gram arcs.
    StateMap states (fsa);
    WeightedSRILMState status (alphabet, states, weight, order);
    status.read (in);

    // Constructs backoff arcs.
    states.backoff ();

    // Adds an arc matching the <OOV> symbol.
    states.addArc ("", "", Fsa::TropicalSemiring -> one (), alphabet -> addSymbol ("<OOV>"));

    // Sets the initial state.
    fsa -> setInitialStateId (states ["<s> "] -> id ());
    
    return Fsa::ConstAutomatonRef (fsa);
  }

}
