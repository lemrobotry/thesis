#include "Application.hh"
#include "Loss.hh"

APPLICATION

using namespace Permute;

// Measures the loss of the source and the target with respect to the target.
class SourceTargetLoss : public Application {
public:
  SourceTargetLoss () :
    Application ("source-target-loss")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    Loss sourceLoss;
    Loss targetLoss;

    std::istream & input = this -> input ();

    for (int sentence = 0;
	 sentence < SENTENCES
	   && readPermutationWithAlphabet (source, input);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, input);
      if (DEPENDENCY) {
	readParents (parents, input);
	readPermutationWithAlphabet (labels, input);
      }
      target = source;
      readAlignment (target, input);

      sourceLoss.add (target, source);
      targetLoss.add (target, target);
    }

    std::cout << "Source: " << std::endl
	      << sourceLoss << std::endl
	      << "Target: " << std::endl
	      << targetLoss << std::endl;

    return EXIT_SUCCESS;
  }
} app;
