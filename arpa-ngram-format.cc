#include <map>
#include <Core/StringUtilities.hh>
#include <Core/TextStream.hh>
#include <Core/Utility.hh>
#include <Fsa/Application.hh>
#include <Fsa/Output.hh>
#include <Fsa/Semiring.hh>
#include <Fsa/Static.hh>

APPLICATION

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
  void addArc (const std::string & source,
	       const std::string & target,
	       const Fsa::Weight & weight,
	       const Fsa::LabelId & label) {
    Fsa::StateRef s = operator [] (source);
    Fsa::StateRef t = operator [] (target);
    s -> newArc (t -> id (), weight, label);
  }
  void addFinal (const std::string & state,
		 const Fsa::Weight & weight) {
    Fsa::StateRef s = operator [] (state);
    s -> setFinal (weight);
  }
};

/**
 *  SYNOPSIS
 *
 * \data\ 
 * ngram 1=n1
 * ngram 2=n2
 * ...
 * ngram N=nN
 * \1-grams:
 * p w [bow]
 * ...
 * \2-grams:
 * p w1 w2 [bow]
 * ...
 * \N-grams:
 * p w1 ... wN
 * ...
 * \end\
 *
 * DESCRIPTION
 *
 * The so-called ARPA (or Doug Paul) format for N-gram backoff models
 * starts with a header, introduced by the keyword \data\, listing the
 * number of N-grams of each length. Following that, N-grams are
 * listed one per line, grouped into sections by length, each section
 * starting with the keyword \N-gram:, where N is the length of the
 * N-grams to follow. Each N-gram line starts with the logarithm (base
 * 10) of conditional probability p of that N-gram, followed by the
 * words w1...wN making up the N-gram. These are optionally followed
 * by the logarithm (base 10) of the backoff weight for the
 * N-gram. The keyword \end\ concludes the model representation.
 *
 * Backoff weights are required only for those N-grams that form a
 * prefix of longer N-grams in the model. The highest-order N-grams in
 * particular will not need backoff weights (they would be useless).
 *
 * Since log(0) (minus infinity) has no portable representation, such
 * values are mapped to a large negative number. However, the
 * designated dummy value (-99 in SRILM) is interpreted as log(0) when
 * read back from file into memory.
 *
 * The correctness of the N-gram counts n1, n2, ... in the header is
 * not enforced by SRILM software when reading models (although a
 * warning is printed when an inconsistency is encountered). This
 * allows easy textual insertion or deletion of parameters in a model
 * file. The proper format can be recovered by passing the model
 * through the command ngram -order N -lm input -write-lm output
 *
 * Note that the format is self-delimiting, allowing multiple models
 * to be stored in one file, or to be surrounded by ancillary
 * information. Some extensions of N-gram models in SRILM store
 * additional parameters after a basic N-gram section in the standard
 * format.
 */
class ARPA_ngram_format : public Fsa::Application {
private:
  static Core::ParameterString paramInput;
  static Core::ParameterInt paramN;
  static Core::ParameterString paramOutput;
  static double log10toWeightfactor_;
  int n_, N_;
public:
  ARPA_ngram_format () :
    n_ (0),
    N_ (0)
  {
    setTitle ("arpa-ngram-format");
    setDefaultLoadConfigurationFile (false);
  }

  Fsa::Weight log10toWeight (double l) {
    return (l == -99.0) ? Fsa::LogSemiring -> zero () : Fsa::Weight (l / log10toWeightfactor_);
  }

  std::vector <std::string> split (const std::string & line);
  void setState (const std::string &);
  void processCount (const std::vector <std::string> &);
  bool validState () const;
  bool ngramState () const;
  void processNgram (StateMap &, Fsa::StaticAlphabet *, const std::vector <std::string> &);
  
