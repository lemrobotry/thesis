#include <tao.h>
#include "Application.hh"
#include "ChartFactory.hh"
#include "MIRA.hh"
#include "Parameter.hh"
#include "kBestChart.hh"

APPLICATION

// Performs MIRA using the k best permutations, according to the model, in the
// neighborhood of the target permutation.
class kBestMIRA : public Permute::Application {
private:
  static Core::ParameterInt paramK;
  int K;
public:
  kBestMIRA () :
    Permute::Application ("k-best-mira") {}

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

    Permute::Permutation source, target, pos (pv.getPOS ());

    Permute::ChartFactoryRef factory = Permute::ChartFactory::kbest ();
    Permute::ParseControllerRef controller = this -> parseController (source);

    int i = 1;
    do {
      Permute::set (previous, current);
      
      std::istream & in = this -> input ();
      for (; Permute::readPermutationWithAlphabet (source, in); ++ i) {
	Permute::readPermutation (pos, in);
	target = source;
	Permute::readAlignment (target, in);

	std::cerr << source << std::endl;

	Permute::ScorerRef scorer = this -> beforeScorer (source, pv, pos);
	Permute::ScorerRef loss = this -> lossScorer (source, target);

	source.permute (target);
	Permute::kBestChart kbc (factory -> chart (source), controller, scorer);
	kbc.permute ();
	Permute::kbestChart::Paths paths = kbc.best (K);

	std::vector <Permute::SparseParameterVector> delta;
	for (Permute::kBestChart::Paths::const_iterator p = paths.begin (); p != paths.end (); ++ p) {
	  source.reorder (* p);
	  double l = loss -> score (target) - loss -> score (source);
	  delta.push_back (Permute::SparseParameterVector (l));
	  pos.permute (target);
	  delta.back ().build (pv, pos, 1.0);
	  pos.permute (source);
	  delta.back ().build (pv, pos, -1.0);
	}
	Permute::MIRA (delta);

	update (weightSum, pv);
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

Core::ParameterInt kBestMIRA::paramK ("k", "the size of the k-best list", 2, 2);
