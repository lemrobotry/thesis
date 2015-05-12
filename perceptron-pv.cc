#include "Application.hh"
#include "ChartFactory.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Trains a PV using standard perceptron updates.  Until convergence, for each
// sentence in the input, searches for the best permutation according to the
// model.  Updates the parameters to prefer the true target permutation and
// disprefer the model best.  Outputs the average weights.
class PerceptronPV : public Application {
private:
  static Core::ParameterFloat paramLearningRate;
  double LEARNING_RATE;
  static Core::ParameterInt paramEpoch;
  int EPOCH;
public:
  PerceptronPV () :
    Application ("perceptron-pv")
  {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramLearningRate.printShortHelp (out);
    paramEpoch.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    LEARNING_RATE = paramLearningRate (config);
    EPOCH = paramEpoch (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    std::vector <WRef> weights;
    for (PV::const_iterator phi = pv.begin (); phi != pv.end (); ++ phi) {
      weights.push_back (phi -> second);
    }

    std::vector <double>
      weightSum (pv.size (), 0.0),
      previous (pv.size (), 0.0),
      current (pv.size (), 0.0);
    Permute::set (current, weights);

    ChartFactoryRef factory = ChartFactory::create ();

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    ParseControllerRef controller = this -> parseController (source);

    double i = 1.0;
    int iteration = 0;
    do {
      Permute::set (previous, current);

      std::istream & input = this -> input ();

      for (int sentence = 0;
	   sentence < SENTENCES
	     && readPermutationWithAlphabet (source, input);
	   ++ sentence, ++ i) {
	readPermutationWithAlphabet (pos, input);
	if (DEPENDENCY) {
	  readParents (parents, input);
	  readPermutationWithAlphabet (labels, input);
	}
	target = source;
	readAlignment (target, input);

	SumBeforeCostRef bc (new SumBeforeCost (source.size (), "PerceptronPV"));
	ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos, parents, labels);
	ChartRef chart = factory -> chart (source);

	double best_score = scorer -> score (source);
	do {
	  Chart::permute (chart, controller, scorer);
	  ConstPathRef bestPath = chart -> getBestPath ();
	  source.changed (false);
	  if (bestPath -> getScore () > best_score) {
	    best_score = bestPath -> getScore ();
	    source.reorder (bestPath);
	  }
	} while (source.changed ());

	if (source != target) {
	  // Add the feature counts of target.
	  for (Permutation::const_iterator i = target.begin (); i != -- target.end (); ++ i) {
	    for (Permutation::const_iterator j = i + 1; j != target.end (); ++ j) {
	      if (* i < * j) {
		(* bc) (* i, * j).add (LEARNING_RATE);
	      }
	    }
	  }
	  // Subtract the feature counts of source.
	  for (Permutation::const_iterator i = source.begin (); i != -- source.end (); ++ i) {
	    for (Permutation::const_iterator j = i + 1; j != source.end (); ++ j) {
	      if (* i < * j) {
		(* bc) (* i, * j).add (- LEARNING_RATE);
	      }
	    }
	  }
	}

	update (weightSum, weights);
	if (EPOCH > 0 && sentence % EPOCH == 0) {
	  std::cerr << sentence << std::endl;
	}
      }

      // Use current to temporarily store the weights.
      Permute::set (current, weights);
      Permute::set (weights, weightSum, 1.0 / i);
      // Decode Dev.
      this -> decodeDev (pv);
      this -> writePV (pv, ++ iteration);
      // Reset weights and update current.
      Permute::set (weights, current);
      Permute::set (current, weightSum, 1.0 / i);

    } while (! this -> converged (previous, current));

//     Permute::set (weights, weightSum, 1.0 / i);
//     this -> writePV (pv);

    return EXIT_SUCCESS;    
  }
} app;

Core::ParameterFloat PerceptronPV::paramLearningRate ("rate", "the learning rate for the perceptron", 1.0, 0.0);
Core::ParameterInt PerceptronPV::paramEpoch ("epoch", "the rate at which to output progress", 0, 0);
