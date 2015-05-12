#include <Fsa/Best.hh>
#include <Fsa/Cache.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Linear.hh>
#include <Fsa/Output.hh>
#include <Fsa/Project.hh>

#include "Application.hh"
#include "Fsa.hh"
#include "Iterator.hh"

APPLICATION

using namespace Permute;

class MonotoneTranslate : public Application {
private:
  BestOutput bestOutput_;
public:
  MonotoneTranslate () :
    Application ("monotone-translate"),
    bestOutput_ ()
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    std::istream & in = this -> input ();
    std::ostream & out = this -> output ();

    std::cerr << "TTABLE_FILE: " << TTABLE_FILE << std::endl
	      << "LMODEL_FILE: " << LMODEL_FILE << std::endl;

    std::vector <double> weightt = paramTTableWeights (config);
    std::cerr << "TTable Weights: "
	      << delimit (weightt.begin (), weightt.end (), " ")
	      << std::endl
	      << "Word Penalty: "
	      << WORD_WEIGHT
	      << std::endl;

    Fsa::ConstAutomatonRef language = this -> lmodel ();
      
    while (readPermutationWithAlphabet (source, in)) {
      readPermutationWithAlphabet (pos, in);
      if (DEPENDENCY) {
	readParents (parents, in);
	readPermutationWithAlphabet (labels, in);
      }
      target = source;
      readAlignment (target, in);

      std::cerr << "Source:  " << source.size () << std::endl;

      Fsa::ConstAutomatonRef
	sentence = fsa (source, Fsa::TropicalSemiring),
	channel = this -> ttable (source, language -> getInputAlphabet ()),
	a = Fsa::cache (Fsa::composeMatching (channel, language, false));

      Fsa::ConstAutomatonRef best = bestOutput_ (sentence, a);
      std::cerr << "Weight:  "
		<< static_cast <double> (Fsa::getLinearWeight (best))
		<< std::endl;
      Fsa::writeLinear (best, out);
      out << std::endl;
    }

    return EXIT_SUCCESS;
  }

} app;
