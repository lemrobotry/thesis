#include "Application.hh"

APPLICATION

using namespace Permute;

// Tests whether the target permutation is in the ITG neighborhood of the
// identity permutation using a shift-reduce parser.  The parser keeps spans on
// the stack, and joins spans sharing endpoints if they are neighbors on the
// stack.  If, when the complete permutation has been parsed, the stack consists
// of a single span, then the permutation is in the ITG neighborhood.
class ShiftReduceITG : public Application {
private:

public:
  ShiftReduceITG () :
    Application ("shift-reduce-itg")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    std::istream & input = this -> input ();

    Permutation source, target, pos;

    for (int sentence = 1;
	 sentence <= SENTENCES && readPermutationWithAlphabet (source, input);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, input);
      target = source;
      readAlignment (target, input);

      // Source contains the identity permutation and target contains another
      // permutation.  Parses target to see if it is in the neighborhood of
      // source.
      typedef std::pair <size_t, size_t> Span;
      std::vector <Span> s;
      for (Permutation::const_iterator it = target.begin (); it != target.end (); ++ it) {
	// Shifts the current target index.
	s.push_back (std::make_pair (* it, 1 + (* it)));
	// Reduces the top two spans on the stack while it can.
	while (s.size () > 1) {
	  const Span & span1 = s [s.size () - 2];
	  const Span & span2 = s.back ();
	  if (span1.second == span2.first) {
	    Span combined = std::make_pair (span1.first, span2.second);
	    s.pop_back ();
	    s.pop_back ();
	    s.push_back (combined);
	  } else if (span1.first == span2.second) {
	    Span combined = std::make_pair (span2.first, span1.second);
	    s.pop_back ();
	    s.pop_back ();
	    s.push_back (combined);
	  } else {
	    break;
	  }
	}
      }

      // If the stack is size 1, then we successfully parsed the target
      // permutation.
      std::cout << sentence << '\t' << source.size () << '\t'
		<< (s.size () == 1 ? "TRUE" : "FALSE") << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;
