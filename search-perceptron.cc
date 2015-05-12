#include "Application.hh"
#include "ChartFactory.hh"
#include "Parameter.hh"

APPLICATION

// Trains a ParameterVector using perceptron updates at each position along a
// search trajectory.  The trajectory is determined by --trajectory in {loss,
// model}.  While convergence has not been reached, for each sentence in the
// input, at each position in the search trajectory, updates the model towards
// the minimum loss neighbor and away from the best neighbor according to the
// model.
class searchPerceptron : public Permute::Application {
private:
  static Core::ParameterFloat paramLearningRate;
  enum TrajectoryType {
    tr_loss,
    tr_model
  };
  static Core::Choice TrajectoryChoice;
  static Core::ParameterChoice paramTrajectoryType;
public:
  searchPerceptron () :
    Permute::Application ("search-perceptron") {}

  int main (const std::vector <std::string> & args) {
    Permute::ParameterVector pv;
    if (! this -> parameters (pv)) {
      return EXIT_FAILURE;
    }
//     for (Permute::ParameterVector::parameter_iterator p = pv.begin_p (); p != pv.end_p (); ++ p) {
//       p -> second = 0.0;
//     }

    std::vector <double>
      weightSum (pv.size (), 0.0),
      previous (pv.size (), 0.0),
      current (pv.size (), 0.0);
    Permute::set (current, pv);

    Permute::Permutation source, helper, target, pos (pv.getPOS ());

    Permute::ChartFactoryRef factory = Permute::ChartFactory::create ();
    Permute::ParseControllerRef controller = this -> parseController (source);

    Permute::FeatureCounter counter (pv, pos);

    long i = 1;
    do {
      Permute::set (previous, current);

      std::istream & in = this -> input ();
      for (int sentence = 0; sentence < paramSentences (config) && Permute::readPermutationWithAlphabet (source, in); ++ sentence) {
	Permute::readPermutation (pos, in);
	helper = source;
	target = source;
	Permute::readAlignment (target, in);

	Permute::ScorerRef scorer = this -> beforeScorer (source, pv, pos);
	Permute::ScorerRef loss = this -> lossScorer (source, target);

	Permute::ChartRef chart = factory -> chart (source);

	double best_score;
	if (paramTrajectoryType (config) == tr_model) {
	  best_score = scorer -> score (source);
	}
	
	do {
	  // Get the min loss candidate in the neighborhood.
	  Permute::Chart::permute (chart, controller, loss);
	  Permute::ConstPathRef min_loss_path = chart -> getBestPath ();
	  double min_loss = min_loss_path -> getScore ();

	  // Get the best candidate according to the current model.
	  Permute::Chart::permute (chart, controller, scorer);
	  Permute::ConstPathRef model_path = chart -> getBestPath ();

	  target.reorder (min_loss_path);
	  helper.reorder (model_path);

	  // Update the parameters and the iteration count.
	  counter.count (target, paramLearningRate (config));
	  counter.count (helper, - paramLearningRate (config));

	  update (weightSum, pv);
	  ++ i;

	  source.changed (false);
	  if (paramTrajectoryType (config) == tr_loss) {
	    source.reorder (min_loss_path);
	  } else if (model_path -> getScore () > best_score) {
	    best_score = model_path -> getScore ();
	    source.reorder (model_path);
	  }
	  
	} while (source.changed ());
      }

      Permute::set (current, weightSum, 1.0 / i);

      this -> decodeDev (pv, current);

      //////////////////////////////////////////////////////////////////////
      // Alternative to decodeDev
      //////////////////////////////////////////////////////////////////////
      double loss_total = 0.0;
      double sentences;
      Permute::ParameterVector pvc (pv);
      Permute::set (pvc, current);

      std::istream & devIn = this -> devInput ();
      for (sentences = 0.0; Permute::readPermutationWithAlphabet (source, devIn); ++ sentences) {
	Permute::readPermutation (pos, devIn);
	target = source;
	Permute::readAlignment (target, in);

	Permute::ScorerRef scorer = this -> beforeScorer (source, pv, pos);
	Permute::ScorerRef loss = this -> lossScorer (source, target);
	Permute::ChartRef chart = factory -> chart (source);

	double best_score = scorer -> score (source);
	do {
	  Permute::Chart::permute (chart, controller, scorer);
	  Permute::ConstPathRef bestPath = chart -> getBestPath ();
	  source.changed (false);
	  if (bestPath -> getScore () > best_score) {
	    best_score = bestPath -> getScore ();
	    source.reorder (bestPath);
	  }
	} while (paramIterateSearch (config) && source.changed ());

	if (source.size () > 1) {
	  loss_total += 2.0 * (loss -> score (target) - loss -> score (source)) / (source.size () * (source.size () - 1.0));
	}
      }

      std::cerr << loss_total / sentences << std::endl;	
      //////////////////////////////////////////////////////////////////////
      
    } while (! this -> converged (previous, current));

    Permute::set (pv, weightSum, 1.0 / i);
    this -> outputParameters (pv);

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterFloat searchPerceptron::paramLearningRate ("rate", "the learning rate for the perceptron", 1.0, 0.0);
Core::Choice searchPerceptron::TrajectoryChoice ("loss", tr_loss, "model", tr_model, CHOICE_END);
Core::ParameterChoice searchPerceptron::paramTrajectoryType ("trajectory", & searchPerceptron::TrajectoryChoice, "the trajectory to follow during search", tr_loss);
