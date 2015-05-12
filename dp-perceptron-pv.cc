#include "Application.hh"
#include "BleuScore.hh"
#include "DependencyOrder.hh"
#include "Iterator.hh"

APPLICATION

using namespace Permute;

class DPPerceptronPV : public Application {
private:
  static Core::ParameterFloat paramLearningRate;
  double LEARNING_RATE;
public:
  DPPerceptronPV () :
    Application ("dp-perceptron-pv")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    LEARNING_RATE = paramLearningRate (config);
    DEPENDENCY = true;
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramLearningRate.printShortHelp (out);
  }

  void decodeDevDP (const PV & pv);

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    double t = 1.0;
    for (int iter = 0; iter < LEARNING_ITERATIONS; ++ iter) {
      std::istream & input = this -> input ();
      for (int sentence = 0;
	   sentence < SENTENCES
	     && readPermutationWithAlphabet (source, input);
	   ++ sentence, ++ t) {
	readPermutationWithAlphabet (pos, input);
	readParents (parents, input);
	readPermutationWithAlphabet (labels, input);
	target = source;
	readAlignment (target, input);

	SumBeforeCostRef bc (new SumBeforeCost (source.size (), "DPPerceptronPV"));
	this -> sumBeforeCost (bc, pv, source, pos, parents, labels);

	dependencyOrder (bc, source, parents);

	if (DEBUG) {
	  std::cerr << "Parents: " << delimit (parents.begin (), parents.end (), " ")
		    << std::endl
		    << "Result:  " << delimit (source.begin (), source.end (), " ")
		    << std::endl
		    << "Score:   " << bc -> score (source)
		    << std::endl;
	}

	// Adds the feature counts of target.
	for (Permutation::const_iterator i = target.begin ();
	     i != -- target.end (); ++ i) {
	  for (Permutation::const_iterator j = i + 1;
	       j != target.end (); ++ j) {
	    if (* i < * j) {
	      (* bc) (* i, * j).add (LEARNING_RATE / t);
	    }
	  }
	}
	// Subtracts the feature counts of source.
	for (Permutation::const_iterator i = source.begin ();
	     i != -- source.end (); ++ i) {
	  for (Permutation::const_iterator j = i + 1;
	       j != source.end (); ++ j) {
	    if (* i < * j) {
	      (* bc) (* i, * j).add (- LEARNING_RATE / t);
	    }
	  }
	}
      }

      if (! DEBUG) {
	decodeDevDP (pv);
      }
    }

    if (! DEBUG) {
      // Writes out the model.
      this -> writePV (pv);
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterFloat DPPerceptronPV::paramLearningRate ("rate", "the learning rate for the perceptron", 1.0, 0.0);

void DPPerceptronPV::decodeDevDP (const PV & pv) {
  Permutation source, target, pos, labels;
  std::vector <int> parents;

  BleuScore bleu;

  std::istream & in = this -> devInput ();
  while (readPermutationWithAlphabet (source, in)) {
    readPermutationWithAlphabet (pos, in);
    readParents (parents, in);
    readPermutationWithAlphabet (labels, in);
    target = source;
    readAlignment (target, in);

    SumBeforeCostRef bc (new SumBeforeCost (source.size (), "DPPerceptronPV::decodeDevDP"));
    this -> sumBeforeCost (bc, pv, source, pos, parents, labels);    
    
    dependencyOrder (bc, source, parents);

    // computeBleuScore given Permutations prints them before evaluation.
    bleu += computeBleuScore (target, source);
  }

  std::cerr << "BLEU = " << bleu << std::endl;
  
  return;  
}
