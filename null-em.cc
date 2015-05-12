// Implements expectation maximization (EM) for positioning NULL-aligned words
// in foreign word order.  Sequences of NULL-aligned words that occur in
// sentence-aligned data are provided as part-of-speech-tag contexts, where a
// sequence of length N is represented by N + 2 POS tags, including the
// preceding and following aligned words or sentence boundaries.
//
// NULL-aligned words are attached to neighboring aligned words, or to other
// NULL-aligned words forming a chain.  (Cycles involving only NULL-aligned
// words are not allowed, as this would indicate no foreign word-order
// position.)  The probability of a configuration is represented as the
// independent probability of one NULL-aligned POS attaching to another POS
// either to the left or to the right.
//
// The expectation step of EM is straightforward, as contexts are independent
// under the model.  The total probability of all allowed configurations of each
// context is computed, then for each NULL-aligned word in the context, the
// conditional probabilities of attaching to the left and to the right are
// computed and added to the appropriate expectations.  The maximization step
// merely sets the parameter probabilities to their (normalized) expectations.

#include <Core/Hash.hh>
#include <Core/StringUtilities.hh>
#include <Fsa/Alphabet.hh>

#include "Application.hh"

APPLICATION

// A feature consists of a token, a neighbor, and the direction in {-1,1} from
// the token to the neighbor.
class NullFeature {
public:
  int token;
  int neighbor;
  int direction;
  NullFeature (int t, int n, int d) :
    token (t),
    neighbor (n),
    direction (d)
  {}
};

class EqualNullFeature {
public:
  bool operator () (const NullFeature & a, const NullFeature & b) const {
    return (a.token == b.token)
      && (a.neighbor == b.neighbor)
      && (a.direction == b.direction);
  }
};

class HashNullFeature {
public:
  size_t operator () (const NullFeature & phi) const {
    return (phi.token << 9) + (phi.neighbor << 1) + (phi.direction == 1);
  }
};

typedef __gnu_cxx::hash_map <NullFeature, double, HashNullFeature, EqualNullFeature> FeatureHash;

// NullFeatureOutput is a ternary operator hack to avoid storing the alphabet
// with the feature representation.  The ostream << NullFeature operator
// produces an instance of NullFeatureOutput, and NullFeatureOutput <<
// ConstAlphabetRef outputs the NullFeature to the ostream using the alphabet
// for symbol look-up, then returns the modified ostream.  So, to print out a
// NullFeature, use ostream << NullFeature << ConstAlphabetRef.
class NullFeatureOutput {
public:
  std::ostream & out;
  const NullFeature & feature;
  NullFeatureOutput (std::ostream & o, const NullFeature & f) :
    out (o),
    feature (f)
  {}
};

NullFeatureOutput operator << (std::ostream & out, const NullFeature & f) {
  return NullFeatureOutput (out, f);
}

std::ostream & operator << (const NullFeatureOutput & f, const Fsa::ConstAlphabetRef & alphabet) {
  if (f.feature.direction < 0) {
    f.out << alphabet -> symbol (f.feature.neighbor)
	  << " <- "
	  << alphabet -> symbol (f.feature.token);
  } else {
    f.out << alphabet -> symbol (f.feature.token)
	  << " -> "
	  << alphabet -> symbol (f.feature.neighbor);
  }
  return f.out;
}

////////////////////////////////////////////////////////////////////////////////

typedef std::vector <int> Context;
typedef std::pair <NullFeature, NullFeature> NullFeatures;

// Returns the right-attaching feature and the left-attaching feature,
// in that order.
NullFeatures ContextFeatures (const Context & context, int index) {
  return std::make_pair (NullFeature (context [index], context [index + 1], 1),
			 NullFeature (context [index], context [index - 1], -1));
}

////////////////////////////////////////////////////////////////////////////////

