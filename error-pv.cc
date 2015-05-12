#include "Application.hh"
#include "ChartFactory.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Counts occurrences of features (from a new PV) that differ between the
// proposed permutation (using an existing PV) and the reference permutation
// (those that would be updated by a perceptron).
class ErrorPV : public Application {
private:
  static Core::ParameterString paramFeatureFile;
  std::string FEATURE_FILE;
public:
  ErrorPV () :
    Application ("error-pv")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    FEATURE_FILE = paramFeatureFile (config);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }

    PV features;
    if (! readFile (features, FEATURE_FILE)) {
      std::cerr << "Could not read LOP feature file: " << FEATURE_FILE << std::endl;
      return EXIT_FAILURE;
    }

    ChartFactoryRef factory = ChartFactory::create ();

    Permutation source, target, pos;

    ParseControllerRef controller = this -> parseController (source);

    std::istream & input = this -> devInput ();

    while (readPermutationWithAlphabet (source, input)) {
      readPermutationWithAlphabet (pos, input);
      target = source;
      readAlignment (target, input);

      SumBeforeCostRef bc (new SumBeforeCost (source.size ()));
      ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos);
      ChartRef chart = factory -> chart (source);

      SumBeforeCostRef fbc (new SumBeforeCost (source.size ()));
      for (Permutation::const_iterator i = source.begin (); i != -- source.end (); ++ i) {
	for (Permutation::const_iterator j = i + 1; j != source.end (); ++ j) {
	  std::vector <std::string> phi;
	  features.features (phi, source, pos, (* i), (* j));
	  for (std::vector <std::string>::const_iterator f = phi.begin (); f != phi.end (); ++ f) {
	    (* fbc) (* i, * j) += features [* f];
	  }
	}
      }

      double best_score = scorer -> score (source);
      do {
	Chart::permute (chart, controller, scorer);
	ConstPathRef bestPath = chart -> getBestPath ();
	source.changed (false);
	if (bestPath -> getScore () > best_score) {
	  best_score = bestPath -> getScore ();
	  source.reorder (bestPath);
	}
      } while (ITERATE_SEARCH && source.changed ());

      // Add the feature counts of target.
      for (Permutation::const_iterator i = target.begin (); i != -- target.end (); ++ i) {
	for (Permutation::const_iterator j = i + 1; j != target.end (); ++ j) {
	  if (* i < * j) {
	    (* fbc) (* i, * j).add (1.0);
	  }
	}
      }
      // Subtract the feature counts of source.
      for (Permutation::const_iterator i = source.begin (); i != -- source.end (); ++ i) {
	for (Permutation::const_iterator j = i + 1; j != source.end (); ++ j) {
	  if (* i < * j) {
	    (* fbc) (* i, * j).add (- 1.0);
	  }
	}
      }
    }

    this -> writePV (features);

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString ErrorPV::paramFeatureFile ("feature-file", "the LOP features to count occurrences of");
