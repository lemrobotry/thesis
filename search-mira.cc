#include <tao.h>
#include "Application.hh"
#include "ChartFactory.hh"
#include "MIRA.hh"
#include "Parameter.hh"
#include "kBestChart.hh"

APPLICATION

// Performs MIRA at each step along a search trajectory (just at the source
// permutation unless --iterate-search is true), making the model prefer the
// minimum loss permutation in the neighborhood over each of the model's k best
// permutations, with margin depending on the difference in loss.
//
// @bug Follows the loss trajectory rather than the model trajectory.  This
// should be determined by a parameter, as with search-perceptron.
class searchMIRA : public Permute::Application {
private:
  static Core::ParameterInt paramK;
  int K;
public:
  searchMIRA () :
    Permute::Application ("search-mira") {}

  virtual void getParameters () {
    Application::getParameters ();
    K = paramK (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    int argc = 0;
    char ** argv = NULL;
    PetscInitialize (& argc, & argv, NULL, "help message");
    TaoInitialize (& argc, & argv, NULL, "help message");

    Permute::ParameterVector pv;
    if (! this -> parameters (pv)) {
      return EXIT_FAILURE;
    }

    std::vector <double>
      weightSum (pv.size (), 0.0),
      previous (pv.size (), 0.0),
      current (pv.size (), 0.0);
    Permute::set (current, pv);

    Permute::Permutation source, helper, target, pos (pv.getPOS ());

    Permute::ChartFactoryRef factory = Permute::ChartFactory::kbest ();
    Permute::ParseControllerRef controller = this -> parseController (source);

    long i = 1;
    do {
      Permute::set (previous, current);

      for (std::istream & in = this -> input (); Permute::readPermutationWithAlphabet (source, in); ) {
	Permute::readPermutation (pos, in);
	helper = source;
	target = source;
	Permute::readAlignment (target, in);

	std::cerr << source << std::endl;

	Permute::ScorerRef scorer = this -> beforeScorer (source, pv, pos);
	Permute::ScorerRef loss = this -> lossScorer (source, target);

	Permute::ChartRef chart = factory -> chart (source);
	Permute::kBestChart kbc (chart, controller, scorer);

	do {
	  // Get the min loss candidate in the neighborhood.
	  Permute::Chart::permute (chart, controller, loss);
	  Permute::ConstPathRef min_loss_path = chart -> getBestPath ();
	  double min_loss = min_loss_path -> getScore ();

	  // Get the k-best candidates according to the current model.
	  kbc.permute ();
	  Permute::kBestChart::Paths paths = kbc.best (K);

	  source.reorder (min_loss_path);

	  // Update the parameters and the iteration count.
	  std::vector <Permute::SparseParameterVector> delta;
	  for (Permute::kBestChart::Paths::const_iterator p = paths.begin (); p != paths.end (); ++ p) {
	    helper.reorder (* p);
	    double l = min_loss - loss -> score (helper);
	    delta.push_back (Permute::SparseParameterVector (l));
	    pos.permute (source);
	    delta.back ().build (pv, pos, 1.0);
	    pos.permute (helper);
	    delta.back ().build (pv, pos, -1.0);
	  }
	  Permute::MIRA (delta);

	  update (weightSum, pv);
	  ++ i;
	  
	} while (ITERATE_SEARCH && source.changed ());

      }

      Permute::set (current, weightSum, 1.0 / i);
    } while (! this -> converged (previous, current));

    Permute::set (pv, weightSum, 1.0 / i);
    this -> outputParameters (pv);

    TaoFinalize ();
    PetscFinalize ();

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt searchMIRA::paramK ("k", "the size of the k-best list", 1, 1);
