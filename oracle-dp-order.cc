#include "Application.hh"
#include "BeforeScorer.hh"
#include "BleuScore.hh"
#include "DependencyOrder.hh"

APPLICATION

using namespace Permute;

class OracleDPOrder : public Application {
public:
  OracleDPOrder () :
    Application ("oracle-dp-order")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    DEPENDENCY = true;
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    std::istream & input = this -> input ();
    std::ostream & output = this -> output ();

    BleuScore bleu;

    for (int sentence = 0;
	 sentence < SENTENCES
	   && readPermutationWithAlphabet (source, input);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, input);
      readParents (parents, input);
      readPermutationWithAlphabet (labels, input);
      target = source;
      readAlignment (target, input);

      BeforeCostRef loss = tauCost (target);

      dependencyOrder (loss, source, parents);

      output << source << std::endl;

      bleu += computeBleuScore (target, source);
    }

    std::cerr << "BLEU = " << bleu << std::endl;

    return EXIT_SUCCESS;
  }
} app;
