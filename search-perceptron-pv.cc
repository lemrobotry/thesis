#include "Application.hh"
#include "ChartFactory.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Trains a PV using perceptron updates at each position in the search
// trajectory.  The --trajectory parameter in {loss, model} determines the
// search trajectory.  At each position, the perceptron update makes the model
// prefer the minimum loss permutation in the neighborhood and disprefer the
// best permutation in the neighborhood according to the model.  Writes out the
// average learned PV after every pass through the training data.
class SearchPerceptronPV : public Application {
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
public:
  SearchPerceptronPV () :
    Application ("search-perceptron-pv")
  {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramLearningRate.printShortHelp (out);
    paramTrajectoryType.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    LEARNING_RATE = paramLearningRate (config);
    TRAJECTORY = paramTrajectoryType (config);
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

    Permutation source, helper, target, pos, labels;
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
	   ++ sentence) {
	readPermutationWithAlphabet (pos, input);
	if (DEPENDENCY) {
	  readParents (parents, input);
	  readPermutationWithAlphabet (labels, input);
	}
	helper = source;
	target = source;
	readAlignment (target, input);

	std::cerr << sentence << " " << source.size () << std::endl;

	SumBeforeCostRef bc (new SumBeforeCost (source.size (),
						"SearchPerceptronPV"));
	ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos, parents, labels);
	ScorerRef loss = this -> lossScorer (source, target);
	ChartRef chart = factory -> chart (source);

	double bestScore = scorer -> score (source);
	do {
	  Chart::permute (chart, controller, loss);
	  ConstPathRef minLossPath = chart -> getBestPath ();
	  double minLoss = minLossPath -> getScore ();
	  
	  Chart::permute (chart, controller, scorer);
	  ConstPathRef modelPath = chart -> getBestPath ();

	  target.reorder (minLossPath);
	  helper.reorder (modelPath);

	  if (helper != target) {
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
	  }

	  update (weightSum, weights);
	  ++ i;
	  
	  source.changed (false);
	  if (TRAJECTORY == tr_loss) {
	    source.reorder (minLossPath);
	  } else if (modelPath -> getScore () > bestScore) {
	    bestScore = modelPath -> getScore ();
	    source.reorder (modelPath);
	  }
	} while (source.changed ());

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

Core::ParameterFloat SearchPerceptronPV::paramLearningRate ("rate", "the learning rate for the perceptron", 1.0, 0.0);
Core::Choice SearchPerceptronPV::TrajectoryChoice ("loss", tr_loss, "model", tr_model, CHOICE_END);
Core::ParameterChoice SearchPerceptronPV::paramTrajectoryType ("trajectory", & SearchPerceptronPV::TrajectoryChoice, "the trajectory to follow during search", tr_loss);