  int main (const std::vector <std::string> & args) {
    N_ = paramN (config);
    
    Core::TextInputStream in (paramInput (config));

    Fsa::StaticAutomaton * fsa =
      new Fsa::StaticAutomaton (Fsa::TypeAcceptor);
    fsa -> setSemiring (Fsa::LogSemiring);
    Fsa::StaticAlphabet * alphabet =
      new Fsa::StaticAlphabet ();
    fsa -> setInputAlphabet (Fsa::ConstAlphabetRef (alphabet));

    StateMap map (fsa);

    std::string line;
    std::vector <std::string> tokens;
    while (validState () && Core::wsgetline (in, line) != EOF) {
      if (line[0] == '\\') {
	setState (line);
      } else {
	tokens = split (line);
	if (! ngramState ()) {
	  processCount (tokens);
	} else if (n_ <= N_) {
	  processNgram (map, alphabet, tokens);
	}
      }
    }

    fsa -> setInitialStateId (map ["<s> "] -> id ());

    Fsa::write (Fsa::ConstAutomatonRef (fsa), paramOutput (config));
    
    return EXIT_SUCCESS;
  }
} app;

/**********************************************************************/

double ARPA_ngram_format::log10toWeightfactor_ (- log10 (exp (1.0)));
Core::ParameterString ARPA_ngram_format::paramInput ("input", "the ngram-format file");
Core::ParameterInt ARPA_ngram_format::paramN ("order", "the order of the model to create", 0, 0);
Core::ParameterString ARPA_ngram_format::paramOutput ("output", "the fsa output file", "ngram.fsa.gz");

/**********************************************************************/

std::vector <std::string> ARPA_ngram_format::split (const std::string & line) {
  std::vector <std::string> tokens;
  Core::StringTokenizer tokenizer (line);
  for (Core::StringTokenizer::iterator token = tokenizer.begin (); token != tokenizer.end (); ++ token) {
    tokens.push_back (* token);
  }
  return tokens;
}

void ARPA_ngram_format::setState (const std::string & line) {
  if (line.compare (0, 6, "\\data\\") == 0) {
    n_ = 0;
  } else {
    int pos = line.find ("-grams:");
    if (pos != std::string::npos) {
      std::istringstream nin (line.substr (1, pos));
      nin >> n_;
    } else {
      n_ = -1;
    }
  }
}

bool ARPA_ngram_format::validState () const {
  return n_ >= 0;
}

bool ARPA_ngram_format::ngramState () const {
  return n_ > 0;
}

void ARPA_ngram_format::processCount (const std::vector <std::string> & tokens) {
  if (tokens[0] == "ngram") {
    std::istringstream nin (tokens [1]);
    int n;
    nin >> n;
    if (paramN (config) == 0 && n > N_) {
      N_ = n;
    }
  }
}

void ARPA_ngram_format::processNgram (StateMap & map, Fsa::StaticAlphabet * alphabet, const std::vector <std::string> & tokens) {
  double w;
  std::string prefix = "";
  
  std::istringstream win (tokens [0]);
  win >> w;
  Fsa::Weight weight = log10toWeight (w);

  // The prefix state is indexed by all but the last token of the
  // n-gram.
  for (int i = 1; i < n_; ++ i) {
    prefix += tokens [i];
    prefix += " ";
  }

  if (weight != Fsa::LogSemiring -> zero ()) {
    if (tokens [n_] == "</s>") {
      // If the final token is </s>, set the final weight of the
      // prefix state.
      map.addFinal (prefix, weight);
    } else {
      // Else create an arc from the prefix state to the suffix state
      // with the conditional weight.  (The suffix state is indexed by
      // the entire n-gram if n < N, or all but the first token if n =
      // N.)
      std::string suffix = "";
      for (int i = (n_ == N_) ? 2 : 1; i <= n_; ++ i) {
	suffix += tokens [i];
	suffix += " ";
      }
      map.addArc (prefix, suffix, weight, alphabet -> addSymbol (tokens [n_]));
    }
  }
  
  if (tokens.size () == (n_ + 2)) {
    // If there is a back-off weight, create an epsilon-arc to the
    // suffix state.
    double bow;
    std::istringstream bowin (tokens.back ());
    bowin >> bow;
    Fsa::Weight backoff_weight = log10toWeight (bow);

    if (backoff_weight != Fsa::LogSemiring -> zero ()) {
      std::string state = prefix + tokens [n_] + " ";
      std::string suffix = state.substr (state.find (' ') + 1);
      map.addArc (state, suffix, backoff_weight, Fsa::Epsilon);
    }
  }
}
