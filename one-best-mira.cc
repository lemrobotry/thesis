#include "Application.hh"
#include "ChartFactory.hh"

APPLICATION

// Performs MIRA to make the model prefer the target permutation over the
// model's best permutation in the target's neighborhood.  Because there is only
// a single Lagrange multiplier in this formulation, uses a closed-form update
// rather than an optimization package.
class OneBestMIRA : public Permute::Application {
private:
  static Core::ParameterBool paramRandomize;
  bool RANDOMIZE;
public:
  OneBestMIRA () :
    Permute::Application ("one-best-mira") {}

  virtual void getParameters () {
    Application::getParameters ();
    RANDOMIZE = paramRandomize (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();
    Permute::ParameterVector pv;
    if (! this -> parameters (pv)) {
      return EXIT_FAILURE;
    }
    if (RANDOMIZE) {
      // Perturb the parameters.
      std::cerr << "** Randomizing parameters **" << std::endl;
      for (Permute::ParameterVector::parameter_iterator p = pv.begin_p (); p != pv.end_p (); ++ p) {
	p -> second = rand () / double (Core::Type <int>::max) - 0.5;
      }
    }

    std::vector <double>
      weightSum (pv.size (), 0.0),
      previous (pv.size (), 0.0),
      current (pv.size (), 0.0);
    Permute::set (current, pv);

    Permute::Permutation source,
      target,
      pos (pv.getPOS ());

    Permute::ChartFactoryRef factory = Permute::ChartFactory::create ();
    Permute::ParseControllerRef controller = this -> parseController (source);

    int i = 1;
    do {
      std::cerr << "<iteration>" << std::endl;
      Permute::set (previous, current);

      std::istream & in = this -> input ();
      for (; Permute::readPermutationWithAlphabet (source, in); ++ i) {
	Permute::readPermutation (pos, in);
	target = source;
	Permute::readAlignment (target, in);

	Permute::ScorerRef scorer = this -> beforeScorer (source, pv, pos);
	Permute::ScorerRef loss = this -> lossScorer (source, target);

	source.permute (target);
	double score = scorer -> score (source);
	Permute::ChartRef chart = factory -> chart (source);
	Permute::Chart::permute (chart, controller, scorer);
	Permute::ConstPathRef bestPath = chart -> getBestPath ();
	source.changed (false);
	if (bestPath -> getScore () > score) {
	  source.reorder (bestPath);
	}

	if (source.changed ()) {
	  double l = loss -> score (target) - loss -> score (source);
	  Permute::SparseParameterVector phi (l);
	  pos.permute (target);
	  phi.build (pv, pos, 1.0);
	  pos.permute (source);
	  phi.build (pv, pos, -1.0);
	  double lambda = - phi.margin () / phi.norm2 ();
	  phi.update (lambda);
	}

	update (weightSum, pv);
      }

      Permute::set (current, weightSum, 1.0 / i);
      
    } while (! this -> converged (previous, current));

    Permute::set (pv, weightSum, 1.0 / i);
    this -> outputParameters (pv);

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool OneBestMIRA::paramRandomize ("randomize", "randomize the parameters", true);
