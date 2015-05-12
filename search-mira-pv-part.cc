#include <tao.h>
#include "Application.hh"
#include "ChartFactory.hh"
#include "MIRA.hh"
#include "PV.hh"
#include "kBestChart.hh"

APPLICATION

using namespace Permute;

// Performs a single iteration of PV training using MIRA updates at each step
// along a search trajectory (just at the source permutation unless
// --iterate-search is true), making the model prefer the minimum loss
// permutation in the neighborhood over each of the model's k best permutations,
// with margin depending on the difference in loss.
class searchMIRApart : public Application {
private:
  static Core::ParameterInt paramBest;
  int BEST;
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
  searchMIRApart () :
    Application ("search-mira-pv-part") {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramBest.printShortHelp (out);
    paramTrajectoryType.printShortHelp (out);
    paramPartK.printShortHelp (out);
    paramPartMod.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    BEST = paramBest (config);
    TRAJECTORY = paramTrajectoryType (config);
    K = paramPartK (config);
    MOD = paramPartMod (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    int argc = 1;
    char * arg0 = "search-mira-pv-part";
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

    std::vector <double> weightSum (pv.size (), 0.0);

    Permutation source, helper, target, pos;

    ChartFactoryRef factory = Permute::ChartFactory::kbest ();
    ParseControllerRef controller = this -> parseController (source);

    double count = 1.0;
    
    std::istream & in = this -> input ();

    for (int sentence = 0;
	 sentence < SENTENCES && readPermutationWithAlphabet (source, in);
	 ++ sentence) {
      Permute::readPermutationWithAlphabet (pos, in);
      helper = source;
      target = source;
      Permute::readAlignment (target, in);

      if (sentence % MOD == K) {
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
	  Permute::kBestChart::Paths paths = kbc.best (BEST);

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
	  ++ count;

	  if (TRAJECTORY == tr_model) {
	    if (paths [0] -> getScore () > bestScore) {
	      bestScore = paths [0] -> getScore ();
	      source.reorder (paths [0]);
	    }
	  }
	} while (ITERATE_SEARCH && source.changed ());
      }
    }

    Permute::set (weights, weightSum, 1.0 / count);
    this -> writePV (pv);

    TaoFinalize ();
    PetscFinalize ();

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt searchMIRApart::paramBest ("best", "the size of the k-best list", 1, 1);
Core::Choice searchMIRApart::TrajectoryChoice ("loss", tr_loss, "model", tr_model, CHOICE_END);
Core::ParameterChoice searchMIRApart::paramTrajectoryType ("trajectory", & searchMIRApart::TrajectoryChoice, "the trajectory to follow during search", tr_loss);
Core::ParameterInt searchMIRApart::paramPartK ("k", "the remainder (mod --mod) of the sentences to use for training", 0, 0);
Core::ParameterInt searchMIRApart::paramPartMod ("mod", "the number of parts in use for training", 1, 1);