// @bug May be over-generating events by using unconditional probabilities
// rather than the appropriate conditional probabilities.
class NullEM : public Permute::Application {
public:
  NullEM () :
    Permute::Application ("null-em") {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters();

    std::istream & input = this -> input();

    // Each line of the input consists of a sequence of POS-tag tokens
    // corresponding to the context of one or more NULL-aligned words.
    std::vector <Context> data;

    Fsa::StaticAlphabet * posAlphabet = new Fsa::StaticAlphabet;
    Fsa::ConstAlphabetRef alphabet (posAlphabet);

    std::string line;
    while (Core::wsgetline (input, line) != EOF) {
      data.push_back (Context (0));
      Core::StringTokenizer tokenizer (line);
      for (Core::StringTokenizer::iterator token = tokenizer.begin ();
	   token != tokenizer.end (); ++ token) {
	data.back ().push_back (posAlphabet -> addSymbol (* token));
      }
    }

    FeatureHash parameters;

    // Computes the set of parameters from the set of contexts.
    for (std::vector <Context>::const_iterator it = data.begin (); it != data.end (); ++ it) {
      for (int i = 1; i < (it -> size () - 1); ++ i) {
	NullFeatures phi = ContextFeatures (* it, i);
	parameters [phi.first] = 1.0;
	parameters [phi.second] = 1.0;
      }
    }
    
    FeatureHash counts;

    for (int it = 0; it < LEARNING_ITERATIONS; ++ it) {
      double log_likelihood = 0.0;

      for (std::vector <Context>::const_iterator context = data.begin ();
	   context != data.end (); ++ context) {
	std::vector <NullFeature> right_features;
	std::vector <NullFeature> left_features;
	for (int i = 1; i < (context -> size () - 1); ++ i) {
	  NullFeatures phi = ContextFeatures (* context, i);
	  right_features.push_back (phi.first);
	  left_features.push_back (phi.second);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Because of the structure of the parameter counts, we only need to
	// compute the total probability of all events up to and including each
	// index.  The individual events start with the leftmost representing
	// all NULL-aligned words attaching to the right, and end with the
	// rightmost representing all NULL-aligned words attaching to the left.
	// Along the way, each events flips one word from right-attaching to
	// left-attaching, starting with the leftmost word.
	//
	// To that end, the following three loops:
	//
	// 1. compute the probability of the leftmost (all right-attaching)
	//    event;
	//
	// 2. compute the probability of the remaining events from left to right
	//    by changing one attachment from right to left in the previous
	//    event probability, and accumulating;
	//
	// 3. update parameter expectation counts using the total probability of
	//    all events in which the given NULL-aligned word attaches to the
	//    right.
	////////////////////////////////////////////////////////////////////////////////

	std::vector <double> total_probs (context -> size () - 1, 1.0);
	// Initializes the probability of the all-right event.
	for (std::vector <NullFeature>::const_iterator phi = right_features.begin ();
	     phi != right_features.end (); ++ phi) {
	  total_probs [0] *= parameters [* phi];
	}
	double event_prob = total_probs [0];
	// Computes the rest of the cumulative event probabilities.
	for (int i = 0; i < right_features.size (); ++ i) {
	  event_prob *= parameters [left_features [i]] / parameters [right_features [i]];
	  total_probs [i + 1] = total_probs [i] + event_prob;
	}
	// Updates the parameter counts.
	for (int i = 0; i < right_features.size (); ++ i) {
	  double right_prob = total_probs [i] / total_probs.back ();
	  counts [right_features [i]] += right_prob;
	  counts [left_features [i]] += (1.0 - right_prob);
	}
	// Bookkeeping for user updates.
	log_likelihood += ::log (total_probs.back ());
      }

      double total_count = 0.0;
      // Computes the sum of all parameter counts.
      for (FeatureHash::const_iterator count = counts.begin (); count != counts.end (); ++ count) {
	total_count += count -> second;
      }
      // Sets the parameters to the normalized expected counts.
      for (FeatureHash::iterator phi = parameters.begin (); phi != parameters.end (); ++ phi) {
	phi -> second = counts [phi -> first] / total_count;
      }

      // Updates the user.
      std::cerr << "Log likelihood (" << it << "): " << log_likelihood << std::endl;

    }

    for (FeatureHash::const_iterator phi = parameters.begin (); phi != parameters.end (); ++ phi) {
      std::cout << phi -> first << alphabet << " " << phi -> second << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;
