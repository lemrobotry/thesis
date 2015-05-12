#include <signal.h>

#include <Core/Statistics.hh>

#include "Application.hh"
#include "BeforeScorer.hh"
#include "ChartFactory.hh"
#include "Parameter.hh"
#include "Permutation.hh"

bool interrupt_active = false;
void handle_interrupt (int sig) {
  interrupt_active = true;
}

APPLICATION

using namespace Permute;

// Trains a ParameterVector using standard perceptron updates.  Until
// convergence is achieved, for each sentence in the input, searches for the
// best permutation according to the model, then updates the parameters to
// prefer the true permutation and disprefer the model best.  If the --local
// parameter is set, starts in the neighborhood of the target permutation and
// does no search.
class Perceptron : public Permute::Application {
private:
  static Core::ParameterBool paramLocal;
  static Core::ParameterFloat paramLearningRate;
  static Core::ParameterBool paramTimer;
  Core::Timer timer;
  Core::XmlWriter out;
public:

  Perceptron () :
    Permute::Application ("perceptron"),
    out (std::cout)
  {}

  int main (const std::vector <std::string> & args) {
    signal (SIGINT, handle_interrupt);
    // Read in the parameters.
    ParameterVector pv; 
    if (! this -> parameters (pv)) {
      return EXIT_FAILURE;
    }
    if (paramLocal (config)) {
      for (ParameterVector::parameter_iterator p = pv.begin_p (); p != pv.end_p (); ++ p) {
	p -> second = rand () / double (Core::Type <int>::max) - 0.5;
      }
    }
    std::vector <double>
      weightSum (pv.size (), 0.0),
      previous (pv.size (), 0.0),
      current (pv.size (), 0.0);
    Permute::set (current, pv);

    ChartFactoryRef factory = ChartFactory::create ();

    Permutation source,
      target,
      pos (pv.getPOS ());

    ParseControllerRef controller = this -> parseController (source);

    FeatureCounter counter (pv, pos);
    
    int i = 1;
    do {
      Permute::set (previous, current);

      std::istream & input = this -> input ();

      out << Core::XmlEmpty ("iteration");
      // For each sentence in the data {
      for (int sentence = 0; sentence < paramSentences (config) && readPermutationWithAlphabet (source, input); ++ sentence, ++ i) {
	if (paramTimer (config)) {
	  timer.start ();
	}
	// Read in the source English word sequence p*, the German POS
	// sequence, and the alignment.
	readPermutation (pos, input);

	// Construct target from source and alignment.
	target = source;
	readAlignment (target, input);

	// Construct the B matrix using the POS sequence.
	ScorerRef scorer = this -> beforeScorer (source, pv, pos);

	// Find the best sequence p^ using iterative parsing.
	if (paramLocal (config)) {
	  source = target;
	}
	ChartRef chart = factory -> chart (source);

	double best_score = scorer -> score (source);
	do {
	  Chart::permute (chart, controller, scorer);
	  ConstPathRef bestPath = chart -> getBestPath ();
	  source.changed (false);
	  if (bestPath -> getScore () > best_score) {
	    best_score = bestPath -> getScore ();
	    source.reorder (bestPath);
	  }
	} while (source.changed () && ! paramLocal (config));

	// Compute the feature counts of p^ and p*.
	// Update the parameters using p* - p^.
	counter.count (target, paramLearningRate(config));
	counter.count (source, - paramLearningRate(config));

	// Update the sum of intermediate parameter vectors.
	update (weightSum, pv);

	if (paramTimer (config)) {
	  timer.stop ();
	  timer.write (out);
	  out << "\n";
	}
      }

      Permute::set (current, weightSum, 1.0 / i);

      this -> decodeDev (pv, current);

    } while (! this -> converged (previous, current) && ! interrupt_active);

    Permute::set (pv, weightSum, 1.0 / i);
    this -> outputParameters (pv);
    
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool Perceptron::paramLocal ("local", "use neighborhood of target permutation only", false);
Core::ParameterFloat Perceptron::paramLearningRate ("rate", "the learning rate for the perceptron", 1.0, 0.0);
Core::ParameterBool Perceptron::paramTimer ("timer", "output permutation times?", false);
