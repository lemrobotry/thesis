#include "Application.hh"
#include "Permutation.hh"

APPLICATION

// Counts the number of symbols in the output alphabet according to the provided
// ttable for the set of sentences in the input, and writes out an R-compatible
// table with the results.
class AlphabetSize : public Permute::Application {
public:
  AlphabetSize () : Permute::Application ("alphabet-size") {}

  int main (const std::vector <std::string> & args) {

    Permute::Permutation p;

    std::ostream & target = this -> output ();
    target << "Length Symbols" << std::endl;
    
    std::istream & source = this -> input ();
    for (int i = 1; readPermutationWithAlphabet (p, source); ++ i) {
      Fsa::ConstAutomatonRef channel = this -> ttable (p);
      Fsa::ConstAlphabetRef output = channel -> getOutputAlphabet ();
      int size = 0;
      for (Fsa::Alphabet::const_iterator s = output -> begin (); s != output -> end (); ++ s, ++ size);
      target << i << " " << p.size () << " " << size << std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;
