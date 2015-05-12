#include <tao.h>
#include "Application.hh"
#include "ChartFactory.hh"
#include "MIRA.hh"
#include "PV.hh"
#include "kBestChart.hh"

APPLICATION

using namespace Permute;

// Performs MIRA at each step along a search trajectory (just at the source
// permutation unless --iterate-search is true), making the model prefer the
// minimum loss permutation in the neighborhood over each of the model's k best
// permutations, with margin depending on the difference in loss.
class searchMIRA : public Application {
private:
  static Core::ParameterInt paramK;
  int K;
  enum TrajectoryType {
    tr_loss,
    tr_model
  };
  static Core::Choice TrajectoryChoice;
  static Core::ParameterChoice paramTrajectoryType;
  int TRAJECTORY;
public:
  searchMIRA () :
    Application ("search-mira-pv") {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramK.printShortHelp (out);
    paramTrajectoryType.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    K = paramK (config);
    TRAJECTORY = paramTrajectoryType (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    
    int argc = 1;
    char * arg0 = "search-mira-pv";
    std::vector <char *> arg_vector (1, arg0);
    char ** argv = & arg_vector [0];
    PetscInitialize (& argc, & argv, NULL, "help message");
    TaoInitialize (& argc, & argv, NULL, "help message");

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    std::vector <WRef> weights;
    for (PV::const_iterator it = pv.begin (); it != pv.end (); ++ it) {
      weights.push_back (it -> second);
    }

    std::vector <double>
      weightSum (pv.size (), 0.0),
      previous (pv.size (), 0.0),
      current (pv.size (), 0.0);
    Permute::set (current, weights);

    Permutation source, helper, target, pos;

    ChartFactoryRef factory = Permute::ChartFactory::kbest ();
    ParseControllerRef controller = this -> parseController (source);

    long i = 1;
    do {
      Permute::set (previous, current);

      std::istream & in = this -> input ();

      for (int sentence = 0;
	   sentence < SENTENCES
	     && readPermutationWithAlphabet (source, in);
	   ++ sentence) {
	Permute::readPermutationWithAlphabet (pos, in);
	helper = source;
	target = source;
	Permute::readAlignment (target, in);

	SumBeforeCostRef bc (new SumBeforeCost (source.size (), "SearchMIRA"));
	ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos);
	ScorerRef loss = this -> lossScorer (source, target);

	ChartRef chart = factory -> chart (source);
	kBestChart kbc (chart, controller, scorer);

	double bestScore = scorer -> score (source);
	do {
	  source.changed (false);
	  // Get the min loss candidate in the neighborhood.
	  Chart::permute (chart, controller, loss);
	  ConstPathRef minLossPath = chart -> getBestPath ();
	  double minLoss = minLossPath -> getScore ();

	  // Get the k-best candidates according to the current model.
	  kbc.permute ();
	  Permute::kBestChart::Paths paths = kbc.best (K);

	  source.reorder (minLossPath);

	  // Update the parameters and the iteration count.
	  std::vector <SparsePV> delta;
	  for (kBestChart::Paths::const_iterator it = paths.begin ();
	       it != paths.end (); ++ it) {
	    helper.reorder (* it);
	    delta.push_back (Permute::SparsePV (minLoss - loss -> score (helper)));
	    pos.permute (source);
	    delta.back ().build (source, bc, 1.0);
	    pos.permute (helper);
	    delta.back ().build (helper, bc, -1.0);
	  }
	  Permute::MIRA (delta);

	  update (weightSum, weights);
	  ++ i;

	  if (TRAJECTORY == tr_model) {
	    if (paths [0] -> getScore () > bestScore) {
	      bestScore = paths [0] -> getScore ();
	      source.reorder (paths [0]);
	    }
	  }
	} while (ITERATE_SEARCH && source.changed ());

      }

      Permute::set (current, weightSum, 1.0 / i);
    } while (! this -> converged (previous, current));

    Permute::set (weights, weightSum, 1.0 / i);
    this -> writePV (pv);

    TaoFinalize ();
    PetscFinalize ();

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt searchMIRA::paramK ("k", "the size of the k-best list", 1, 1);
Core::Choice searchMIRA::TrajectoryChoice ("loss", tr_loss, "model", tr_model, CHOICE_END);
Core::ParameterChoice searchMIRA::paramTrajectoryType ("trajectory", & searchMIRA::TrajectoryChoice, "the trajectory to follow during search", tr_loss);
