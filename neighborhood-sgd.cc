#include "Application.hh"
#include "GradientChart.hh"
#include "ParseController.hh"
#include "PV.hh"
#include "SGD.hh"

APPLICATION

using namespace Permute;

class NeighborhoodSGD : public Application {
private:
  static Core::ParameterFloat paramLearningRate;
  double LEARNING_RATE;
  static Core::ParameterInt paramPartK;
  static Core::ParameterInt paramPartMod;
  int K, MOD;
public:
  NeighborhoodSGD () :
    Application ("neighborhood-sgd")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    LEARNING_RATE = paramLearningRate (config);
    K = paramPartK (config);
    MOD = paramPartMod (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramLearningRate.printShortHelp (out);
    paramPartK.printShortHelp (out);
    paramPartMod.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    UpdateSGD update_sgd (LEARNING_RATE);

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    std::vector <WRef> weights (pv.size ());
    std::transform (pv.begin (), pv.end (),
		    weights.begin (),
		    std::select2nd <PV::value_type> ());

    std::vector <double> values (weights.size ());
    
    std::vector <double> weightSum (weights.size (), 0.0);
    double count = 0.0;

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    ParseControllerRef controller (CubicParseController::create ());

    std::istream & input = this -> input ();

    for (int sentence = 0;
	 sentence < SENTENCES && readPermutationWithAlphabet (source, input);
	 ++ sentence) {
      std::cerr << sentence << " " << source.size () << std::endl;
      readPermutationWithAlphabet (pos, input);
      if (DEPENDENCY) {
	readParents (parents, input);
	readPermutationWithAlphabet (labels, input);
      }
      target = source;
      readAlignment (target, input);

      if (sentence % MOD == K) {
	// Copies the current parameter values so gradients can be accumulated
	// in place.
	std::copy (weights.begin (), weights.end (),
		   values.begin ());
	// Computes the gradient of the parameters with respect to the log
	// likelihood of the current target permutation given its neighborhood.
	SumBeforeCostRef bc (new SumBeforeCost (source.size (), "NeighborhoodSGD"));
	this -> sumBeforeCost (bc, pv, source, pos, parents, labels);
	GradientScorer scorer (bc, target);
	GradientChart chart (target);
	chart.parse (controller, scorer, pv);
	// Updates the parameters: values holds the current parameters, and
	// weights holds their gradients.  Transforms the pair using UpdateSGD
	// and assigns to weights.
	std::transform(values.begin (), values.end (),
		       weights.begin (),
		       weights.begin (),
		       update_sgd);

	update (weightSum, weights);
	++ count;
      }
    }

    Permute::set (weights, weightSum, 1.0 / count);
    this -> writePV (pv);
    
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterFloat NeighborhoodSGD::paramLearningRate ("rate", "the learning rate for SGD", 1.0, 0.0);
Core::ParameterInt NeighborhoodSGD::paramPartK ("k", "the remainder (mod --mod) of the sentences to use for training", 0, 0);
Core::ParameterInt NeighborhoodSGD::paramPartMod ("mod", "the number of parts in use for training", 1, 1);
