#include "AdjacentLoss.hh"
#include "Application.hh"
#include "Parameter.hh"
#include "ParseController.hh"
#include "PV.hh"
#include "SGD.hh"

APPLICATION

using namespace Permute;

class AdjacentSGD : public Application {
private:
  static Core::ParameterFloat paramLearningRate;
  double LEARNING_RATE;
  static Core::ParameterBool paramLearningClock;
  bool LEARNING_CLOCK;
  static Core::ParameterBool paramZeroParameters;
  bool ZERO_PARAMETERS;
  static Core::ParameterInt paramPartK;
  static Core::ParameterInt paramPartMod;
  int K, MOD;
public:
  AdjacentSGD () :
    Application ("adjacent-sgd")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    LEARNING_RATE = paramLearningRate (config);
    LEARNING_CLOCK = paramLearningClock (config);
    ZERO_PARAMETERS = paramZeroParameters (config);
    K = paramPartK (config);
    MOD = paramPartMod (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramLearningRate.printShortHelp (out);
    paramLearningClock.printShortHelp (out);
    paramZeroParameters.printShortHelp (out);
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
    if (ZERO_PARAMETERS) {
      std::fill (weights.begin (), weights.end (), 0.0);
    }

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
#ifndef NDEBUG
	bool redo = true;
	double expectation = 0.0;
	do {
#endif
	// Copies the current parameter values so gradients can be accumulated
	// in place.
	std::copy (weights.begin (), weights.end (),
		   values.begin ());
	// Computes the gradient of the parameters with respect to the log
	// likelihood of the current target permutation given its neighborhood.
	SumBeforeCostRef bc (new SumBeforeCost (source.size (), "AdjacentSGD"));
	this -> sumBeforeCost (bc, pv, source, pos, parents, labels);
	ExpectationGradientScorer scorer (bc, target);
	AdjacentGradientChart chart (target);
	chart.parse (controller, scorer, pv);
#ifndef NDEBUG
	std::cerr.precision (15);
	std::cerr << "Z: "
		  << chart.Z () << " = "
		  << chart.Z ().expectation () << std::endl;
	std::cerr << "Change in expectation: "
		  << chart.Z ().expectation () - expectation << std::endl;
	expectation = chart.Z ().expectation ();
	double norm_g = std::pow (norm (weights), 2.0);
	std::cerr << "Predicted change in expectation: "
		  << update_sgd.learningRate () << " * "
		  << norm_g << " = "
		  << update_sgd.learningRate () * norm_g << std::endl;
#endif
	// Updates the parameters: values holds the current parameters, and
	// weights holds their gradients.  Transforms the pair using UpdateSGD
	// and assigns to weights.
	std::transform (values.begin (), values.end (),
			weights.begin (),
			weights.begin (),
			update_sgd);
	if (LEARNING_CLOCK) {
	  update_sgd.tick ();
	}

	update (weightSum, weights);
	++ count;
#ifndef NDEBUG
	std::cerr << "Distance: " << distance (values, weights) << std::endl;
	} while (redo);
#endif
      }
    }

    if (! LEARNING_CLOCK) {
      Permute::set (weights, weightSum, 1.0 / count);
    }
    this -> writePV (pv);

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterFloat AdjacentSGD::paramLearningRate ("rate", "the learning rate for SGD", 1.0, 0.0);
Core::ParameterBool AdjacentSGD::paramLearningClock ("clock", "degrade learning rate as 1/t", false);
Core::ParameterBool AdjacentSGD::paramZeroParameters ("zero", "set all parameters to zero before learning", false);
Core::ParameterInt AdjacentSGD::paramPartK ("k", "the remainder (mod --mod) of the sentences to use for training", 0, 0);
Core::ParameterInt AdjacentSGD::paramPartMod ("mod", "the number of parts in use for training", 1, 1);
