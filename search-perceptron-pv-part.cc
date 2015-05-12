#include "Application.hh"
#include "ChartFactory.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Runs a single iteration of PV training using perceptron updates at each
// position in the search trajectory determined by the --trajectory parameter.
// Writes out the learned PV.
class SearchPerceptronPVPart : public Application {
private:
  static Core::ParameterFloat paramLearningRate;
  double LEARNING_RATE;
  enum TrajectoryType {
    tr_loss,
    tr_model
  };
  static Core::Choice TrajectoryChoice;
  static Core::ParameterChoice paramTrajectoryType;
  int TRAJECTORY;
  static Core::ParameterInt paramPartK;
  static Core::ParameterInt paramPartMod;
  int K, MOD;
public:
  SearchPerceptronPVPart () :
    Application ("search-perceptron-pv-part")
  {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramLearningRate.printShortHelp (out);
    paramTrajectoryType.printShortHelp (out);
    paramPartK.printShortHelp (out);
    paramPartMod.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    LEARNING_RATE = paramLearningRate (config);
    TRAJECTORY = paramTrajectoryType (config);
    K = paramPartK (config);
    MOD = paramPartMod (config);
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

    std::vector <double> weightSum (pv.size (), 0.0);

    ChartFactoryRef factory = ChartFactory::create ();

    Permutation source, helper, target, pos, labels;
    std::vector <int> parents;

    ParseControllerRef controller = this -> parseController (source);

    double count = 1.0;

    std::istream & input = this -> input ();

    for (int sentence = 0;
	 sentence < SENTENCES && readPermutationWithAlphabet (source, input);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, input);
      if (DEPENDENCY) {
	readParents (parents, input);
	readPermutationWithAlphabet (labels, input);
      }
      helper = source;
      target = source;
      readAlignment (target, input);

      if (sentence % MOD == K) {
	SumBeforeCostRef bc (new SumBeforeCost (source.size (), "SearchPerceptronPVPart"));
	ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos, parents, labels);
	ScorerRef loss = this -> lossScorer (source, target);
	ChartRef chart = factory -> chart (source);

	double bestScore = scorer -> score (source);
	do {
	  Chart::permute (chart, controller, loss);
	  ConstPathRef minLossPath = chart -> getBestPath ();

	  Chart::permute (chart, controller, scorer);
	  ConstPathRef modelPath = chart -> getBestPath ();

	  target.reorder (minLossPath);
	  helper.reorder (modelPath);

	  // Add the feature counts of target.
	  for (Permutation::const_iterator i = target.begin (); i != -- target.end (); ++ i) {
	    for (Permutation::const_iterator j = i + 1; j != target.end (); ++ j) {
	      if (* i < * j) {
		(* bc) (* i, * j).add (LEARNING_RATE);
	      }
	    }
	  }
	  // Subtract the feature counts of helper.
	  for (Permutation::const_iterator i = helper.begin (); i != -- helper.end (); ++ i) {
	    for (Permutation::const_iterator j = i + 1; j != helper.end (); ++ j) {
	      if (* i < * j) {
		(* bc) (* i, * j).add (- LEARNING_RATE);
	      }
	    }
	  }

	  update (weightSum, weights);
	  ++ count;

	  source.changed (false);
	  if (TRAJECTORY == tr_loss) {
	    source.reorder (minLossPath);
	  } else if (modelPath -> getScore () > bestScore) {
	    bestScore = modelPath -> getScore ();
	    source.reorder (modelPath);
	  }
	} while (source.changed ());
      }
    }

    Permute::set (weights, weightSum, 1.0 / count);
    this -> writePV (pv);

    return EXIT_SUCCESS;
  }
} app;
  
Core::ParameterFloat SearchPerceptronPVPart::paramLearningRate ("rate", "the learning rate for the perceptron", 1.0, 0.0);
Core::Choice SearchPerceptronPVPart::TrajectoryChoice ("loss", tr_loss, "model", tr_model, CHOICE_END);
Core::ParameterChoice SearchPerceptronPVPart::paramTrajectoryType ("trajectory", & SearchPerceptronPVPart::TrajectoryChoice, "the trajectory to follow during search", tr_loss);
Core::ParameterInt SearchPerceptronPVPart::paramPartK ("k", "the remainder (mod --mod) of the sentences to use for training", 0, 0);
Core::ParameterInt SearchPerceptronPVPart::paramPartMod ("mod", "the number of parts in use for training", 1, 1);
